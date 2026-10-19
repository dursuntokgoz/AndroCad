// Copyright (c) 2026 dursuntokgoz. All rights reserved.
package com.androcad.kernel

import java.io.Closeable
import java.nio.ByteBuffer

/**
 * Result of analytical solid box creation and tessellation.
 */
data class BoxMeshResult(
    val success: Boolean,
    val volume: Double,
    val faceCount: Int,
    val triangleCount: Int,
    val vertexBuffer: ByteBuffer?,
    val indexBuffer: ByteBuffer?,
    val errorMessage: String? = null,
    private val nativeHandle: Long = 0L
) : Closeable {
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
                // Handled gracefully in mock environments
            }
        }
    }

    /**
     * Constructs a solid rectangular prism and tessellates its boundary representation.
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
            BoxMeshResult(
                success = true,
                volume = nativeGetVolume(handle),
                faceCount = nativeGetFaceCount(handle),
                triangleCount = nativeGetTriangleCount(handle),
                vertexBuffer = nativeGetVertexBuffer(handle),
                indexBuffer = nativeGetIndexBuffer(handle),
                nativeHandle = handle
            )
        } else {
            val error = nativeGetErrorMessage(handle)
            nativeReleaseMesh(handle)
            BoxMeshResult(
                success = false,
                volume = 0.0,
                faceCount = 0,
                triangleCount = 0,
                vertexBuffer = null,
                indexBuffer = null,
                errorMessage = error
            )
        }
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
    internal external fun nativeGetTriangleCount(handle: Long): Int
    internal external fun nativeGetErrorMessage(handle: Long): String?
    internal external fun nativeGetVertexBuffer(handle: Long): ByteBuffer?
    internal external fun nativeGetIndexBuffer(handle: Long): ByteBuffer?
    internal external fun nativeReleaseMesh(handle: Long)

    internal fun releaseMesh(handle: Long) {
        nativeReleaseMesh(handle)
    }
}
