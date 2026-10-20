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

enum class BooleanType(val id: Int) {
    JOIN(0),
    CUT(1),
    INTERSECT(2)
}

typealias SolidMeshResult = BoxMeshResult

/**
 * Milestone 6: Result of STEP / STL / Project File I/O operations.
 */
data class IOResult(
    val success: Boolean,
    val errorMessage: String = "",
    val filePath: String = ""
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

    fun fillet(radius: Double, edgeId: Int = -1): BoxMeshResult =
        NativeKernel.fillet(this, radius, edgeId)

    fun chamfer(distance: Double, edgeId: Int = -1): BoxMeshResult =
        NativeKernel.chamfer(this, distance, edgeId)

    fun hole(px: Double, py: Double, pz: Double, dx: Double, dy: Double, dz: Double, radius: Double, depth: Double): BoxMeshResult =
        NativeKernel.hole(this, px, py, pz, dx, dy, dz, radius, depth)

    fun shell(thickness: Double, removeFaceId: Int = -1): BoxMeshResult =
        NativeKernel.shell(this, thickness, removeFaceId)

    fun join(other: BoxMeshResult): BoxMeshResult =
        NativeKernel.booleanOp(this, other, BooleanType.JOIN)

    fun cut(other: BoxMeshResult): BoxMeshResult =
        NativeKernel.booleanOp(this, other, BooleanType.CUT)

    fun intersect(other: BoxMeshResult): BoxMeshResult =
        NativeKernel.booleanOp(this, other, BooleanType.INTERSECT)

    fun pressPull(distance: Double, faceId: Int = 1): BoxMeshResult =
        NativeKernel.pressPull(this, faceId, distance)

    fun thicken(thickness: Double): BoxMeshResult =
        NativeKernel.thicken(this, thickness)

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

    fun extrude(dx: Double, dy: Double, dz: Double): BoxMeshResult =
        NativeKernel.extrude(this, dx, dy, dz)

    fun revolve(ax: Double, ay: Double, az: Double, dx: Double, dy: Double, dz: Double, angleRad: Double): BoxMeshResult =
        NativeKernel.revolve(this, ax, ay, az, dx, dy, dz, angleRad)

    override fun close() {
        if (handle != 0L) {
            NativeKernel.nativeReleaseSketch(handle)
        }
    }
}

/**
 * Milestone 5: Parametric Timeline DAG & Persistent Naming.
 */
class NativeTimeline(val handle: Long) : Closeable {
    fun setParameter(name: String, expression: String): Boolean {
        if (handle == 0L) return false
        return NativeKernel.nativeTimelineSetParameter(handle, name, expression)
    }

    fun getParameter(name: String): Double {
        if (handle == 0L) return 0.0
        return NativeKernel.nativeTimelineGetParameter(handle, name)
    }

    fun addBox(name: String, dx: String, dy: String, dz: String): Int {
        if (handle == 0L) return -1
        return NativeKernel.nativeTimelineAddBox(handle, name, dx, dy, dz)
    }

    fun setRollbackIndex(index: Int) {
        if (handle == 0L) return
        NativeKernel.nativeTimelineSetRollback(handle, index)
    }

    fun rebuild(): BoxMeshResult {
        if (handle == 0L) {
            return BoxMeshResult(
                success = false,
                volume = 0.0,
                faceCount = 0,
                edgeCount = 0,
                triangleCount = 0,
                vertexBuffer = null,
                indexBuffer = null,
                edgeVertexBuffer = null,
                boundingBox = BoundingBox(),
                errorMessage = "Invalid timeline handle."
            )
        }
        val meshHandle = NativeKernel.nativeTimelineRebuild(handle)
        return NativeKernel.buildMeshResult(meshHandle)
    }

    fun saveProject(filePath: String): IOResult {
        if (handle == 0L) return IOResult(false, "Invalid timeline handle.", filePath)
        val errArr = arrayOfNulls<String>(1)
        val ok = NativeKernel.nativeSaveProject(handle, filePath, errArr)
        return IOResult(ok, errArr[0] ?: "", filePath)
    }

