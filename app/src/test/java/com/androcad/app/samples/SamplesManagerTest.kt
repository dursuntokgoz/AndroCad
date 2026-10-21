// Copyright (c) 2026 dursuntokgoz. All rights reserved.
package com.androcad.app.samples

import androidx.test.core.app.ApplicationProvider
import org.junit.Assert.assertEquals
import org.junit.Assert.assertNotEquals
import org.junit.Assert.assertNotNull
import org.junit.Assert.assertTrue
import org.junit.Test
import org.junit.runner.RunWith
import org.robolectric.RobolectricTestRunner
import org.robolectric.annotation.Config
import java.io.File

@RunWith(RobolectricTestRunner::class)
@Config(sdk = [34])
class SamplesManagerTest {

    private val context = ApplicationProvider.getApplicationContext<android.content.Context>()

    @Test
    fun testAllFourSamplesAreRegistered() {
        assertEquals("Exactly 4 original samples should be registered", 4, SamplesManager.SAMPLES.size)
        val ids = SamplesManager.SAMPLES.map { it.id }.toSet()
        assertTrue("Contains L-Bracket", ids.contains("l_bracket"))
        assertTrue("Contains Flanged Bushing", ids.contains("flanged_bushing"))
        assertTrue("Contains Phone Stand", ids.contains("phone_stand"))
        assertTrue("Contains Perforated Plate", ids.contains("perforated_plate"))
    }

    @Test
    fun testSampleAssetsExistAndAreReadable() {
        for (sample in SamplesManager.SAMPLES) {
            context.assets.open(sample.assetFileName).use { input ->
                val content = input.bufferedReader().readText()
                assertTrue("Sample ${sample.id} asset should not be empty", content.length > 50)
                assertTrue("Sample ${sample.id} should have valid JSON schema", content.contains("\"schema\""))
                assertTrue("Sample ${sample.id} should have nodes array", content.contains("\"nodes\""))
            }

            context.assets.open(sample.previewAssetFileName).use { input ->
                val bytes = input.readBytes()
                assertTrue("Preview for ${sample.id} should be non-empty", bytes.isNotEmpty())
                assertTrue("Preview for ${sample.id} should be under 100 KB", bytes.size < 100 * 1024)
            }
        }
    }

    @Test
    fun testCopySampleToWorkingFileCreatesIndependentCopy() {
        for (sample in SamplesManager.SAMPLES) {
            val workingFile = SamplesManager.copySampleToWorkingFile(context, sample)
            assertTrue("Working file should exist on disk", workingFile.exists())
            assertTrue("Working file should be writable", workingFile.canWrite())
            assertTrue("Working file should be in app filesDir", workingFile.absolutePath.contains("files/samples"))

            // Read asset original bytes
            val originalAssetBytes = context.assets.open(sample.assetFileName).use { it.readBytes() }
            val copiedBytes = workingFile.readBytes()
            assertEquals("Copied content should match asset", String(originalAssetBytes), String(copiedBytes))

            // Mutate copied working file
            workingFile.writeText("{\"mutated\": true}")

            // Assert that the original asset is still pristine (Immutable read-only asset in APK)
            val pristineAssetBytes = context.assets.open(sample.assetFileName).use { it.readBytes() }
            assertEquals("Asset in APK must remain immutable", String(originalAssetBytes), String(pristineAssetBytes))
            assertNotEquals("Working file was mutated", String(pristineAssetBytes), workingFile.readText())

            // Clean up
            workingFile.delete()
        }
    }
}
