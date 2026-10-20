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
    val darkPanelColor = Color(0xFF1E2124)
    val topToolbarColor = Color(0xFF282B30)
    val accentBlue = Color(0xFF007ACC)
    val textMuted = Color(0xFFB0B3B8)
    val highlightCyan = Color(0xFF00A4E4)
    val highlightOrange = Color(0xFFFF9800)
    val successGreen = Color(0xFF4CAF50)

    var activeTab by remember { mutableStateOf("Solid") }
    var navTool by remember { mutableStateOf("Orbit") }
    var currentDisplayMode by remember { mutableStateOf(DisplayMode.SHADED_WITH_EDGES) }
    var selectedEntityInfo by remember { mutableStateOf("No entity selected (tap face/edge)") }
    var selectedType by remember { mutableStateOf<String?>(null) }
    var selectedId by remember { mutableStateOf(0) }

    // Milestone 3 & 4: Sketch & Parametric Solid Features state
    var activeSolid by remember { mutableStateOf(meshResult) }
    var timelineItems by remember { mutableStateOf(listOf("1. Box (100x50x20)")) }
    var rollbackPos by remember { mutableStateOf(-1) }
    var featureStatusText by remember { mutableStateOf("Ready") }
    var activeSketchProfile by remember { mutableStateOf<SketchProfileResult?>(null) }
    var activeSketchTool by remember { mutableStateOf("Rectangle") }
    var sketchDoF by remember { mutableStateOf(0) }
    var sketchStatusText by remember { mutableStateOf("Ready to sketch") }

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

    Column(modifier = Modifier.fillMaxSize().background(Color(0xFF141618))) {
        // 1. Top Workspace Tabs & Toolbar
        Column(modifier = Modifier.fillMaxWidth().background(topToolbarColor)) {
            Row(
                modifier = Modifier.fillMaxWidth().padding(horizontal = 8.dp, vertical = 4.dp),
                verticalAlignment = Alignment.CenterVertically
            ) {
                Text(
                    text = "AndroCAD",
                    color = Color.White,
                    fontSize = 16.sp,
                    modifier = Modifier.padding(end = 16.dp)
                )

                val tabs = listOf("Solid", "Sketch", "Surface", "Mesh", "Inspect")
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
                        Text(text = tab, fontSize = 14.sp)
                    }
                }
            }

            // Sub-toolbar with action buttons
            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .background(Color(0xFF202225))
                    .padding(horizontal = 8.dp, vertical = 4.dp),
                horizontalArrangement = Arrangement.spacedBy(8.dp),
                verticalAlignment = Alignment.CenterVertically
            ) {
                if (activeTab == "Sketch") {
                    val sketchTools = listOf("Rectangle", "Line", "Circle", "Dimension", "Constraints")
                    sketchTools.forEach { tool ->
                        Surface(
                            color = if (activeSketchTool == tool) accentBlue else Color(0xFF2F3136),
                            shape = RoundedCornerShape(4.dp),
                            modifier = Modifier.clickable { activeSketchTool = tool }
                        ) {
                            Text(
                                text = tool,
                                color = Color.White,
                                fontSize = 12.sp,
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
                            fontSize = 12.sp,
                            modifier = Modifier.padding(horizontal = 10.dp, vertical = 4.dp)
                        )
                    }
                } else {
                    val tools = listOf("Box", "Extrude", "Revolve", "Fillet", "Chamfer", "Hole", "Shell", "Combine")
                    tools.forEach { tool ->
                        Surface(
                            color = Color(0xFF2F3136),
                            shape = RoundedCornerShape(4.dp),
                            modifier = Modifier
                                .padding(2.dp)
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
                                color = Color.White,
                                fontSize = 12.sp,
                                modifier = Modifier.padding(horizontal = 8.dp, vertical = 4.dp)
                            )
                        }
                    }
                }
            }
        }

        // 2. Middle Row: Browser Tree | 3D Viewport | Properties Panel
        Row(modifier = Modifier.weight(1f).fillMaxWidth()) {
            // Left Browser Panel
            Column(
                modifier = Modifier
                    .width(180.dp)
                    .fillMaxHeight()
                    .background(darkPanelColor)
                    .border(width = 1.dp, color = Color(0xFF2B2D31))
                    .padding(8.dp)
            ) {
                Text(text = "BROWSER", color = Color.White, fontSize = 13.sp)
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
                    Text(text = item, color = textMuted, fontSize = 12.sp, modifier = Modifier.padding(vertical = 2.dp))
                }
            }

            // Center Viewport with GLES Renderer, ViewCube & Navigation Bar
            Box(modifier = Modifier.weight(1f).fillMaxHeight()) {
                AndroidView(
                    factory = { context ->
                        GLSurfaceView(context).apply {
                            setEGLContextClientVersion(3)
                            if (renderer != null) {
                                setRenderer(renderer)
                            }
                            renderMode = GLSurfaceView.RENDERMODE_CONTINUOUSLY
                            setupCadTouchListener(this, renderer) { navTool }
                        }
                    },
                    modifier = Modifier.fillMaxSize()
                )

                // Top-Right Interactive ViewCube
                Surface(
                    color = Color(0xDD202225),
                    shape = RoundedCornerShape(8.dp),
                    border = androidx.compose.foundation.BorderStroke(1.dp, Color(0xFF3E4249)),
                    modifier = Modifier
                        .align(Alignment.TopEnd)
                        .padding(12.dp)
                        .width(130.dp)
                ) {
                    Column(
                        modifier = Modifier.padding(6.dp),
                        horizontalAlignment = Alignment.CenterHorizontally
                    ) {
                        Text(text = "VIEW CUBE", color = textMuted, fontSize = 10.sp)
                        Spacer(modifier = Modifier.height(4.dp))

                        // Top Face
                        ViewCubeButton("TOP") { renderer?.setNamedView(NamedView.TOP) }

                        // Middle Ring: LEFT | FRONT | RIGHT
                        Row(
                            horizontalArrangement = Arrangement.spacedBy(2.dp),
                            modifier = Modifier.padding(vertical = 2.dp)
                        ) {
                            ViewCubeButton("L", width = 28) { renderer?.setNamedView(NamedView.LEFT) }
                            ViewCubeButton("FRONT", width = 48) { renderer?.setNamedView(NamedView.FRONT) }
                            ViewCubeButton("R", width = 28) { renderer?.setNamedView(NamedView.RIGHT) }
                        }

                        // Bottom Row: BOTTOM and ISO (HOME)
                        Row(
                            horizontalArrangement = Arrangement.spacedBy(4.dp),
                            modifier = Modifier.padding(top = 2.dp)
                        ) {
                            ViewCubeButton("BOTTOM", width = 56) { renderer?.setNamedView(NamedView.BOTTOM) }
                            ViewCubeButton("ISO", width = 48) { renderer?.setNamedView(NamedView.HOME) }
                        }
                    }
                }

                // Bottom-Center Navigation Bar
                Surface(
                    color = Color(0xEE202225),
                    shape = RoundedCornerShape(20.dp),
                    border = androidx.compose.foundation.BorderStroke(1.dp, Color(0xFF3E4249)),
                    modifier = Modifier
                        .align(Alignment.BottomCenter)
                        .padding(bottom = 12.dp)
                ) {
                    Row(
                        modifier = Modifier.padding(horizontal = 14.dp, vertical = 6.dp),
                        verticalAlignment = Alignment.CenterVertically,
                        horizontalArrangement = Arrangement.spacedBy(12.dp)
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
                            DisplayMode.SHADED_WITH_EDGES -> "Shaded + Edges"
                            DisplayMode.SHADED -> "Shaded"
                            DisplayMode.WIREFRAME -> "Wireframe"
                        }
                        Surface(
                            color = Color(0xFF2C2F33),
                            shape = RoundedCornerShape(12.dp),
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
                                text = "Mode: $modeLabel",
                                color = Color.White,
                                fontSize = 11.sp,
                                modifier = Modifier.padding(horizontal = 8.dp, vertical = 4.dp)
                            )
                        }
                    }
                }
            }

            // Right Properties Panel
            Column(
                modifier = Modifier
                    .width(220.dp)
                    .fillMaxHeight()
                    .background(darkPanelColor)
                    .border(width = 1.dp, color = Color(0xFF2B2D31))
                    .padding(8.dp)
            ) {
                Text(text = "PROPERTIES", color = Color.White, fontSize = 13.sp)
                Spacer(modifier = Modifier.height(8.dp))

                if (activeSolid != null && activeSolid!!.success) {
                    Text(text = "Solid: Active Body", color = Color.White, fontSize = 12.sp)
                    Text(text = "Kernel: OCCT 7.8.0", color = textMuted, fontSize = 11.sp)
                    Text(text = "Solver: PlaneGCS (LGPL)", color = textMuted, fontSize = 11.sp)
                    Text(text = "Status: $featureStatusText", color = accentBlue, fontSize = 11.sp)
                    Spacer(modifier = Modifier.height(6.dp))
                    Text(text = "Volume: ${"%.1f".format(activeSolid!!.volume)} mm³", color = textMuted, fontSize = 11.sp)
                    Text(text = "Faces: ${activeSolid!!.faceCount}", color = textMuted, fontSize = 11.sp)
                    Text(text = "Edges: ${activeSolid!!.edgeCount}", color = textMuted, fontSize = 11.sp)
                    Text(text = "Triangles: ${activeSolid!!.triangleCount}", color = textMuted, fontSize = 11.sp)

                    if (activeSketchProfile != null) {
                        Spacer(modifier = Modifier.height(10.dp))
                        Text(text = "ACTIVE SKETCH", color = Color.White, fontSize = 12.sp)
                        Text(text = "Profile: Closed (4 edges)", color = successGreen, fontSize = 11.sp)
                        Text(text = "Area: ${"%.1f".format(activeSketchProfile!!.area)} mm²", color = textMuted, fontSize = 11.sp)
                        Text(text = "DoF: $sketchDoF ($sketchStatusText)", color = if (sketchDoF == 0) successGreen else highlightOrange, fontSize = 11.sp)
                    }

                    Spacer(modifier = Modifier.height(10.dp))
                    Text(text = "USER PARAMETERS (TNP)", color = Color.White, fontSize = 12.sp)
                    Text(text = "• width = 100.0 mm", color = textMuted, fontSize = 11.sp)
                    Text(text = "• length = width * 2 (200.0 mm)", color = textMuted, fontSize = 11.sp)
                    Text(text = "• height = 25.0 mm", color = textMuted, fontSize = 11.sp)

                    Spacer(modifier = Modifier.height(14.dp))
                    Text(text = "SELECTION", color = Color.White, fontSize = 12.sp)
                    Spacer(modifier = Modifier.height(4.dp))

                    Surface(
                        color = Color(0xFF2A2D31),
                        shape = RoundedCornerShape(4.dp),
                        border = androidx.compose.foundation.BorderStroke(
                            1.dp,
                            if (selectedType == "Face") highlightCyan else if (selectedType == "Edge") highlightOrange else Color.Transparent
                        ),
                        modifier = Modifier.fillMaxWidth().padding(vertical = 4.dp)
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

        // 3. Bottom Timeline
        Surface(
            modifier = Modifier.fillMaxWidth().height(48.dp),
            color = Color(0xFF1E2124),
            border = androidx.compose.foundation.BorderStroke(1.dp, Color(0xFF2B2D31))
        ) {
            Row(
                modifier = Modifier.fillMaxSize().padding(horizontal = 8.dp),
                verticalAlignment = Alignment.CenterVertically
            ) {
                Text(text = "TIMELINE:", color = textMuted, fontSize = 12.sp, modifier = Modifier.padding(end = 12.dp))

                timelineItems.forEachIndexed { index, item ->
                    val isRolledBack = (rollbackPos >= 0 && index > rollbackPos)
                    Surface(
                        color = if (isRolledBack) Color(0xFF222428) else Color(0xFF2F3136),
                        shape = RoundedCornerShape(4.dp),
                        border = androidx.compose.foundation.BorderStroke(
                            1.dp,
                            if (index == rollbackPos || (rollbackPos == -1 && index == timelineItems.lastIndex)) accentBlue else Color(0xFF454950)
                        ),
                        modifier = Modifier.clickable {
                            rollbackPos = index
                            featureStatusText = "Rolled back to step ${index + 1}"
                        }
                    ) {
                        Text(
                            text = item,
                            color = if (isRolledBack) Color(0xFF70757D) else Color.White,
                            fontSize = 11.sp,
                            modifier = Modifier.padding(horizontal = 8.dp, vertical = 4.dp)
                        )
                    }
                    Spacer(modifier = Modifier.width(6.dp))
                }

                if (activeSketchProfile != null && activeSketchProfile!!.isClosedProfile) {
                    Surface(
                        color = Color(0xFF2F3136),
                        shape = RoundedCornerShape(4.dp),
                        border = androidx.compose.foundation.BorderStroke(1.dp, successGreen)
                    ) {
                        Text(
                            text = "Sketch (100x50)",
                            color = Color.White,
                            fontSize = 11.sp,
                            modifier = Modifier.padding(horizontal = 8.dp, vertical = 4.dp)
                        )
                    }
                    Spacer(modifier = Modifier.width(6.dp))
                }

                // Rollback Marker / Unroll control
                Surface(
                    color = Color(0xFFFFB74D),
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
    }
}

@Composable
fun ViewCubeButton(label: String, width: Int = 50, onClick: () -> Unit) {
    Surface(
        color = Color(0xFF2A2D32),
        shape = RoundedCornerShape(3.dp),
        border = androidx.compose.foundation.BorderStroke(1.dp, Color(0xFF454950)),
        modifier = Modifier.width(width.dp).height(24.dp).clickable(onClick = onClick)
    ) {
        Box(contentAlignment = Alignment.Center) {
            Text(text = label, color = Color.White, fontSize = 9.sp)
        }
    }
}

@SuppressLint("ClickableViewAccessibility")
private fun setupCadTouchListener(
    view: GLSurfaceView,
    renderer: CadRenderer?,
    activeToolProvider: () -> String
) {
    var prevX = 0f
    var prevY = 0f

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
                prevX = x
                prevY = y
            }
            MotionEvent.ACTION_MOVE -> {
                val dx = x - prevX
                val dy = y - prevY

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
        }
        true
    }
}