    fun loadProject(filePath: String): IOResult {
        if (handle == 0L) return IOResult(false, "Invalid timeline handle.", filePath)
        val errArr = arrayOfNulls<String>(1)
        val ok = NativeKernel.nativeLoadProject(handle, filePath, errArr)
        return IOResult(ok, errArr[0] ?: "", filePath)
    }

    override fun close() {
        if (handle != 0L) {
            NativeKernel.nativeReleaseTimeline(handle)
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
                    "TKGeomAlgo",
                    "TKTopAlgo",
                    "TKPrim",
                    "TKBO",
                    "TKBool",
                    "TKFillet",
                    "TKOffset",
                    "TKMesh",
                    "TKXSBase",
                    "TKDESTEP",
                    "TKDESTL",
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

    internal fun buildMeshResult(handle: Long): BoxMeshResult {
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
        return buildMeshResult(handle)
    }

    /**
     * Extrudes a 2D closed sketch profile into a 3D solid prism along vector (dx, dy, dz).
     */
    fun extrude(
        sketch: NativeSketch,
        dx: Double,
        dy: Double,
        dz: Double,
        linearDeflection: Double = 0.1,
        angularDeflection: Double = 0.5
    ): BoxMeshResult {
        if (sketch.handle == 0L) {
            return BoxMeshResult(
                success = false,
                volume = 0.0,
                faceCount = 0,
                edgeCount = 0,
                triangleCount = 0,
                vertexBuffer = null,
                indexBuffer = null,
                edgeVertexBuffer = null,
                boundingBox = BoundingBox(),
                errorMessage = "Invalid sketch handle."
            )
        }
        val handle = nativeExtrude(sketch.handle, dx, dy, dz, linearDeflection, angularDeflection)
        return buildMeshResult(handle)
    }

    /**
     * Revolves a 2D closed sketch profile around an axis by an angle in radians.
     */
    fun revolve(
        sketch: NativeSketch,
        ax: Double, ay: Double, az: Double,
        dx: Double, dy: Double, dz: Double,
        angleRad: Double,
        linearDeflection: Double = 0.1,
        angularDeflection: Double = 0.5
    ): BoxMeshResult {
        if (sketch.handle == 0L) {
            return BoxMeshResult(
                success = false,
                volume = 0.0,
                faceCount = 0,
                edgeCount = 0,
                triangleCount = 0,
                vertexBuffer = null,
                indexBuffer = null,
                edgeVertexBuffer = null,
                boundingBox = BoundingBox(),
                errorMessage = "Invalid sketch handle."
            )
        }
        val handle = nativeRevolve(sketch.handle, ax, ay, az, dx, dy, dz, angleRad, linearDeflection, angularDeflection)
        return buildMeshResult(handle)
    }

    /**
     * Applies constant-radius rounding fillet to target edge (or all edges if targetEdgeId = -1).
     */
    fun fillet(
        solid: BoxMeshResult,
        radius: Double,
        targetEdgeId: Int = -1,
        linearDeflection: Double = 0.1,
        angularDeflection: Double = 0.5
    ): BoxMeshResult {
        if (solid.nativeHandle == 0L || !solid.success) {
            return BoxMeshResult(
                success = false,
                volume = 0.0,
                faceCount = 0,
                edgeCount = 0,
                triangleCount = 0,
                vertexBuffer = null,
                indexBuffer = null,
                edgeVertexBuffer = null,
                boundingBox = BoundingBox(),
                errorMessage = "Invalid solid handle or failed solid."
            )
        }
        val handle = nativeFillet(solid.nativeHandle, radius, targetEdgeId, linearDeflection, angularDeflection)
        return buildMeshResult(handle)
    }

    /**
     * Applies chamfer bevel to target edge (or first edge if targetEdgeId = -1).
     */
    fun chamfer(
        solid: BoxMeshResult,
        distance: Double,
        targetEdgeId: Int = -1,
        linearDeflection: Double = 0.1,
        angularDeflection: Double = 0.5
    ): BoxMeshResult {
        if (solid.nativeHandle == 0L || !solid.success) {
            return BoxMeshResult(
                success = false,
                volume = 0.0,
                faceCount = 0,
                edgeCount = 0,
                triangleCount = 0,
                vertexBuffer = null,
                indexBuffer = null,
                edgeVertexBuffer = null,
                boundingBox = BoundingBox(),
                errorMessage = "Invalid solid handle or failed solid."
            )
        }
        val handle = nativeChamfer(solid.nativeHandle, distance, targetEdgeId, linearDeflection, angularDeflection)
        return buildMeshResult(handle)
    }

    /**
     * Cuts a cylindrical hole through a solid at position (px, py, pz) along direction (dx, dy, dz).
     */
    fun hole(
        solid: BoxMeshResult,
        px: Double, py: Double, pz: Double,
        dx: Double, dy: Double, dz: Double,
        radius: Double,
        depth: Double,
        linearDeflection: Double = 0.1,
        angularDeflection: Double = 0.5
    ): BoxMeshResult {
        if (solid.nativeHandle == 0L || !solid.success) {
            return BoxMeshResult(
                success = false,
                volume = 0.0,
                faceCount = 0,
                edgeCount = 0,
                triangleCount = 0,
                vertexBuffer = null,
                indexBuffer = null,
                edgeVertexBuffer = null,
                boundingBox = BoundingBox(),
                errorMessage = "Invalid solid handle or failed solid."
            )
        }
        val handle = nativeHole(solid.nativeHandle, px, py, pz, dx, dy, dz, radius, depth, linearDeflection, angularDeflection)
        return buildMeshResult(handle)
    }

    /**
     * Hollows out a solid with wall thickness, removing opening face if specified.
     */
    fun shell(
        solid: BoxMeshResult,
        thickness: Double,
        removeFaceId: Int = -1,
        linearDeflection: Double = 0.1,
        angularDeflection: Double = 0.5
    ): BoxMeshResult {
        if (solid.nativeHandle == 0L || !solid.success) {
            return BoxMeshResult(
                success = false,
                volume = 0.0,
                faceCount = 0,
                edgeCount = 0,
                triangleCount = 0,
                vertexBuffer = null,
                indexBuffer = null,
                edgeVertexBuffer = null,
                boundingBox = BoundingBox(),
                errorMessage = "Invalid solid handle or failed solid."
            )
        }
        val handle = nativeShell(solid.nativeHandle, thickness, removeFaceId, linearDeflection, angularDeflection)
        return buildMeshResult(handle)
    }

    /**
     * Performs Boolean operation (Join/Fuse, Cut/Difference, Intersect/Common) between two solids.
     */
    fun booleanOp(
        solidA: BoxMeshResult,
        solidB: BoxMeshResult,
        type: BooleanType,
        fuzzy: Double = 1e-5,
        linearDeflection: Double = 0.1,
        angularDeflection: Double = 0.5
    ): BoxMeshResult {
        if (solidA.nativeHandle == 0L || !solidA.success || solidB.nativeHandle == 0L || !solidB.success) {
            return BoxMeshResult(
                success = false,
                volume = 0.0,
                faceCount = 0,
                edgeCount = 0,
                triangleCount = 0,
                vertexBuffer = null,
                indexBuffer = null,
                edgeVertexBuffer = null,
                boundingBox = BoundingBox(),
                errorMessage = "Invalid solid handles or failed solid."
            )
        }
        val handle = nativeBoolean(solidA.nativeHandle, solidB.nativeHandle, type.id, fuzzy, linearDeflection, angularDeflection)
        return buildMeshResult(handle)
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
        val handle = try {
            nativeCreateSketch(ox, oy, oz, ux, uy, uz, vx, vy, vz, nx, ny, nz)
        } catch (t: Throwable) {
            0L
        }
        return NativeSketch(handle)
    }

    /**
     * Milestone 5: Evaluates mathematical expression with optional variable substitutions.
     */
    fun evaluateExpression(expr: String, variables: Map<String, Double> = emptyMap()): Pair<Boolean, Double> {
        return try {
            val names = variables.keys.toTypedArray()
            val values = variables.values.toDoubleArray()
            val res = nativeEvaluateExpression(expr, names, values) ?: return Pair(false, 0.0)
            Pair(res[0] > 0.5, res[1])
        } catch (t: Throwable) {
            Pair(false, 0.0)
        }
    }

    /**
     * Milestone 5: Creates a parametric Timeline feature DAG.
     */
    fun createTimeline(): NativeTimeline {
        val handle = try {
            nativeCreateTimeline()
        } catch (t: Throwable) {
            0L
        }
        return NativeTimeline(handle)
    }

    /**
     * Milestone 6: Exports solid model to STEP format (AP214 or AP242).
     */
    fun exportStep(solid: BoxMeshResult, filePath: String, schema: String = "AP214"): IOResult {
        if (solid.nativeHandle == 0L || !solid.success) {
            return IOResult(false, "Invalid solid mesh result.", filePath)
        }
        val errArr = arrayOfNulls<String>(1)
        val ok = try {
            nativeExportStep(solid.nativeHandle, filePath, schema, errArr)
        } catch (t: Throwable) {
            false
        }
        return IOResult(ok, errArr[0] ?: if (!ok) "Native export failed." else "", filePath)
    }

    /**
     * Milestone 6: Imports solid model from STEP file.
     */
    fun importStep(filePath: String): BoxMeshResult {
        val errArr = arrayOfNulls<String>(1)
        val meshHandle = try {
            nativeImportStep(filePath, errArr)
        } catch (t: Throwable) {
            0L
        }
        if (meshHandle == 0L) {
            return BoxMeshResult(
                success = false,
                volume = 0.0,
                faceCount = 0,
                edgeCount = 0,
                triangleCount = 0,
                vertexBuffer = null,
                indexBuffer = null,
                edgeVertexBuffer = null,
                boundingBox = BoundingBox(),
                errorMessage = errArr[0] ?: "Failed to import STEP file."
            )
        }
        return buildMeshResult(meshHandle)
    }

    /**
     * Milestone 8: Creates a loft solid transitioning from bottom rectangle (100x50) to top circle (radius).
     */
    fun createLoft(
        zHeight: Double = 40.0,
        radius: Double = 20.0,
        linearDeflection: Double = 0.1,
        angularDeflection: Double = 0.5
    ): BoxMeshResult {
        val handle = nativeCreateLoftRectToCircle(zHeight, radius, linearDeflection, angularDeflection)
        return buildMeshResult(handle)
    }

    /**
     * Milestone 8: Direct modeling Press-Pull face offset.
     */
    fun pressPull(
        solid: BoxMeshResult,
        faceId: Int,
        distance: Double,
        linearDeflection: Double = 0.1,
        angularDeflection: Double = 0.5
    ): BoxMeshResult {
        if (solid.nativeHandle == 0L || !solid.success) {
            return BoxMeshResult(
                success = false,
                volume = 0.0,
                faceCount = 0,
                edgeCount = 0,
                triangleCount = 0,
                vertexBuffer = null,
                indexBuffer = null,
                edgeVertexBuffer = null,
                boundingBox = BoundingBox(),
                errorMessage = "Invalid solid handle for Press-Pull."
            )
        }
        val handle = nativePressPullFace(solid.nativeHandle, faceId, distance, linearDeflection, angularDeflection)
        return buildMeshResult(handle)
    }

    /**
     * Milestone 8: Shell thicken operation.
     */
    fun thicken(
        solid: BoxMeshResult,
        thickness: Double,
        linearDeflection: Double = 0.1,
        angularDeflection: Double = 0.5
    ): BoxMeshResult {
        if (solid.nativeHandle == 0L || !solid.success) {
            return BoxMeshResult(
                success = false,
                volume = 0.0,
                faceCount = 0,
                edgeCount = 0,
                triangleCount = 0,
                vertexBuffer = null,
                indexBuffer = null,
                edgeVertexBuffer = null,
                boundingBox = BoundingBox(),
                errorMessage = "Invalid solid handle for Thicken."
            )
        }
        val handle = nativeThicken(solid.nativeHandle, thickness, linearDeflection, angularDeflection)
        return buildMeshResult(handle)
    }

    /**
     * Milestone 6: Exports solid model to STL mesh format.
     */
    fun exportStl(solid: BoxMeshResult, filePath: String, linearDeflection: Double = 0.1, binary: Boolean = true): IOResult {
        if (solid.nativeHandle == 0L || !solid.success) {
            return IOResult(false, "Invalid solid mesh result.", filePath)
        }
        val errArr = arrayOfNulls<String>(1)
        val ok = try {
            nativeExportStl(solid.nativeHandle, filePath, linearDeflection, binary, errArr)
        } catch (t: Throwable) {
            false
        }
        return IOResult(ok, errArr[0] ?: if (!ok) "Native export failed." else "", filePath)
    }

    /**
     * Milestone 6: Imports triangle mesh from STL file.
     */
    fun importStl(filePath: String): BoxMeshResult {
        val errArr = arrayOfNulls<String>(1)
        val meshHandle = try {
            nativeImportStl(filePath, errArr)
        } catch (t: Throwable) {
            0L
        }
        if (meshHandle == 0L) {
            return BoxMeshResult(
                success = false,
                volume = 0.0,
                faceCount = 0,
                edgeCount = 0,
                triangleCount = 0,
                vertexBuffer = null,
                indexBuffer = null,
                edgeVertexBuffer = null,
                boundingBox = BoundingBox(),
                errorMessage = errArr[0] ?: "Failed to import STL file."
            )
        }
        return buildMeshResult(meshHandle)
    }

    /**
     * Milestone 8: Creates a loft solid transitioning from bottom rectangle (100x50) to top circle (radius).
     */
    fun createLoft(
        zHeight: Double = 40.0,
        radius: Double = 20.0,
        linearDeflection: Double = 0.1,
        angularDeflection: Double = 0.5
    ): BoxMeshResult {
        val handle = nativeCreateLoftRectToCircle(zHeight, radius, linearDeflection, angularDeflection)
        return buildMeshResult(handle)
    }

    /**
     * Milestone 8: Direct modeling Press-Pull face offset.
     */
    fun pressPull(
        solid: BoxMeshResult,
        faceId: Int,
        distance: Double,
        linearDeflection: Double = 0.1,
        angularDeflection: Double = 0.5
    ): BoxMeshResult {
        if (solid.nativeHandle == 0L || !solid.success) {
            return BoxMeshResult(
                success = false,
                volume = 0.0,
                faceCount = 0,
                edgeCount = 0,
                triangleCount = 0,
                vertexBuffer = null,
                indexBuffer = null,
                edgeVertexBuffer = null,
                boundingBox = BoundingBox(),
                errorMessage = "Invalid solid handle for Press-Pull."
            )
        }
        val handle = nativePressPullFace(solid.nativeHandle, faceId, distance, linearDeflection, angularDeflection)
        return buildMeshResult(handle)
    }

    /**
     * Milestone 8: Shell thicken operation.
     */
    fun thicken(
        solid: BoxMeshResult,
        thickness: Double,
        linearDeflection: Double = 0.1,
        angularDeflection: Double = 0.5
    ): BoxMeshResult {
        if (solid.nativeHandle == 0L || !solid.success) {
            return BoxMeshResult(
                success = false,
                volume = 0.0,
                faceCount = 0,
                edgeCount = 0,
                triangleCount = 0,
                vertexBuffer = null,
                indexBuffer = null,
                edgeVertexBuffer = null,
                boundingBox = BoundingBox(),
                errorMessage = "Invalid solid handle for Thicken."
            )
        }
        val handle = nativeThicken(solid.nativeHandle, thickness, linearDeflection, angularDeflection)
        return buildMeshResult(handle)
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

    // Milestone 4 JNI Declarations
    internal external fun nativeExtrude(
        sketchHandle: Long,
        dx: Double, dy: Double, dz: Double,
        linearDeflection: Double, angularDeflection: Double
    ): Long

    internal external fun nativeRevolve(
        sketchHandle: Long,
        ax: Double, ay: Double, az: Double,
        dx: Double, dy: Double, dz: Double,
        angleRad: Double,
        linearDeflection: Double, angularDeflection: Double
    ): Long

    internal external fun nativeFillet(
        solidHandle: Long,
        radius: Double,
        targetEdgeId: Int,
        linearDeflection: Double, angularDeflection: Double
    ): Long

    internal external fun nativeChamfer(
        solidHandle: Long,
        distance: Double,
        targetEdgeId: Int,
        linearDeflection: Double, angularDeflection: Double
    ): Long

    internal external fun nativeHole(
        solidHandle: Long,
        px: Double, py: Double, pz: Double,
        dx: Double, dy: Double, dz: Double,
        radius: Double, depth: Double,
        linearDeflection: Double, angularDeflection: Double
    ): Long

    internal external fun nativeShell(
        solidHandle: Long,
        thickness: Double,
        removeFaceId: Int,
        linearDeflection: Double, angularDeflection: Double
    ): Long

    internal external fun nativeBoolean(
        solidAHandle: Long,
        solidBHandle: Long,
        opType: Int,
        fuzzyValue: Double,
        linearDeflection: Double, angularDeflection: Double
    ): Long

    // Milestone 5 JNI Declarations
    internal external fun nativeEvaluateExpression(expr: String, varNames: Array<String>, varValues: DoubleArray): DoubleArray?
    internal external fun nativeCreateTimeline(): Long
    internal external fun nativeTimelineSetParameter(handle: Long, name: String, expr: String): Boolean
    internal external fun nativeTimelineGetParameter(handle: Long, name: String): Double
    internal external fun nativeTimelineAddBox(handle: Long, name: String, dx: String, dy: String, dz: String): Int
    internal external fun nativeTimelineSetRollback(handle: Long, rollbackIndex: Int)
    internal external fun nativeTimelineRebuild(handle: Long): Long
    internal external fun nativeReleaseTimeline(handle: Long)

    // Milestone 6 JNI Declarations (STEP / STL / Project I/O)
    internal external fun nativeExportStep(handle: Long, filePath: String, schema: String, outError: Array<String?>?): Boolean
    internal external fun nativeImportStep(filePath: String, outError: Array<String?>?): Long
    internal external fun nativeExportStl(handle: Long, filePath: String, deflection: Double, binary: Boolean, outError: Array<String?>?): Boolean
    internal external fun nativeImportStl(filePath: String, outError: Array<String?>?): Long
    internal external fun nativeSaveProject(handle: Long, filePath: String, outError: Array<String?>?): Boolean
    internal external fun nativeLoadProject(handle: Long, filePath: String, outError: Array<String?>?): Boolean

    // Milestone 8 JNI Declarations (Surface & Direct Modeling)
    internal external fun nativeCreateLoftRectToCircle(
        zHeight: Double,
        radius: Double,
        linearDeflection: Double,
        angularDeflection: Double
    ): Long

    internal external fun nativeThicken(
        solidHandle: Long,
        thickness: Double,
        linearDeflection: Double,
        angularDeflection: Double
    ): Long

    internal external fun nativePressPullFace(
        solidHandle: Long,
        faceId: Int,
        distance: Double,
        linearDeflection: Double,
        angularDeflection: Double
    ): Long
}