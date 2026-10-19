// Copyright (c) 2026 dursuntokgoz. All rights reserved.
package com.androcad.kernel

import org.junit.Assert.assertEquals
import org.junit.Assert.assertNotNull
import org.junit.Test

class NativeKernelTest {

    @Test
    fun testBoxMeshResultDataContract() {
        // Validates data class contract and lifecycle
        val result = BoxMeshResult(
            success = true,
            volume = 1000.0,
            faceCount = 6,
            triangleCount = 12,
            vertexBuffer = null,
            indexBuffer = null,
            errorMessage = null,
            nativeHandle = 0L
        )

        assertEquals(true, result.success)
        assertEquals(1000.0, result.volume, 1e-4)
        assertEquals(6, result.faceCount)
        assertEquals(12, result.triangleCount)
        result.close() // Verify close does not throw on 0L handle
    }
}
