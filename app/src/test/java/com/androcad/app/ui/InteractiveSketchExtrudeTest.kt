// Copyright (c) 2026 dursuntokgoz. All rights reserved.
package com.androcad.app.ui

import androidx.compose.ui.test.*
import androidx.compose.ui.test.junit4.createComposeRule
import com.androcad.app.AndroCadApp
import com.androcad.app.model.SketchClosedProfile
import com.androcad.app.model.SketchEntity
import com.androcad.app.model.SketchMath
import com.androcad.app.tutorial.TutorialManager
import com.androcad.kernel.NativeKernel
import org.junit.After
import org.junit.Assert.*
import org.junit.Before
import org.junit.Rule
import org.junit.Test
import org.junit.runner.RunWith
import org.robolectric.RobolectricTestRunner
import org.robolectric.annotation.Config

@RunWith(RobolectricTestRunner::class)
@Config(sdk = [34])
class InteractiveSketchExtrudeTest {

    @get:Rule
    val composeTestRule = createComposeRule()

    @Before
    fun setUp() {
        TutorialManager.disableForTesting(true)
    }

    @After
    fun tearDown() {
        TutorialManager.disableForTesting(false)
    }

    private fun setContentForTest() {
        val fallbackBox = NativeKernel.createFallbackBox(100.0, 50.0, 20.0)
        composeTestRule.setContent {
            AndroCadApp(meshResult = fallbackBox, renderer = null)
        }
    }

    @Test
    fun testSketchMathUnitCalculations() {
        // 1. Rectangle profile math
        val rect = SketchEntity.Rectangle("r1", 0f, 0f, 100f, 50f)
        val rectProfile = SketchMath.evaluateProfile(listOf(rect))
        assertNotNull(rectProfile)
        assertTrue(rectProfile!!.isClosed)
        assertEquals(5000.0, rectProfile.area, 0.001)

        // 2. Circle profile math
        val circ = SketchEntity.Circle("c1", 0f, 0f, 25f)
        val circProfile = SketchMath.evaluateProfile(listOf(circ))
        assertNotNull(circProfile)
        assertTrue(circProfile!!.isClosed)
        assertEquals(Math.PI * 25.0 * 25.0, circProfile.area, 0.1)

        // 3. Grid snapping math
        assertEquals(20f, SketchMath.snapToGrid(23.4f, 10f), 0.001f)
        assertEquals(30f, SketchMath.snapToGrid(27.8f, 10f), 0.001f)
        assertEquals(25f, SketchMath.snapToGrid(24.0f, 5f), 0.001f)

        // 4. Empty and open entities
        assertNull(SketchMath.evaluateProfile(emptyList()))
        val openLine = SketchEntity.Line("l1", 0f, 0f, 50f, 50f)
        val openProfile = SketchMath.evaluateProfile(listOf(openLine))
        assertNotNull(openProfile)
        assertFalse(openProfile!!.isClosed)
    }

    @Test
    fun testSketchOverlayDisplayedInSketchTab() {
        setContentForTest()

        // Switch to Sketch tab
        composeTestRule.onNodeWithTag("tab_Sketch").performClick()

        // Verify overlay and HUD buttons are visible
        composeTestRule.onNodeWithTag("sketch_canvas_overlay").assertIsDisplayed()
        composeTestRule.onNodeWithTag("sketch_status_badge").assertIsDisplayed()
        composeTestRule.onNodeWithTag("btn_sketch_snap").assertIsDisplayed()
        composeTestRule.onNodeWithTag("btn_sketch_preset").assertIsDisplayed()
        composeTestRule.onNodeWithTag("btn_sketch_finish_hud").assertIsDisplayed()
    }

    @Test
    fun testPresetSketchCreationAndProfileStatus() {
        setContentForTest()

        composeTestRule.onNodeWithTag("tab_Sketch").performClick()

        // Add 100x50 mm preset rectangle
        composeTestRule.onNodeWithTag("btn_sketch_preset").performClick()

        // Verify Undo & Clear buttons appear
        composeTestRule.onNodeWithTag("btn_sketch_undo").assertIsDisplayed()
        composeTestRule.onNodeWithTag("btn_sketch_clear").assertIsDisplayed()

        // Test Undo
        composeTestRule.onNodeWithTag("btn_sketch_undo").performClick()
        composeTestRule.onNodeWithTag("btn_sketch_undo").assertDoesNotExist()
    }

