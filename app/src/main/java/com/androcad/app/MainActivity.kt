// Copyright (c) 2026 dursuntokgoz. All rights reserved.
package com.androcad.app

import android.annotation.SuppressLint
import android.opengl.GLSurfaceView
import android.os.Bundle
import android.view.MotionEvent
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.background
import androidx.compose.foundation.border
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
import com.androcad.app.renderer.BoxRenderer
import com.androcad.kernel.BoxMeshResult
import com.androcad.kernel.NativeKernel

class MainActivity : ComponentActivity() {

    private var meshResult: BoxMeshResult? = null
    private var renderer: BoxRenderer? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        // Initialize kernel smoke test: construct 100x50x20 analytical box
        try {
            meshResult = NativeKernel.createBox(100.0, 50.0, 20.0, 0.1, 0.5)
        } catch (t: Throwable) {
            meshResult = null
        }

        renderer = BoxRenderer(meshResult)

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
fun AndroCadApp(meshResult: BoxMeshResult?, renderer: BoxRenderer?) {
    val darkPanelColor = Color(0xFF1E2124)
    val topToolbarColor = Color(0xFF282B30)
    val accentBlue = Color(0xFF007ACC)
    val textMuted = Color(0xFFB0B3B8)

    var activeTab by remember { mutableStateOf("Solid") }

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

            // Center Viewport with GLES Renderer
            Box(modifier = Modifier.weight(1f).fillMaxHeight()) {
                AndroidView(
                    factory = { context ->
                        GLSurfaceView(context).apply {
                            setEGLContextClientVersion(3)
                            if (renderer != null) {
                                setRenderer(renderer)
                            }
                            renderMode = GLSurfaceView.RENDERMODE_CONTINUOUSLY
                            setupTouchListener(this, renderer)
                        }
                    },
                    modifier = Modifier.fillMaxSize()
                )

                // Top-Right ViewCube Overlay
                Surface(
                    color = Color(0xAA202225),
                    shape = RoundedCornerShape(6.dp),
                    modifier = Modifier
                        .align(Alignment.TopEnd)
                        .padding(16.dp)
                        .size(70.dp)
                ) {
                    Box(contentAlignment = Alignment.Center) {
                        Text(text = "TOP\nFRONT", color = Color.White, fontSize = 11.sp)
                    }
                }

                // Bottom-Center Navigation Bar
                Surface(
                    color = Color(0xCC202225),
                    shape = RoundedCornerShape(20.dp),
                    modifier = Modifier
                        .align(Alignment.BottomCenter)
                        .padding(bottom = 12.dp)
                ) {
                    Row(
                        modifier = Modifier.padding(horizontal = 12.dp, vertical = 6.dp),
                        horizontalArrangement = Arrangement.spacedBy(16.dp)
                    ) {
                        Text(text = "Orbit", color = accentBlue, fontSize = 12.sp)
                        Text(text = "Pan", color = textMuted, fontSize = 12.sp)
                        Text(text = "Zoom", color = textMuted, fontSize = 12.sp)
                        Text(text = "Fit", color = textMuted, fontSize = 12.sp)
                    }
                }
            }

            // Right Properties Panel
            Column(
                modifier = Modifier
                    .width(200.dp)
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
                    Text(text = "Triangles: ${meshResult.triangleCount}", color = textMuted, fontSize = 11.sp)
                    Spacer(modifier = Modifier.height(6.dp))
                    Text(text = "Dimensions:", color = Color.White, fontSize = 11.sp)
                    Text(text = "  X = 100.0 mm", color = textMuted, fontSize = 11.sp)
                    Text(text = "  Y = 50.0 mm", color = textMuted, fontSize = 11.sp)
                    Text(text = "  Z = 20.0 mm", color = textMuted, fontSize = 11.sp)
                } else {
                    Text(text = "No selection", color = textMuted, fontSize = 12.sp)
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

@SuppressLint("ClickableViewAccessibility")
private fun setupTouchListener(view: GLSurfaceView, renderer: BoxRenderer?) {
    var prevX = 0f
    var prevY = 0f

    view.setOnTouchListener { _, event ->
        val x = event.x
        val y = event.y

        when (event.action) {
            MotionEvent.ACTION_MOVE -> {
                val dx = x - prevX
                val dy = y - prevY
                if (renderer != null) {
                    renderer.angleY += dx * 0.4f
                    renderer.angleX += dy * 0.4f
                }
            }
        }
        prevX = x
        prevY = y
        true
    }
}
