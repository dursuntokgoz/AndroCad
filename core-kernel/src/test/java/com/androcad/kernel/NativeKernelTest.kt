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
}
