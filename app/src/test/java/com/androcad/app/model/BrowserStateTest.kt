// Copyright (c) 2026 dursuntokgoz. All rights reserved.
package com.androcad.app.model

import org.junit.Assert.*
import org.junit.Test

class BrowserStateTest {

    @Test
    fun testBrowserInitialSectionsAndExpansion() {
        val browser = BrowserState(
            bodies = mutableListOf(
                CadBodyItem("body_1", "Solid Box 1", isVisible = true, isSelected = false)
            ),
            sketches = mutableListOf(
                CadSketchItem("sketch_1", "Sketch 1", isVisible = true, isSelected = false)
            )
        )

        assertTrue("Browser panel should be expanded by default", browser.isExpanded)
        assertTrue("Bodies group should be expanded by default", browser.isBodiesExpanded)
        assertTrue("Sketches group should be expanded by default", browser.isSketchesExpanded)
        assertFalse("Construction group should be collapsed by default", browser.isConstructionExpanded)
        assertFalse("Origin group should be collapsed by default", browser.isOriginExpanded)

        // Toggle Construction
        browser.isConstructionExpanded = !browser.isConstructionExpanded
        assertTrue("Construction group should now be expanded", browser.isConstructionExpanded)

        // Toggle Main Panel
        browser.isExpanded = false
        assertFalse("Browser panel should be collapsed", browser.isExpanded)
    }

    @Test
    fun testBrowserItemSelection() {
        val body1 = CadBodyItem("body_1", "Solid Box 1", isVisible = true)
        val body2 = CadBodyItem("body_2", "Cylinder 1", isVisible = true)
        val browser = BrowserState(bodies = mutableListOf(body1, body2))

        assertNull(browser.selectedEntityId)

        // Select body 1
        browser.selectItem("body_1")
        assertEquals("body_1", browser.selectedEntityId)
        assertTrue(body1.isSelected)
        assertFalse(body2.isSelected)

        // Select body 2
        browser.selectItem("body_2")
        assertEquals("body_2", browser.selectedEntityId)
        assertFalse(body1.isSelected)
        assertTrue(body2.isSelected)
    }

    @Test
    fun testBrowserVisibilityToggle() {
        val body1 = CadBodyItem("body_1", "Solid Box 1", isVisible = true)
        val browser = BrowserState(bodies = mutableListOf(body1))

        assertTrue(body1.isVisible)
        browser.toggleVisibility("body_1")
        assertFalse("Body 1 should be hidden after toggle", body1.isVisible)

        browser.toggleVisibility("body_1")
        assertTrue("Body 1 should be visible after second toggle", body1.isVisible)
    }

    @Test
    fun testBrowserRenameAndDeletion() {
        val body1 = CadBodyItem("body_1", "Solid Box 1", isVisible = true)
        val browser = BrowserState(bodies = mutableListOf(body1))

        // Rename
        browser.renameBody("body_1", "Base Plate")
        assertEquals("Base Plate", body1.name)

        // Select then Delete
        browser.selectItem("body_1")
        assertEquals("body_1", browser.selectedEntityId)

        browser.deleteBody("body_1")
        assertEquals("Bodies list should be empty", 0, browser.bodies.size)
        assertNull("Selected entity should be cleared on deletion", browser.selectedEntityId)
    }

    @Test
    fun testOriginPlanesVisibility() {
        val browser = BrowserState()
        assertTrue(browser.originXyVisible)
        assertTrue(browser.originXzVisible)
        assertTrue(browser.originYzVisible)

        browser.originXyVisible = false
        assertFalse(browser.originXyVisible)
    }
}
