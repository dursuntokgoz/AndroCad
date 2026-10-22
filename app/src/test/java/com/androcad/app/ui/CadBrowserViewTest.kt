// Copyright (c) 2026 dursuntokgoz. All rights reserved.
package com.androcad.app.ui

import androidx.compose.ui.test.*
import androidx.compose.ui.test.junit4.createComposeRule
import com.androcad.app.model.BrowserState
import com.androcad.app.model.CadBodyItem
import com.androcad.app.model.CadSketchItem
import org.junit.Assert.*
import org.junit.Rule
import org.junit.Test
import org.junit.runner.RunWith
import org.robolectric.RobolectricTestRunner
import org.robolectric.annotation.Config

@RunWith(RobolectricTestRunner::class)
@Config(sdk = [34])
class CadBrowserViewTest {

    @get:Rule
    val composeTestRule = createComposeRule()

    @Test
    fun testBrowserPanelToggle() {
        val state = BrowserState()
        composeTestRule.setContent {
            CadBrowserView(
                browserState = state,
                isDarkTheme = true,
                onSelectEntity = {},
                onToggleVisibility = {},
                onRenameBody = { _, _ -> },
                onDeleteBody = {},
                onNamedViewClick = {}
            )
        }

        val header = composeTestRule.onNodeWithTag("browser_header")
        header.assertIsDisplayed()
        composeTestRule.onNodeWithTag("browser_section_bodies").assertIsDisplayed()

        // Collapse
        header.performClick()
        composeTestRule.onNodeWithTag("browser_section_bodies").assertDoesNotExist()

        // Expand again
        header.performClick()
        composeTestRule.onNodeWithTag("browser_section_bodies").assertIsDisplayed()
    }

    @Test
    fun testBrowserNodeExpansion() {
        val state = BrowserState()
        composeTestRule.setContent {
            CadBrowserView(
                browserState = state,
                isDarkTheme = true,
                onSelectEntity = {},
                onToggleVisibility = {},
                onRenameBody = { _, _ -> },
                onDeleteBody = {},
                onNamedViewClick = {}
            )
        }

        // Document settings
        val docSettings = composeTestRule.onNodeWithTag("browser_section_doc_settings")
        docSettings.performScrollTo().assertIsDisplayed().performClick()
        composeTestRule.onNodeWithText("• Units: Millimeters (mm)").performScrollTo().assertIsDisplayed()

        // Origin planes
        val origin = composeTestRule.onNodeWithTag("browser_section_origin")
        origin.performScrollTo().assertIsDisplayed().performClick()
        composeTestRule.onNodeWithTag("browser_item_origin_xy").performScrollTo().assertIsDisplayed()
        composeTestRule.onNodeWithTag("browser_item_origin_xz").performScrollTo().assertIsDisplayed()
        composeTestRule.onNodeWithTag("browser_item_origin_yz").performScrollTo().assertIsDisplayed()

        // Construction
        val construction = composeTestRule.onNodeWithTag("browser_section_construction")
        construction.performScrollTo().assertIsDisplayed().performClick()
        composeTestRule.onNodeWithText("• Plane 1 (Offset 10mm)").performScrollTo().assertIsDisplayed()
    }

    @Test
    fun testBrowserBodiesTreeAndSelection() {
        var selectedId: String? = null
        val state = BrowserState(
            bodies = mutableListOf(
                CadBodyItem("body_1", "Solid Box 1", isVisible = true, isSelected = false)
            )
        )

        composeTestRule.setContent {
            CadBrowserView(
                browserState = state,
                isDarkTheme = true,
                onSelectEntity = { id -> selectedId = id },
                onToggleVisibility = {},
                onRenameBody = { _, _ -> },
                onDeleteBody = {},
                onNamedViewClick = {}
            )
        }

        composeTestRule.onNodeWithTag("browser_section_bodies").assertIsDisplayed()
        val bodyItem = composeTestRule.onNodeWithTag("browser_item_body_1")
        bodyItem.assertIsDisplayed().performClick()
        assertEquals("body_1", selectedId)
    }

    @Test
    fun testBrowserVisibilityToggleAction() {
        var toggledId: String? = null
        val state = BrowserState(
            bodies = mutableListOf(
                CadBodyItem("body_1", "Solid Box 1", isVisible = true, isSelected = false)
            )
        )

        composeTestRule.setContent {
            CadBrowserView(
                browserState = state,
                isDarkTheme = true,
                onSelectEntity = {},
                onToggleVisibility = { id -> toggledId = id },
                onRenameBody = { _, _ -> },
                onDeleteBody = {},
                onNamedViewClick = {}
            )
        }

        val visBtn = composeTestRule.onNodeWithTag("browser_vis_body_1")
        visBtn.assertIsDisplayed().performClick()
        assertEquals("body_1", toggledId)
    }

    @Test
    fun testBrowserDeleteItemAction() {
        var deletedId: String? = null
        val state = BrowserState(
            bodies = mutableListOf(
                CadBodyItem("body_1", "Solid Box 1", isVisible = true, isSelected = false)
            )
        )

        composeTestRule.setContent {
            CadBrowserView(
                browserState = state,
                isDarkTheme = true,
                onSelectEntity = {},
                onToggleVisibility = {},
                onRenameBody = { _, _ -> },
                onDeleteBody = { id -> deletedId = id },
                onNamedViewClick = {}
            )
        }

        val deleteBtn = composeTestRule.onNodeWithTag("browser_delete_body_1")
        deleteBtn.assertIsDisplayed().performClick()
        assertEquals("body_1", deletedId)
    }
}
