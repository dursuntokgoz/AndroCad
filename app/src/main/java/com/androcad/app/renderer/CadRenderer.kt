// Copyright (c) 2026 dursuntokgoz. All rights reserved.
package com.androcad.app.renderer

import android.opengl.GLES30
import android.opengl.GLSurfaceView
import android.opengl.Matrix
import com.androcad.kernel.BoxMeshResult
import com.androcad.kernel.PickResult
import java.nio.ByteBuffer
import java.nio.ByteOrder
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10
import kotlin.math.*

enum class DisplayMode {
    SHADED,
    SHADED_WITH_EDGES,
    WIREFRAME
}

enum class NamedView {
    HOME,
    TOP,
    BOTTOM,
    FRONT,
    BACK,
    RIGHT,
    LEFT
}

class CadRenderer(private var meshResult: BoxMeshResult?) : GLSurfaceView.Renderer {

    // Camera parameters (spherical orbit)
    var azimuth = 35.0f      // Horizontal orbit angle
    var elevation = 25.0f    // Vertical elevation angle
    var distance = 250.0f    // Distance from target
    var targetX = 0.0f
    var targetY = 0.0f
    var targetZ = 0.0f

    var displayMode = DisplayMode.SHADED_WITH_EDGES
    var selectedFaceId = 0
    var selectedEdgeId = 0

    private var viewportWidth = 1
    private var viewportHeight = 1

    // Matrices
    private val projectionMatrix = FloatArray(16)
    private val viewMatrix = FloatArray(16)
    private val modelMatrix = FloatArray(16)
    private val mvpMatrix = FloatArray(16)
    private val cameraPos = FloatArray(3)

    // Shaders & VAOs
    private var faceProgram = 0
    private var edgeProgram = 0
    private var pickingFaceProgram = 0
    private var pickingEdgeProgram = 0

    private var faceVao = 0
    private var faceVbo = 0
    private var faceIbo = 0
    private var triangleIndexCount = 0

    private var edgeVao = 0
    private var edgeVbo = 0
    private var edgeVertexCount = 0

    // Floor Tiles & Coordinate Grid (Zemin Karoları)
    private var gridProgram = 0
    private var floorProgram = 0
    private var gridVao = 0
    private var gridVbo = 0
    private var gridVertexCount = 0
    private var floorVao = 0
    private var floorVbo = 0

    // Off-screen Picking FBO
    private var pickingFbo = 0
    private var pickingTexture = 0
    private var pickingDepthRb = 0
    private var pendingPickRequest: Pair<Float, Float>? = null
    var onPickResultListener: ((PickResult?) -> Unit)? = null

    var selectedHitPoint: FloatArray? = null
        private set
    var selectedFaceNormal: FloatArray? = null
        private set

    private var targetMarkerVao = 0
    private var targetMarkerVbo = 0
    private var markerVertexCount = 0

    @Volatile
    private var pendingMesh: BoxMeshResult? = null
    @Volatile
    private var meshNeedsUpload = false

    // 1. Shaders
    private val faceVertexShader = """
        #version 300 es
        layout (location = 0) in vec3 aPosition;
        layout (location = 1) in vec3 aNormal;
        layout (location = 2) in uint aFaceId;

        uniform mat4 uMVPMatrix;
        uniform mat4 uModelMatrix;

        out vec3 vNormal;
        out vec3 vPosition;
        flat out uint vFaceId;

        void main() {
            gl_Position = uMVPMatrix * vec4(aPosition, 1.0);
            vNormal = mat3(uModelMatrix) * aNormal;
            vPosition = vec3(uModelMatrix * vec4(aPosition, 1.0));
            vFaceId = aFaceId;
        }
    """.trimIndent()

    private val faceFragmentShader = """
        #version 300 es
        precision mediump float;

        in vec3 vNormal;
        in vec3 vPosition;
        flat in uint vFaceId;

        uniform uint uSelectedFaceId;
        uniform vec3 uCameraPos;

        out vec4 fragColor;

        void main() {
            vec3 lightDir = normalize(vec3(0.5, 0.8, 1.0));
            vec3 normal = normalize(vNormal);
            float diff = max(dot(normal, lightDir), 0.25);
            
            vec3 viewDir = normalize(uCameraPos - vPosition);
            vec3 reflectDir = reflect(-lightDir, normal);
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16.0) * 0.25;

            vec3 baseColor;
            if (uSelectedFaceId > 0u && vFaceId == uSelectedFaceId) {
                // Fusion 360 selection cyan highlight
                baseColor = vec3(0.0, 0.64, 0.92);
            } else {
                // Clean slate steel CAD body with subtle face distinction
                float tone = float(vFaceId) * 0.015;
                baseColor = vec3(0.35 + tone, 0.44 + tone, 0.56 + tone);
            }

            vec3 finalColor = baseColor * diff + vec3(spec);
            fragColor = vec4(finalColor, 1.0);
        }
    """.trimIndent()

    private val edgeVertexShader = """
        #version 300 es
        layout (location = 0) in vec3 aPosition;
        layout (location = 1) in uint aEdgeId;

        uniform mat4 uMVPMatrix;
        flat out uint vEdgeId;

        void main() {
            gl_Position = uMVPMatrix * vec4(aPosition, 1.0);
            vEdgeId = aEdgeId;
        }
    """.trimIndent()

