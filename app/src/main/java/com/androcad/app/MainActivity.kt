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

    var activeTab by remember { mutableStateOf("Solid") }
    var navTool by remember { mutableStateOf("Orbit") }
    var currentDisplayMode by remember { mutableStateOf(DisplayMode.SHADED_WITH_EDGES) }
    var selectedEntityInfo by remember { mutableStateOf("No entity selected (tap face/edge)") }
    var selectedType by remember { mutableStateOf<String?>(null) }
    var selectedId by remember { mutableStateOf(0) }

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
                        onClick = { activeTab = tab },
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
                horizontalArrangement = Arrangement.spacedBy(8.dp)
            ) {
                val tools = listOf("Create Box", "Extrude", "Revolve", "Fillet", "Combine", "Shell")
                tools.forEach { tool ->
                    Surface(
                        color = Color(0xFF2F3136),
                        shape = RoundedCornerShape(4.dp),
                        modifier = Modifier.padding(2.dp)
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

                val treeItems = listOf(
                    "▶ Document Settings",
                    "▶ Named Views",
                    "▼ Origin",
                    "  • XY, XZ, YZ Planes",
                    "▼ Bodies",
                    "  [✓] Solid Box 1",
                    "▶ Sketches"
                )
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

                        // Middle Ring: LEFT | FRONT | RIGHT | BACK
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

                if (meshResult != null && meshResult.success) {
                    Text(text = "Solid: Box 1", color = Color.White, fontSize = 12.sp)
                    Text(text = "Kernel: OCCT 7.8.0", color = textMuted, fontSize = 11.sp)
                    Spacer(modifier = Modifier.height(6.dp))
                    Text(text = "Volume: ${"%.1f".format(meshResult.volume)} mm³", color = textMuted, fontSize = 11.sp)
                    Text(text = "Faces: ${meshResult.faceCount}", color = textMuted, fontSize = 11.sp)
                    Text(text = "Edges: ${meshResult.edgeCount}", color = textMuted, fontSize = 11.sp)
                    Text(text = "Triangles: ${meshResult.triangleCount}", color = textMuted, fontSize = 11.sp)
                    Spacer(modifier = Modifier.height(6.dp))
                    Text(text = "Bounding Dimensions:", color = Color.White, fontSize = 11.sp)
                    Text(text = "  X = 100.0 mm", color = textMuted, fontSize = 11.sp)
                    Text(text = "  Y = 50.0 mm", color = textMuted, fontSize = 11.sp)
                    Text(text = "  Z = 20.0 mm", color = textMuted, fontSize = 11.sp)

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

                // Feature 1: Box 1
                Surface(
                    color = Color(0xFF2F3136),
                    shape = RoundedCornerShape(4.dp),
                    border = androidx.compose.foundation.BorderStroke(1.dp, accentBlue)
                ) {
                    Text(
                        text = "1. Box (100x50x20)",
                        color = Color.White,
                        fontSize = 11.sp,
                        modifier = Modifier.padding(horizontal = 8.dp, vertical = 4.dp)
                    )
                }

                Spacer(modifier = Modifier.width(8.dp))
                // Rollback Marker
                Box(
                    modifier = Modifier
                        .width(4.dp)
                        .height(24.dp)
                        .background(Color(0xFFFFB74D))
                )
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
    var touchDownX = 0f
    var touchDownY = 0f

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
                // Request GPU color-coded picking pass at tap coordinates
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
                touchDownX = x
                touchDownY = y
            }
            MotionEvent.ACTION_MOVE -> {
                val dx = x - prevX
                val dy = y - prevY

                if (pointerCount == 1 && !scaleDetector.isInProgress) {
                    if (activeToolProvider() == "Pan") {
                        renderer?.pan(dx, dy)
                    } else {
                        // Orbit mode (default)
                        renderer?.orbit(dx, dy)
                    }
                } else if (pointerCount == 2) {
                    // Two-finger Pan
                    renderer?.pan(dx, dy)
                }
                prevX = x
                prevY = y
            }
        }
        true
    }
}