    @Test
    fun testFinishSketchOpensExtrudeInspector() {
        setContentForTest()

        composeTestRule.onNodeWithTag("tab_Sketch").performClick()
        composeTestRule.onNodeWithTag("btn_sketch_preset").performClick()

        // Finish sketch via HUD button
        composeTestRule.onNodeWithTag("btn_sketch_finish_hud").performClick()

        // Verify Extrude Inspector is shown in Solid tab
        composeTestRule.onNodeWithTag("extrude_inspector_card").assertIsDisplayed()
        composeTestRule.onNodeWithTag("text_extrude_depth").assertIsDisplayed()
        composeTestRule.onNodeWithTag("slider_extrude_depth").performScrollTo().assertIsDisplayed()
        composeTestRule.onNodeWithTag("btn_extrude_ok").performScrollTo().assertIsDisplayed()

        // Confirm extrusion
        composeTestRule.onNodeWithTag("btn_extrude_ok").performScrollTo().performClick()

        // Verify dialog dismissed
        composeTestRule.onNodeWithTag("extrude_inspector_card").assertDoesNotExist()
    }

    @Test
    fun testSolidTabToolExtrudeOpensInspector() {
        setContentForTest()

        composeTestRule.onNodeWithTag("tab_Solid").performClick()
        composeTestRule.onNodeWithTag("tool_Extrude").performClick()

        composeTestRule.onNodeWithTag("extrude_inspector_card").assertIsDisplayed()

        // Test stepper button
        composeTestRule.onNodeWithTag("btn_extrude_step_plus_10").performScrollTo().performClick()
        composeTestRule.onNodeWithTag("btn_extrude_step_minus_1").performScrollTo().performClick()

        // Test cancel
        composeTestRule.onNodeWithTag("btn_extrude_cancel").performScrollTo().performClick()
        composeTestRule.onNodeWithTag("extrude_inspector_card").assertDoesNotExist()
    }

    @Test
    fun testSolidTabBoxInspector() {
        setContentForTest()

        composeTestRule.onNodeWithTag("tab_Solid").performClick()
        composeTestRule.onNodeWithTag("tool_Box").performClick()

        composeTestRule.onNodeWithTag("box_inspector_card").assertIsDisplayed()
        composeTestRule.onNodeWithTag("btn_box_ok").performScrollTo().assertIsDisplayed().performClick()
        composeTestRule.onNodeWithTag("box_inspector_card").assertDoesNotExist()
    }

    @Test
    fun testSolidTabFilletChamferHoleInspectors() {
        setContentForTest()

        composeTestRule.onNodeWithTag("tab_Solid").performClick()

        // Fillet
        composeTestRule.onNodeWithTag("tool_Fillet").performScrollTo().performClick()
        composeTestRule.onNodeWithTag("fillet_inspector_card").assertIsDisplayed()
        composeTestRule.onNodeWithTag("btn_fillet_ok").performScrollTo().performClick()
        composeTestRule.onNodeWithTag("fillet_inspector_card").assertDoesNotExist()

        // Chamfer
        composeTestRule.onNodeWithTag("tool_Chamfer").performScrollTo().performClick()
        composeTestRule.onNodeWithTag("chamfer_inspector_card").assertIsDisplayed()
        composeTestRule.onNodeWithTag("btn_chamfer_ok").performScrollTo().performClick()
        composeTestRule.onNodeWithTag("chamfer_inspector_card").assertDoesNotExist()

        // Hole
        composeTestRule.onNodeWithTag("tool_Hole").performScrollTo().performClick()
        composeTestRule.onNodeWithTag("hole_inspector_card").assertIsDisplayed()
        composeTestRule.onNodeWithTag("btn_hole_ok").performScrollTo().performClick()
        composeTestRule.onNodeWithTag("hole_inspector_card").assertDoesNotExist()
    }

    @Test
    fun testChamferAndFilletCancelAndSliders() {
        setContentForTest()

        composeTestRule.onNodeWithTag("tab_Solid").performClick()

        // Chamfer with Cancel
        composeTestRule.onNodeWithTag("tool_Chamfer").performScrollTo().performClick()
        composeTestRule.onNodeWithTag("chamfer_inspector_card").assertIsDisplayed()
        composeTestRule.onNodeWithTag("slider_chamfer_distance").performScrollTo().assertIsDisplayed()
        composeTestRule.onNodeWithTag("btn_chamfer_cancel").performScrollTo().performClick()
        composeTestRule.onNodeWithTag("chamfer_inspector_card").assertDoesNotExist()

        // Fillet with Cancel
        composeTestRule.onNodeWithTag("tool_Fillet").performScrollTo().performClick()
        composeTestRule.onNodeWithTag("fillet_inspector_card").assertIsDisplayed()
        composeTestRule.onNodeWithTag("slider_fillet_radius").performScrollTo().assertIsDisplayed()
        composeTestRule.onNodeWithTag("btn_fillet_cancel").performScrollTo().performClick()
        composeTestRule.onNodeWithTag("fillet_inspector_card").assertDoesNotExist()
    }