    private val edgeFragmentShader = """
        #version 300 es
        precision mediump float;

        flat in uint vEdgeId;
        uniform uint uSelectedEdgeId;
        uniform int uIsWireframe;

        out vec4 fragColor;

        void main() {
            if (uSelectedEdgeId > 0u && vEdgeId == uSelectedEdgeId) {
                // Highlighted edge: vibrant orange
                fragColor = vec4(1.0, 0.60, 0.0, 1.0);
            } else if (uIsWireframe != 0) {
                // Wireframe mode lines: soft light gray
                fragColor = vec4(0.85, 0.88, 0.92, 1.0);
            } else {
                // Shaded mode boundary edges: crisp dark charcoal
                fragColor = vec4(0.12, 0.13, 0.15, 1.0);
            }
        }
    """.trimIndent()

    private val pickingFaceFragShader = """
        #version 300 es
        precision mediump float;
        flat in uint vFaceId;
        out vec4 fragColor;
        void main() {
            uint id = vFaceId;
            float r = float(id & 0xFFu) / 255.0;
            float g = float((id >> 8) & 0xFFu) / 255.0;
            float b = float((id >> 16) & 0xFFu) / 255.0;
            fragColor = vec4(r, g, b, 1.0);
        }
    """.trimIndent()

    private val pickingEdgeFragShader = """
        #version 300 es
        precision mediump float;
        flat in uint vEdgeId;
        out vec4 fragColor;
        void main() {
            // High bit (0x800000) identifies edge vs face
            uint id = (vEdgeId & 0x7FFFFFu) | 0x800000u;
            float r = float(id & 0xFFu) / 255.0;
            float g = float((id >> 8) & 0xFFu) / 255.0;
            float b = float((id >> 16) & 0xFFu) / 255.0;
            fragColor = vec4(r, g, b, 1.0);
        }
    """.trimIndent()

    // Floor Tiles & Coordinate Grid Shaders
    private val gridVertexShader = """
        #version 300 es
        layout (location = 0) in vec3 aPosition;
        layout (location = 1) in vec4 aColor;
        uniform mat4 uMVPMatrix;
        out vec4 vColor;
        void main() {
            gl_Position = uMVPMatrix * vec4(aPosition, 1.0);
            vColor = aColor;
        }
    """.trimIndent()

    private val gridFragmentShader = """
        #version 300 es
        precision mediump float;
        in vec4 vColor;
        out vec4 fragColor;
        void main() {
            fragColor = vColor;
        }
    """.trimIndent()

    private val floorVertexShader = """
        #version 300 es
        layout (location = 0) in vec3 aPosition;
        uniform mat4 uMVPMatrix;
        out vec2 vWorldPos;
        void main() {
            gl_Position = uMVPMatrix * vec4(aPosition, 1.0);
            vWorldPos = aPosition.xz;
        }
    """.trimIndent()

    private val floorFragmentShader = """
        #version 300 es
        precision mediump float;
        in vec2 vWorldPos;
        out vec4 fragColor;
        void main() {
            // 25mm checkered ground floor tiles (zemin karoları)
            vec2 tileCoord = floor(vWorldPos / 25.0);
            float pattern = mod(tileCoord.x + tileCoord.y, 2.0);
            vec3 colorA = vec3(0.20, 0.22, 0.25);
            vec3 colorB = vec3(0.14, 0.16, 0.18);
            vec3 baseTile = mix(colorA, colorB, pattern);
            float dist = length(vWorldPos);
            float alpha = clamp(1.0 - (dist / 280.0), 0.0, 0.85);
            fragColor = vec4(baseTile, alpha);
        }
    """.trimIndent()

    override fun onSurfaceCreated(gl: GL10?, config: EGLConfig?) {
        GLES30.glClearColor(0.16f, 0.18f, 0.20f, 1.0f) // Fusion 360-style viewport dark gray
        GLES30.glEnable(GLES30.GL_DEPTH_TEST)

        faceProgram = createProgram(faceVertexShader, faceFragmentShader)
        edgeProgram = createProgram(edgeVertexShader, edgeFragmentShader)
        pickingFaceProgram = createProgram(faceVertexShader, pickingFaceFragShader)
        pickingEdgeProgram = createProgram(edgeVertexShader, pickingEdgeFragShader)
        gridProgram = createProgram(gridVertexShader, gridFragmentShader)
        floorProgram = createProgram(floorVertexShader, floorFragmentShader)

        setupGridBuffers()
        setupMeshBuffers()
        zoomToFit()
    }

