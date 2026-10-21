// Copyright (c) 2026 dursuntokgoz. All rights reserved.
package com.androcad.kernel

import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test

class NativeKernelTest {

    @Test
    fun testBoxMeshResultDataContract() {
        val bbox = BoundingBox(0f, 0f, 0f, 100f, 50f, 20f)
        val result = BoxMeshResult(
            success = true,
            volume = 100000.0,
            faceCount = 6,
            edgeCount = 12,
            triangleCount = 12,
            vertexBuffer = null,
            indexBuffer = null,
            edgeVertexBuffer = null,
            boundingBox = bbox,
            errorMessage = null,
            nativeHandle = 0L
        )

        assertTrue(result.success)
        assertEquals(100000.0, result.volume, 1e-4)
        assertEquals(6, result.faceCount)
        assertEquals(12, result.edgeCount)
        assertEquals(12, result.triangleCount)
        assertEquals(50f, bbox.centerX, 1e-4f)
        assertEquals(25f, bbox.centerY, 1e-4f)
        assertEquals(10f, bbox.centerZ, 1e-4f)
        assertTrue(bbox.diagonal > 100f)
        result.close() // Verify close does not throw on 0L handle
    }

    @Test
    fun testPickResultContract() {
        val pickFace = PickResult(
            hit = true,
            entityId = 3,
            isEdge = false,
            distance = 45.2f,
            hitX = 50f,
            hitY = 25f,
            hitZ = 20f
        )
        assertTrue(pickFace.hit)
        assertEquals(3, pickFace.entityId)
        assertFalse(pickFace.isEdge)
        assertEquals(45.2f, pickFace.distance, 1e-4f)

        val pickEdge = PickResult(
            hit = true,
            entityId = 7,
            isEdge = true,
            distance = 12.0f,
            hitX = 100f,
            hitY = 25f,
            hitZ = 20f
        )
        assertTrue(pickEdge.isEdge)
        assertEquals(7, pickEdge.entityId)
    }

    @Test
    fun testSketchDataContract() {
        val profileRes = SketchProfileResult(
            success = true,
            isClosedProfile = true,
            area = 5000.0,
            degreesOfFreedom = 0,
            solveStatus = SolveStatus.FULLY_CONSTRAINED
        )
        assertTrue(profileRes.success)
        assertTrue(profileRes.isClosedProfile)
        assertEquals(5000.0, profileRes.area, 1e-4)
        assertEquals(0, profileRes.degreesOfFreedom)
        assertEquals(SolveStatus.FULLY_CONSTRAINED, profileRes.solveStatus)

        val sketch = NativeSketch(0L)
        sketch.close() // Verify safe close on 0L handle
    }

    @Test
    fun testBooleanTypeEnum() {
        assertEquals(0, BooleanType.JOIN.id)
        assertEquals(1, BooleanType.CUT.id)
        assertEquals(2, BooleanType.INTERSECT.id)
    }

    @Test
    fun testFeatureOperationsContract() {
        val bbox = BoundingBox(0f, 0f, 0f, 100f, 50f, 30f)
        val solid = BoxMeshResult(
            success = true,
            volume = 150000.0,
            faceCount = 6,
            edgeCount = 12,
            triangleCount = 12,
            vertexBuffer = null,
            indexBuffer = null,
            edgeVertexBuffer = null,
            boundingBox = bbox,
            errorMessage = null,
            nativeHandle = 0L
        )

        // Verifying default mock error behavior on 0L handle (does not crash JVM)
        val filletRes = solid.fillet(5.0, 1)
        assertFalse(filletRes.success)

        val chamferRes = solid.chamfer(3.0, 1)
        assertFalse(chamferRes.success)

        val holeRes = solid.hole(50.0, 25.0, 0.0, 0.0, 0.0, 1.0, 10.0, 30.0)
        assertFalse(holeRes.success)

        val shellRes = solid.shell(2.0, 1)
        assertFalse(shellRes.success)

        val booleanRes = solid.join(solid)
        assertFalse(booleanRes.success)
    }

    @Test
    fun testTimelineContract() {
        val timeline = NativeTimeline(0L)
        assertFalse(timeline.setParameter("width", "100.0"))
        assertEquals(0.0, timeline.getParameter("width"), 1e-6)
        assertEquals(-1, timeline.addBox("Box1", "10", "20", "30"))
        timeline.setRollbackIndex(0)

        val rebuildRes = timeline.rebuild()
        assertFalse(rebuildRes.success)
        assertEquals("Invalid timeline handle.", rebuildRes.errorMessage)
        timeline.close() // Verify close does not throw on 0L handle
    }

    @Test
    fun testExpressionEvaluationMockContract() {
        // In JVM test environment without native binary, should return Pair(false, 0.0) without crashing
        val res = NativeKernel.evaluateExpression("10 + 20")
        assertFalse(res.first)
    }

