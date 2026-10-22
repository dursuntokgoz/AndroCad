// Copyright (c) 2026 dursuntokgoz. All rights reserved.
package com.androcad.app.ui

import androidx.compose.ui.test.assertIsDisplayed
import androidx.compose.ui.test.junit4.createComposeRule
import androidx.compose.ui.test.onNodeWithTag
import androidx.compose.ui.test.performClick
import androidx.compose.ui.test.performScrollTo
import com.androcad.app.samples.SampleDesign
import org.junit.Assert.assertEquals
import org.junit.Assert.assertNotNull
import org.junit.Assert.assertTrue
import org.junit.Rule
import org.junit.Test
import org.junit.runner.RunWith
import org.robolectric.RobolectricTestRunner
import org.robolectric.annotation.Config

@RunWith(RobolectricTestRunner::class)
@Config(sdk = [34])
class SamplesGalleryDialogTest {

    @get:Rule
    val composeTestRule = createComposeRule()

    @Test
    fun testSamplesGalleryPresentation() {
        var dismissed = false
        var selectedSample: SampleDesign? = null

        composeTestRule.setContent {
            SamplesGalleryDialog(
                isDarkTheme = true,
                onSelectSample = { selectedSample = it },
                onDismiss = { dismissed = true }
            )
        }

        // Dialog container and close button
        composeTestRule.onNodeWithTag("samples_gallery_dialog").assertIsDisplayed()
        composeTestRule.onNodeWithTag("samples_btn_close").assertIsDisplayed().performClick()
        assertTrue("Dismiss callback should be invoked", dismissed)
    }

    @Test
    fun testOpenSampleLBracket() {
        var selectedSample: SampleDesign? = null
        composeTestRule.setContent {
            SamplesGalleryDialog(
                isDarkTheme = true,
                onSelectSample = { selectedSample = it },
                onDismiss = {}
            )
        }

        composeTestRule.onNodeWithTag("sample_open_l_bracket").assertIsDisplayed().performClick()
        assertNotNull(selectedSample)
        assertEquals("l_bracket", selectedSample?.id)
    }

    @Test
    fun testOpenSampleFlangedBushing() {
        var selectedSample: SampleDesign? = null
        composeTestRule.setContent {
            SamplesGalleryDialog(
                isDarkTheme = true,
                onSelectSample = { selectedSample = it },
                onDismiss = {}
            )
        }

        composeTestRule.onNodeWithTag("sample_open_flanged_bushing").assertIsDisplayed().performClick()
        assertNotNull(selectedSample)
        assertEquals("flanged_bushing", selectedSample?.id)
    }

    @Test
    fun testOpenSamplePhoneStand() {
        var selectedSample: SampleDesign? = null
        composeTestRule.setContent {
            SamplesGalleryDialog(
                isDarkTheme = true,
                onSelectSample = { selectedSample = it },
                onDismiss = {}
            )
        }

        composeTestRule.onNodeWithTag("sample_open_phone_stand").performScrollTo().assertIsDisplayed().performClick()
        assertNotNull(selectedSample)
        assertEquals("phone_stand", selectedSample?.id)
    }

    @Test
    fun testOpenSamplePerforatedPlate() {
        var selectedSample: SampleDesign? = null
        composeTestRule.setContent {
            SamplesGalleryDialog(
                isDarkTheme = true,
                onSelectSample = { selectedSample = it },
                onDismiss = {}
            )
        }

        composeTestRule.onNodeWithTag("sample_open_perforated_plate").performScrollTo().assertIsDisplayed().performClick()
        assertNotNull(selectedSample)
        assertEquals("perforated_plate", selectedSample?.id)
    }
}
