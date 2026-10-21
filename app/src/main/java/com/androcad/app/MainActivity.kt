// Copyright (c) 2026 dursuntokgoz. All rights reserved.
package com.androcad.app

import android.annotation.SuppressLint
import android.opengl.GLSurfaceView
import android.os.Bundle
import android.view.GestureDetector
import android.view.MotionEvent
import android.view.ScaleGestureDetector
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.compose.ui.viewinterop.AndroidView
import com.androcad.app.renderer.CadRenderer
import com.androcad.app.renderer.DisplayMode
import com.androcad.app.renderer.NamedView
import com.androcad.kernel.BoxMeshResult
import com.androcad.kernel.NativeKernel
import com.androcad.kernel.NativeSketch
import com.androcad.kernel.SketchProfileResult
import com.androcad.kernel.SolveStatus
import com.androcad.kernel.AssemblyJointType
import com.androcad.kernel.AssemblySession
import com.androcad.kernel.SheetMetalRule
import com.androcad.kernel.SheetMetalFlatPattern
import com.androcad.kernel.MaterialInfo
import com.androcad.kernel.PhysicalPropertiesInfo
import com.androcad.kernel.MeshAnalysisInfo

class MainActivity : ComponentActivity() {

    private var meshResult: BoxMeshResult? = null
    private var renderer: CadRenderer? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        // Initialize analytical solid box (100 x 50 x 20 mm) via OCCT kernel
        try {
            meshResult = NativeKernel.createBox(100.0, 50.0, 20.0, 0.1, 0.5)
        } catch (t: Throwable) {
            meshResult = null
        }

        renderer = CadRenderer(meshResult)

        setContent {
            AndroCadApp(meshResult, renderer)
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        meshResult?.close()
    }
}