    private fun setupGridBuffers() {
        val extent = 250.0f
        val step = 10.0f
        val lines = mutableListOf<Float>()

        fun addV(x: Float, y: Float, z: Float, r: Float, g: Float, b: Float, a: Float) {
            lines.add(x); lines.add(y); lines.add(z)
            lines.add(r); lines.add(g); lines.add(b); lines.add(a)
        }

        var x = -extent
        while (x <= extent + 0.1f) {
            val isZAxis = abs(x) < 0.1f
            val isMajor = abs(x % 50.0f) < 0.1f
            val (r, g, b, a) = when {
                isZAxis -> floatArrayOf(0.25f, 0.55f, 0.95f, 0.95f) // Blue Z-axis
                isMajor -> floatArrayOf(0.40f, 0.44f, 0.50f, 0.70f)
                else -> floatArrayOf(0.26f, 0.29f, 0.34f, 0.40f)
            }
            addV(x, 0.001f, -extent, r, g, b, a)
            addV(x, 0.001f, extent, r, g, b, a)
            x += step
        }

        var z = -extent
        while (z <= extent + 0.1f) {
            val isXAxis = abs(z) < 0.1f
            val isMajor = abs(z % 50.0f) < 0.1f
            val (r, g, b, a) = when {
                isXAxis -> floatArrayOf(0.88f, 0.22f, 0.22f, 0.95f) // Red X-axis
                isMajor -> floatArrayOf(0.40f, 0.44f, 0.50f, 0.70f)
                else -> floatArrayOf(0.26f, 0.29f, 0.34f, 0.40f)
            }
            addV(-extent, 0.001f, z, r, g, b, a)
            addV(extent, 0.001f, z, r, g, b, a)
            z += step
        }

        gridVertexCount = lines.size / 7
        val gridBuffer = ByteBuffer.allocateDirect(lines.size * 4)
            .order(ByteOrder.nativeOrder())
            .asFloatBuffer()
        for (f in lines) gridBuffer.put(f)
        gridBuffer.position(0)

        val vaos = IntArray(1)
        val vbos = IntArray(1)
        GLES30.glGenVertexArrays(1, vaos, 0)
        GLES30.glGenBuffers(1, vbos, 0)
        gridVao = vaos[0]
        gridVbo = vbos[0]

        GLES30.glBindVertexArray(gridVao)
        GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, gridVbo)
        GLES30.glBufferData(GLES30.GL_ARRAY_BUFFER, lines.size * 4, gridBuffer, GLES30.GL_STATIC_DRAW)
        GLES30.glEnableVertexAttribArray(0)
        GLES30.glVertexAttribPointer(0, 3, GLES30.GL_FLOAT, false, 7 * 4, 0)
        GLES30.glEnableVertexAttribArray(1)
        GLES30.glVertexAttribPointer(1, 4, GLES30.GL_FLOAT, false, 7 * 4, 3 * 4)
        GLES30.glBindVertexArray(0)

        // Floor Quad (6 vertices covering [-extent, extent] in X and Z)
        val floorVertices = floatArrayOf(
            -extent, 0.0f, -extent,
             extent, 0.0f, -extent,
             extent, 0.0f,  extent,

            -extent, 0.0f, -extent,
             extent, 0.0f,  extent,
            -extent, 0.0f,  extent
        )
        val floorBuffer = ByteBuffer.allocateDirect(floorVertices.size * 4)
            .order(ByteOrder.nativeOrder())
            .asFloatBuffer()
        floorBuffer.put(floorVertices)
        floorBuffer.position(0)

        val fVaos = IntArray(1)
        val fVbos = IntArray(1)
        GLES30.glGenVertexArrays(1, fVaos, 0)
        GLES30.glGenBuffers(1, fVbos, 0)
        floorVao = fVaos[0]
        floorVbo = fVbos[0]