    @Test
    fun testDataExchangeContract() {
        val ioRes = IOResult(success = true, errorMessage = "", filePath = "test.step")
        assertTrue(ioRes.success)
        assertEquals("test.step", ioRes.filePath)

        val bbox = BoundingBox(0f, 0f, 0f, 10f, 10f, 10f)
        val dummySolid = BoxMeshResult(
            success = true,
            volume = 1000.0,
            faceCount = 6,
            edgeCount = 12,
            triangleCount = 12,
            vertexBuffer = null,
            indexBuffer = null,
            edgeVertexBuffer = null,
            boundingBox = bbox,
            nativeHandle = 0L
        )

        // Verify export/import guards against 0L handles without throwing
        val stepExportRes = NativeKernel.exportStep(dummySolid, "dummy.step")
        assertFalse(stepExportRes.success)

        val stlExportRes = NativeKernel.exportStl(dummySolid, "dummy.stl")
        assertFalse(stlExportRes.success)

        val stepImportRes = NativeKernel.importStep("dummy.step")
        assertFalse(stepImportRes.success)

        val stlImportRes = NativeKernel.importStl("dummy.stl")
        assertFalse(stlImportRes.success)

        val timeline = NativeTimeline(0L)
        val saveProjRes = timeline.saveProject("dummy.androcad")
        assertFalse(saveProjRes.success)

        val loadProjRes = timeline.loadProject("dummy.androcad")
        assertFalse(loadProjRes.success)
    }

    @Test
    fun testSheetMetalContracts() {
        val rule = SheetMetalRule(thickness = 2.0, bendRadius = 2.0, kFactor = 0.44)
        assertEquals(2.0, rule.thickness, 1e-4)
        assertEquals(2.0, rule.bendRadius, 1e-4)
        assertEquals(0.44, rule.kFactor, 1e-4)

        val bend = SheetMetalBendLine(
            id = 1,
            startX = 38.26,
            startY = 0.0,
            endX = 38.26,
            endY = 50.0,
            angleDeg = 90.0,
            radius = 2.0,
            isBendUp = true
        )
        assertEquals(1, bend.id)
        assertEquals(38.26, bend.startX, 1e-4)
        assertTrue(bend.isBendUp)

        val pattern = SheetMetalFlatPattern(
            success = true,
            flatLength = 173.0478,
            flatWidth = 50.0,
            flatArea = 8652.39,
            bends = listOf(bend)
        )
        assertTrue(pattern.success)
        assertEquals(173.0478, pattern.flatLength, 1e-4)
        assertEquals(50.0, pattern.flatWidth, 1e-4)
        assertEquals(1, pattern.bends.size)
    }

    @Test
    fun testDrawingSheetContracts() {
        val sheet = DrawingSheetInfo(
            sheetWidth = 420.0,
            sheetHeight = 297.0,
            partName = "TestBracket",
            scaleStr = "1:1",
            svgContent = "<svg></svg>",
            dxfContent = "0\nEOF\n"
        )
        assertEquals(420.0, sheet.sheetWidth, 1e-4)
        assertEquals(297.0, sheet.sheetHeight, 1e-4)
        assertEquals("TestBracket", sheet.partName)
        assertEquals("1:1", sheet.scaleStr)
        assertTrue(sheet.svgContent.isNotEmpty())
        assertTrue(sheet.dxfContent.isNotEmpty())
    }

    @Test
    fun testMaterialAndPhysicalPropertiesContracts() {
        val mat = MaterialInfo(
            id = 0,
            name = "Steel (Structural)",
            category = "Metal",
            density = 7.85,
            youngModulus = 210.0,
            hexColor = "#9EA5AD",
            metallic = 0.85f,
            roughness = 0.35f
        )
        assertEquals(0, mat.id)
        assertEquals(7.85, mat.density, 1e-4)
        assertEquals(210.0, mat.youngModulus, 1e-4)

        val props = PhysicalPropertiesInfo(
            success = true,
            materialName = "Steel (Structural)",
            density = 7.85,
            volumeMm3 = 100000.0,
            surfaceAreaMm2 = 16000.0,
            massGrams = 785.0,
            massKg = 0.785,
            cogX = 50.0, cogY = 25.0, cogZ = 10.0,
            ixx = 10.0, iyy = 20.0, izz = 30.0,
            hexColor = "#9EA5AD",
            metallic = 0.85f,
            roughness = 0.35f
        )
        assertTrue(props.success)
        assertEquals(100000.0, props.volumeMm3, 1e-4)
        assertEquals(785.0, props.massGrams, 1e-4)
        assertEquals(0.785, props.massKg, 1e-4)
        assertEquals(50.0, props.cogX, 1e-4)
    }

