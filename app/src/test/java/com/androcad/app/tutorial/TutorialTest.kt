// Copyright (c) 2026 dursuntokgoz. All rights reserved.
package com.androcad.app.tutorial

import androidx.compose.ui.test.assertIsDisplayed
import androidx.compose.ui.test.junit4.createComposeRule
import androidx.compose.ui.test.onNodeWithTag
import androidx.compose.ui.test.onNodeWithText
import androidx.compose.ui.test.performClick
import androidx.compose.ui.test.performScrollTo
import androidx.test.core.app.ApplicationProvider
import com.androcad.app.AndroCadApp
import com.androcad.kernel.NativeKernel
import kotlinx.coroutines.runBlocking
import org.junit.After
import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Before
import org.junit.Rule
import org.junit.Test
import org.junit.runner.RunWith
import org.robolectric.RobolectricTestRunner
import org.robolectric.annotation.Config

@RunWith(RobolectricTestRunner::class)
@Config(sdk = [34])
class TutorialTest {

    @get:Rule
    val composeTestRule = createComposeRule()

    private val context = ApplicationProvider.getApplicationContext<android.content.Context>()

    @Before
    fun setUp() {
        TutorialManager.disableForTesting(false)
        runBlocking {
            TutorialManager.resetTutorial(context)
        }
    }

    @After
    fun tearDown() {
        TutorialManager.disableForTesting(false)
    }

    @Test
    fun testTutorialManagerPersistence() {
        runBlocking {
            TutorialManager.resetTutorial(context)
            assertFalse(TutorialManager.isCompleted(context))

            TutorialManager.setCompleted(context, true)
            assertTrue(TutorialManager.isCompleted(context))

            TutorialManager.resetTutorial(context)
            assertFalse(TutorialManager.isCompleted(context))
        }
    }

    @Test
    fun testTutorialInitialVisibilityOnFirstRun() {
        val fallbackBox = NativeKernel.createFallbackBox(100.0, 50.0, 20.0)
        composeTestRule.setContent {
            AndroCadApp(meshResult = fallbackBox, renderer = null)
        }

        // On first run, tutorial overlay should be visible immediately
        composeTestRule.onNodeWithTag("tutorial_overlay").assertIsDisplayed()
        composeTestRule.onNodeWithTag("tutorial_step_indicator", useUnmergedTree = true).assertIsDisplayed()
        composeTestRule.onNodeWithTag("tutorial_btn_skip").assertIsDisplayed()
        composeTestRule.onNodeWithTag("tutorial_btn_next").assertIsDisplayed()
    }

    @Test
    fun testTutorialStepNavigationAndCompletion() {
        var completed = false
        var skipped = false

        composeTestRule.setContent {
            TutorialOverlayView(
                stepIndex = 0,
                totalSteps = TutorialManager.STEPS.size,
                step = TutorialManager.STEPS[0],
                isDarkTheme = true,
                onNext = {},
                onPrev = {},
                onSkip = { skipped = true },
                onFinish = { completed = true }
            )
        }

        composeTestRule.onNodeWithTag("tutorial_overlay").assertIsDisplayed()
        composeTestRule.onNodeWithTag("tutorial_step_indicator", useUnmergedTree = true).assertIsDisplayed()
        composeTestRule.onNodeWithTag("tutorial_btn_skip").assertIsDisplayed()
        composeTestRule.onNodeWithTag("tutorial_btn_next").assertIsDisplayed()

        // Test last step shows finish button
        composeTestRule.setContent {
            val lastIndex = TutorialManager.STEPS.size - 1
            TutorialOverlayView(
                stepIndex = lastIndex,
                totalSteps = TutorialManager.STEPS.size,
                step = TutorialManager.STEPS[lastIndex],
                isDarkTheme = true,
                onNext = {},
                onPrev = {},
                onSkip = { skipped = true },
                onFinish = { completed = true }
            )
        }

        composeTestRule.onNodeWithTag("tutorial_btn_finish").assertIsDisplayed()
        composeTestRule.onNodeWithTag("tutorial_btn_finish").performClick()
        assertTrue("Finish button callback should have been triggered", completed)
    }

    @Test
    fun testTutorialSkipAction() {
        var skipped = false
        composeTestRule.setContent {
            TutorialOverlayView(
                stepIndex = 1,
                totalSteps = TutorialManager.STEPS.size,
                step = TutorialManager.STEPS[1],
                isDarkTheme = true,
                onNext = {},
                onPrev = {},
                onSkip = { skipped = true },
                onFinish = {}
            )
        }

        composeTestRule.onNodeWithTag("tutorial_btn_skip").performClick()
        assertTrue("Skip callback should have been triggered", skipped)
    }

    @Test
    fun testRestartTutorialFromMenu() {
        runBlocking {
            // Mark tutorial as already completed
            TutorialManager.setCompleted(context, true)
        }

        val fallbackBox = NativeKernel.createFallbackBox(100.0, 50.0, 20.0)
        composeTestRule.setContent {
            AndroCadApp(meshResult = fallbackBox, renderer = null)
        }

        // Overlay should not be shown because it's completed
        composeTestRule.onNodeWithTag("tutorial_overlay").assertDoesNotExist()

        // Tap restart tutorial button in top bar (perform scroll to ensure it's in view)
        composeTestRule.onNodeWithTag("btn_restart_tutorial").performScrollTo().assertIsDisplayed()
        composeTestRule.onNodeWithTag("btn_restart_tutorial").performClick()

        // Tutorial overlay should now be visible again
        composeTestRule.onNodeWithTag("tutorial_overlay").assertIsDisplayed()
        composeTestRule.onNodeWithTag("tutorial_step_indicator", useUnmergedTree = true).assertIsDisplayed()
    }
}