    @Test
    fun testSolidTabCylinderInspector() {
        setContentForTest()

        composeTestRule.onNodeWithTag("tab_Solid").performClick()
        composeTestRule.onNodeWithTag("tool_Cylinder").performScrollTo().performClick()

        composeTestRule.onNodeWithTag("cylinder_inspector_card").assertIsDisplayed()
        composeTestRule.onNodeWithTag("slider_cylinder_radius").performScrollTo().assertIsDisplayed()
        composeTestRule.onNodeWithTag("slider_cylinder_height").performScrollTo().assertIsDisplayed()
        composeTestRule.onNodeWithTag("btn_cylinder_ok").performScrollTo().performClick()
        composeTestRule.onNodeWithTag("cylinder_inspector_card").assertDoesNotExist()
    }

    @Test
    fun testSolidTabCreateSketchButton() {
        setContentForTest()

        composeTestRule.onNodeWithTag("tab_Solid").performClick()
        composeTestRule.onNodeWithTag("btn_solid_create_sketch").performScrollTo().performClick()

        // Should switch to Sketch tab with overlay displayed
        composeTestRule.onNodeWithTag("sketch_canvas_overlay").assertIsDisplayed()
    }

    @Test
    fun testDirectExtrudeButtonOnClosedProfile() {
        setContentForTest()

        composeTestRule.onNodeWithTag("tab_Sketch").performClick()
        composeTestRule.onNodeWithTag("btn_sketch_preset").performClick()

        // Since preset creates a closed profile, direct extrude button should be visible
        composeTestRule.onNodeWithTag("btn_sketch_extrude_direct").assertIsDisplayed().performClick()

        // Should open Extrude Inspector
        composeTestRule.onNodeWithTag("extrude_inspector_card").assertIsDisplayed()
        composeTestRule.onNodeWithTag("btn_extrude_ok").performScrollTo().performClick()
        composeTestRule.onNodeWithTag("extrude_inspector_card").assertDoesNotExist()
    }

    @Test
    @Config(qualifiers = "w800dp-h1024dp")
    fun testBrowserDeleteBodyClearsMeshAndEmptiesPlateTablet() {
        setContentForTest()

        // Verify initial body exists in browser
        composeTestRule.onNodeWithTag("browser_item_body_1").assertIsDisplayed()

        // Delete body_1 from Browser
        composeTestRule.onNodeWithTag("browser_delete_body_1").performClick()

        // Verify body item is removed from tree
        composeTestRule.onNodeWithTag("browser_item_body_1").assertDoesNotExist()

        // When all bodies are deleted, EmptyStateView appears
        composeTestRule.onNodeWithTag("empty_state_view").assertIsDisplayed()
    }

    @Test
    @Config(qualifiers = "w360dp-h640dp")
    fun testBrowserDeleteBodyClearsMeshAndEmptiesPlatePhone() {
        setContentForTest()

        // Open tree bottom sheet
        composeTestRule.onNodeWithTag("phone_nav_tree").performClick()

        // Verify initial body exists in browser
        composeTestRule.onNodeWithTag("browser_item_body_1").assertIsDisplayed()

        // Delete body_1 from Browser
        composeTestRule.onNodeWithTag("browser_delete_body_1").performClick()

        // Verify body item is removed from tree
        composeTestRule.onNodeWithTag("browser_item_body_1").assertDoesNotExist()

        // EmptyStateView appears
        composeTestRule.onNodeWithTag("empty_state_view").assertIsDisplayed()
    }

    @Test
    @Config(qualifiers = "w800dp-h1024dp")
    fun testFilletDoesNotSpawnRandomBoxWhenPlateIsEmpty() {
        setContentForTest()

        // Delete initial body
        composeTestRule.onNodeWithTag("browser_delete_body_1").performClick()
        composeTestRule.onNodeWithTag("empty_state_view").assertIsDisplayed()

        // Click Fillet tool
        composeTestRule.onNodeWithTag("tab_Solid").performClick()
        composeTestRule.onNodeWithTag("tool_Fillet").performScrollTo().performClick()

        // Fillet inspector dialog should NOT open because there is no active solid
        composeTestRule.onNodeWithTag("fillet_inspector_card").assertDoesNotExist()
    }
}
