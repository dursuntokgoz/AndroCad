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

// -------------------------------------------------------------------------
// Milestone 3: 2D Sketcher & PlaneGCS Constraint Data Models
// -------------------------------------------------------------------------

enum class SolveStatus {
    SUCCESS,
    CONVERGED,
    UNDER_CONSTRAINED,
    FULLY_CONSTRAINED,
    OVER_CONSTRAINED,
    FAILED
}

enum class ConstraintType(val id: Int) {
    COINCIDENT(0),
    HORIZONTAL(1),
    VERTICAL(2),
    DISTANCE(3),
    PARALLEL(4),
    PERPENDICULAR(5),
    EQUAL_LENGTH(6)
}

data class SketchPoint(val id: Int, val u: Double, val v: Double, val isFixed: Boolean)

data class SketchProfileResult(
    val success: Boolean,
    val isClosedProfile: Boolean,
    val area: Double,
    val degreesOfFreedom: Int,
    val solveStatus: SolveStatus
)

class NativeSketch(val handle: Long) : Closeable {
    fun addPoint(u: Double, v: Double, isFixed: Boolean = false): Int {
        return NativeKernel.nativeSketchAddPoint(handle, u, v, isFixed)
    }

    fun addLine(pStart: Int, pEnd: Int): Int {
        return NativeKernel.nativeSketchAddLine(handle, pStart, pEnd)
    }

    fun addCircle(pCenter: Int, radius: Double): Int {
        return NativeKernel.nativeSketchAddCircle(handle, pCenter, radius)
    }

    fun addRectangle(u: Double, v: Double, width: Double, height: Double): Int {
        return NativeKernel.nativeSketchAddRectangle(handle, u, v, width, height)
    }

    fun addConstraint(type: ConstraintType, p1: Int, p2: Int, p3: Int = -1, p4: Int = -1, value: Double = 0.0): Int {
        return NativeKernel.nativeSketchAddConstraint(handle, type.id, p1, p2, p3, p4, value)
    }

    fun solve(): SolveStatus {
        val statusCode = NativeKernel.nativeSketchSolve(handle)
        return when (statusCode) {
            0 -> SolveStatus.SUCCESS
            1 -> SolveStatus.CONVERGED
            2 -> SolveStatus.UNDER_CONSTRAINED
            3 -> SolveStatus.FULLY_CONSTRAINED
            4 -> SolveStatus.OVER_CONSTRAINED
            else -> SolveStatus.FAILED
        }
    }

    fun getDegreesOfFreedom(): Int {
        return NativeKernel.nativeSketchGetDoF(handle)
    }

    fun getPoint(pointId: Int): Pair<Double, Double> {
        val coords = DoubleArray(2)
        NativeKernel.nativeSketchGetPoint(handle, pointId, coords)
        return Pair(coords[0], coords[1])
    }

    fun buildProfileFace(): SketchProfileResult {
        val res = DoubleArray(4)
        val success = NativeKernel.nativeSketchBuildProfileFace(handle, res)
        val status = when (res[3].toInt()) {
            0 -> SolveStatus.SUCCESS
            1 -> SolveStatus.CONVERGED
            2 -> SolveStatus.UNDER_CONSTRAINED
            3 -> SolveStatus.FULLY_CONSTRAINED
            4 -> SolveStatus.OVER_CONSTRAINED
            else -> SolveStatus.FAILED
        }
        return SketchProfileResult(
            success = success,
            isClosedProfile = res[0] > 0.5,
            area = res[1],
            degreesOfFreedom = res[2].toInt(),
            solveStatus = status
        )
    }

    override fun close() {
        if (handle != 0L) {
            NativeKernel.nativeReleaseSketch(handle)
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

    /**
     * Creates a 2D parametric sketch on a specified 3D datum plane.
     */
    fun createSketch(
        ox: Double = 0.0, oy: Double = 0.0, oz: Double = 0.0,
        ux: Double = 1.0, uy: Double = 0.0, uz: Double = 0.0,
        vx: Double = 0.0, vy: Double = 1.0, vz: Double = 0.0,
        nx: Double = 0.0, ny: Double = 0.0, nz: Double = 1.0
    ): NativeSketch {
        val handle = nativeCreateSketch(ox, oy, oz, ux, uy, uz, vx, vy, vz, nx, ny, nz)
        return NativeSketch(handle)
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
    internal fun releaseMesh(handle: Long) { nativeReleaseMesh(handle) }

    // Milestone 3 JNI Declarations
    internal external fun nativeCreateSketch(
        ox: Double, oy: Double, oz: Double,
        ux: Double, uy: Double, uz: Double,
        vx: Double, vy: Double, vz: Double,
        nx: Double, ny: Double, nz: Double
    ): Long
    internal external fun nativeSketchAddPoint(handle: Long, u: Double, v: Double, isFixed: Boolean): Int
    internal external fun nativeSketchAddLine(handle: Long, pStart: Int, pEnd: Int): Int
    internal external fun nativeSketchAddCircle(handle: Long, pCenter: Int, radius: Double): Int
    internal external fun nativeSketchAddRectangle(handle: Long, u: Double, v: Double, width: Double, height: Double): Int
    internal external fun nativeSketchAddConstraint(handle: Long, type: Int, p1: Int, p2: Int, p3: Int, p4: Int, value: Double): Int
    internal external fun nativeSketchSolve(handle: Long): Int
    internal external fun nativeSketchGetDoF(handle: Long): Int
    internal external fun nativeSketchGetPoint(handle: Long, pointId: Int, outCoords: DoubleArray)
    internal external fun nativeSketchBuildProfileFace(handle: Long, outResult: DoubleArray): Boolean
    internal external fun nativeReleaseSketch(handle: Long)
}