        GLES30.glBindVertexArray(floorVao)
        GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, floorVbo)
        GLES30.glBufferData(GLES30.GL_ARRAY_BUFFER, floorVertices.size * 4, floorBuffer, GLES30.GL_STATIC_DRAW)
        GLES30.glEnableVertexAttribArray(0)
        GLES30.glVertexAttribPointer(0, 3, GLES30.GL_FLOAT, false, 3 * 4, 0)
        GLES30.glBindVertexArray(0)
    }

    fun updateMesh(newMesh: BoxMeshResult?) {
        meshResult = newMesh
        pendingMesh = newMesh
        meshNeedsUpload = true
    }

    private fun setupMeshBuffers() {
        if (faceVbo != 0) {
            GLES30.glDeleteBuffers(2, intArrayOf(faceVbo, faceIbo), 0)
            faceVbo = 0
            faceIbo = 0
        }
        if (faceVao != 0) {
            GLES30.glDeleteVertexArrays(1, intArrayOf(faceVao), 0)
            faceVao = 0
        }
        if (edgeVbo != 0) {
            GLES30.glDeleteBuffers(1, intArrayOf(edgeVbo), 0)
            edgeVbo = 0
        }
        if (edgeVao != 0) {
            GLES30.glDeleteVertexArrays(1, intArrayOf(edgeVao), 0)
            edgeVao = 0
        }

        val currentMesh = meshResult
        if (currentMesh == null || !currentMesh.success) {
            triangleIndexCount = 0
            edgeVertexCount = 0
            selectedFaceId = 0
            selectedEdgeId = 0
            selectedHitPoint = null
            selectedFaceNormal = null
            return
        }

        // 1. Setup Face Buffers
        val vBuf = currentMesh.vertexBuffer
        val iBuf = currentMesh.indexBuffer
        if (vBuf != null && iBuf != null) {
            triangleIndexCount = currentMesh.triangleCount * 3

            val vaos = IntArray(1)
            GLES30.glGenVertexArrays(1, vaos, 0)
            faceVao = vaos[0]
            GLES30.glBindVertexArray(faceVao)

            val buffers = IntArray(2)
            GLES30.glGenBuffers(2, buffers, 0)
            faceVbo = buffers[0]
            faceIbo = buffers[1]

            val vNative = vBuf.order(ByteOrder.nativeOrder())
            vNative.position(0)
            GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, faceVbo)
            GLES30.glBufferData(GLES30.GL_ARRAY_BUFFER, vNative.capacity(), vNative, GLES30.GL_STATIC_DRAW)

            val iNative = iBuf.order(ByteOrder.nativeOrder())
            iNative.position(0)
            GLES30.glBindBuffer(GLES30.GL_ELEMENT_ARRAY_BUFFER, faceIbo)
            GLES30.glBufferData(GLES30.GL_ELEMENT_ARRAY_BUFFER, iNative.capacity(), iNative, GLES30.GL_STATIC_DRAW)

            val stride = (3 * 4) + (3 * 4) + 4 // pos (12) + normal (12) + faceId (4) = 28 bytes
            GLES30.glEnableVertexAttribArray(0)
            GLES30.glVertexAttribPointer(0, 3, GLES30.GL_FLOAT, false, stride, 0)

            GLES30.glEnableVertexAttribArray(1)
            GLES30.glVertexAttribPointer(1, 3, GLES30.GL_FLOAT, false, stride, 12)

            GLES30.glEnableVertexAttribArray(2)
            GLES30.glVertexAttribIPointer(2, 1, GLES30.GL_UNSIGNED_INT, stride, 24)

            GLES30.glBindVertexArray(0)
        }

        // 2. Setup Edge Buffers (Milestone 2)
        val edgeBuf = currentMesh.edgeVertexBuffer
        if (edgeBuf != null) {
            val edgeStride = (3 * 4) + 4 // pos (12) + edgeId (4) = 16 bytes
            edgeVertexCount = edgeBuf.capacity() / edgeStride

            val vaos = IntArray(1)
            GLES30.glGenVertexArrays(1, vaos, 0)
            edgeVao = vaos[0]
            GLES30.glBindVertexArray(edgeVao)

            val buffers = IntArray(1)
            GLES30.glGenBuffers(1, buffers, 0)
            edgeVbo = buffers[0]

            val eNative = edgeBuf.order(ByteOrder.nativeOrder())
            eNative.position(0)
            GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, edgeVbo)
            GLES30.glBufferData(GLES30.GL_ARRAY_BUFFER, eNative.capacity(), eNative, GLES30.GL_STATIC_DRAW)

            GLES30.glEnableVertexAttribArray(0)
            GLES30.glVertexAttribPointer(0, 3, GLES30.GL_FLOAT, false, edgeStride, 0)

            GLES30.glEnableVertexAttribArray(1)
            GLES30.glVertexAttribIPointer(1, 1, GLES30.GL_UNSIGNED_INT, edgeStride, 12)

            GLES30.glBindVertexArray(0)
        }
    }

    override fun onSurfaceChanged(gl: GL10?, width: Int, height: Int) {
        viewportWidth = width
        viewportHeight = height
        GLES30.glViewport(0, 0, width, height)

        val ratio = width.toFloat() / height.toFloat()
        Matrix.perspectiveM(projectionMatrix, 0, 45f, ratio, 1f, 5000f)

        setupPickingFbo(width, height)
    }

    private fun setupPickingFbo(w: Int, h: Int) {
        if (w <= 0 || h <= 0) return

        if (pickingFbo != 0) {
            GLES30.glDeleteFramebuffers(1, intArrayOf(pickingFbo), 0)
            GLES30.glDeleteTextures(1, intArrayOf(pickingTexture), 0)
            GLES30.glDeleteRenderbuffers(1, intArrayOf(pickingDepthRb), 0)
        }

        val fbos = IntArray(1)
        GLES30.glGenFramebuffers(1, fbos, 0)
        pickingFbo = fbos[0]
        GLES30.glBindFramebuffer(GLES30.GL_FRAMEBUFFER, pickingFbo)

        val texs = IntArray(1)
        GLES30.glGenTextures(1, texs, 0)
        pickingTexture = texs[0]
        GLES30.glBindTexture(GLES30.GL_TEXTURE_2D, pickingTexture)
        GLES30.glTexImage2D(GLES30.GL_TEXTURE_2D, 0, GLES30.GL_RGBA8, w, h, 0, GLES30.GL_RGBA, GLES30.GL_UNSIGNED_BYTE, null)
        GLES30.glTexParameteri(GLES30.GL_TEXTURE_2D, GLES30.GL_TEXTURE_MIN_FILTER, GLES30.GL_NEAREST)
        GLES30.glTexParameteri(GLES30.GL_TEXTURE_2D, GLES30.GL_TEXTURE_MAG_FILTER, GLES30.GL_NEAREST)
        GLES30.glFramebufferTexture2D(GLES30.GL_FRAMEBUFFER, GLES30.GL_COLOR_ATTACHMENT0, GLES30.GL_TEXTURE_2D, pickingTexture, 0)

        val rbs = IntArray(1)
        GLES30.glGenRenderbuffers(1, rbs, 0)
        pickingDepthRb = rbs[0]
        GLES30.glBindRenderbuffer(GLES30.GL_RENDERBUFFER, pickingDepthRb)
        GLES30.glRenderbufferStorage(GLES30.GL_RENDERBUFFER, GLES30.GL_DEPTH_COMPONENT24, w, h)
        GLES30.glFramebufferRenderbuffer(GLES30.GL_FRAMEBUFFER, GLES30.GL_DEPTH_ATTACHMENT, GLES30.GL_RENDERBUFFER, pickingDepthRb)

        GLES30.glBindFramebuffer(GLES30.GL_FRAMEBUFFER, 0)
    }

    override fun onDrawFrame(gl: GL10?) {
        if (meshNeedsUpload) {
            meshNeedsUpload = false
            meshResult = pendingMesh
            setupMeshBuffers()
            zoomToFit()
        }
        updateMatrices()

        // 1. Check if a pick was requested
        pendingPickRequest?.let { (x, y) ->
            executePickingPass(x, y)
            pendingPickRequest = null
        }

        // 2. Normal Viewport Render
        GLES30.glBindFramebuffer(GLES30.GL_FRAMEBUFFER, 0)
        GLES30.glViewport(0, 0, viewportWidth, viewportHeight)
        GLES30.glClear(GLES30.GL_COLOR_BUFFER_BIT or GLES30.GL_DEPTH_BUFFER_BIT)

        // Draw CAD Floor Tiles & Coordinate Grid (Zemin Karoları ve Izgara)
        if (floorVao != 0 || gridVao != 0) {
            val gridMVP = FloatArray(16)
            val gridModel = FloatArray(16)
            Matrix.setIdentityM(gridModel, 0)
            val groundY = meshResult?.let {
                val bbox = it.boundingBox
                - (bbox.maxY - bbox.minY) * 0.5f
            } ?: 0.0f
            Matrix.translateM(gridModel, 0, 0.0f, groundY, 0.0f)
            Matrix.multiplyMM(gridMVP, 0, viewMatrix, 0, gridModel, 0)
            Matrix.multiplyMM(gridMVP, 0, projectionMatrix, 0, gridMVP, 0)

            GLES30.glEnable(GLES30.GL_BLEND)
            GLES30.glBlendFunc(GLES30.GL_SRC_ALPHA, GLES30.GL_ONE_MINUS_SRC_ALPHA)

            // 1. Checkered Ground Floor Tiles
            if (floorVao != 0) {
                GLES30.glUseProgram(floorProgram)
                GLES30.glUniformMatrix4fv(GLES30.glGetUniformLocation(floorProgram, "uMVPMatrix"), 1, false, gridMVP, 0)
                GLES30.glBindVertexArray(floorVao)
                GLES30.glDrawArrays(GLES30.GL_TRIANGLES, 0, 6)
                GLES30.glBindVertexArray(0)
            }

            // 2. Coordinate Grid Lines & Red X / Blue Z Axes
            if (gridVao != 0 && gridVertexCount > 0) {
                GLES30.glUseProgram(gridProgram)
                GLES30.glUniformMatrix4fv(GLES30.glGetUniformLocation(gridProgram, "uMVPMatrix"), 1, false, gridMVP, 0)
                GLES30.glLineWidth(1.8f)
                GLES30.glBindVertexArray(gridVao)
                GLES30.glDrawArrays(GLES30.GL_LINES, 0, gridVertexCount)
                GLES30.glBindVertexArray(0)
            }

            GLES30.glDisable(GLES30.GL_BLEND)
        }

        // Draw solid faces if Shaded or Shaded+Edges
        if (displayMode != DisplayMode.WIREFRAME && faceVao != 0 && triangleIndexCount > 0) {
            if (displayMode == DisplayMode.SHADED_WITH_EDGES) {
                // Polygon offset pushes faces slightly back to eliminate Z-fighting with boundary edges
                GLES30.glEnable(GLES30.GL_POLYGON_OFFSET_FILL)
                GLES30.glPolygonOffset(1.0f, 1.0f)
            }

            GLES30.glUseProgram(faceProgram)
            GLES30.glUniformMatrix4fv(GLES30.glGetUniformLocation(faceProgram, "uMVPMatrix"), 1, false, mvpMatrix, 0)
            GLES30.glUniformMatrix4fv(GLES30.glGetUniformLocation(faceProgram, "uModelMatrix"), 1, false, modelMatrix, 0)
            GLES30.glUniform3fv(GLES30.glGetUniformLocation(faceProgram, "uCameraPos"), 1, cameraPos, 0)
            GLES30.glUniform1ui(GLES30.glGetUniformLocation(faceProgram, "uSelectedFaceId"), selectedFaceId)

            GLES30.glBindVertexArray(faceVao)
            GLES30.glDrawElements(GLES30.GL_TRIANGLES, triangleIndexCount, GLES30.GL_UNSIGNED_INT, 0)
            GLES30.glBindVertexArray(0)

            if (displayMode == DisplayMode.SHADED_WITH_EDGES) {
                GLES30.glDisable(GLES30.GL_POLYGON_OFFSET_FILL)
            }
        }

        // Draw boundary/feature edges if Shaded+Edges or Wireframe
        if (displayMode != DisplayMode.SHADED && edgeVao != 0 && edgeVertexCount > 0) {
            GLES30.glUseProgram(edgeProgram)
            GLES30.glUniformMatrix4fv(GLES30.glGetUniformLocation(edgeProgram, "uMVPMatrix"), 1, false, mvpMatrix, 0)
            GLES30.glUniform1ui(GLES30.glGetUniformLocation(edgeProgram, "uSelectedEdgeId"), selectedEdgeId)
            GLES30.glUniform1i(GLES30.glGetUniformLocation(edgeProgram, "uIsWireframe"), if (displayMode == DisplayMode.WIREFRAME) 1 else 0)

            GLES30.glLineWidth(2.5f)
            GLES30.glBindVertexArray(edgeVao)
            GLES30.glDrawArrays(GLES30.GL_LINES, 0, edgeVertexCount)
            GLES30.glBindVertexArray(0)
        }

        // Draw 3D Target Marker at tapped coordinates if an entity is selected
        if (targetMarkerVao != 0 && markerVertexCount > 0 && selectedHitPoint != null) {
            GLES30.glUseProgram(gridProgram)
            GLES30.glUniformMatrix4fv(GLES30.glGetUniformLocation(gridProgram, "uMVPMatrix"), 1, false, mvpMatrix, 0)
            GLES30.glLineWidth(3.5f)
            GLES30.glBindVertexArray(targetMarkerVao)
            GLES30.glDrawArrays(GLES30.GL_LINES, 0, markerVertexCount)
            GLES30.glBindVertexArray(0)
        }
    }

    fun screenToRay(touchX: Float, touchY: Float): Pair<FloatArray, FloatArray>? {
        if (viewportWidth <= 0 || viewportHeight <= 0) return null
        val invMVP = FloatArray(16)
        if (!Matrix.invertM(invMVP, 0, mvpMatrix, 0)) return null

        val ndcX = (touchX / viewportWidth.toFloat()) * 2f - 1f
        val ndcY = 1f - (touchY / viewportHeight.toFloat()) * 2f

        val nearPointHomo = FloatArray(4)
        val farPointHomo = FloatArray(4)
        val nearInput = floatArrayOf(ndcX, ndcY, -1f, 1f)
        val farInput = floatArrayOf(ndcX, ndcY, 1f, 1f)

        Matrix.multiplyMV(nearPointHomo, 0, invMVP, 0, nearInput, 0)
        Matrix.multiplyMV(farPointHomo, 0, invMVP, 0, farInput, 0)

        if (nearPointHomo[3] == 0f || farPointHomo[3] == 0f) return null

        val near = FloatArray(3) { nearPointHomo[it] / nearPointHomo[3] }
        val far = FloatArray(3) { farPointHomo[it] / farPointHomo[3] }

        val dir = FloatArray(3) { far[it] - near[it] }
        val len = sqrt((dir[0] * dir[0] + dir[1] * dir[1] + dir[2] * dir[2]).toDouble()).toFloat()
        if (len < 1e-6f) return null
        dir[0] /= len
        dir[1] /= len
        dir[2] /= len

        return Pair(near, dir)
    }

    private fun updateTargetMarker() {
        val pt = selectedHitPoint ?: return
        val normal = selectedFaceNormal ?: floatArrayOf(0f, 0f, 1f)

        val nx = normal[0]
        val ny = normal[1]
        val nz = normal[2]

        val ux: Float
        val uy: Float
        val uz: Float
        if (abs(nz) < 0.9f) {
            val lx = -ny
            val ly = nx
            val len = sqrt((lx * lx + ly * ly).toDouble()).toFloat().coerceAtLeast(1e-4f)
            ux = lx / len
            uy = ly / len
            uz = 0f
        } else {
            val ly = -nz
            val lz = ny
            val len = sqrt((ly * ly + lz * lz).toDouble()).toFloat().coerceAtLeast(1e-4f)
            ux = 0f
            uy = ly / len
            uz = lz / len
        }

        val vx = ny * uz - nz * uy
        val vy = nz * ux - nx * uz
        val vz = nx * uy - ny * ux

        val radius = 4.0f
        val segments = 16
        val lines = ArrayList<Float>()

        val colR = 0.0f
        val colG = 0.9f
        val colB = 1.0f
        val colA = 1.0f

        for (i in 0 until segments) {
            val angle1 = (2.0 * Math.PI * i / segments).toFloat()
            val angle2 = (2.0 * Math.PI * (i + 1) / segments).toFloat()

            val p1x = pt[0] + radius * (cos(angle1) * ux + sin(angle1) * vx) + nx * 0.15f
            val p1y = pt[1] + radius * (cos(angle1) * uy + sin(angle1) * vy) + ny * 0.15f
            val p1z = pt[2] + radius * (cos(angle1) * uz + sin(angle1) * vz) + nz * 0.15f

            val p2x = pt[0] + radius * (cos(angle2) * ux + sin(angle2) * vx) + nx * 0.15f
            val p2y = pt[1] + radius * (cos(angle2) * uy + sin(angle2) * vy) + ny * 0.15f
            val p2z = pt[2] + radius * (cos(angle2) * uz + sin(angle2) * vz) + nz * 0.15f

            lines.add(p1x); lines.add(p1y); lines.add(p1z)
            lines.add(colR); lines.add(colG); lines.add(colB); lines.add(colA)
            lines.add(p2x); lines.add(p2y); lines.add(p2z)
            lines.add(colR); lines.add(colG); lines.add(colB); lines.add(colA)
        }

        val c1x = pt[0] - radius * 1.3f * ux + nx * 0.15f
        val c1y = pt[1] - radius * 1.3f * uy + ny * 0.15f
        val c1z = pt[2] - radius * 1.3f * uz + nz * 0.15f
        val c2x = pt[0] + radius * 1.3f * ux + nx * 0.15f
        val c2y = pt[1] + radius * 1.3f * uy + ny * 0.15f
        val c2z = pt[2] + radius * 1.3f * uz + nz * 0.15f
        lines.add(c1x); lines.add(c1y); lines.add(c1z); lines.add(colR); lines.add(colG); lines.add(colB); lines.add(colA)
        lines.add(c2x); lines.add(c2y); lines.add(c2z); lines.add(colR); lines.add(colG); lines.add(colB); lines.add(colA)

        val d1x = pt[0] - radius * 1.3f * vx + nx * 0.15f
        val d1y = pt[1] - radius * 1.3f * vy + ny * 0.15f
        val d1z = pt[2] - radius * 1.3f * vz + nz * 0.15f
        val d2x = pt[0] + radius * 1.3f * vx + nx * 0.15f
        val d2y = pt[1] + radius * 1.3f * vy + ny * 0.15f
        val d2z = pt[2] + radius * 1.3f * vz + nz * 0.15f
        lines.add(d1x); lines.add(d1y); lines.add(d1z); lines.add(colR); lines.add(colG); lines.add(colB); lines.add(colA)
        lines.add(d2x); lines.add(d2y); lines.add(d2z); lines.add(colR); lines.add(colG); lines.add(colB); lines.add(colA)

        markerVertexCount = lines.size / 7
        val markerBuf = ByteBuffer.allocateDirect(lines.size * 4)
            .order(ByteOrder.nativeOrder())
            .asFloatBuffer()
        for (f in lines) markerBuf.put(f)
        markerBuf.position(0)

        if (targetMarkerVao == 0) {
            val vaos = IntArray(1)
            val vbos = IntArray(1)
            GLES30.glGenVertexArrays(1, vaos, 0)
            GLES30.glGenBuffers(1, vbos, 0)
            targetMarkerVao = vaos[0]
            targetMarkerVbo = vbos[0]
        }

        GLES30.glBindVertexArray(targetMarkerVao)
        GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, targetMarkerVbo)
        GLES30.glBufferData(GLES30.GL_ARRAY_BUFFER, lines.size * 4, markerBuf, GLES30.GL_DYNAMIC_DRAW)
        GLES30.glEnableVertexAttribArray(0)
        GLES30.glVertexAttribPointer(0, 3, GLES30.GL_FLOAT, false, 7 * 4, 0)
        GLES30.glEnableVertexAttribArray(1)
        GLES30.glVertexAttribPointer(1, 4, GLES30.GL_FLOAT, false, 7 * 4, 3 * 4)
        GLES30.glBindVertexArray(0)
    }

    private fun executePickingPass(touchX: Float, touchY: Float) {
        if (pickingFbo == 0) return

        val curMesh = meshResult
        if (curMesh == null || !curMesh.success) {
            selectedFaceId = 0
            selectedEdgeId = 0
            selectedHitPoint = null
            selectedFaceNormal = null
            onPickResultListener?.invoke(null)
            return
        }

        GLES30.glBindFramebuffer(GLES30.GL_FRAMEBUFFER, pickingFbo)
        GLES30.glViewport(0, 0, viewportWidth, viewportHeight)
        GLES30.glClearColor(0f, 0f, 0f, 0f)
        GLES30.glClear(GLES30.GL_COLOR_BUFFER_BIT or GLES30.GL_DEPTH_BUFFER_BIT)

        // Draw faces with face picking shader
        if (faceVao != 0 && triangleIndexCount > 0) {
            GLES30.glEnable(GLES30.GL_POLYGON_OFFSET_FILL)
            GLES30.glPolygonOffset(1.0f, 1.0f)
            GLES30.glUseProgram(pickingFaceProgram)
            GLES30.glUniformMatrix4fv(GLES30.glGetUniformLocation(pickingFaceProgram, "uMVPMatrix"), 1, false, mvpMatrix, 0)
            GLES30.glBindVertexArray(faceVao)
            GLES30.glDrawElements(GLES30.GL_TRIANGLES, triangleIndexCount, GLES30.GL_UNSIGNED_INT, 0)
            GLES30.glBindVertexArray(0)
            GLES30.glDisable(GLES30.GL_POLYGON_OFFSET_FILL)
        }

        // Draw edges with edge picking shader (thick lines for touch friendliness)
        if (edgeVao != 0 && edgeVertexCount > 0) {
            GLES30.glUseProgram(pickingEdgeProgram)
            GLES30.glUniformMatrix4fv(GLES30.glGetUniformLocation(pickingEdgeProgram, "uMVPMatrix"), 1, false, mvpMatrix, 0)
            GLES30.glLineWidth(8.0f) // Generous touch target for edges
            GLES30.glBindVertexArray(edgeVao)
            GLES30.glDrawArrays(GLES30.GL_LINES, 0, edgeVertexCount)
            GLES30.glBindVertexArray(0)
        }

        // Read 1 pixel at touch location (OpenGL coordinates have Y flipped)
        val glY = (viewportHeight - touchY.toInt()).coerceIn(0, viewportHeight - 1)
        val glX = touchX.toInt().coerceIn(0, viewportWidth - 1)

        val pixel = ByteBuffer.allocateDirect(4).order(ByteOrder.nativeOrder())
        GLES30.glReadPixels(glX, glY, 1, 1, GLES30.GL_RGBA, GLES30.GL_UNSIGNED_BYTE, pixel)

        val r = pixel.get(0).toInt() and 0xFF
        val g = pixel.get(1).toInt() and 0xFF
        val b = pixel.get(2).toInt() and 0xFF

        val rawId = r or (g shl 8) or (b shl 16)
        val gpuHit = (rawId != 0)
        val gpuIsEdge = (rawId and 0x800000) != 0
        val gpuEntityId = rawId and 0x7FFFFF

        val ray = screenToRay(touchX, touchY)
        val rayPick = if (ray != null) {
            curMesh.pickByRay(ray.first[0], ray.first[1], ray.first[2], ray.second[0], ray.second[1], ray.second[2])
        } else null

        val result: PickResult? = when {
            rayPick != null && rayPick.hit -> {
                val finalEntityId = if (gpuHit && !gpuIsEdge) gpuEntityId else rayPick.entityId
                val finalIsEdge = if (gpuHit) gpuIsEdge else rayPick.isEdge
                selectedHitPoint = floatArrayOf(rayPick.hitX, rayPick.hitY, rayPick.hitZ)
                selectedFaceNormal = floatArrayOf(rayPick.normalX, rayPick.normalY, rayPick.normalZ)
                PickResult(
                    hit = true,
                    entityId = finalEntityId,
                    isEdge = finalIsEdge,
                    distance = rayPick.distance,
                    hitX = rayPick.hitX,
                    hitY = rayPick.hitY,
                    hitZ = rayPick.hitZ,
                    normalX = rayPick.normalX,
                    normalY = rayPick.normalY,
                    normalZ = rayPick.normalZ
                )
            }
            gpuHit -> {
                var hx = 0f
                var hy = 0f
                var hz = 0f
                var nx = 0f
                var ny = 0f
                var nz = 1f
                if (!gpuIsEdge) {
                    val centerNorm = curMesh.getFaceCenterAndNormal(gpuEntityId)
                    if (centerNorm != null) {
                        hx = centerNorm.first.first
                        hy = centerNorm.first.second
                        hz = centerNorm.first.third
                        nx = centerNorm.second.first
                        ny = centerNorm.second.second
                        nz = centerNorm.second.third
                    }
                }
                selectedHitPoint = floatArrayOf(hx, hy, hz)
                selectedFaceNormal = floatArrayOf(nx, ny, nz)
                PickResult(
                    hit = true,
                    entityId = gpuEntityId,
                    isEdge = gpuIsEdge,
                    distance = 0f,
                    hitX = hx, hitY = hy, hitZ = hz,
                    normalX = nx, normalY = ny, normalZ = nz
                )
            }
            else -> {
                selectedHitPoint = null
                selectedFaceNormal = null
                null
            }
        }

        if (result != null) {
            if (result.isEdge) {
                selectedEdgeId = result.entityId
                selectedFaceId = 0
            } else {
                selectedFaceId = result.entityId
                selectedEdgeId = 0
            }
            updateTargetMarker()
        } else {
            selectedFaceId = 0
            selectedEdgeId = 0
            selectedHitPoint = null
            selectedFaceNormal = null
        }

        onPickResultListener?.invoke(result)
        GLES30.glBindFramebuffer(GLES30.GL_FRAMEBUFFER, 0)
    }

    private fun updateMatrices() {
        val radElev = Math.toRadians(elevation.toDouble()).toFloat()
        val radAzim = Math.toRadians(azimuth.toDouble()).toFloat()

        val cosElev = cos(radElev)
        val sinElev = sin(radElev)
        val cosAzim = cos(radAzim)
        val sinAzim = sin(radAzim)

        // Spherical camera position relative to target
        cameraPos[0] = targetX + distance * cosElev * sinAzim
        cameraPos[1] = targetY + distance * sinElev
        cameraPos[2] = targetZ + distance * cosElev * cosAzim

        Matrix.setLookAtM(
            viewMatrix, 0,
            cameraPos[0], cameraPos[1], cameraPos[2],
            targetX, targetY, targetZ,
            0f, 1f, 0f
        )

        Matrix.setIdentityM(modelMatrix, 0)

        // Center box around origin
        meshResult?.let {
            val bbox = it.boundingBox
            Matrix.translateM(modelMatrix, 0, -bbox.centerX, -bbox.centerY, -bbox.centerZ)
        }

        Matrix.multiplyMM(mvpMatrix, 0, viewMatrix, 0, modelMatrix, 0)
        Matrix.multiplyMM(mvpMatrix, 0, projectionMatrix, 0, mvpMatrix, 0)
    }

    fun orbit(deltaX: Float, deltaY: Float) {
        azimuth = (azimuth + deltaX * 0.4f) % 360f
        elevation = (elevation + deltaY * 0.4f).coerceIn(-89.5f, 89.5f)
    }

    fun pan(deltaX: Float, deltaY: Float) {
        val factor = (distance / 800f).coerceAtLeast(0.05f)
        val radAzim = Math.toRadians(azimuth.toDouble()).toFloat()
        val rightX = cos(radAzim)
        val rightZ = -sin(radAzim)

        targetX -= rightX * deltaX * factor
        targetZ -= rightZ * deltaX * factor
        targetY += deltaY * factor
    }

    fun zoom(scaleFactor: Float) {
        distance = (distance / scaleFactor).coerceIn(10.0f, 3000.0f)
    }

    fun zoomToFit() {
        val currentMesh = meshResult
        if (currentMesh != null && currentMesh.success) {
            val bbox = currentMesh.boundingBox
            targetX = 0f
            targetY = 0f
            targetZ = 0f
            distance = (bbox.diagonal * 1.6f).coerceAtLeast(50f)
            azimuth = 35.0f
            elevation = 25.0f
        }
    }

    fun setNamedView(view: NamedView) {
        when (view) {
            NamedView.HOME -> { azimuth = 35f; elevation = 25f }
            NamedView.TOP -> { azimuth = 0f; elevation = 89.5f }
            NamedView.BOTTOM -> { azimuth = 0f; elevation = -89.5f }
            NamedView.FRONT -> { azimuth = 0f; elevation = 0f }
            NamedView.BACK -> { azimuth = 180f; elevation = 0f }
            NamedView.RIGHT -> { azimuth = 90f; elevation = 0f }
            NamedView.LEFT -> { azimuth = -90f; elevation = 0f }
        }
    }

    fun requestPick(x: Float, y: Float) {
        pendingPickRequest = Pair(x, y)
    }

    private fun loadShader(type: Int, shaderCode: String): Int {
        val shader = GLES30.glCreateShader(type)
        GLES30.glShaderSource(shader, shaderCode)
        GLES30.glCompileShader(shader)
        return shader
    }

    private fun createProgram(vCode: String, fCode: String): Int {
        val vs = loadShader(GLES30.GL_VERTEX_SHADER, vCode)
        val fs = loadShader(GLES30.GL_FRAGMENT_SHADER, fCode)
        val prog = GLES30.glCreateProgram()
        GLES30.glAttachShader(prog, vs)
        GLES30.glAttachShader(prog, fs)
        GLES30.glLinkProgram(prog)
        return prog
    }
}