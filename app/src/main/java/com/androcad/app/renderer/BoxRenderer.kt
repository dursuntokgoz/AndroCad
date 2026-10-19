// Copyright (c) 2026 dursuntokgoz. All rights reserved.
package com.androcad.app.renderer

import android.opengl.GLES30
import android.opengl.GLSurfaceView
import android.opengl.Matrix
import com.androcad.kernel.BoxMeshResult
import java.nio.ByteOrder
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

class BoxRenderer(private val meshResult: BoxMeshResult?) : GLSurfaceView.Renderer {

    private val vPMatrix = FloatArray(16)
    private val projectionMatrix = FloatArray(16)
    private val viewMatrix = FloatArray(16)
    private val rotationMatrix = FloatArray(16)
    private val modelMatrix = FloatArray(16)

    var angleX = 25f
    var angleY = -35f
    var zoom = -250f

    private var program = 0
    private var vao = 0
    private var vbo = 0
    private var ibo = 0
    private var indexCount = 0

    private val vertexShaderCode = """
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

    private val fragmentShaderCode = """
        #version 300 es
        precision mediump float;

        in vec3 vNormal;
        in vec3 vPosition;
        flat in uint vFaceId;

        out vec4 fragColor;

        void main() {
            vec3 lightDir = normalize(vec3(0.5, 0.8, 1.0));
            vec3 normal = normalize(vNormal);
            float diff = max(dot(normal, lightDir), 0.25);
            
            // Soft steel-blue CAD material with face ID tone variation
            float tone = float(vFaceId) * 0.05;
            vec3 baseColor = vec3(0.25 + tone, 0.45 + tone, 0.70 + tone);
            vec3 color = baseColor * diff;
            
            fragColor = vec4(color, 1.0);
        }
    """.trimIndent()

    override fun onSurfaceCreated(gl: GL10?, config: EGLConfig?) {
        GLES30.glClearColor(0.18f, 0.20f, 0.22f, 1.0f) // Fusion 360-style viewport dark gray
        GLES30.glEnable(GLES30.GL_DEPTH_TEST)

        program = createProgram(vertexShaderCode, fragmentShaderCode)
        setupMesh()
    }

    private fun setupMesh() {
        if (meshResult == null || !meshResult.success || meshResult.vertexBuffer == null || meshResult.indexBuffer == null) {
            return
        }

        indexCount = meshResult.triangleCount * 3

        val vaos = IntArray(1)
        GLES30.glGenVertexArrays(1, vaos, 0)
        vao = vaos[0]
        GLES30.glBindVertexArray(vao)

        val buffers = IntArray(2)
        GLES30.glGenBuffers(2, buffers, 0)
        vbo = buffers[0]
        ibo = buffers[1]

        val vertexBuffer = meshResult.vertexBuffer ?: return
        val indexBuffer = meshResult.indexBuffer ?: return

        // Vertex buffer
        val vBuf = vertexBuffer.order(ByteOrder.nativeOrder())
        vBuf.position(0)
        GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, vbo)
        GLES30.glBufferData(GLES30.GL_ARRAY_BUFFER, vBuf.capacity(), vBuf, GLES30.GL_STATIC_DRAW)

        // Index buffer
        val iBuf = indexBuffer.order(ByteOrder.nativeOrder())
        iBuf.position(0)
        GLES30.glBindBuffer(GLES30.GL_ELEMENT_ARRAY_BUFFER, ibo)
        GLES30.glBufferData(GLES30.GL_ELEMENT_ARRAY_BUFFER, iBuf.capacity(), iBuf, GLES30.GL_STATIC_DRAW)

        val stride = (3 * 4) + (3 * 4) + 4 // 3 float pos, 3 float norm, 1 uint faceId = 28 bytes

        // Position: location 0
        GLES30.glEnableVertexAttribArray(0)
        GLES30.glVertexAttribPointer(0, 3, GLES30.GL_FLOAT, false, stride, 0)

        // Normal: location 1
        GLES30.glEnableVertexAttribArray(1)
        GLES30.glVertexAttribPointer(1, 3, GLES30.GL_FLOAT, false, stride, 12)

        // FaceId: location 2
        GLES30.glEnableVertexAttribArray(2)
        GLES30.glVertexAttribIPointer(2, 1, GLES30.GL_UNSIGNED_INT, stride, 24)

        GLES30.glBindVertexArray(0)
    }

    override fun onSurfaceChanged(gl: GL10?, width: Int, height: Int) {
        GLES30.glViewport(0, 0, width, height)
        val ratio = width.toFloat() / height.toFloat()
        Matrix.perspectiveM(projectionMatrix, 0, 45f, ratio, 1f, 1000f)
    }

    override fun onDrawFrame(gl: GL10?) {
        GLES30.glClear(GLES30.GL_COLOR_BUFFER_BIT or GLES30.GL_DEPTH_BUFFER_BIT)

        if (vao == 0 || indexCount == 0) return

        Matrix.setLookAtM(viewMatrix, 0, 0f, 0f, zoom, 0f, 0f, 0f, 0f, 1f, 0f)

        Matrix.setIdentityM(modelMatrix, 0)
        Matrix.setRotateM(rotationMatrix, 0, angleX, 1f, 0f, 0f)
        Matrix.rotateM(rotationMatrix, 0, angleY, 0f, 1f, 0f)
        Matrix.multiplyMM(modelMatrix, 0, rotationMatrix, 0, modelMatrix, 0)
        // Center the 100x50x20 box around origin
        Matrix.translateM(modelMatrix, 0, -50f, -25f, -10f)

        Matrix.multiplyMM(vPMatrix, 0, viewMatrix, 0, modelMatrix, 0)
        Matrix.multiplyMM(vPMatrix, 0, projectionMatrix, 0, vPMatrix, 0)

        GLES30.glUseProgram(program)
        val mvpHandle = GLES30.glGetUniformLocation(program, "uMVPMatrix")
        val modelHandle = GLES30.glGetUniformLocation(program, "uModelMatrix")

        GLES30.glUniformMatrix4fv(mvpHandle, 1, false, vPMatrix, 0)
        GLES30.glUniformMatrix4fv(modelHandle, 1, false, modelMatrix, 0)

        GLES30.glBindVertexArray(vao)
        GLES30.glDrawElements(GLES30.GL_TRIANGLES, indexCount, GLES30.GL_UNSIGNED_INT, 0)
        GLES30.glBindVertexArray(0)
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