    @Test
    fun testMeshAnalysisContracts() {
        val analysis = MeshAnalysisInfo(
            success = true,
            vertexCount = 8,
            triangleCount = 12,
            edgeCount = 18,
            surfaceArea = 16000.0,
            enclosedVolume = 100000.0,
            isClosedManifold = true,
            eulerCharacteristic = 2
        )
        assertTrue(analysis.success)
        assertEquals(8L, analysis.vertexCount)
        assertEquals(12L, analysis.triangleCount)
        assertEquals(18L, analysis.edgeCount)
        assertEquals(2, analysis.eulerCharacteristic)
        assertTrue(analysis.isClosedManifold)
        assertEquals(100000.0, analysis.enclosedVolume, 1e-4)
    }

    @Test
    fun testSimulationContracts() {
        val simResult = SimulationResultInfo(
            success = true,
            nodeCount = 525,
            elementCount = 2000,
            maxVonMisesStressMPa = 58.4,
            minVonMisesStressMPa = 0.5,
            maxDisplacementMm = 0.198,
            minSafetyFactor = 4.28
        )
        assertTrue(simResult.success)
        assertEquals(525L, simResult.nodeCount)
        assertEquals(2000L, simResult.elementCount)
        assertEquals(58.4, simResult.maxVonMisesStressMPa, 1e-4)
        assertEquals(0.198, simResult.maxDisplacementMm, 1e-4)
        assertEquals(4.28, simResult.minSafetyFactor, 1e-4)

        val invalidSolid = BoxMeshResult(
            success = false,
            volume = 0.0,
            faceCount = 0,
            edgeCount = 0,
            triangleCount = 0,
            vertexBuffer = null,
            indexBuffer = null,
            edgeVertexBuffer = null,
            boundingBox = BoundingBox(),
            nativeHandle = 0L
        )
        val simResInvalid = NativeKernel.simulateSolid(invalidSolid)
        assertFalse(simResInvalid.success)
        assertEquals("Invalid solid", simResInvalid.errorMessage)
    }

    @Test
    fun testCAMGCodeContracts() {
        val camInfo = CAMGCodeInfo(
            success = true,
            lineCount = 142,
            cuttingTimeSec = 45.2,
            rapidTimeSec = 3.8,
            totalLengthMm = 650.0,
            materialVolumeRemovedCm3 = 35.0,
            minX = -5.0,
            maxX = 105.0,
            minY = -5.0,
            maxY = 55.0,
            minZ = -2.0,
            maxZ = 31.0,
            gcode = "G21\nG90\nG00 Z31.000\nM30\n"
        )
        assertTrue(camInfo.success)
        assertEquals(142, camInfo.lineCount)
        assertEquals(45.2, camInfo.cuttingTimeSec, 1e-4)
        assertEquals(3.8, camInfo.rapidTimeSec, 1e-4)
        assertEquals(650.0, camInfo.totalLengthMm, 1e-4)
        assertEquals(35.0, camInfo.materialVolumeRemovedCm3, 1e-4)
        assertTrue(camInfo.gcode.contains("G21"))
        assertTrue(camInfo.gcode.contains("M30"))
        assertEquals(CAMPostDialect.ISO_GRBL.id, 0)
        assertEquals(CAMPostDialect.HAAS_CNC.id, 1)
        assertEquals(CAMPostDialect.FANUC_CNC.id, 2)
    }

    @Test
    fun testPatternDataContracts() {
        val patternInfo = PatternResultInfo(
            success = true,
            instanceCount = 12,
            totalVolumeMm3 = 12000.0,
            meshResult = null,
            errorMessage = ""
        )
        assertTrue(patternInfo.success)
        assertEquals(12, patternInfo.instanceCount)
        assertEquals(12000.0, patternInfo.totalVolumeMm3, 1e-4)
        assertEquals("", patternInfo.errorMessage)

        val circularInfo = PatternResultInfo(
            success = true,
            instanceCount = 6,
            totalVolumeMm3 = 7068.58,
            meshResult = null
        )
        assertTrue(circularInfo.success)
        assertEquals(6, circularInfo.instanceCount)
        assertEquals(7068.58, circularInfo.totalVolumeMm3, 1e-2)

        val mirrorInfo = PatternResultInfo(
            success = true,
            instanceCount = 2,
            totalVolumeMm3 = 6000.0,
            meshResult = null
        )
        assertTrue(mirrorInfo.success)
        assertEquals(2, mirrorInfo.instanceCount)

        val draftInfo = PatternResultInfo(
            success = true,
            instanceCount = 1,
            totalVolumeMm3 = 11850.0,
            meshResult = null
        )
        assertTrue(draftInfo.success)
        assertEquals(1, draftInfo.instanceCount)
    }
}