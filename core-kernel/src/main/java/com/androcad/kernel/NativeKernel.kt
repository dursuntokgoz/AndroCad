// Copyright (c) 2026 dursuntokgoz. All rights reserved.
package com.androcad.kernel

import java.io.Closeable
import java.nio.ByteBuffer
import kotlin.math.sqrt

/**
 * 3D Axis-Aligned Bounding Box.
 */
data class BoundingBox(
    val minX: Float = 0f,
    val minY: Float = 0f,
    val minZ: Float = 0f,
    val maxX: Float = 0f,
    val maxY: Float = 0f,
    val maxZ: Float = 0f
) {
    val centerX: Float get() = (minX + maxX) * 0.5f
    val centerY: Float get() = (minY + maxY) * 0.5f
    val centerZ: Float get() = (minZ + maxZ) * 0.5f
    val sizeX: Float get() = maxX - minX
    val sizeY: Float get() = maxY - minY
    val sizeZ: Float get() = maxZ - minZ
    val diagonal: Float get() = sqrt(sizeX * sizeX + sizeY * sizeY + sizeZ * sizeZ)
}

/**
 * Result of picking an entity (face or edge) via ray or GPU color buffer.
 */
data class PickResult(
    val hit: Boolean,
    val entityId: Int, // 1-based face ID or edge ID
    val isEdge: Boolean,
    val distance: Float,
    val hitX: Float,
    val hitY: Float,
    val hitZ: Float
)

/**
 * Result of analytical solid box creation, edge extraction, and tessellation.
 */
data class BoxMeshResult(
    val success: Boolean,
    val volume: Double,
    val faceCount: Int,
    val edgeCount: Int,
    val triangleCount: Int,
    val vertexBuffer: ByteBuffer?,
    val indexBuffer: ByteBuffer?,
    val edgeVertexBuffer: ByteBuffer?,
    val boundingBox: BoundingBox,
    val errorMessage: String? = null,
    val nativeHandle: Long = 0L
) : Closeable {

    /**
     * Casts a ray in model space to pick a face or boundary edge.
     */
    fun pickByRay(
        ox: Float, oy: Float, oz: Float,
        dx: Float, dy: Float, dz: Float,
        edgeTolerance: Float = 1.5f
    ): PickResult? {
        if (nativeHandle == 0L || !success) return null
        return NativeKernel.pickByRay(nativeHandle, ox, oy, oz, dx, dy, dz, edgeTolerance)
    }

    override fun close() {
        if (nativeHandle != 0L) {
            NativeKernel.releaseMesh(nativeHandle)
        }
    }
}

/**
 * High-performance JNI bridge to the OpenCASCADE native CAD kernel.
 */
object NativeKernel {
    private var isLoaded = false

    init {
        loadLibraries()
    }

    @Synchronized
    fun loadLibraries() {
        if (!isLoaded) {
            try {
                // Load foundation and modeling toolkits in dependency order
                val toolkits = listOf(
                    "TKernel",
                    "TKMath",
                    "TKG2d",
                    "TKG3d",
                    "TKGeomBase",
                    "TKBRep",
                    "TKTopAlgo",
                    "TKPrim",
                    "TKBO",
                    "TKBool",
                    "TKMesh",
                    "androcad_kernel"
                )
                for (tk in toolkits) {
                    try {
                        System.loadLibrary(tk)
                    } catch (e: UnsatisfiedLinkError) {
                        // Some toolkits may be statically linked into androcad_kernel or preloaded
                    }
                }
                isLoaded = true
            } catch (t: Throwable) {
                // Handled gracefully in mock / host JVM test environments
            }
        }
    }

    /**
     * Constructs a solid rectangular prism, computes volume, extracts boundary edges,
     * and tessellates its boundary representation into direct ByteBuffers.
     */
    fun createBox(
        dx: Double,
        dy: Double,
        dz: Double,
        linearDeflection: Double = 0.1,
        angularDeflection: Double = 0.5
    ): BoxMeshResult {
        val handle = nativeCreateBox(dx, dy, dz, linearDeflection, angularDeflection)
        val success = nativeIsSuccess(handle)
        return if (success) {
            val boundsArray = FloatArray(6)
            nativeGetBoundingBox(handle, boundsArray)
            val bbox = BoundingBox(
                minX = boundsArray[0],
                minY = boundsArray[1],
                minZ = boundsArray[2],
                maxX = boundsArray[3],
                maxY = boundsArray[4],
                maxZ = boundsArray[5]
            )

            BoxMeshResult(
                success = true,
                volume = nativeGetVolume(handle),
                faceCount = nativeGetFaceCount(handle),
                edgeCount = nativeGetEdgeCount(handle),
                triangleCount = nativeGetTriangleCount(handle),
                vertexBuffer = nativeGetVertexBuffer(handle),
                indexBuffer = nativeGetIndexBuffer(handle),
                edgeVertexBuffer = nativeGetEdgeVertexBuffer(handle),
                boundingBox = bbox,
                nativeHandle = handle
            )
        } else {
            val error = nativeGetErrorMessage(handle)
            nativeReleaseMesh(handle)
            BoxMeshResult(
                success = false,
                volume = 0.0,
                faceCount = 0,
                edgeCount = 0,
                triangleCount = 0,
                vertexBuffer = null,
                indexBuffer = null,
                edgeVertexBuffer = null,
                boundingBox = BoundingBox(),
                errorMessage = error
            )
        }
    }

    /**
     * Native CPU-side ray picker.
     */
    fun pickByRay(
        handle: Long,
        ox: Float, oy: Float, oz: Float,
        dx: Float, dy: Float, dz: Float,
        edgeTolerance: Float = 1.5f
    ): PickResult? {
        val result = FloatArray(7)
        val hit = nativePickByRay(handle, ox, oy, oz, dx, dy, dz, edgeTolerance, result)
        if (!hit || result[0] == 0.0f) return null

        return PickResult(
            hit = true,
            entityId = result[1].toInt(),
            isEdge = result[2] > 0.5f,
            distance = result[3],
            hitX = result[4],
            hitY = result[5],
            hitZ = result[6]
        )
    }

    internal external fun nativeCreateBox(
        dx: Double,
        dy: Double,
        dz: Double,
        linearDeflection: Double,
        angularDeflection: Double
    ): Long

    internal external fun nativeIsSuccess(handle: Long): Boolean
    internal external fun nativeGetVolume(handle: Long): Double
    internal external fun nativeGetFaceCount(handle: Long): Int
    internal external fun nativeGetEdgeCount(handle: Long): Int
    internal external fun nativeGetTriangleCount(handle: Long): Int
    internal external fun nativeGetErrorMessage(handle: Long): String?
    internal external fun nativeGetVertexBuffer(handle: Long): ByteBuffer?
    internal external fun nativeGetIndexBuffer(handle: Long): ByteBuffer?
    internal external fun nativeGetEdgeVertexBuffer(handle: Long): ByteBuffer?
    internal external fun nativeGetBoundingBox(handle: Long, outBounds: FloatArray)
    internal external fun nativePickByRay(
        handle: Long,
        ox: Float, oy: Float, oz: Float,
        dx: Float, dy: Float, dz: Float,
        edgeTolerance: Float,
        outResult: FloatArray
    ): Boolean
    internal external fun nativeReleaseMesh(handle: Long)

    internal fun releaseMesh(handle: Long) {
        nativeReleaseMesh(handle)
    }
}
