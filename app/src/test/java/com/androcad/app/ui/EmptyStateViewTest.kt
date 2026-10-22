// Copyright (c) 2026 dursuntokgoz. All rights reserved.
package com.androcad.app.ui

import androidx.compose.ui.test.*
import androidx.compose.ui.test.junit4.createComposeRule
import org.junit.Assert.assertTrue
import org.junit.Rule
import org.junit.Test
import org.junit.runner.RunWith
import org.robolectric.RobolectricTestRunner
import org.robolectric.annotation.Config

@RunWith(RobolectricTestRunner::class)
@Config(sdk = [34])
class EmptyStateViewTest {

    @get:Rule
    val composeTestRule = createComposeRule()

    @Test
    fun testEmptyStatePresentationAndActions() {
        var tutorialClicked = false
        var samplesClicked = false
        var importClicked = false
        var newPartClicked = false

        composeTestRule.setContent {
            EmptyStateView(
                isDarkTheme = true,
                onStartTutorial = { tutorialClicked = true },
                onOpenSamples = { samplesClicked = true },
                onImportModel = { importClicked = true },
                onNewPart = { newPartClicked = true }
            )
        }

        // Screen is displayed
        composeTestRule.onNodeWithTag("empty_state_screen").assertIsDisplayed()

        // 1. Tutorial action
        composeTestRule.onNodeWithTag("empty_state_btn_tutorial")
            .assertIsDisplayed()
            .performClick()
        assertTrue("Tutorial callback should be invoked", tutorialClicked)

        // 2. Samples action
        composeTestRule.onNodeWithTag("empty_state_btn_samples")
            .assertIsDisplayed()
            .performClick()
        assertTrue("Samples callback should be invoked", samplesClicked)

        // 3. Import action
        composeTestRule.onNodeWithTag("empty_state_btn_import")
            .assertIsDisplayed()
            .performClick()
        assertTrue("Import callback should be invoked", importClicked)

        // 4. New Part action
        composeTestRule.onNodeWithTag("empty_state_btn_new_part")
            .performScrollTo()
            .assertIsDisplayed()
            .performClick()
        assertTrue("New Part callback should be invoked", newPartClicked)
    }
}