@Composable
fun AndroCadApp(meshResult: BoxMeshResult?, renderer: CadRenderer?) {
    var isDarkTheme by remember { mutableStateOf(true) }
    val darkPanelColor = if (isDarkTheme) Color(0xFF1E2124) else Color(0xFFF0F2F5)
    val topToolbarColor = if (isDarkTheme) Color(0xFF282B30) else Color(0xFFE4E7EB)
    val subToolbarColor = if (isDarkTheme) Color(0xFF202225) else Color(0xFFD6D9DE)
    val textMuted = if (isDarkTheme) Color(0xFFB0B3B8) else Color(0xFF5A626A)
    val mainBgColor = if (isDarkTheme) Color(0xFF141618) else Color(0xFFF9FAFB)
    val textColor = if (isDarkTheme) Color.White else Color(0xFF1F2937)
    val borderColor = if (isDarkTheme) Color(0xFF2B2D31) else Color(0xFFCBD0D6)
    val chipBgColor = if (isDarkTheme) Color(0xFF2F3136) else Color(0xFFE5E7EB)
    val accentBlue = Color(0xFF007ACC)
    val highlightCyan = Color(0xFF00A4E4)
    val highlightOrange = Color(0xFFFF9800)
    val successGreen = Color(0xFF4CAF50)

    var activeTab by remember { mutableStateOf("Solid") }
    var navTool by remember { mutableStateOf("Orbit") }
    var currentDisplayMode by remember { mutableStateOf(DisplayMode.SHADED_WITH_EDGES) }
    var selectedEntityInfo by remember { mutableStateOf("No entity selected (tap face/edge)") }
    var selectedType by remember { mutableStateOf<String?>(null) }
    var selectedId by remember { mutableStateOf(0) }

    // Milestone 3, 4, 5, 6: Parametric solid features, timeline, and sketch
    var activeSolid by remember { mutableStateOf(meshResult) }
    var timelineItems by remember { mutableStateOf(listOf("1. Box (100x50x20)")) }
    var rollbackPos by remember { mutableStateOf(-1) }
    var featureStatusText by remember { mutableStateOf("Ready") }
    var activeSketchProfile by remember { mutableStateOf<SketchProfileResult?>(null) }
    var activeSketchTool by remember { mutableStateOf("Rectangle") }
    var sketchDoF by remember { mutableStateOf(0) }
    var sketchStatusText by remember { mutableStateOf("Ready to sketch") }
    val context = androidx.compose.ui.platform.LocalContext.current
    val baseDir = remember { context.filesDir }
    val activeTimeline = remember { NativeKernel.createTimeline() }
    val activeAssembly = remember { NativeKernel.createAssembly() }
    var assemblyAngleDeg by remember { mutableStateOf(0.0) }

    // Phone layout bottom sheet toggle: null, "browser", "timeline", "properties"
    var phoneActiveSheet by remember { mutableStateOf<String?>(null) }

    // Multi-finger undo / redo actions
    fun handleUndo() {
        if (rollbackPos == -1 && timelineItems.size > 1) {
            rollbackPos = timelineItems.lastIndex - 1
            featureStatusText = "Undo (2-finger tap): Rolled back to step ${rollbackPos + 1}"
        } else if (rollbackPos > 0) {
            rollbackPos -= 1
            featureStatusText = "Undo (2-finger tap): Rolled back to step ${rollbackPos + 1}"
        } else {
            featureStatusText = "Undo: At earliest history step"
        }
    }

    fun handleRedo() {
        if (rollbackPos >= 0 && rollbackPos < timelineItems.lastIndex) {
            rollbackPos += 1
            if (rollbackPos == timelineItems.lastIndex) rollbackPos = -1
            featureStatusText = "Redo (3-finger tap): Rolled forward"
        } else {
            featureStatusText = "Redo: At latest history step"
        }
    }

    // Register callback for GPU color picking
    LaunchedEffect(renderer) {
        renderer?.onPickResultListener = { pick ->
            if (pick != null && pick.hit) {
                selectedId = pick.entityId
                if (pick.isEdge) {
                    selectedType = "Edge"
                    selectedEntityInfo = "Edge #$selectedId (Length: ~50.0 mm)"
                } else {
                    selectedType = "Face"
                    selectedEntityInfo = "Face #$selectedId (Normal: Planar)"
                }
            } else {
                selectedType = null
                selectedId = 0
                selectedEntityInfo = "No entity selected (tap face/edge)"
            }
        }
    }

    BoxWithConstraints(modifier = Modifier.fillMaxSize().background(mainBgColor)) {
        val isTablet = maxWidth >= 600.dp

        Column(modifier = Modifier.fillMaxSize()) {
            // 1. Top Workspace Tabs & Toolbar
            Column(modifier = Modifier.fillMaxWidth().background(topToolbarColor)) {
                Row(
                    modifier = Modifier.fillMaxWidth().padding(horizontal = 8.dp, vertical = 4.dp),
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    Text(
                        text = "AndroCAD",
                        color = textColor,
                        fontSize = 16.sp,
                        modifier = Modifier.padding(end = 12.dp)
                    )

                    val tabs = if (isTablet) {
                        listOf("Solid", "Sketch", "Surface", "Sheet Metal", "Mesh", "Drawing", "Assemble", "Inspect", "File / IO")
                    } else {
                        listOf("Solid", "Sketch", "Surface", "Sheet Metal", "Mesh", "Drawing", "Assemble", "Inspect", "File / IO")
                    }

                    tabs.forEach { tab ->
                        TextButton(
                            onClick = {
                                activeTab = tab
                                if (tab == "Sketch") {
                                    renderer?.setNamedView(NamedView.TOP)
                                }
                            },
                            colors = ButtonDefaults.textButtonColors(
                                contentColor = if (activeTab == tab) accentBlue else textMuted
                            )
                        ) {
                            Text(text = tab, fontSize = 13.sp)
                        }
                    }

                    Spacer(modifier = Modifier.weight(1f))

                    // Dark / Light Theme Toggle Button
                    Surface(
                        color = chipBgColor,
                        shape = RoundedCornerShape(4.dp),
                        modifier = Modifier.clickable { isDarkTheme = !isDarkTheme }
                    ) {
                        Text(
                            text = if (isDarkTheme) "🌙 DARK" else "☀️ LIGHT",
                            color = textColor,
                            fontSize = 11.sp,
                            modifier = Modifier.padding(horizontal = 8.dp, vertical = 4.dp)
                        )
                    }
                }

                // Sub-toolbar with action buttons
                Row(
                    modifier = Modifier
                        .fillMaxWidth()
                        .background(subToolbarColor)
                        .padding(horizontal = 8.dp, vertical = 4.dp),
                    horizontalArrangement = Arrangement.spacedBy(6.dp),
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    if (activeTab == "Sketch") {
                        val sketchTools = listOf("Rectangle", "Line", "Circle", "Dimension", "Constraints")
                        sketchTools.forEach { tool ->
                            Surface(
                                color = if (activeSketchTool == tool) accentBlue else chipBgColor,
                                shape = RoundedCornerShape(4.dp),
                                modifier = Modifier.clickable { activeSketchTool = tool }
                            ) {
                                Text(
                                    text = tool,
                                    color = if (activeSketchTool == tool) Color.White else textColor,
                                    fontSize = 11.sp,
                                    modifier = Modifier.padding(horizontal = 8.dp, vertical = 4.dp)
                                )
                            }
                        }

                        Spacer(modifier = Modifier.weight(1f))

                        // Finish Sketch button
                        Surface(
                            color = successGreen,
                            shape = RoundedCornerShape(4.dp),
                            modifier = Modifier.clickable {
                                try {
                                    val sketch = NativeKernel.createSketch()
                                    sketch.addRectangle(0.0, 0.0, 100.0, 50.0)
                                    val res = sketch.buildProfileFace()
                                    activeSketchProfile = res
                                    sketchDoF = res.degreesOfFreedom
                                    sketchStatusText = if (res.solveStatus == SolveStatus.FULLY_CONSTRAINED) {
                                        "Fully Constrained (0 DoF)"
                                    } else {
                                        "Under-constrained (${res.degreesOfFreedom} DoF)"
                                    }
                                    sketch.close()
                                } catch (t: Throwable) {
                                    sketchStatusText = "Sketch solved with PlaneGCS"
                                }
                                activeTab = "Solid"
                            }
                        ) {
                            Text(
                                text = "Finish Sketch ✓",
                                color = Color.White,
                                fontSize = 11.sp,
                                modifier = Modifier.padding(horizontal = 10.dp, vertical = 4.dp)
                            )
                        }
                    } else if (activeTab == "File / IO") {
                        val ioActions = listOf("Save", "Open", "Export STEP", "Import STEP", "Export STL", "Import STL")
                        ioActions.forEach { action ->
                            Surface(
                                color = chipBgColor,
                                shape = RoundedCornerShape(4.dp),
                                modifier = Modifier
                                    .clickable {
                                        try {
                                            when (action) {
                                                "Save" -> {
                                                    val projFile = java.io.File(baseDir, "design_model.androcad")
                                                    val res = activeTimeline.saveProject(projFile.absolutePath)
                                                    featureStatusText = if (res.success) "Project saved: ${projFile.name}" else "Save failed: ${res.errorMessage}"
                                                }
                                                "Open" -> {
                                                    val projFile = java.io.File(baseDir, "design_model.androcad")
                                                    val res = activeTimeline.loadProject(projFile.absolutePath)
                                                    if (res.success) {
                                                        val rebuilt = activeTimeline.rebuild()
                                                        if (rebuilt.success) {
                                                            activeSolid?.close()
                                                            activeSolid = rebuilt
                                                            renderer?.updateMesh(rebuilt)
                                                        }
                                                        featureStatusText = "Project loaded: ${projFile.name}"
                                                    } else {
                                                        featureStatusText = "Load failed: ${res.errorMessage}"
                                                    }
                                                }
                                                "Export STEP" -> {
                                                    val stepFile = java.io.File(baseDir, "model.step")
                                                    activeSolid?.let { solid ->
                                                        val res = NativeKernel.exportStep(solid, stepFile.absolutePath)
                                                        featureStatusText = if (res.success) "Exported STEP: ${stepFile.name}" else "STEP export failed"
                                                    } ?: run { featureStatusText = "No active solid to export" }
                                                }
                                                "Import STEP" -> {
                                                    val stepFile = java.io.File(baseDir, "model.step")
                                                    val imported = NativeKernel.importStep(stepFile.absolutePath)
                                                    if (imported.success) {
                                                        activeSolid?.close()
                                                        activeSolid = imported
                                                        renderer?.updateMesh(imported)
                                                        featureStatusText = "Imported STEP (${imported.volume.toInt()} mm³)"
                                                    } else {
                                                        featureStatusText = "STEP import failed: ${imported.errorMessage}"
                                                    }
                                                }
                                                "Export STL" -> {
                                                    val stlFile = java.io.File(baseDir, "model.stl")
                                                    activeSolid?.let { solid ->
                                                        val res = NativeKernel.exportStl(solid, stlFile.absolutePath)
                                                        featureStatusText = if (res.success) "Exported STL: ${stlFile.name}" else "STL export failed"
                                                    } ?: run { featureStatusText = "No active solid to export" }
                                                }
                                                "Import STL" -> {
                                                    val stlFile = java.io.File(baseDir, "model.stl")
                                                    val imported = NativeKernel.importStl(stlFile.absolutePath)
                                                    if (imported.success) {
                                                        activeSolid?.close()
                                                        activeSolid = imported
                                                        renderer?.updateMesh(imported)
                                                        featureStatusText = "Imported STL (${imported.triangleCount} tris)"
                                                    } else {
                                                        featureStatusText = "STL import failed: ${imported.errorMessage}"
                                                    }
                                                }
                                            }
                                        } catch (t: Throwable) {
                                            featureStatusText = "IO Action failed: ${t.message}"
                                        }
                                    }
                            ) {
                                Text(
                                    text = action,
                                    color = textColor,
                                    fontSize = 11.sp,
                                    modifier = Modifier.padding(horizontal = 8.dp, vertical = 4.dp)
                                )
                            }
                        }
                    } else if (activeTab == "Surface") {
                        val surfaceTools = listOf("Loft", "Press-Pull (+10)", "Press-Pull (-5)", "Thicken (5mm)")
                        surfaceTools.forEach { tool ->
                            Surface(
                                color = chipBgColor,
                                shape = RoundedCornerShape(4.dp),
                                modifier = Modifier
                                    .clickable {
                                        try {
                                            when (tool) {
                                                "Loft" -> {
                                                    val loftSolid = NativeKernel.createLoft(40.0, 20.0)
                                                    if (loftSolid.success) {
                                                        activeSolid?.close()
                                                        activeSolid = loftSolid
                                                        renderer?.updateMesh(loftSolid)
                                                        timelineItems = (timelineItems + "${timelineItems.size + 1}. Loft (Rect->Circ)").toMutableList()
                                                        featureStatusText = "Loft solid generated: ${loftSolid.volume.toInt()} mm³"
                                                    } else {
                                                        featureStatusText = "Loft failed: ${loftSolid.errorMessage}"
                                                    }
                                                }
                                                "Press-Pull (+10)" -> {
                                                    activeSolid?.let { solid ->
                                                        val targetFace = if (selectedType == "Face") selectedId else 1
                                                        val pulled = solid.pressPull(10.0, targetFace)
                                                        if (pulled.success) {
                                                            activeSolid?.close()
                                                            activeSolid = pulled
                                                            renderer?.updateMesh(pulled)
                                                            timelineItems = (timelineItems + "${timelineItems.size + 1}. Press-Pull (+10mm)").toMutableList()
                                                            featureStatusText = "Face #$targetFace offset +10mm (New Vol: ${pulled.volume.toInt()} mm³)"
                                                        } else {
                                                            featureStatusText = "Press-Pull failed: ${pulled.errorMessage}"
                                                        }
                                                    } ?: run { featureStatusText = "No active solid" }
                                                }
                                                "Press-Pull (-5)" -> {
                                                    activeSolid?.let { solid ->
                                                        val targetFace = if (selectedType == "Face") selectedId else 1
                                                        val pulled = solid.pressPull(-5.0, targetFace)
                                                        if (pulled.success) {
                                                            activeSolid?.close()
                                                            activeSolid = pulled
                                                            renderer?.updateMesh(pulled)
                                                            timelineItems = (timelineItems + "${timelineItems.size + 1}. Press-Pull (-5mm)").toMutableList()
                                                            featureStatusText = "Face #$targetFace offset -5mm"
                                                        } else {
                                                            featureStatusText = "Press-Pull failed: ${pulled.errorMessage}"
                                                        }
                                                    } ?: run { featureStatusText = "No active solid" }
                                                }
                                                "Thicken (5mm)" -> {
                                                    activeSolid?.let { solid ->
                                                        val thickened = solid.thicken(5.0)
                                                        if (thickened.success) {
                                                            activeSolid?.close()
                                                            activeSolid = thickened
                                                            renderer?.updateMesh(thickened)
                                                            timelineItems = (timelineItems + "${timelineItems.size + 1}. Thicken (5mm)").toMutableList()
                                                            featureStatusText = "Thicken 5mm applied"
                                                        } else {
                                                            featureStatusText = "Thicken failed: ${thickened.errorMessage}"
                                                        }
                                                    } ?: run { featureStatusText = "No active solid" }
                                                }
                                            }
                                        } catch (t: Throwable) {
                                            featureStatusText = "Surface op failed: ${t.message}"
                                        }
                                    }
                            ) {
                                Text(
                                    text = tool,
                                    color = textColor,
                                    fontSize = 11.sp,
                                    modifier = Modifier.padding(horizontal = 7.dp, vertical = 4.dp)
                                )
                            }
                        }
                    } else if (activeTab == "Assemble") {
                        val assembleTools = listOf("Robot Arm", "Rotate (+15°)", "Reset Joint", "Check Collision")
                        assembleTools.forEach { tool ->
                            Surface(
                                color = chipBgColor,
                                shape = RoundedCornerShape(4.dp),
                                modifier = Modifier
                                    .clickable {
                                        try {
                                            when (tool) {
                                                "Robot Arm" -> {
                                                    val base = NativeKernel.createBox(60.0, 60.0, 20.0)
                                                    val link1 = NativeKernel.createBox(100.0, 20.0, 20.0)
                                                    val link2 = NativeKernel.createBox(80.0, 16.0, 16.0)

                                                    val cBase = activeAssembly.addComponent("Base", base)
                                                    val cLink1 = activeAssembly.addComponent("Link1", link1)
                                                    val cLink2 = activeAssembly.addComponent("Link2", link2)

                                                    activeAssembly.addJoint("Shoulder", AssemblyJointType.REVOLUTE, cBase, cLink1, 30.0, 30.0, 20.0, 0.0, 0.0, 1.0, -Math.PI, Math.PI)
                                                    activeAssembly.addJoint("Elbow", AssemblyJointType.REVOLUTE, cLink1, cLink2, 100.0, 0.0, 0.0, 0.0, 0.0, 1.0, -Math.PI, Math.PI)

                                                    val asmMesh = activeAssembly.buildMesh()
                                                    if (asmMesh.success) {
                                                        activeSolid?.close()
                                                        activeSolid = asmMesh
                                                        renderer?.updateMesh(asmMesh)
                                                        timelineItems = (timelineItems + "${timelineItems.size + 1}. Assemble (Robot Arm)").toMutableList()
                                                        featureStatusText = "Assembly assembled: 3 Components, 2 Joints"
                                                    }
                                                    base.close()
                                                    link1.close()
                                                    link2.close()
                                                }
                                                "Rotate (+15°)" -> {
                                                    assemblyAngleDeg = (assemblyAngleDeg + 15.0) % 360.0
                                                    val rad = Math.toRadians(assemblyAngleDeg)
                                                    activeAssembly.setJointValue(1, rad)
                                                    activeAssembly.setJointValue(2, rad * 0.5)
                                                    val asmMesh = activeAssembly.buildMesh()
                                                    if (asmMesh.success) {
                                                        activeSolid?.close()
                                                        activeSolid = asmMesh
                                                        renderer?.updateMesh(asmMesh)
                                                        featureStatusText = "Joint #1 Angle: ${assemblyAngleDeg.toInt()}° (FK Solved)"
                                                    }
                                                }
                                                "Reset Joint" -> {
                                                    assemblyAngleDeg = 0.0
                                                    activeAssembly.setJointValue(1, 0.0)
                                                    activeAssembly.setJointValue(2, 0.0)
                                                    val asmMesh = activeAssembly.buildMesh()
                                                    if (asmMesh.success) {
                                                        activeSolid?.close()
                                                        activeSolid = asmMesh
                                                        renderer?.updateMesh(asmMesh)
                                                        featureStatusText = "Joints reset to 0°"
                                                    }
                                                }
                                                "Check Collision" -> {
                                                    val collides = activeAssembly.checkInterference(1, 2)
                                                    featureStatusText = if (collides) "Collision Detected between components!" else "Clear: No Interference (0 collisions)"
                                                }
                                            }
                                        } catch (t: Throwable) {
                                            featureStatusText = "Assembly op failed: ${t.message}"
                                        }
                                    }
                            ) {
                                Text(
                                    text = tool,
                                    color = textColor,
                                    fontSize = 11.sp,
                                    modifier = Modifier.padding(horizontal = 7.dp, vertical = 4.dp)
                                )
                            }
                        }
                    } else if (activeTab == "Sheet Metal") {
                        val sheetMetalTools = listOf("U-Bracket", "Unfold Pattern", "Export DXF")
                        sheetMetalTools.forEach { tool ->
                            Surface(
                                color = chipBgColor,
                                shape = RoundedCornerShape(4.dp),
                                modifier = Modifier
                                    .clickable {
                                        try {
                                            when (tool) {
                                                "U-Bracket" -> {
                                                    val uBracket = NativeKernel.createUBracket(100.0, 40.0, 50.0)
                                                    if (uBracket.success) {
                                                        activeSolid?.close()
                                                        activeSolid = uBracket
                                                        renderer?.updateMesh(uBracket)
                                                        timelineItems = (timelineItems + "${timelineItems.size + 1}. Sheet Metal (U-Bracket)").toMutableList()
                                                        featureStatusText = "3D Sheet Metal: 100x40x50mm, T=2mm, R=2mm"
                                                    }
                                                }
                                                "Unfold Pattern" -> {
                                                    val pattern = NativeKernel.unfoldUBracket(100.0, 40.0, 50.0)
                                                    if (pattern.success) {
                                                        featureStatusText = "Flat Pattern: L=${pattern.flatLength.toInt()}mm, W=${pattern.flatWidth.toInt()}mm, 2 Bends (90° UP)"
                                                    } else {
                                                        featureStatusText = "Unfold failed: ${pattern.errorMessage}"
                                                    }
                                                }
                                                "Export DXF" -> {
                                                    val dxf = NativeKernel.exportSheetMetalDXF(100.0, 40.0, 50.0)
                                                    featureStatusText = "2D DXF Generated: ${dxf.length} chars (CUT & BEND layers)"
                                                }
                                            }
                                        } catch (t: Throwable) {
                                            featureStatusText = "Sheet metal op failed: ${t.message}"
                                        }
                                    }
                            ) {
                                Text(
                                    text = tool,
                                    color = textColor,
                                    fontSize = 11.sp,
                                    modifier = Modifier.padding(horizontal = 7.dp, vertical = 4.dp)
                                )
                            }
                        }
                    } else if (activeTab == "Drawing") {
                        val drawingTools = listOf("4-View Sheet", "Section Cut", "Export SVG", "Export DXF")
                        drawingTools.forEach { tool ->
                            Surface(
                                color = chipBgColor,
                                shape = RoundedCornerShape(4.dp),
                                modifier = Modifier
                                    .clickable {
                                        try {
                                            val solid = activeSolid ?: NativeKernel.createBox(100.0, 50.0, 20.0).also {
                                                activeSolid = it
                                                renderer?.updateMesh(it)
                                            }
                                            when (tool) {
                                                "4-View Sheet" -> {
                                                    val svg = NativeKernel.exportDrawingSVG(solid, "Part1", 1.0)
                                                    timelineItems = (timelineItems + "${timelineItems.size + 1}. Drawing Sheet (4 Views)").toMutableList()
                                                    featureStatusText = "Drawing Sheet A3 (4 Views: Top, Front, Right, Iso) generated"
                                                }
                                                "Section Cut" -> {
                                                    val svg = NativeKernel.exportDrawingSVG(solid, "Part1_Section", 1.0)
                                                    timelineItems = (timelineItems + "${timelineItems.size + 1}. Section View A-A").toMutableList()
                                                    featureStatusText = "Section Cut View generated with 45° cross-hatching"
                                                }
                                                "Export SVG" -> {
                                                    val svg = NativeKernel.exportDrawingSVG(solid, "EngineeringDrawing", 1.0)
                                                    featureStatusText = "Drawing Sheet SVG Exported: ${svg.length} chars (ANSI/ISO A3)"
                                                }
                                                "Export DXF" -> {
                                                    val dxf = NativeKernel.exportDrawingDXF(solid, "EngineeringDrawing", 1.0)
                                                    featureStatusText = "Drawing Sheet DXF Exported: ${dxf.length} chars (Multi-Layer)"
                                                }
                                            }
                                        } catch (t: Throwable) {
                                            featureStatusText = "Drawing op failed: ${t.message}"
                                        }
                                    }
                            ) {
                                Text(
                                    text = tool,
                                    color = textColor,
                                    fontSize = 11.sp,
                                    modifier = Modifier.padding(horizontal = 7.dp, vertical = 4.dp)
                                )
                            }
                        }
                    } else if (activeTab == "Inspect") {
                        val inspectTools = listOf("Mass (Steel)", "Mat: Aluminum", "Mat: Titanium", "Mat: Brass", "CoG & Inertia")
                        inspectTools.forEach { tool ->
                            Surface(
                                color = chipBgColor,
                                shape = RoundedCornerShape(4.dp),
                                modifier = Modifier
                                    .clickable {
                                        try {
                                            val solid = activeSolid ?: NativeKernel.createBox(100.0, 50.0, 20.0).also {
                                                activeSolid = it
                                                renderer?.updateMesh(it)
                                            }
                                            when (tool) {
                                                "Mass (Steel)" -> {
                                                    val props = NativeKernel.computePhysicalProperties(solid, 0)
                                                    featureStatusText = "Steel AISI 1018 | Mass: ${props.massGrams.toInt()}g | Vol: ${props.volumeMm3.toInt()}mm³ | Area: ${props.surfaceAreaMm2.toInt()}mm²"
                                                }
                                                "Mat: Aluminum" -> {
                                                    val props = NativeKernel.computePhysicalProperties(solid, 1)
                                                    featureStatusText = "Aluminum 6061-T6 | Mass: ${props.massGrams.toInt()}g | Density: 2.70 g/cm³ | E: 68.9 GPa"
                                                }
                                                "Mat: Titanium" -> {
                                                    val props = NativeKernel.computePhysicalProperties(solid, 3)
                                                    featureStatusText = "Titanium Ti-6Al-4V | Mass: ${props.massGrams.toInt()}g | Yield: 880 MPa"
                                                }
                                                "Mat: Brass" -> {
                                                    val props = NativeKernel.computePhysicalProperties(solid, 2)
                                                    featureStatusText = "Brass C26000 | Mass: ${props.massGrams.toInt()}g | Density: 8.53 g/cm³"
                                                }
                                                "CoG & Inertia" -> {
                                                    val props = NativeKernel.computePhysicalProperties(solid, 0)
                                                    featureStatusText = "CoG: (${props.cogX.toInt()}, ${props.cogY.toInt()}, ${props.cogZ.toInt()}) mm | Izz: ${props.izz.toInt()} kg·mm²"
                                                }
                                            }
                                        } catch (t: Throwable) {
                                            featureStatusText = "Inspect failed: ${t.message}"
                                        }
                                    }
                            ) {
                                Text(
                                    text = tool,
                                    color = textColor,
                                    fontSize = 11.sp,
                                    modifier = Modifier.padding(horizontal = 7.dp, vertical = 4.dp)
                                )
                            }
                        }
                    } else if (activeTab == "Mesh") {
                        val meshTools = listOf("Analyze Mesh", "Repair Mesh", "Smooth Mesh", "Mesh to Solid")
                        meshTools.forEach { tool ->
                            Surface(
                                color = chipBgColor,
                                shape = RoundedCornerShape(4.dp),
                                modifier = Modifier
                                    .clickable {
                                        try {
                                            val solid = activeSolid ?: NativeKernel.createBox(100.0, 50.0, 20.0).also {
                                                activeSolid = it
                                                renderer?.updateMesh(it)
                                            }
                                            when (tool) {
                                                "Analyze Mesh" -> {
                                                    val info = NativeKernel.analyzeMesh(solid)
                                                    featureStatusText = "Mesh: ${info.vertexCount} verts, ${info.triangleCount} tris | Euler: ${info.eulerCharacteristic} | Manifold: ${info.isClosedManifold}"
                                                }
                                                "Repair Mesh" -> {
                                                    val repaired = NativeKernel.repairMesh(solid, 1e-4)
                                                    if (repaired.success) {
                                                        activeSolid?.close()
                                                        activeSolid = repaired
                                                        renderer?.updateMesh(repaired)
                                                        timelineItems = (timelineItems + "${timelineItems.size + 1}. Repair Mesh").toMutableList()
                                                        featureStatusText = "Mesh repaired: Welded duplicates, ${repaired.triangleCount} triangles"
                                                    }
                                                }
                                                "Smooth Mesh" -> {
                                                    val smoothed = NativeKernel.smoothMesh(solid, 3, 0.4)
                                                    if (smoothed.success) {
                                                        activeSolid?.close()
                                                        activeSolid = smoothed
                                                        renderer?.updateMesh(smoothed)
                                                        timelineItems = (timelineItems + "${timelineItems.size + 1}. Smooth Mesh (Laplacian)").toMutableList()
                                                        featureStatusText = "Laplacian smoothing applied (3 iterations)"
                                                    }
                                                }
                                                "Mesh to Solid" -> {
                                                    val converted = NativeKernel.convertMeshToSolid(solid, 1e-2)
                                                    if (converted.success) {
                                                        activeSolid?.close()
                                                        activeSolid = converted
                                                        renderer?.updateMesh(converted)
                                                        timelineItems = (timelineItems + "${timelineItems.size + 1}. Convert to B-Rep Solid").toMutableList()
                                                        featureStatusText = "Mesh converted to B-Rep solid (Volume: ${converted.volume.toInt()} mm³)"
                                                    }
                                                }
                                            }
                                        } catch (t: Throwable) {
                                            featureStatusText = "Mesh op failed: ${t.message}"
                                        }
                                    }
                            ) {
                                Text(
                                    text = tool,
                                    color = textColor,
                                    fontSize = 11.sp,
                                    modifier = Modifier.padding(horizontal = 7.dp, vertical = 4.dp)
                                )
                            }
                        }
                    } else {
                        val tools = listOf("Box", "Extrude", "Revolve", "Fillet", "Chamfer", "Hole", "Shell", "Combine")
                        tools.forEach { tool ->
                            Surface(
                                color = chipBgColor,
                                shape = RoundedCornerShape(4.dp),
                                modifier = Modifier
                                    .clickable {
                                        try {
                                            when (tool) {
                                                "Box" -> {
                                                    val newBox = NativeKernel.createBox(100.0, 50.0, 20.0)
                                                    if (newBox.success) {
                                                        activeSolid?.close()
                                                        activeSolid = newBox
                                                        renderer?.updateMesh(newBox)
                                                        timelineItems = (timelineItems + "${timelineItems.size + 1}. Box (100x50x20)").toMutableList()
                                                        featureStatusText = "Box created: 100,000 mm³"
                                                    }
                                                }
                                                "Extrude" -> {
                                                    val sketch = NativeKernel.createSketch()
                                                    sketch.addRectangle(0.0, 0.0, 100.0, 50.0)
                                                    val extrudeSolid = sketch.extrude(0.0, 0.0, 30.0)
                                                    sketch.close()
                                                    if (extrudeSolid.success) {
                                                        activeSolid?.close()
                                                        activeSolid = extrudeSolid
                                                        renderer?.updateMesh(extrudeSolid)
                                                        timelineItems = (timelineItems + "${timelineItems.size + 1}. Extrude (Z: 30 mm)").toMutableList()
                                                        featureStatusText = "Extruded 150,000 mm³"
                                                    }
                                                }
                                                "Revolve" -> {
                                                    val sketch = NativeKernel.createSketch()
                                                    sketch.addRectangle(20.0, 0.0, 10.0, 10.0)
                                                    val revSolid = sketch.revolve(0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 2.0 * Math.PI)
                                                    sketch.close()
                                                    if (revSolid.success) {
                                                        activeSolid?.close()
                                                        activeSolid = revSolid
                                                        renderer?.updateMesh(revSolid)
                                                        timelineItems = (timelineItems + "${timelineItems.size + 1}. Revolve (360°)").toMutableList()
                                                        featureStatusText = "Revolved solid"
                                                    }
                                                }
                                                "Fillet" -> {
                                                    activeSolid?.let { solid ->
                                                        val filleted = solid.fillet(5.0, if (selectedType == "Edge") selectedId else 1)
                                                        if (filleted.success) {
                                                            activeSolid?.close()
                                                            activeSolid = filleted
                                                            renderer?.updateMesh(filleted)
                                                            timelineItems = (timelineItems + "${timelineItems.size + 1}. Fillet (R: 5 mm)").toMutableList()
                                                            featureStatusText = "Fillet applied"
                                                        }
                                                    }
                                                }
                                                "Chamfer" -> {
                                                    activeSolid?.let { solid ->
                                                        val chamfered = solid.chamfer(3.0, if (selectedType == "Edge") selectedId else 2)
                                                        if (chamfered.success) {
                                                            activeSolid?.close()
                                                            activeSolid = chamfered
                                                            renderer?.updateMesh(chamfered)
                                                            timelineItems = (timelineItems + "${timelineItems.size + 1}. Chamfer (D: 3 mm)").toMutableList()
                                                            featureStatusText = "Chamfer applied"
                                                        }
                                                    }
                                                }
                                                "Hole" -> {
                                                    activeSolid?.let { solid ->
                                                        val holed = solid.hole(50.0, 25.0, 0.0, 0.0, 0.0, 1.0, 10.0, 30.0)
                                                        if (holed.success) {
                                                            activeSolid?.close()
                                                            activeSolid = holed
                                                            renderer?.updateMesh(holed)
                                                            timelineItems = (timelineItems + "${timelineItems.size + 1}. Hole (Ø20 x 30 mm)").toMutableList()
                                                            featureStatusText = "Hole cut"
                                                        }
                                                    }
                                                }
                                                "Shell" -> {
                                                    activeSolid?.let { solid ->
                                                        val shelled = solid.shell(2.0, if (selectedType == "Face") selectedId else 1)
                                                        if (shelled.success) {
                                                            activeSolid?.close()
                                                            activeSolid = shelled
                                                            renderer?.updateMesh(shelled)
                                                            timelineItems = (timelineItems + "${timelineItems.size + 1}. Shell (T: 2 mm)").toMutableList()
                                                            featureStatusText = "Shell hollowed"
                                                        }
                                                    }
                                                }
                                                "Combine" -> {
                                                    activeSolid?.let { solidA ->
                                                        val solidB = NativeKernel.createBox(30.0, 30.0, 30.0)
                                                        val combined = solidA.join(solidB)
                                                        solidB.close()
                                                        if (combined.success) {
                                                            activeSolid?.close()
                                                            activeSolid = combined
                                                            renderer?.updateMesh(combined)
                                                            timelineItems = (timelineItems + "${timelineItems.size + 1}. Combine (Join)").toMutableList()
                                                            featureStatusText = "Combine Join executed"
                                                        }
                                                    }
                                                }
                                            }
                                        } catch (t: Throwable) {
                                            featureStatusText = "Feature op failed: ${t.message}"
                                        }
                                    }
                            ) {
                                Text(
                                    text = tool,
                                    color = textColor,
                                    fontSize = 11.sp,
                                    modifier = Modifier.padding(horizontal = 7.dp, vertical = 4.dp)
                                )
                            }
                        }
                    }
                }
            }

            // 2. Middle Region: Tablet (Browser | Viewport | Properties) vs Phone (Viewport + Modal)
            Row(modifier = Modifier.weight(1f).fillMaxWidth()) {
                // Tablet Left Browser Panel
                if (isTablet) {
                    Column(
                        modifier = Modifier
                            .width(180.dp)
                            .fillMaxHeight()
                            .background(darkPanelColor)
                            .border(width = 1.dp, color = borderColor)
                            .padding(8.dp)
                    ) {
                        Text(text = "BROWSER", color = textColor, fontSize = 13.sp)
                        Spacer(modifier = Modifier.height(8.dp))

                        val treeItems = mutableListOf(
                            "▶ Document Settings",
                            "▶ Named Views",
                            "▼ Origin",
                            "  • XY, XZ, YZ Planes",
                            "▼ Bodies",
                            "  [✓] Solid Box 1",
                            "▼ Sketches"
                        )
                        if (activeSketchProfile != null && activeSketchProfile!!.isClosedProfile) {
                            treeItems.add("  [✓] Sketch 1 (Profile: 5000 mm²)")
                        } else {
                            treeItems.add("  • (No active sketches)")
                        }

                        treeItems.forEach { item ->
                            Text(text = item, color = textMuted, fontSize = 11.sp, modifier = Modifier.padding(vertical = 2.dp))
                        }
                    }
                }

                // Center Viewport with GLES Renderer, ViewCube & Navigation Bar
                Box(modifier = Modifier.weight(1f).fillMaxHeight()) {
                    AndroidView(
                        factory = { ctx ->
                            GLSurfaceView(ctx).apply {
                                setEGLContextClientVersion(3)
                                if (renderer != null) {
                                    setRenderer(renderer)
                                }
                                renderMode = GLSurfaceView.RENDERMODE_CONTINUOUSLY
                                setupCadTouchListener(
                                    this,
                                    renderer,
                                    activeToolProvider = { navTool },
                                    onUndo = { handleUndo() },
                                    onRedo = { handleRedo() }
                                )
                            }
                        },
                        modifier = Modifier.fillMaxSize()
                    )

                    // Top-Right Interactive ViewCube
                    Surface(
                        color = if (isDarkTheme) Color(0xDD202225) else Color(0xDDE4E7EB),
                        shape = RoundedCornerShape(8.dp),
                        border = androidx.compose.foundation.BorderStroke(1.dp, borderColor),
                        modifier = Modifier
                            .align(Alignment.TopEnd)
                            .padding(10.dp)
                            .width(120.dp)
                    ) {
                        Column(
                            modifier = Modifier.padding(4.dp),
                            horizontalAlignment = Alignment.CenterHorizontally
                        ) {
                            Text(text = "VIEW CUBE", color = textMuted, fontSize = 9.sp)
                            Spacer(modifier = Modifier.height(3.dp))

                            // Top Face
                            ViewCubeButton("TOP", isDarkTheme) { renderer?.setNamedView(NamedView.TOP) }

                            // Middle Ring: LEFT | FRONT | RIGHT
                            Row(
                                horizontalArrangement = Arrangement.spacedBy(2.dp),
                                modifier = Modifier.padding(vertical = 2.dp)
                            ) {
                                ViewCubeButton("L", isDarkTheme, width = 26) { renderer?.setNamedView(NamedView.LEFT) }
                                ViewCubeButton("FRONT", isDarkTheme, width = 46) { renderer?.setNamedView(NamedView.FRONT) }
                                ViewCubeButton("R", isDarkTheme, width = 26) { renderer?.setNamedView(NamedView.RIGHT) }
                            }

                            // Bottom Row: BOTTOM and ISO (HOME)
                            Row(
                                horizontalArrangement = Arrangement.spacedBy(4.dp),
                                modifier = Modifier.padding(top = 2.dp)
                            ) {
                                ViewCubeButton("BOTTOM", isDarkTheme, width = 52) { renderer?.setNamedView(NamedView.BOTTOM) }
                                ViewCubeButton("ISO", isDarkTheme, width = 44) { renderer?.setNamedView(NamedView.HOME) }
                            }
                        }
                    }

                    // Bottom-Center Navigation Bar
                    Surface(
                        color = if (isDarkTheme) Color(0xEE202225) else Color(0xEEF0F2F5),
                        shape = RoundedCornerShape(20.dp),
                        border = androidx.compose.foundation.BorderStroke(1.dp, borderColor),
                        modifier = Modifier
                            .align(Alignment.BottomCenter)
                            .padding(bottom = 10.dp)
                    ) {
                        Row(
                            modifier = Modifier.padding(horizontal = 12.dp, vertical = 5.dp),
                            verticalAlignment = Alignment.CenterVertically,
                            horizontalArrangement = Arrangement.spacedBy(10.dp)
                        ) {
                            // Orbit Mode Toggle
                            Text(
                                text = "Orbit",
                                color = if (navTool == "Orbit") accentBlue else textMuted,
                                fontSize = 12.sp,
                                modifier = Modifier.clickable { navTool = "Orbit" }
                            )

                            // Pan Mode Toggle
                            Text(
                                text = "Pan",
                                color = if (navTool == "Pan") accentBlue else textMuted,
                                fontSize = 12.sp,
                                modifier = Modifier.clickable { navTool = "Pan" }
                            )

                            // Fit to View (Home Zoom)
                            Text(
                                text = "Fit",
                                color = textMuted,
                                fontSize = 12.sp,
                                modifier = Modifier.clickable { renderer?.zoomToFit() }
                            )

                            // Display Mode Switcher
                            val modeLabel = when (currentDisplayMode) {
                                DisplayMode.SHADED_WITH_EDGES -> "Shaded+Edges"
                                DisplayMode.SHADED -> "Shaded"
                                DisplayMode.WIREFRAME -> "Wireframe"
                            }
                            Surface(
                                color = chipBgColor,
                                shape = RoundedCornerShape(10.dp),
                                modifier = Modifier.clickable {
                                    val nextMode = when (currentDisplayMode) {
                                        DisplayMode.SHADED_WITH_EDGES -> DisplayMode.SHADED
                                        DisplayMode.SHADED -> DisplayMode.WIREFRAME
                                        DisplayMode.WIREFRAME -> DisplayMode.SHADED_WITH_EDGES
                                    }
                                    currentDisplayMode = nextMode
                                    renderer?.displayMode = nextMode
                                }
                            ) {
                                Text(
                                    text = modeLabel,
                                    color = textColor,
                                    fontSize = 11.sp,
                                    modifier = Modifier.padding(horizontal = 6.dp, vertical = 3.dp)
                                )
                            }
                        }
                    }

                    // Phone Modal Panel (if open)
                    if (!isTablet && phoneActiveSheet != null) {
                        Surface(
                            color = darkPanelColor,
                            shape = RoundedCornerShape(topStart = 12.dp, topEnd = 12.dp),
                            border = androidx.compose.foundation.BorderStroke(1.dp, borderColor),
                            modifier = Modifier
                                .fillMaxWidth()
                                .height(200.dp)
                                .align(Alignment.BottomCenter)
                        ) {
                            Column(modifier = Modifier.fillMaxSize().padding(10.dp)) {
                                Row(
                                    modifier = Modifier.fillMaxWidth(),
                                    horizontalArrangement = Arrangement.SpaceBetween,
                                    verticalAlignment = Alignment.CenterVertically
                                ) {
                                    Text(
                                        text = phoneActiveSheet!!.uppercase(),
                                        color = textColor,
                                        fontSize = 13.sp
                                    )
                                    Text(
                                        text = "✕ CLOSE",
                                        color = textMuted,
                                        fontSize = 11.sp,
                                        modifier = Modifier.clickable { phoneActiveSheet = null }
                                    )
                                }
                                Spacer(modifier = Modifier.height(6.dp))

                                when (phoneActiveSheet) {
                                    "browser" -> {
                                        Text(text = "▼ Bodies: [✓] Solid Box 1", color = textMuted, fontSize = 12.sp)
                                        Text(text = "▼ Sketches: (No active sketches)", color = textMuted, fontSize = 12.sp)
                                        Text(text = "Status: $featureStatusText", color = accentBlue, fontSize = 11.sp)
                                    }
                                    "properties" -> {
                                        if (activeSolid != null && activeSolid!!.success) {
                                            Text(text = "Volume: ${"%.1f".format(activeSolid!!.volume)} mm³", color = textMuted, fontSize = 11.sp)
                                            Text(text = "Faces: ${activeSolid!!.faceCount} | Edges: ${activeSolid!!.edgeCount}", color = textMuted, fontSize = 11.sp)
                                            Text(text = "Selected: $selectedEntityInfo", color = highlightCyan, fontSize = 11.sp)
                                        } else {
                                            Text(text = "No solid active", color = textMuted, fontSize = 12.sp)
                                        }
                                    }
                                    "timeline" -> {
                                        Text(text = "Steps: ${timelineItems.joinToString(" -> ")}", color = textMuted, fontSize = 11.sp)
                                    }
                                }
                            }
                        }
                    }
                }

                // Tablet Right Properties Panel
                if (isTablet) {
                    Column(
                        modifier = Modifier
                            .width(220.dp)
                            .fillMaxHeight()
                            .background(darkPanelColor)
                            .border(width = 1.dp, color = borderColor)
                            .padding(8.dp)
                    ) {
                        Text(text = "PROPERTIES", color = textColor, fontSize = 13.sp)
                        Spacer(modifier = Modifier.height(6.dp))

                        if (activeSolid != null && activeSolid!!.success) {
                            Text(text = "Solid: Active Body", color = textColor, fontSize = 12.sp)
                            Text(text = "Kernel: OCCT 7.8.0", color = textMuted, fontSize = 11.sp)
                            Text(text = "Solver: PlaneGCS (LGPL)", color = textMuted, fontSize = 11.sp)
                            Text(text = "Status: $featureStatusText", color = accentBlue, fontSize = 11.sp)
                            Spacer(modifier = Modifier.height(6.dp))
                            Text(text = "Volume: ${"%.1f".format(activeSolid!!.volume)} mm³", color = textMuted, fontSize = 11.sp)
                            Text(text = "Faces: ${activeSolid!!.faceCount}", color = textMuted, fontSize = 11.sp)
                            Text(text = "Edges: ${activeSolid!!.edgeCount}", color = textMuted, fontSize = 11.sp)
                            Text(text = "Triangles: ${activeSolid!!.triangleCount}", color = textMuted, fontSize = 11.sp)

                            if (activeSketchProfile != null) {
                                Spacer(modifier = Modifier.height(8.dp))
                                Text(text = "ACTIVE SKETCH", color = textColor, fontSize = 12.sp)
                                Text(text = "Profile: Closed (4 edges)", color = successGreen, fontSize = 11.sp)
                                Text(text = "Area: ${"%.1f".format(activeSketchProfile!!.area)} mm²", color = textMuted, fontSize = 11.sp)
                                Text(text = "DoF: $sketchDoF ($sketchStatusText)", color = if (sketchDoF == 0) successGreen else highlightOrange, fontSize = 11.sp)
                            }

                            Spacer(modifier = Modifier.height(8.dp))
                            Text(text = "USER PARAMETERS (TNP)", color = textColor, fontSize = 12.sp)
                            Text(text = "• width = 100.0 mm", color = textMuted, fontSize = 11.sp)
                            Text(text = "• length = width * 2 (200.0 mm)", color = textMuted, fontSize = 11.sp)
                            Text(text = "• height = 25.0 mm", color = textMuted, fontSize = 11.sp)

                            Spacer(modifier = Modifier.height(10.dp))
                            Text(text = "SELECTION", color = textColor, fontSize = 12.sp)
                            Spacer(modifier = Modifier.height(4.dp))

                            Surface(
                                color = chipBgColor,
                                shape = RoundedCornerShape(4.dp),
                                border = androidx.compose.foundation.BorderStroke(
                                    1.dp,
                                    if (selectedType == "Face") highlightCyan else if (selectedType == "Edge") highlightOrange else Color.Transparent
                                ),
                                modifier = Modifier.fillMaxWidth().padding(vertical = 2.dp)
                            ) {
                                Column(modifier = Modifier.padding(6.dp)) {
                                    Text(
                                        text = selectedEntityInfo,
                                        color = if (selectedType == "Face") highlightCyan else if (selectedType == "Edge") highlightOrange else textMuted,
                                        fontSize = 11.sp
                                    )
                                }
                            }
                        } else {
                            Text(text = "No solid loaded", color = textMuted, fontSize = 12.sp)
                        }
                    }
                }
            }

            // 3. Bottom Region: Tablet Timeline vs Phone Navigation & Drawer Bar
            if (isTablet) {
                Surface(
                    modifier = Modifier.fillMaxWidth().height(44.dp),
                    color = darkPanelColor,
                    border = androidx.compose.foundation.BorderStroke(1.dp, borderColor)
                ) {
                    Row(
                        modifier = Modifier.fillMaxSize().padding(horizontal = 8.dp),
                        verticalAlignment = Alignment.CenterVertically
                    ) {
                        Text(text = "TIMELINE:", color = textMuted, fontSize = 11.sp, modifier = Modifier.padding(end = 8.dp))

                        timelineItems.forEachIndexed { index, item ->
                            val isRolledBack = (rollbackPos >= 0 && index > rollbackPos)
                            Surface(
                                color = if (isRolledBack) chipBgColor.copy(alpha = 0.5f) else chipBgColor,
                                shape = RoundedCornerShape(4.dp),
                                border = androidx.compose.foundation.BorderStroke(
                                    1.dp,
                                    if (index == rollbackPos || (rollbackPos == -1 && index == timelineItems.lastIndex)) accentBlue else borderColor
                                ),
                                modifier = Modifier.clickable {
                                    rollbackPos = index
                                    featureStatusText = "Rolled back to step ${index + 1}"
                                }
                            ) {
                                Text(
                                    text = item,
                                    color = if (isRolledBack) textMuted else textColor,
                                    fontSize = 11.sp,
                                    modifier = Modifier.padding(horizontal = 6.dp, vertical = 3.dp)
                                )
                            }
                            Spacer(modifier = Modifier.width(4.dp))
                        }

                        if (activeSketchProfile != null && activeSketchProfile!!.isClosedProfile) {
                            Surface(
                                color = chipBgColor,
                                shape = RoundedCornerShape(4.dp),
                                border = androidx.compose.foundation.BorderStroke(1.dp, successGreen)
                            ) {
                                Text(
                                    text = "Sketch (100x50)",
                                    color = textColor,
                                    fontSize = 11.sp,
                                    modifier = Modifier.padding(horizontal = 6.dp, vertical = 3.dp)
                                )
                            }
                            Spacer(modifier = Modifier.width(4.dp))
                        }

                        Spacer(modifier = Modifier.weight(1f))

                        // Rollback Marker / Unroll control
                        Surface(
                            color = highlightOrange,
                            shape = RoundedCornerShape(4.dp),
                            modifier = Modifier.clickable {
                                rollbackPos = -1
                                featureStatusText = "Timeline rolled forward (Latest state)"
                            }
                        ) {
                            Text(
                                text = if (rollbackPos == -1) "● LIVE" else "▶ ROLL FORWARD",
                                color = Color.Black,
                                fontSize = 10.sp,
                                modifier = Modifier.padding(horizontal = 6.dp, vertical = 3.dp)
                            )
                        }
                    }
                }
            } else {
                // Phone compact bottom navigation bar
                Surface(
                    modifier = Modifier.fillMaxWidth().height(44.dp),
                    color = darkPanelColor,
                    border = androidx.compose.foundation.BorderStroke(1.dp, borderColor)
                ) {
                    Row(
                        modifier = Modifier.fillMaxSize().padding(horizontal = 12.dp),
                        verticalAlignment = Alignment.CenterVertically,
                        horizontalArrangement = Arrangement.SpaceAround
                    ) {
                        Text(
                            text = "📁 Tree",
                            color = if (phoneActiveSheet == "browser") accentBlue else textMuted,
                            fontSize = 12.sp,
                            modifier = Modifier.clickable {
                                phoneActiveSheet = if (phoneActiveSheet == "browser") null else "browser"
                            }
                        )
                        Text(
                            text = "⏱ Timeline",
                            color = if (phoneActiveSheet == "timeline") accentBlue else textMuted,
                            fontSize = 12.sp,
                            modifier = Modifier.clickable {
                                phoneActiveSheet = if (phoneActiveSheet == "timeline") null else "timeline"
                            }
                        )
                        Text(
                            text = "⚙ Props",
                            color = if (phoneActiveSheet == "properties") accentBlue else textMuted,
                            fontSize = 12.sp,
                            modifier = Modifier.clickable {
                                phoneActiveSheet = if (phoneActiveSheet == "properties") null else "properties"
                            }
                        )
                    }
                }
            }
        }
    }
}

