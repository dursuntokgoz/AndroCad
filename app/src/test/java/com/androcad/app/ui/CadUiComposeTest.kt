// Copyright (c) 2026 dursuntokgoz. All rights reserved.
package com.androcad.app.ui

import androidx.compose.ui.test.*
import androidx.compose.ui.test.junit4.createComposeRule
import com.androcad.app.AndroCadApp
import com.androcad.kernel.NativeKernel
import org.junit.Rule
import org.junit.Test
import org.junit.runner.RunWith
import org.robolectric.RobolectricTestRunner
import org.robolectric.annotation.Config

@RunWith(RobolectricTestRunner::class)
@Config(sdk = [34])
class CadUiComposeTest {

    @get:Rule
    val composeTestRule = createComposeRule()

    private fun setContentForTest() {
        val fallbackBox = NativeKernel.createFallbackBox(100.0, 50.0, 20.0)
        composeTestRule.setContent {
            AndroCadApp(meshResult = fallbackBox, renderer = null)
        }
    }

    @Test
    fun testTopBarControls() {
        setContentForTest()
        composeTestRule.onNodeWithTag("app_logo").assertIsDisplayed()
        composeTestRule.onNodeWithTag("tab_Solid").assertIsDisplayed()
        composeTestRule.onNodeWithTag("tab_Sketch").assertIsDisplayed()
        composeTestRule.onNodeWithTag("tab_FileIO").assertIsDisplayed()
    }

    @Test
    fun testThemeToggle() {
        setContentForTest()
        val toggleNode = composeTestRule.onNodeWithTag("btn_theme_toggle")
        toggleNode.assertIsDisplayed()
        toggleNode.performClick()
        toggleNode.assertIsDisplayed()
    }

    @Test
    fun testWorkspaceTabSwitching() {
        setContentForTest()
        // Switch to Sketch tab
        composeTestRule.onNodeWithTag("tab_Sketch").performClick()
        composeTestRule.onNodeWithTag("tool_Rectangle").assertIsDisplayed()
        composeTestRule.onNodeWithTag("btn_finish_sketch").assertIsDisplayed()

        // Switch to File / IO tab
        composeTestRule.onNodeWithTag("tab_FileIO").performClick()
        composeTestRule.onNodeWithTag("tool_ExportSTEP").assertIsDisplayed()
        composeTestRule.onNodeWithTag("tool_ImportSTEP").assertIsDisplayed()

        // Switch back to Solid tab
        composeTestRule.onNodeWithTag("tab_Solid").performClick()
        composeTestRule.onNodeWithTag("tool_Box").assertIsDisplayed()
    }

    @Test
    fun testSolidTools() {
        setContentForTest()
        composeTestRule.onNodeWithTag("tab_Solid").performClick()
        composeTestRule.onNodeWithTag("tool_Box").assertIsDisplayed().performClick()
        composeTestRule.onNodeWithTag("tool_Fillet").assertIsDisplayed()
        composeTestRule.onNodeWithTag("tool_Chamfer").assertIsDisplayed()
        composeTestRule.onNodeWithTag("tool_Hole").assertIsDisplayed()
    }

    @Test
    fun testSketchTools() {
        setContentForTest()
        composeTestRule.onNodeWithTag("tab_Sketch").performClick()
        composeTestRule.onNodeWithTag("tool_Rectangle").assertIsDisplayed().performClick()
        composeTestRule.onNodeWithTag("tool_Circle").assertIsDisplayed().performClick()
        composeTestRule.onNodeWithTag("btn_finish_sketch").assertIsDisplayed().performClick()
    }

    @Test
    fun testViewCubeNavigation() {
        setContentForTest()
        composeTestRule.onNodeWithTag("viewcube_TOP").assertIsDisplayed().performClick()
        composeTestRule.onNodeWithTag("viewcube_FRONT").assertIsDisplayed().performClick()
        composeTestRule.onNodeWithTag("viewcube_ISO").assertIsDisplayed().performClick()
    }

    @Test
    fun testNavigationControls() {
        setContentForTest()
        composeTestRule.onNodeWithTag("nav_orbit").assertIsDisplayed().performClick()
        composeTestRule.onNodeWithTag("nav_pan").assertIsDisplayed().performClick()
        composeTestRule.onNodeWithTag("nav_fit").assertIsDisplayed().performClick()
        composeTestRule.onNodeWithTag("nav_display_mode").assertIsDisplayed().performClick()
    }
}
