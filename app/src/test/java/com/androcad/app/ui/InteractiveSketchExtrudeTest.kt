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

        // 5. Out-of-order closed polygon loop detection
        val l1 = SketchEntity.Line("l1", 0f, 0f, 40f, 0f)
        val l3 = SketchEntity.Line("l3", 40f, 30f, 0f, 30f)
        val l2 = SketchEntity.Line("l2", 40f, 0f, 40f, 30f)
        val l4 = SketchEntity.Line("l4", 0f, 30f, 0f, 0f)
        val loopProfile = SketchMath.evaluateProfile(listOf(l1, l3, l2, l4))
        assertNotNull(loopProfile)
        assertTrue(loopProfile!!.isClosed)
        assertEquals(1200.0, loopProfile.area, 1.0)

        // 6. Regular Polygon (Hexagon) profile math
        val hex = SketchEntity.Polygon("p1", 0f, 0f, 30f, 6)
        val hexProfile = SketchMath.evaluateProfile(listOf(hex))
        assertNotNull(hexProfile)
        assertTrue(hexProfile!!.isClosed)
        val expectedHexArea = 0.5 * 6 * 30.0 * 30.0 * Math.sin(2.0 * Math.PI / 6.0)
        assertEquals(expectedHexArea, hexProfile.area, 1.0)

        // 7. Slot profile math
        val slot = SketchEntity.Slot("s1", -20f, 0f, 20f, 0f, 10f)
        val slotProfile = SketchMath.evaluateProfile(listOf(slot))
        assertNotNull(slotProfile)
        assertTrue(slotProfile!!.isClosed)
        val expectedSlotArea = (40.0 * 20.0) + (Math.PI * 10.0 * 10.0)
        assertEquals(expectedSlotArea, slotProfile.area, 1.0)

        // 8. Offset entity math
        val offsetRect = SketchMath.offsetLastEntity(listOf(rect), 5f)
        assertTrue(offsetRect is SketchEntity.Rectangle)
        val rOff = offsetRect as SketchEntity.Rectangle
        assertEquals(110f, rOff.width, 0.01f)
        assertEquals(60f, rOff.height, 0.01f)
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
        composeTestRule.onNodeWithTag("empty_state_screen").assertIsDisplayed()
    }

    @Test
    @Config(qualifiers = "w360dp-h640dp")
    fun testBrowserDeleteBodyClearsMeshAndEmptiesPlatePhone() {
        setContentForTest()

        // Open tree bottom sheet
        composeTestRule.onNodeWithTag("phone_nav_tree").performClick()

        // Verify initial body exists in browser
        composeTestRule.onNodeWithTag("browser_item_body_1").performScrollTo().assertIsDisplayed()

        // Delete body_1 from Browser
        composeTestRule.onNodeWithTag("browser_delete_body_1").performScrollTo().performClick()

        // Verify body item is removed from tree
        composeTestRule.onNodeWithTag("browser_item_body_1").assertDoesNotExist()

        // EmptyStateView appears
        composeTestRule.onNodeWithTag("empty_state_screen").assertIsDisplayed()
    }

    @Test
    @Config(qualifiers = "w800dp-h1024dp")
    fun testFilletDoesNotSpawnRandomBoxWhenPlateIsEmpty() {
        setContentForTest()

        // Delete initial body
        composeTestRule.onNodeWithTag("browser_delete_body_1").performClick()
        composeTestRule.onNodeWithTag("empty_state_screen").assertIsDisplayed()

        // Click Fillet tool
        composeTestRule.onNodeWithTag("tab_Solid").performClick()
        composeTestRule.onNodeWithTag("tool_Fillet").performScrollTo().performClick()

        // Fillet inspector dialog should NOT open because there is no active solid
        composeTestRule.onNodeWithTag("fillet_inspector_card").assertDoesNotExist()
    }

    @Test
    fun testSolidTabSphereRevolveShellMoveRotateInspectors() {
        setContentForTest()

        composeTestRule.onNodeWithTag("tab_Solid").performClick()

        // Sphere
        composeTestRule.onNodeWithTag("tool_Sphere").performScrollTo().performClick()
        composeTestRule.onNodeWithTag("sphere_inspector_card").assertIsDisplayed()
        composeTestRule.onNodeWithTag("btn_sphere_ok").performScrollTo().performClick()
        composeTestRule.onNodeWithTag("sphere_inspector_card").assertDoesNotExist()

        // Revolve
        composeTestRule.onNodeWithTag("tool_Revolve").performScrollTo().performClick()
        composeTestRule.onNodeWithTag("revolve_inspector_card").assertIsDisplayed()
        composeTestRule.onNodeWithTag("btn_revolve_ok").performScrollTo().performClick()
        composeTestRule.onNodeWithTag("revolve_inspector_card").assertDoesNotExist()

        // Shell
        composeTestRule.onNodeWithTag("tool_Shell").performScrollTo().performClick()
        composeTestRule.onNodeWithTag("shell_inspector_card").assertIsDisplayed()
        composeTestRule.onNodeWithTag("btn_shell_ok").performScrollTo().performClick()
        composeTestRule.onNodeWithTag("shell_inspector_card").assertDoesNotExist()

        // Move / Rotate
        composeTestRule.onNodeWithTag("tool_MoveRotate").performScrollTo().performClick()
        composeTestRule.onNodeWithTag("transform_inspector_card").assertIsDisplayed()
        composeTestRule.onNodeWithTag("btn_transform_ok").performScrollTo().performClick()
        composeTestRule.onNodeWithTag("transform_inspector_card").assertDoesNotExist()
    }

    @Test
    fun testSmartDimensionEditorAndUndoRedoToolbar() {
        setContentForTest()

        // Verify Undo and Redo buttons are displayed in top toolbar
        composeTestRule.onNodeWithTag("btn_undo").assertIsDisplayed()
        composeTestRule.onNodeWithTag("btn_redo").assertIsDisplayed()

        // Switch to Sketch tab, create preset rectangle, and activate Dimension tool
        composeTestRule.onNodeWithTag("tab_Sketch").performClick()
        composeTestRule.onNodeWithTag("btn_sketch_preset").performClick()
        composeTestRule.onNodeWithTag("tool_Dimension").performScrollTo().performClick()

        // Smart Dimension Editor bar should appear
        composeTestRule.onNodeWithTag("sketch_dimension_bar").assertIsDisplayed()
        composeTestRule.onNodeWithTag("btn_dim_w_plus").performClick()
        composeTestRule.onNodeWithTag("btn_dim_h_plus").performClick()
    }
}