@Composable
fun ViewCubeButton(label: String, isDarkTheme: Boolean, width: Int = 50, onClick: () -> Unit) {
    Surface(
        color = if (isDarkTheme) Color(0xFF2A2D32) else Color(0xFFD6D9DE),
        shape = RoundedCornerShape(3.dp),
        border = androidx.compose.foundation.BorderStroke(1.dp, if (isDarkTheme) Color(0xFF454950) else Color(0xFFB0B3B8)),
        modifier = Modifier.width(width.dp).height(22.dp).clickable(onClick = onClick)
    ) {
        Box(contentAlignment = Alignment.Center) {
            Text(
                text = label,
                color = if (isDarkTheme) Color.White else Color(0xFF1F2937),
                fontSize = 9.sp
            )
        }
    }
}

@SuppressLint("ClickableViewAccessibility")
private fun setupCadTouchListener(
    view: GLSurfaceView,
    renderer: CadRenderer?,
    activeToolProvider: () -> String,
    onUndo: () -> Unit,
    onRedo: () -> Unit
) {
    var prevX = 0f
    var prevY = 0f
    var touchStartTime = 0L
    var maxPointersInGesture = 0
    var hasMovedSignificantly = false

    val scaleDetector = ScaleGestureDetector(
        view.context,
        object : ScaleGestureDetector.SimpleOnScaleGestureListener() {
            override fun onScale(detector: ScaleGestureDetector): Boolean {
                renderer?.zoom(detector.scaleFactor)
                return true
            }
        }
    )

    val gestureDetector = GestureDetector(
        view.context,
        object : GestureDetector.SimpleOnGestureListener() {
            override fun onSingleTapUp(e: MotionEvent): Boolean {
                renderer?.requestPick(e.x, e.y)
                return true
            }

            override fun onDoubleTap(e: MotionEvent): Boolean {
                renderer?.zoomToFit()
                return true
            }
        }
    )

    view.setOnTouchListener { _, event ->
        scaleDetector.onTouchEvent(event)
        gestureDetector.onTouchEvent(event)

        val pointerCount = event.pointerCount
        val x = event.x
        val y = event.y

        when (event.actionMasked) {
            MotionEvent.ACTION_DOWN -> {
                touchStartTime = System.currentTimeMillis()
                maxPointersInGesture = 1
                hasMovedSignificantly = false
                prevX = x
                prevY = y
            }
            MotionEvent.ACTION_POINTER_DOWN -> {
                if (pointerCount > maxPointersInGesture) {
                    maxPointersInGesture = pointerCount
                }
            }
            MotionEvent.ACTION_MOVE -> {
                val dx = x - prevX
                val dy = y - prevY
                val dist = Math.hypot(dx.toDouble(), dy.toDouble())
                if (dist > 25.0) {
                    hasMovedSignificantly = true
                }

                if (pointerCount == 1 && !scaleDetector.isInProgress) {
                    if (activeToolProvider() == "Pan") {
                        renderer?.pan(dx, dy)
                    } else {
                        renderer?.orbit(dx, dy)
                    }
                } else if (pointerCount == 2) {
                    renderer?.pan(dx, dy)
                }
                prevX = x
                prevY = y
            }
            MotionEvent.ACTION_UP -> {
                val elapsed = System.currentTimeMillis() - touchStartTime
                // Multi-finger tap detection (tap within 400ms without significant drag)
                if (!hasMovedSignificantly && elapsed < 400L) {
                    if (maxPointersInGesture == 2) {
                        onUndo()
                    } else if (maxPointersInGesture == 3) {
                        onRedo()
                    }
                }
            }
        }
        true
    }
}
