// Copyright (c) 2026 dursuntokgoz. All rights reserved.
package com.androcad.app.io

import android.content.Context
import android.net.Uri
import androidx.test.core.app.ApplicationProvider
import com.androcad.kernel.NativeKernel
import org.junit.Assert.*
import org.junit.Test
import org.junit.runner.RunWith
import org.robolectric.RobolectricTestRunner
import org.robolectric.annotation.Config
import java.io.File

@RunWith(RobolectricTestRunner::class)
@Config(sdk = [34])
class CadStorageManagerTest {

    @Test
    fun testExportAndImportStepViaUri() {
        val context = ApplicationProvider.getApplicationContext<Context>()
        val solid = NativeKernel.createFallbackBox(80.0, 40.0, 20.0)

        val targetFile = File(context.filesDir, "test_saf_model.step")
        val targetUri = Uri.fromFile(targetFile)

        // 1. Export STEP via SAF bridge
        val exportSuccess = CadStorageManager.exportStepToUri(context, solid, targetUri)
        assertTrue("STEP export to URI should succeed", exportSuccess)
        assertTrue("Exported file should exist and have bytes", targetFile.exists() && targetFile.length() > 0L)

        // 2. Import STEP via SAF bridge
        val imported = CadStorageManager.importStepFromUri(context, targetUri)
        assertTrue("STEP import from URI should succeed", imported.success)
        assertTrue("Imported volume should match original within tolerance", imported.volume > 0.0)

        // Cleanup
        targetFile.delete()
    }

    @Test
    fun testExportAndImportStlViaUri() {
        val context = ApplicationProvider.getApplicationContext<Context>()
        val solid = NativeKernel.createFallbackBox(60.0, 30.0, 15.0)

        val targetFile = File(context.filesDir, "test_saf_model.stl")
        val targetUri = Uri.fromFile(targetFile)

        // 1. Export STL via SAF bridge
        val exportSuccess = CadStorageManager.exportStlToUri(context, solid, targetUri)
        assertTrue("STL export to URI should succeed", exportSuccess)
        assertTrue("Exported STL file should exist", targetFile.exists() && targetFile.length() > 0L)

        // 2. Import STL via SAF bridge
        val imported = CadStorageManager.importStlFromUri(context, targetUri)
        assertTrue("STL import from URI should succeed", imported.success)
        assertTrue("Imported triangle count should be > 0", imported.triangleCount > 0)

        // Cleanup
        targetFile.delete()
    }

    @Test
    fun testSaveAndLoadProjectViaUri() {
        val context = ApplicationProvider.getApplicationContext<Context>()
        val timeline = NativeKernel.createTimeline()

        val targetFile = File(context.filesDir, "test_saf_project.androcad")
        val targetUri = Uri.fromFile(targetFile)

        // Save
        val saveSuccess = CadStorageManager.saveProjectToUri(context, timeline, targetUri)
        assertTrue("Project save to URI should succeed", saveSuccess)
        assertTrue("Project file should exist", targetFile.exists())

        // Load
        val loadSuccess = CadStorageManager.loadProjectFromUri(context, timeline, targetUri)
        assertTrue("Project load from URI should succeed", loadSuccess)

        // Cleanup
        targetFile.delete()
    }

    @Test
    fun testSafImportHandlesInvalidUriGracefully() {
        val context = ApplicationProvider.getApplicationContext<Context>()
        val invalidUri = Uri.parse("content://invalid.provider/missing.step")

        val result = CadStorageManager.importStepFromUri(context, invalidUri)
        assertFalse("Importing non-existent URI should fail gracefully without crash", result.success)
        assertNotNull(result.errorMessage)
    }
}
