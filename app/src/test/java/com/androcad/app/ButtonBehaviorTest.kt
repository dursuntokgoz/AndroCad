// Copyright (c) 2026 dursuntokgoz. All rights reserved.
package com.androcad.app

import org.junit.Assert.assertNotEquals
import org.junit.Assert.assertTrue
import org.junit.Test

class ButtonBehaviorTest {

    private val allRegisteredTools = listOf(
        // Solid
        "Box", "Extrude", "Revolve", "Fillet", "Chamfer", "Hole", "Shell",
        "Combine", "Rect Pattern", "Circular Pattern", "Mirror", "Draft Angle",
        "ISO Hole (M6 CB)", "Threaded Bolt (M8)",
        // Sketch
        "Rectangle", "Line", "Circle", "B-Spline", "Project Edge", "Dimension", "Constraints",
        // Surface
        "Loft", "Press-Pull (+10)", "Press-Pull (-5)", "Thicken (5mm)",
        // Sheet Metal
        "U-Bracket", "Unfold Pattern", "Export DXF",
        // Drawing
        "4-View Sheet", "Section Cut", "BOM & Balloons", "GD&T Frame",
        "Detail View 2:1", "Auxiliary View", "Multi-Sheet Doc", "Export SVG",
        // Inspect
        "Mass (Steel)", "Mat: Aluminum", "Mat: Titanium", "Mat: Brass", "CoG & Inertia",
        // Mesh
        "Analyze Mesh", "Repair Mesh", "Smooth Mesh", "Mesh to Solid",
        // Simulation
        "Solve Beam FEA", "Tet10 Quadratic FEA", "Modal Analysis",
        "Mode 1 Vibration", "Mode 2 Vibration", "Mode 3 Vibration",
        "Solve Part FEA", "Fixed Support", "Apply Load", "Safety Factor", "Displacement Field",
        // CAM
        "Generate G-Code (GRBL)", "Haas G-Code", "Fanuc G-Code",
        "3D Adaptive Clearing", "3D Scallop Finishing", "Facing Pass",
        "2D Pocketing", "2D Contour", "Peck Drilling",
        // Additive
        "Slice Solid (0.2mm)", "Honeycomb Infill (30%)", "Grid Infill (20%)", "Export G-Code",
        // Assemble
        "Robot Arm", "Rotate (+15°)", "Reset Joint", "Check Collision",
        // File / IO
        "Save", "Open", "Export STEP", "Import STEP", "Export STL", "Import STL"
    )

    @Test
    fun testAllToolsHaveSpecificCadIcons() {
        for (tool in allRegisteredTools) {
            val iconRes = getToolIconRes(tool)
            assertTrue("Icon resource for '$tool' should be valid (> 0)", iconRes > 0)
            if (tool != "Box") {
                assertNotEquals(
                    "Tool '$tool' should have a dedicated CAD icon instead of falling back to default ic_box",
                    R.drawable.ic_box,
                    iconRes
                )
            }
        }
    }
}
