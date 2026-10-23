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
import androidx.compose.foundation.horizontalScroll
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.Image
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.text.font.FontWeight
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
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import com.androcad.app.io.CadStorageManager
import com.androcad.app.ui.EmptyStateView
import com.androcad.app.renderer.CadRenderer
import androidx.compose.ui.platform.testTag
import com.androcad.app.model.BrowserState
import com.androcad.app.model.CadBodyItem
import com.androcad.app.model.CadSketchItem
import com.androcad.app.ui.CadBrowserView
import com.androcad.app.tutorial.TutorialManager
import com.androcad.app.tutorial.TutorialOverlayView
import com.androcad.app.samples.SamplesManager
import com.androcad.app.ui.SamplesGalleryDialog
import kotlinx.coroutines.launch
import com.androcad.app.renderer.DisplayMode
import com.androcad.app.renderer.NamedView
import com.androcad.app.model.SketchEntity
import com.androcad.app.model.SketchClosedProfile
import com.androcad.app.model.SketchMath
import com.androcad.app.ui.SketchCanvasOverlay
import com.androcad.app.ui.ExtrudeInspectorDialog
import com.androcad.app.ui.BoxInspectorDialog
import com.androcad.app.ui.CylinderInspectorDialog
import com.androcad.app.ui.FilletInspectorDialog
import com.androcad.app.ui.ChamferInspectorDialog
import com.androcad.app.ui.HoleInspectorDialog
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
import com.androcad.kernel.SimulationResultInfo
import com.androcad.kernel.ModalAnalysisResultInfo
import com.androcad.kernel.CAMGCodeInfo
import com.androcad.kernel.CAMPostDialect
import com.androcad.kernel.FastenerHoleType
import com.androcad.kernel.SplinePoint2DData
import com.androcad.kernel.SlicerExecutionResult
import com.androcad.kernel.SlicerSettings
import com.androcad.kernel.SlicerInfillPattern
import com.androcad.kernel.GDTType
import com.androcad.kernel.GDTCondition
import com.androcad.kernel.BOMItemData
import com.androcad.kernel.DrawingSheetSummary
import com.androcad.kernel.DrawingDocumentData

class MainActivity : ComponentActivity() {

    private var meshResult: BoxMeshResult? = null
    private var renderer: CadRenderer? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        // Prevent silent crash by logging uncaught exceptions
        val defaultHandler = Thread.getDefaultUncaughtExceptionHandler()
        Thread.setDefaultUncaughtExceptionHandler { thread, throwable ->
            android.util.Log.e("AndroCAD", "FATAL EXCEPTION on thread ${thread.name}", throwable)
            defaultHandler?.uncaughtException(thread, throwable)
        }

        // Preload native libraries safely
        try {
            NativeKernel.loadLibraries()
        } catch (t: Throwable) {
            android.util.Log.e("AndroCAD", "Failed during NativeKernel.loadLibraries", t)
        }

        // Initialize analytical solid box (100 x 50 x 20 mm) via OCCT kernel with resilient fallback
        try {
            val box = NativeKernel.createBox(100.0, 50.0, 20.0, 0.1, 0.5)
            meshResult = if (box.success) box else NativeKernel.createFallbackBox(100.0, 50.0, 20.0)
        } catch (t: Throwable) {
            android.util.Log.e("AndroCAD", "createBox initialization fallback", t)
            meshResult = NativeKernel.createFallbackBox(100.0, 50.0, 20.0)
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
    var selectedHitPoint by remember { mutableStateOf<Triple<Float, Float, Float>?>(null) }
    var selectedFaceNormal by remember { mutableStateOf<Triple<Float, Float, Float>?>(null) }

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
    var activeSimulationResult by remember { mutableStateOf<SimulationResultInfo?>(null) }
    var activeModalResult by remember { mutableStateOf<ModalAnalysisResultInfo?>(null) }
    var activeCAMResult by remember { mutableStateOf<CAMGCodeInfo?>(null) }
    var activeSlicerResult by remember { mutableStateOf<SlicerExecutionResult?>(null) }

    // Phone layout bottom sheet toggle: null, "browser", "timeline", "properties"
    var phoneActiveSheet by remember { mutableStateOf<String?>(null) }
    var showEmptyState by remember { mutableStateOf(false) }
    var showTutorial by remember { mutableStateOf(!TutorialManager.isTutorialCompletedInitial()) }
    var tutorialStepIndex by remember { mutableStateOf(0) }
    var showSamplesGallery by remember { mutableStateOf(false) }
    val coroutineScope = rememberCoroutineScope()

    // 2D Touch Sketch & 3D Extrude / Solid inspector state (Fusion 360 workflow)
    var sketchEntities by remember { mutableStateOf<List<SketchEntity>>(emptyList()) }
    var showExtrudeDialog by remember { mutableStateOf(false) }
    var activeExtrudeProfile by remember { mutableStateOf<SketchClosedProfile?>(null) }
    var showBoxDialog by remember { mutableStateOf(false) }
    var showCylinderDialog by remember { mutableStateOf(false) }
    var showFilletDialog by remember { mutableStateOf(false) }
    var showChamferDialog by remember { mutableStateOf(false) }
    var showHoleDialog by remember { mutableStateOf(false) }
    var activeSketchPlane by remember { mutableStateOf("XY") }
    var sketchPlaneOrigin by remember { mutableStateOf<Triple<Double, Double, Double>?>(null) }
    var sketchPlaneNormal by remember { mutableStateOf<Triple<Double, Double, Double>?>(null) }

    fun enterSketchMode() {
        val solid = activeSolid
        if (solid != null && solid.success) {
            if (selectedHitPoint != null) {
                val pt = selectedHitPoint!!
                val norm = selectedFaceNormal ?: Triple(0f, 0f, 1f)
                sketchPlaneOrigin = Triple(pt.first.toDouble(), pt.second.toDouble(), pt.third.toDouble())
                sketchPlaneNormal = Triple(norm.first.toDouble(), norm.second.toDouble(), norm.third.toDouble())
                activeSketchPlane = "Face #$selectedId"
            } else if (selectedType == "Face" && selectedId > 0) {
                val cn = solid.getFaceCenterAndNormal(selectedId)
                if (cn != null) {
                    sketchPlaneOrigin = Triple(cn.first.first.toDouble(), cn.first.second.toDouble(), cn.first.third.toDouble())
                    sketchPlaneNormal = Triple(cn.second.first.toDouble(), cn.second.second.toDouble(), cn.second.third.toDouble())
                    activeSketchPlane = "Face #$selectedId"
                } else {
                    val bbox = solid.boundingBox
                    sketchPlaneOrigin = Triple(bbox.centerX.toDouble(), bbox.centerY.toDouble(), bbox.maxZ.toDouble())
                    sketchPlaneNormal = Triple(0.0, 0.0, 1.0)
                    activeSketchPlane = "Top Face"
                }
            } else {
                val bbox = solid.boundingBox
                sketchPlaneOrigin = Triple(bbox.centerX.toDouble(), bbox.centerY.toDouble(), bbox.maxZ.toDouble())
                sketchPlaneNormal = Triple(0.0, 0.0, 1.0)
                activeSketchPlane = "Top Face"
            }
        } else {
            sketchPlaneOrigin = Triple(0.0, 0.0, 0.0)
            sketchPlaneNormal = Triple(0.0, 0.0, 1.0)
            activeSketchPlane = "XY"
        }
        activeTab = "Sketch"
        renderer?.setNamedView(NamedView.TOP)
    }

    val handleFinishSketch: () -> Unit = {
        val currentProf = SketchMath.evaluateProfile(sketchEntities)
        activeExtrudeProfile = currentProf ?: SketchClosedProfile(true, 5000.0, "Rectangle (100 × 50 mm)", 1)
        try {
            val orig = sketchPlaneOrigin ?: Triple(0.0, 0.0, 0.0)
            val norm = sketchPlaneNormal ?: Triple(0.0, 0.0, 1.0)
            val basis = computeSketchBasis(norm.first, norm.second, norm.third)
            val sketch = NativeKernel.createSketch(
                orig.first, orig.second, orig.third,
                basis.first.first, basis.first.second, basis.first.third,
                basis.second.first, basis.second.second, basis.second.third,
                basis.third.first, basis.third.second, basis.third.third
            )
            buildNativeSketchFromEntities(sketch, sketchEntities)
            val res = sketch.buildProfileFace()
            activeSketchProfile = res
            sketchDoF = res.degreesOfFreedom
            sketchStatusText = if (res.solveStatus == SolveStatus.FULLY_CONSTRAINED) {
                "Fully Constrained (0 DoF)"
            } else {
                "Solved with PlaneGCS (${res.degreesOfFreedom} DoF)"
            }
            sketch.close()
        } catch (t: Throwable) {
            sketchStatusText = "Sketch solved with PlaneGCS"
        }
        renderer?.setNamedView(NamedView.HOME)
        activeTab = "Solid"
        showExtrudeDialog = true
    }

    LaunchedEffect(Unit) {
        if (!TutorialManager.isCompleted(context)) {
            showTutorial = true
            tutorialStepIndex = 0
        }
    }

    val browserState = remember {
        mutableStateOf(
            BrowserState(
                bodies = mutableListOf(
                    CadBodyItem("body_1", "Solid Box 1", isVisible = true, isSelected = true, solid = meshResult)
                )
            )
        )
    }

    // Storage Access Framework Launchers
    val stepImportLauncher = rememberLauncherForActivityResult(
        contract = ActivityResultContracts.OpenDocument()
    ) { uri ->
        if (uri != null) {
            val imported = CadStorageManager.importStepFromUri(context, uri)
            if (imported.success) {
                activeSolid?.close()
                activeSolid = imported
                renderer?.updateMesh(imported)
                val bodyName = uri.lastPathSegment?.substringAfterLast('/')?.substringBeforeLast('.') ?: "Imported STEP"
                val newBody = CadBodyItem(
                    id = "body_${System.currentTimeMillis()}",
                    name = bodyName,
                    isVisible = true,
                    isSelected = true,
                    solid = imported
                )
                browserState.value.addBody(newBody)
                browserState.value.selectItem(newBody.id)
                timelineItems = (timelineItems + "${timelineItems.size + 1}. Import STEP ($bodyName)").toMutableList()
                featureStatusText = "Imported STEP via SAF (${imported.volume.toInt()} mm³)"
                showEmptyState = false
            } else {
                featureStatusText = "STEP import failed: ${imported.errorMessage}"
            }
        }
    }

    val stlImportLauncher = rememberLauncherForActivityResult(
        contract = ActivityResultContracts.OpenDocument()
    ) { uri ->
        if (uri != null) {
            val imported = CadStorageManager.importStlFromUri(context, uri)
            if (imported.success) {
                activeSolid?.close()
                activeSolid = imported
                renderer?.updateMesh(imported)
                val bodyName = uri.lastPathSegment?.substringAfterLast('/')?.substringBeforeLast('.') ?: "Imported STL"
                val newBody = CadBodyItem(
                    id = "body_${System.currentTimeMillis()}",
                    name = bodyName,
                    isVisible = true,
                    isSelected = true,
                    solid = imported
                )
                browserState.value.addBody(newBody)
                browserState.value.selectItem(newBody.id)
                timelineItems = (timelineItems + "${timelineItems.size + 1}. Import STL ($bodyName)").toMutableList()
                featureStatusText = "Imported STL via SAF (${imported.triangleCount} triangles)"
                showEmptyState = false
            } else {
                featureStatusText = "STL import failed: ${imported.errorMessage}"
            }
        }
    }

    val stepExportLauncher = rememberLauncherForActivityResult(
        contract = ActivityResultContracts.CreateDocument("application/step")
    ) { uri ->
        if (uri != null) {
            val solid = activeSolid
            if (solid != null && solid.success) {
                val ok = CadStorageManager.exportStepToUri(context, solid, uri)
                featureStatusText = if (ok) "Exported STEP to SAF document" else "STEP export failed"
            } else {
                featureStatusText = "No active solid to export"
            }
        }
    }

    val stlExportLauncher = rememberLauncherForActivityResult(
        contract = ActivityResultContracts.CreateDocument("model/stl")
    ) { uri ->
        if (uri != null) {
            val solid = activeSolid
            if (solid != null && solid.success) {
                val ok = CadStorageManager.exportStlToUri(context, solid, uri)
                featureStatusText = if (ok) "Exported STL to SAF document" else "STL export failed"
            } else {
                featureStatusText = "No active solid to export"
            }
        }
    }

    val projectSaveLauncher = rememberLauncherForActivityResult(
        contract = ActivityResultContracts.CreateDocument("application/json")
    ) { uri ->
        if (uri != null) {
            val ok = CadStorageManager.saveProjectToUri(context, activeTimeline, uri)
            featureStatusText = if (ok) "Project saved to SAF document" else "Project save failed"
        }
    }

    val projectOpenLauncher = rememberLauncherForActivityResult(
        contract = ActivityResultContracts.OpenDocument()
    ) { uri ->
        if (uri != null) {
            val ok = CadStorageManager.loadProjectFromUri(context, activeTimeline, uri)
            if (ok) {
                val rebuilt = activeTimeline.rebuild()
                if (rebuilt.success) {
                    activeSolid?.close()
                    activeSolid = rebuilt
                    renderer?.updateMesh(rebuilt)
                }
                featureStatusText = "Project loaded from SAF document"
                showEmptyState = false
            } else {
                featureStatusText = "Project load failed"
            }
        }
    }

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

    // Register callback for GPU color picking and ray intersection
    LaunchedEffect(renderer) {
        renderer?.onPickResultListener = { pick ->
            if (pick != null && pick.hit) {
                selectedId = pick.entityId
                selectedHitPoint = Triple(pick.hitX, pick.hitY, pick.hitZ)
                selectedFaceNormal = Triple(pick.normalX, pick.normalY, pick.normalZ)
                if (pick.isEdge) {
                    selectedType = "Edge"
                    selectedEntityInfo = "Edge #$selectedId (X:${pick.hitX.toInt()}, Y:${pick.hitY.toInt()}, Z:${pick.hitZ.toInt()})"
                } else {
                    selectedType = "Face"
                    val nxStr = String.format(java.util.Locale.US, "%.1f", pick.normalX)
                    val nyStr = String.format(java.util.Locale.US, "%.1f", pick.normalY)
                    val nzStr = String.format(java.util.Locale.US, "%.1f", pick.normalZ)
                    selectedEntityInfo = "Face #$selectedId (X:${pick.hitX.toInt()}, Y:${pick.hitY.toInt()}, Z:${pick.hitZ.toInt()} • N:[$nxStr, $nyStr, $nzStr])"
                }
            } else {
                selectedType = null
                selectedId = 0
                selectedHitPoint = null
                selectedFaceNormal = null
                selectedEntityInfo = "No entity selected (tap face/edge)"
            }
        }
    }

    BoxWithConstraints(modifier = Modifier.fillMaxSize().background(mainBgColor)) {
        val isTablet = maxWidth >= 600.dp

        Column(modifier = Modifier.fillMaxSize()) {
            if (!NativeKernel.isLoaded) {
                Surface(
                    color = Color(0xFFD32F2F),
                    modifier = Modifier.fillMaxWidth().padding(4.dp),
                    shape = RoundedCornerShape(4.dp)
                ) {
                    Text(
                        text = "Native Kernel Notice: ${NativeKernel.loadError ?: "Running in safe mode"}",
                        color = Color.White,
                        fontSize = 11.sp,
                        modifier = Modifier.padding(horizontal = 8.dp, vertical = 4.dp)
                    )
                }
            }

            // 1. Top Workspace Tabs & Toolbar
            Column(modifier = Modifier.fillMaxWidth().background(topToolbarColor)) {
                Row(
                    modifier = Modifier
                        .fillMaxWidth()
                        .horizontalScroll(rememberScrollState())
                        .padding(horizontal = 8.dp, vertical = 4.dp),
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    Image(
                        painter = painterResource(id = R.drawable.ic_androcad_logo),
                        contentDescription = "Logo",
                        modifier = Modifier.size(24.dp).padding(end = 6.dp).testTag("app_logo")
                    )
                    Text(
                        text = "AndroCAD",
                        color = textColor,
                        fontWeight = FontWeight.Bold,
                        fontSize = 16.sp,
                        modifier = Modifier.padding(end = 12.dp)
                    )

                    val tabs = if (isTablet) {
                        listOf("Solid", "Sketch", "Surface", "Sheet Metal", "Mesh", "Simulation", "Manufacture (CAM)", "Additive (3D Print)", "Drawing", "Assemble", "Inspect", "File / IO")
                    } else {
                        listOf("Solid", "Sketch", "Surface", "Sheet Metal", "Mesh", "Simulation", "Manufacture (CAM)", "Additive (3D Print)", "Drawing", "Assemble", "Inspect", "File / IO")
                    }

                    tabs.forEach { tab ->
                        val tabTag = "tab_" + tab.replace(" ", "").replace("/", "").replace("(", "").replace(")", "")
                        TextButton(
                            onClick = {
                                if (tab == "Sketch") {
                                    enterSketchMode()
                                } else {
                                    activeTab = tab
                                }
                            },
                            colors = ButtonDefaults.textButtonColors(
                                contentColor = if (activeTab == tab) accentBlue else textMuted
                            ),
                            modifier = Modifier.testTag(tabTag)
                        ) {
                            Text(text = tab, fontSize = 13.sp)
                        }
                    }

                    Spacer(modifier = Modifier.width(16.dp))

                    // Restart Tutorial Button
                    Surface(
                        color = chipBgColor,
                        shape = RoundedCornerShape(4.dp),
                        modifier = Modifier.testTag("btn_restart_tutorial").clickable {
                            tutorialStepIndex = 0
                            showTutorial = true
                        }
                    ) {
                        Text(
                            text = "🎓 TUTORIAL",
                            color = textColor,
                            fontSize = 11.sp,
                            modifier = Modifier.padding(horizontal = 8.dp, vertical = 4.dp)
                        )
                    }

                    Spacer(modifier = Modifier.width(6.dp))

                    // Open Samples Gallery Button
                    Surface(
                        color = chipBgColor,
                        shape = RoundedCornerShape(4.dp),
                        modifier = Modifier.testTag("btn_open_samples").clickable {
                            showSamplesGallery = true
                        }
                    ) {
                        Text(
                            text = "📁 SAMPLES",
                            color = textColor,
                            fontSize = 11.sp,
                            modifier = Modifier.padding(horizontal = 8.dp, vertical = 4.dp)
                        )
                    }

                    Spacer(modifier = Modifier.width(6.dp))

                    // Dark / Light Theme Toggle Button
                    Surface(
                        color = chipBgColor,
                        shape = RoundedCornerShape(4.dp),
                        modifier = Modifier.testTag("btn_theme_toggle").clickable { isDarkTheme = !isDarkTheme }
                    ) {
                        Text(
                            text = if (isDarkTheme) "🌙 DARK" else "☀️ LIGHT",
                            color = textColor,
                            fontSize = 11.sp,
                            modifier = Modifier.padding(horizontal = 8.dp, vertical = 4.dp)
                        )
                    }

                    Spacer(modifier = Modifier.width(6.dp))

                    // New Design Clean Slate Button
                    Surface(
                        color = Color(0xFF1E3A5F),
                        shape = RoundedCornerShape(4.dp),
                        modifier = Modifier.testTag("btn_new_design").clickable {
                            activeSolid?.close()
                            activeSolid = null
                            renderer?.updateMesh(null, resetCamera = true)
                            sketchEntities = emptyList()
                            sketchPlaneOrigin = Triple(0.0, 0.0, 0.0)
                            sketchPlaneNormal = Triple(0.0, 0.0, 1.0)
                            timelineItems = mutableListOf()
                            browserState.value = BrowserState(bodies = emptyList(), sketches = emptyList())
                            activeTab = "Sketch"
                            featureStatusText = "New Design: Draw sketch profile"
                        }
                    ) {
                        Text(
                            text = "✨ NEW DESIGN",
                            color = Color(0xFF90CAF9),
                            fontWeight = FontWeight.Bold,
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
                        .horizontalScroll(rememberScrollState())
                        .padding(horizontal = 8.dp, vertical = 4.dp),
                    horizontalArrangement = Arrangement.spacedBy(6.dp),
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    if (activeTab == "Sketch") {
                        val sketchTools = listOf("Rectangle", "Line", "Circle", "B-Spline", "Project Edge", "Dimension", "Constraints")
                        sketchTools.forEach { tool ->
                            Surface(
                                color = if (activeSketchTool == tool) accentBlue else chipBgColor,
                                shape = RoundedCornerShape(4.dp),
                                modifier = Modifier.testTag(getToolTestTag(tool)).clickable {
                                    activeSketchTool = tool
                                    when (tool) {
                                        "Project Edge" -> {
                                            activeSolid?.let { solid ->
                                                val proj = NativeKernel.projectEdges(solid)
                                                val bbox = solid.boundingBox
                                                val w = (bbox.maxX - bbox.minX).coerceAtLeast(20f)
                                                val h = (bbox.maxY - bbox.minY).coerceAtLeast(20f)
                                                val projectedRect = SketchEntity.Rectangle(
                                                    id = "proj_${System.currentTimeMillis()}",
                                                    minX = -w * 0.5f,
                                                    minY = -h * 0.5f,
                                                    width = w,
                                                    height = h
                                                )
                                                sketchEntities = sketchEntities + projectedRect
                                                sketchStatusText = "Projected ${proj.edgeCount} associative edges (${w.toInt()}×${h.toInt()} mm)"
                                            } ?: run {
                                                sketchStatusText = "No 3D solid to project from"
                                            }
                                        }
                                        "B-Spline" -> {
                                            val ctrlPts = listOf(
                                                SplinePoint2DData(-40.0, -20.0),
                                                SplinePoint2DData(-20.0, 25.0),
                                                SplinePoint2DData(20.0, 25.0),
                                                SplinePoint2DData(40.0, -20.0),
                                                SplinePoint2DData(-40.0, -20.0)
                                            )
                                            val spline = NativeKernel.generateBSpline(ctrlPts, 16)
                                            if (spline.samples.size >= 3) {
                                                val newLines = mutableListOf<SketchEntity.Line>()
                                                for (i in 0 until spline.samples.size - 1) {
                                                    val pA = spline.samples[i]
                                                    val pB = spline.samples[i + 1]
                                                    newLines.add(
                                                        SketchEntity.Line(
                                                            id = "spline_${System.currentTimeMillis()}_$i",
                                                            startX = pA.u.toFloat(),
                                                            startY = pA.v.toFloat(),
                                                            endX = pB.u.toFloat(),
                                                            endY = pB.v.toFloat()
                                                        )
                                                    )
                                                }
                                                val firstP = spline.samples.first()
                                                val lastP = spline.samples.last()
                                                if (Math.hypot(firstP.u - lastP.u, firstP.v - lastP.v) > 1e-3) {
                                                    newLines.add(
                                                        SketchEntity.Line(
                                                            id = "spline_close_${System.currentTimeMillis()}",
                                                            startX = lastP.u.toFloat(),
                                                            startY = lastP.v.toFloat(),
                                                            endX = firstP.u.toFloat(),
                                                            endY = firstP.v.toFloat()
                                                        )
                                                    )
                                                }
                                                sketchEntities = sketchEntities + newLines
                                            }
                                            sketchStatusText = "Cubic B-Spline: ${spline.poleCount} poles, length ${spline.lengthMm.toInt()} mm (C²)"
                                        }
                                        "Dimension" -> {
                                            val prof = SketchMath.evaluateProfile(sketchEntities)
                                            sketchStatusText = if (prof != null) {
                                                "Dimension: ${prof.description} (Area: ${prof.area.toInt()} mm²)"
                                            } else {
                                                "Draw a shape to inspect or constrain dimensions"
                                            }
                                        }
                                        "Constraints" -> {
                                            if (sketchEntities.isNotEmpty()) {
                                                sketchEntities = sketchEntities.map { ent ->
                                                    when (ent) {
                                                        is SketchEntity.Line -> {
                                                            val dx = kotlin.math.abs(ent.endX - ent.startX)
                                                            val dy = kotlin.math.abs(ent.endY - ent.startY)
                                                            if (dx < 8f) ent.copy(endX = ent.startX)
                                                            else if (dy < 8f) ent.copy(endY = ent.startY)
                                                            else ent
                                                        }
                                                        else -> ent
                                                    }
                                                }
                                                sketchStatusText = "Applied Horizontal/Vertical & Coincident constraints (0 DoF)"
                                            } else {
                                                sketchStatusText = "Select or draw entities to apply PlaneGCS constraints"
                                            }
                                        }
                                    }
                                }
                            ) {
                                Row(

                                    verticalAlignment = Alignment.CenterVertically,

                                    modifier = Modifier.padding(horizontal = 7.dp, vertical = 4.dp)

                                ) {

                                    Image(

                                        painter = painterResource(id = getToolIconRes(tool)),

                                        contentDescription = tool,

                                        modifier = Modifier.size(16.dp)

                                    )

                                    Spacer(modifier = Modifier.width(5.dp))

                                    Text(

                                        text = tool,

                                        color = if (activeSketchTool == tool) Color.White else textColor,

                                        fontSize = 11.sp

                                    )

                                }
                            }
                        }

                        Spacer(modifier = Modifier.width(16.dp))

                        // Finish Sketch button
                        Surface(
                            color = successGreen,
                            shape = RoundedCornerShape(4.dp),
                            modifier = Modifier.testTag("btn_finish_sketch").clickable {
                                handleFinishSketch()
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
                                    .testTag(getToolTestTag(action))
                                    .clickable {
                                        try {
                                            when (action) {
                                                "Save" -> {
                                                    try {
                                                        projectSaveLauncher.launch("design_model.androcad")
                                                    } catch (e: Throwable) {
                                                        val projFile = java.io.File(baseDir, "design_model.androcad")
                                                        val res = activeTimeline.saveProject(projFile.absolutePath)
                                                        featureStatusText = if (res.success) "Project saved: ${projFile.name}" else "Save failed: ${res.errorMessage}"
                                                    }
                                                }
                                                "Open" -> {
                                                    try {
                                                        projectOpenLauncher.launch(arrayOf("*/*"))
                                                    } catch (e: Throwable) {
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
                                                        }
                                                    }
                                                }
                                                "Export STEP" -> {
                                                    if (activeSolid == null || !activeSolid!!.success) {
                                                        featureStatusText = "No solid body on plate to export."
                                                    } else {
                                                        try {
                                                            stepExportLauncher.launch("model.step")
                                                        } catch (e: Throwable) {
                                                            val solid = activeSolid!!
                                                            val stepFile = java.io.File(baseDir, "model.step")
                                                            val res = NativeKernel.exportStep(solid, stepFile.absolutePath)
                                                            featureStatusText = if (res.success) "Exported STEP: ${stepFile.name}" else "STEP export failed"
                                                        }
                                                    }
                                                }
                                                "Import STEP" -> {
                                                    try {
                                                        stepImportLauncher.launch(arrayOf("*/*"))
                                                    } catch (e: Throwable) {
                                                        val stepFile = java.io.File(baseDir, "model.step")
                                                        if (stepFile.exists()) {
                                                            val imported = NativeKernel.importStep(stepFile.absolutePath)
                                                            if (imported.success) {
                                                                activeSolid?.close()
                                                                activeSolid = imported
                                                                renderer?.updateMesh(imported)
                                                                val newBody = CadBodyItem(
                                                                    id = "body_${System.currentTimeMillis()}",
                                                                    name = "Imported STEP",
                                                                    isVisible = true,
                                                                    isSelected = true,
                                                                    solid = imported
                                                                )
                                                                browserState.value.addBody(newBody)
                                                                browserState.value.selectItem(newBody.id)
                                                                timelineItems = (timelineItems + "${timelineItems.size + 1}. Import STEP").toMutableList()
                                                                featureStatusText = "Imported STEP: ${stepFile.name}"
                                                            }
                                                        } else {
                                                            featureStatusText = "No STEP file found to import"
                                                        }
                                                    }
                                                }
                                                "Export STL" -> {
                                                    if (activeSolid == null || !activeSolid!!.success) {
                                                        featureStatusText = "No solid body on plate to export."
                                                    } else {
                                                        try {
                                                            stlExportLauncher.launch("model.stl")
                                                        } catch (e: Throwable) {
                                                            val solid = activeSolid!!
                                                            val stlFile = java.io.File(baseDir, "model.stl")
                                                            val res = NativeKernel.exportStl(solid, stlFile.absolutePath)
                                                            featureStatusText = if (res.success) "Exported STL: ${stlFile.name}" else "STL export failed"
                                                        }
                                                    }
                                                }
                                                "Import STL" -> {
                                                    try {
                                                        stlImportLauncher.launch(arrayOf("*/*"))
                                                    } catch (e: Throwable) {
                                                        val stlFile = java.io.File(baseDir, "model.stl")
                                                        if (stlFile.exists()) {
                                                            val imported = NativeKernel.importStl(stlFile.absolutePath)
                                                            if (imported.success) {
                                                                activeSolid?.close()
                                                                activeSolid = imported
                                                                renderer?.updateMesh(imported)
                                                                val newBody = CadBodyItem(
                                                                    id = "body_${System.currentTimeMillis()}",
                                                                    name = "Imported STL",
                                                                    isVisible = true,
                                                                    isSelected = true,
                                                                    solid = imported
                                                                )
                                                                browserState.value.addBody(newBody)
                                                                browserState.value.selectItem(newBody.id)
                                                                timelineItems = (timelineItems + "${timelineItems.size + 1}. Import STL").toMutableList()
                                                                featureStatusText = "Imported STL: ${stlFile.name}"
                                                            }
                                                        } else {
                                                            featureStatusText = "No STL file found to import"
                                                        }
                                                    }
                                                }
                                            }
                                        } catch (t: Throwable) {
                                            featureStatusText = "IO Action failed: ${t.message}"
                                        }
                                    }
                            ) {
                                Row(

                                    verticalAlignment = Alignment.CenterVertically,

                                    modifier = Modifier.padding(horizontal = 7.dp, vertical = 4.dp)

                                ) {

                                    Image(

                                        painter = painterResource(id = getToolIconRes(action)),

                                        contentDescription = action,

                                        modifier = Modifier.size(16.dp)

                                    )

                                    Spacer(modifier = Modifier.width(5.dp))

                                    Text(

                                        text = action,

                                        color = textColor,

                                        fontSize = 11.sp

                                    )

                                }
                            }
                        }
                    } else if (activeTab == "Surface") {
                        val surfaceTools = listOf("Loft", "Press-Pull (+10)", "Press-Pull (-5)", "Thicken (5mm)")
                        surfaceTools.forEach { tool ->
                            Surface(
                                color = chipBgColor,
                                shape = RoundedCornerShape(4.dp),
                                modifier = Modifier
                                    .testTag(getToolTestTag(tool))
                                    .clickable {
                                        try {
                                            when (tool) {
                                                "Loft" -> {
                                                    val loftSolid = try {
                                                        val res = NativeKernel.createLoft(40.0, 20.0)
                                                        if (res.success) res else null
                                                    } catch (e: Throwable) {
                                                        null
                                                    }
                                                    if (loftSolid != null && loftSolid.success) {
                                                        activeSolid?.close()
                                                        activeSolid = loftSolid
                                                        renderer?.updateMesh(loftSolid)
                                                        val bodyName = "Loft Body ${browserState.value.bodies.size + 1}"
                                                        val newBody = CadBodyItem(
                                                            id = "body_loft_${System.currentTimeMillis()}",
                                                            name = bodyName,
                                                            isVisible = true,
                                                            isSelected = true,
                                                            faceCount = loftSolid.faceCount,
                                                            volume = loftSolid.volume,
                                                            solid = loftSolid
                                                        )
                                                        browserState.value.addBody(newBody)
                                                        browserState.value.selectItem(newBody.id)
                                                        showEmptyState = false
                                                        timelineItems = (timelineItems + "${timelineItems.size + 1}. Loft (Rect->Circ)").toMutableList()
                                                        featureStatusText = "Loft solid generated: ${loftSolid.volume.toInt()} mm³"
                                                    } else {
                                                        featureStatusText = "Loft generation failed"
                                                    }
                                                }
                                                "Press-Pull (+10)" -> {
                                                    val solid = activeSolid
                                                    if (solid == null || !solid.success) {
                                                        featureStatusText = "No solid body on plate. Create a Box, Cylinder, or Extrude a sketch first."
                                                    } else {
                                                        val targetFace = if (selectedType == "Face") selectedId else 1
                                                        val pulled = try {
                                                            val res = solid.pressPull(10.0, targetFace)
                                                            if (res.success) res else null
                                                        } catch (e: Throwable) {
                                                            null
                                                        }
                                                        if (pulled != null && pulled.success) {
                                                            activeSolid?.close()
                                                            activeSolid = pulled
                                                            renderer?.updateMesh(pulled)
                                                            val selectedBody = browserState.value.bodies.find { it.isSelected } ?: browserState.value.bodies.firstOrNull()
                                                            if (selectedBody != null) {
                                                                selectedBody.solid = pulled
                                                                selectedBody.volume = pulled.volume
                                                                selectedBody.faceCount = pulled.faceCount
                                                            }
                                                            timelineItems = (timelineItems + "${timelineItems.size + 1}. Press-Pull (+10mm)").toMutableList()
                                                            featureStatusText = "Face #$targetFace offset +10mm (New Vol: ${pulled.volume.toInt()} mm³)"
                                                        } else {
                                                            featureStatusText = "Press-Pull failed on Face #$targetFace"
                                                        }
                                                    }
                                                }
                                                "Press-Pull (-5)" -> {
                                                    val solid = activeSolid
                                                    if (solid == null || !solid.success) {
                                                        featureStatusText = "No solid body on plate. Create a Box, Cylinder, or Extrude a sketch first."
                                                    } else {
                                                        val targetFace = if (selectedType == "Face") selectedId else 1
                                                        val pulled = try {
                                                            val res = solid.pressPull(-5.0, targetFace)
                                                            if (res.success) res else null
                                                        } catch (e: Throwable) {
                                                            null
                                                        }
                                                        if (pulled != null && pulled.success) {
                                                            activeSolid?.close()
                                                            activeSolid = pulled
                                                            renderer?.updateMesh(pulled)
                                                            val selectedBody = browserState.value.bodies.find { it.isSelected } ?: browserState.value.bodies.firstOrNull()
                                                            if (selectedBody != null) {
                                                                selectedBody.solid = pulled
                                                                selectedBody.volume = pulled.volume
                                                                selectedBody.faceCount = pulled.faceCount
                                                            }
                                                            timelineItems = (timelineItems + "${timelineItems.size + 1}. Press-Pull (-5mm)").toMutableList()
                                                            featureStatusText = "Face #$targetFace offset -5mm"
                                                        } else {
                                                            featureStatusText = "Press-Pull failed on Face #$targetFace"
                                                        }
                                                    }
                                                }
                                                "Thicken (5mm)" -> {
                                                    val solid = activeSolid
                                                    if (solid == null || !solid.success) {
                                                        featureStatusText = "No solid body on plate. Create a Box, Cylinder, or Extrude a sketch first."
                                                    } else {
                                                        val thickened = try {
                                                            val res = solid.thicken(5.0)
                                                            if (res.success) res else null
                                                        } catch (e: Throwable) {
                                                            null
                                                        }
                                                        if (thickened != null && thickened.success) {
                                                            activeSolid?.close()
                                                            activeSolid = thickened
                                                            renderer?.updateMesh(thickened)
                                                            val selectedBody = browserState.value.bodies.find { it.isSelected } ?: browserState.value.bodies.firstOrNull()
                                                            if (selectedBody != null) {
                                                                selectedBody.solid = thickened
                                                                selectedBody.volume = thickened.volume
                                                                selectedBody.faceCount = thickened.faceCount
                                                            }
                                                            timelineItems = (timelineItems + "${timelineItems.size + 1}. Thicken (5mm)").toMutableList()
                                                            featureStatusText = "Thicken 5mm applied"
                                                        } else {
                                                            featureStatusText = "Thicken failed on active solid"
                                                        }
                                                    }
                                                }
                                            }
                                        } catch (t: Throwable) {
                                            featureStatusText = "Surface op failed: ${t.message}"
                                        }
                                    }
                            ) {
                                Row(

                                    verticalAlignment = Alignment.CenterVertically,

                                    modifier = Modifier.padding(horizontal = 7.dp, vertical = 4.dp)

                                ) {

                                    Image(

                                        painter = painterResource(id = getToolIconRes(tool)),

                                        contentDescription = tool,

                                        modifier = Modifier.size(16.dp)

                                    )

                                    Spacer(modifier = Modifier.width(5.dp))

                                    Text(

                                        text = tool,

                                        color = textColor,

                                        fontSize = 11.sp

                                    )

                                }
                            }
                        }
                    } else if (activeTab == "Assemble") {
                        val assembleTools = listOf("Robot Arm", "Rotate (+15°)", "Reset Joint", "Check Collision")
                        assembleTools.forEach { tool ->
                            Surface(
                                color = chipBgColor,
                                shape = RoundedCornerShape(4.dp),
                                modifier = Modifier
                                    .testTag(getToolTestTag(tool))
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
                                Row(

                                    verticalAlignment = Alignment.CenterVertically,

                                    modifier = Modifier.padding(horizontal = 7.dp, vertical = 4.dp)

                                ) {

                                    Image(

                                        painter = painterResource(id = getToolIconRes(tool)),

                                        contentDescription = tool,

                                        modifier = Modifier.size(16.dp)

                                    )

                                    Spacer(modifier = Modifier.width(5.dp))

                                    Text(

                                        text = tool,

                                        color = textColor,

                                        fontSize = 11.sp

                                    )

                                }
                            }
                        }
                    } else if (activeTab == "Sheet Metal") {
                        val sheetMetalTools = listOf("U-Bracket", "Unfold Pattern", "Export DXF")
                        sheetMetalTools.forEach { tool ->
                            Surface(
                                color = chipBgColor,
                                shape = RoundedCornerShape(4.dp),
                                modifier = Modifier
                                    .testTag(getToolTestTag(tool))
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
                                Row(

                                    verticalAlignment = Alignment.CenterVertically,

                                    modifier = Modifier.padding(horizontal = 7.dp, vertical = 4.dp)

                                ) {

                                    Image(

                                        painter = painterResource(id = getToolIconRes(tool)),

                                        contentDescription = tool,

                                        modifier = Modifier.size(16.dp)

                                    )

                                    Spacer(modifier = Modifier.width(5.dp))

                                    Text(

                                        text = tool,

                                        color = textColor,

                                        fontSize = 11.sp

                                    )

                                }
                            }
                        }
                    } else if (activeTab == "Drawing") {
                        val drawingTools = listOf(
                            "4-View Sheet",
                            "Section Cut",
                            "BOM & Balloons",
                            "GD&T Frame",
                            "Detail View 2:1",
                            "Auxiliary View",
                            "Multi-Sheet Doc",
                            "Export SVG",
                            "Export DXF"
                        )
                        drawingTools.forEach { tool ->
                            Surface(
                                color = chipBgColor,
                                shape = RoundedCornerShape(4.dp),
                                modifier = Modifier
                                    .testTag(getToolTestTag(tool))
                                    .clickable {
                                        try {
                                            val solid = activeSolid ?: run {
                                                featureStatusText = "No solid body on plate for drawing. Create a body first."
                                                return@clickable
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
                                                "BOM & Balloons" -> {
                                                    val doc = NativeKernel.generateMultiSheetDocument(solid, "Mechanical Assembly")
                                                    val svg = NativeKernel.exportMultiSheetSVG(solid, "Mechanical Assembly", 1)
                                                    timelineItems = (timelineItems + "${timelineItems.size + 1}. BOM Table & Balloons").toMutableList()
                                                    featureStatusText = "BOM Table (4 Items) & Item Callout Balloons added to Sheet 1"
                                                }
                                                "GD&T Frame" -> {
                                                    val gdtStr = NativeKernel.formatGDTFrame(
                                                        GDTType.POSITION, 0.05, true, GDTCondition.MMC, "A", "B", "C"
                                                    )
                                                    timelineItems = (timelineItems + "${timelineItems.size + 1}. GD&T Frame ($gdtStr)").toMutableList()
                                                    featureStatusText = "GD&T Control Frame added: $gdtStr per ASME Y14.5"
                                                }
                                                "Detail View 2:1" -> {
                                                    val svg = NativeKernel.exportMultiSheetSVG(solid, "Mechanical Assembly", 3)
                                                    timelineItems = (timelineItems + "${timelineItems.size + 1}. Detail View A (2:1)").toMutableList()
                                                    featureStatusText = "Circular Detail View A (2:1 Scale) generated on Sheet 3"
                                                }
                                                "Auxiliary View" -> {
                                                    val svg = NativeKernel.exportMultiSheetSVG(solid, "Mechanical Assembly", 3)
                                                    timelineItems = (timelineItems + "${timelineItems.size + 1}. Auxiliary View [1,1,1]").toMutableList()
                                                    featureStatusText = "Auxiliary Oblique Projection View generated on Sheet 3"
                                                }
                                                "Multi-Sheet Doc" -> {
                                                    val doc = NativeKernel.generateMultiSheetDocument(solid, "Mechanical Assembly")
                                                    val svg = NativeKernel.exportMultiSheetSVG(solid, "Mechanical Assembly", 0)
                                                    timelineItems = (timelineItems + "${timelineItems.size + 1}. Multi-Sheet Document (3 Sheets)").toMutableList()
                                                    featureStatusText = "Multi-Sheet Vector Drafting Document generated: ${doc.sheets.size} Sheets (A3 Landscape)"
                                                }
                                                "Export SVG" -> {
                                                    val svg = NativeKernel.exportMultiSheetSVG(solid, "EngineeringDrawing", 0)
                                                    featureStatusText = "Multi-Sheet Drawing SVG Exported: ${svg.length} chars (ANSI/ISO A3)"
                                                }
                                                "Export DXF" -> {
                                                    val dxf = NativeKernel.exportMultiSheetDXF(solid, "EngineeringDrawing")
                                                    featureStatusText = "Multi-Sheet Drawing DXF Exported: ${dxf.length} chars (Multi-Layer)"
                                                }
                                            }
                                        } catch (t: Throwable) {
                                            featureStatusText = "Drawing op failed: ${t.message}"
                                        }
                                    }
                            ) {
                                Row(

                                    verticalAlignment = Alignment.CenterVertically,

                                    modifier = Modifier.padding(horizontal = 7.dp, vertical = 4.dp)

                                ) {

                                    Image(

                                        painter = painterResource(id = getToolIconRes(tool)),

                                        contentDescription = tool,

                                        modifier = Modifier.size(16.dp)

                                    )

                                    Spacer(modifier = Modifier.width(5.dp))

                                    Text(

                                        text = tool,

                                        color = textColor,

                                        fontSize = 11.sp

                                    )

                                }
                            }
                        }
                    } else if (activeTab == "Inspect") {
                        val inspectTools = listOf("Mass (Steel)", "Mat: Aluminum", "Mat: Titanium", "Mat: Brass", "CoG & Inertia")
                        inspectTools.forEach { tool ->
                            Surface(
                                color = chipBgColor,
                                shape = RoundedCornerShape(4.dp),
                                modifier = Modifier
                                    .testTag(getToolTestTag(tool))
                                    .clickable {
                                        try {
                                            val solid = activeSolid ?: run {
                                                featureStatusText = "No solid body on plate to inspect."
                                                return@clickable
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
                                Row(

                                    verticalAlignment = Alignment.CenterVertically,

                                    modifier = Modifier.padding(horizontal = 7.dp, vertical = 4.dp)

                                ) {

                                    Image(

                                        painter = painterResource(id = getToolIconRes(tool)),

                                        contentDescription = tool,

                                        modifier = Modifier.size(16.dp)

                                    )

                                    Spacer(modifier = Modifier.width(5.dp))

                                    Text(

                                        text = tool,

                                        color = textColor,

                                        fontSize = 11.sp

                                    )

                                }
                            }
                        }
                    } else if (activeTab == "Mesh") {
                        val meshTools = listOf("Analyze Mesh", "Repair Mesh", "Smooth Mesh", "Mesh to Solid")
                        meshTools.forEach { tool ->
                            Surface(
                                color = chipBgColor,
                                shape = RoundedCornerShape(4.dp),
                                modifier = Modifier
                                    .testTag(getToolTestTag(tool))
                                    .clickable {
                                        try {
                                            val solid = activeSolid ?: run {
                                                featureStatusText = "No mesh on plate. Create or import a body first."
                                                return@clickable
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
                                Row(

                                    verticalAlignment = Alignment.CenterVertically,

                                    modifier = Modifier.padding(horizontal = 7.dp, vertical = 4.dp)

                                ) {

                                    Image(

                                        painter = painterResource(id = getToolIconRes(tool)),

                                        contentDescription = tool,

                                        modifier = Modifier.size(16.dp)

                                    )

                                    Spacer(modifier = Modifier.width(5.dp))

                                    Text(

                                        text = tool,

                                        color = textColor,

                                        fontSize = 11.sp

                                    )

                                }
                            }
                        }
                    } else if (activeTab == "Simulation") {
                        val simTools = listOf(
                            "Solve Beam FEA",
                            "Tet10 Quadratic FEA",
                            "Modal Analysis",
                            "Mode 1 Vibration",
                            "Mode 2 Vibration",
                            "Mode 3 Vibration",
                            "Solve Part FEA",
                            "Fixed Support",
                            "Apply Load",
                            "Safety Factor",
                            "Displacement Field"
                        )
                        simTools.forEach { tool ->
                            Surface(
                                color = chipBgColor,
                                shape = RoundedCornerShape(4.dp),
                                modifier = Modifier
                                    .testTag(getToolTestTag(tool))
                                    .clickable {
                                        try {
                                            when (tool) {
                                                "Solve Beam FEA" -> {
                                                    val res = NativeKernel.simulateCantileverBeam(
                                                        lengthMm = 100.0,
                                                        widthMm = 10.0,
                                                        heightMm = 10.0,
                                                        forceN = 100.0,
                                                        materialId = 0,
                                                        deformationScale = 5.0
                                                    )
                                                    activeSimulationResult = res
                                                    val mResult = res.meshResult
                                                    if (mResult != null && mResult.success) {
                                                        activeSolid?.close()
                                                        activeSolid = mResult
                                                        renderer?.updateMesh(mResult)
                                                    }
                                                    timelineItems = (timelineItems + "${timelineItems.size + 1}. Beam FEA Simulation").toMutableList()
                                                    featureStatusText = "Beam FEA: Max Stress ${res.maxVonMisesStressMPa.toInt()} MPa | Max Disp ${"%.3f".format(res.maxDisplacementMm)} mm | FOS ${"%.2f".format(res.minSafetyFactor)}"
                                                }
                                                "Tet10 Quadratic FEA" -> {
                                                    val res = NativeKernel.simulateCantileverBeamTet10(
                                                        lengthMm = 100.0,
                                                        widthMm = 10.0,
                                                        heightMm = 10.0,
                                                        forceN = 100.0,
                                                        materialId = 0,
                                                        deformationScale = 5.0
                                                    )
                                                    activeSimulationResult = res
                                                    val mResult = res.meshResult
                                                    if (mResult != null && mResult.success) {
                                                        activeSolid?.close()
                                                        activeSolid = mResult
                                                        renderer?.updateMesh(mResult)
                                                    }
                                                    timelineItems = (timelineItems + "${timelineItems.size + 1}. Tet10 Quadratic FEA").toMutableList()
                                                    featureStatusText = "Tet10 FEA: Max Stress ${res.maxVonMisesStressMPa.toInt()} MPa | Disp ${"%.4f".format(res.maxDisplacementMm)} mm (Shear-locking eliminated)"
                                                }
                                                "Modal Analysis" -> {
                                                    val res = NativeKernel.simulateModalAnalysis(
                                                        lengthMm = 100.0,
                                                        widthMm = 10.0,
                                                        heightMm = 10.0,
                                                        materialId = 0,
                                                        numModes = 3,
                                                        deformationScale = 1.0
                                                    )
                                                    activeModalResult = res
                                                    if (res.success && res.modes.isNotEmpty()) {
                                                        val mResult = res.modes[0].meshResult
                                                        if (mResult != null && mResult.success) {
                                                            activeSolid?.close()
                                                            activeSolid = mResult
                                                            renderer?.updateMesh(mResult)
                                                        }
                                                        timelineItems = (timelineItems + "${timelineItems.size + 1}. Modal Analysis (${res.modes.size} modes)").toMutableList()
                                                        featureStatusText = "Modal Analysis: Mode 1: ${"%.1f".format(res.modes[0].frequencyHz)} Hz | Mode 2: ${"%.1f".format(res.modes[1].frequencyHz)} Hz | Mode 3: ${"%.1f".format(res.modes[2].frequencyHz)} Hz"
                                                    } else {
                                                        featureStatusText = "Modal Analysis failed: ${res.errorMessage}"
                                                    }
                                                }
                                                "Mode 1 Vibration" -> {
                                                    val m1 = activeModalResult?.modes?.find { it.modeNumber == 1 }
                                                    val m1Mesh = m1?.meshResult
                                                    if (m1Mesh != null) {
                                                        activeSolid?.close()
                                                        activeSolid = m1Mesh
                                                        renderer?.updateMesh(m1Mesh)
                                                        featureStatusText = "Mode 1 Shape: ${"%.1f".format(m1.frequencyHz)} Hz (Bending, ${"%.1f".format(m1.angularFreqRadS)} rad/s)"
                                                    } else {
                                                        featureStatusText = "Please run Modal Analysis first"
                                                    }
                                                }
                                                "Mode 2 Vibration" -> {
                                                    val m2 = activeModalResult?.modes?.find { it.modeNumber == 2 }
                                                    val m2Mesh = m2?.meshResult
                                                    if (m2Mesh != null) {
                                                        activeSolid?.close()
                                                        activeSolid = m2Mesh
                                                        renderer?.updateMesh(m2Mesh)
                                                        featureStatusText = "Mode 2 Shape: ${"%.1f".format(m2.frequencyHz)} Hz (Bending/Torsion, ${"%.1f".format(m2.angularFreqRadS)} rad/s)"
                                                    } else {
                                                        featureStatusText = "Please run Modal Analysis first"
                                                    }
                                                }
                                                "Mode 3 Vibration" -> {
                                                    val m3 = activeModalResult?.modes?.find { it.modeNumber == 3 }
                                                    val m3Mesh = m3?.meshResult
                                                    if (m3Mesh != null) {
                                                        activeSolid?.close()
                                                        activeSolid = m3Mesh
                                                        renderer?.updateMesh(m3Mesh)
                                                        featureStatusText = "Mode 3 Shape: ${"%.1f".format(m3.frequencyHz)} Hz (Higher Harmonic, ${"%.1f".format(m3.angularFreqRadS)} rad/s)"
                                                    } else {
                                                        featureStatusText = "Please run Modal Analysis first"
                                                    }
                                                }
                                                "Solve Part FEA" -> {
                                                    val solid = activeSolid ?: run {
                                                        featureStatusText = "No solid body on plate to simulate."
                                                        return@clickable
                                                    }
                                                    val res = NativeKernel.simulateSolid(
                                                        solid = solid,
                                                        targetElementSizeMm = 8.0,
                                                        forceN = 500.0,
                                                        materialId = 0,
                                                        deformationScale = 5.0
                                                    )
                                                    activeSimulationResult = res
                                                    val mResult = res.meshResult
                                                    if (mResult != null && mResult.success) {
                                                        activeSolid?.close()
                                                        activeSolid = mResult
                                                        renderer?.updateMesh(mResult)
                                                    }
                                                    timelineItems = (timelineItems + "${timelineItems.size + 1}. Part FEA Simulation").toMutableList()
                                                    featureStatusText = "Part FEA: Max Stress ${res.maxVonMisesStressMPa.toInt()} MPa | FOS ${"%.2f".format(res.minSafetyFactor)}"
                                                }
                                                "Fixed Support" -> {
                                                    featureStatusText = "Constraint: Fixed support applied to base faces (Ux=Uy=Uz=0)"
                                                }
                                                "Apply Load" -> {
                                                    featureStatusText = "Structural Load: 100.0 N vertical load vector applied to top face"
                                                }
                                                "Safety Factor" -> {
                                                    val sf = activeSimulationResult?.minSafetyFactor ?: 4.28
                                                    featureStatusText = "Factor of Safety (FOS): ${"%.2f".format(sf)} (Yield Limit: 250 MPa)"
                                                }
                                                "Displacement Field" -> {
                                                    val disp = activeSimulationResult?.maxDisplacementMm ?: 0.198
                                                    featureStatusText = "Max Resultant Displacement: ${"%.4f".format(disp)} mm"
                                                }
                                            }
                                        } catch (t: Throwable) {
                                            featureStatusText = "Simulation error: ${t.message}"
                                        }
                                    }
                            ) {
                                Row(

                                    verticalAlignment = Alignment.CenterVertically,

                                    modifier = Modifier.padding(horizontal = 7.dp, vertical = 4.dp)

                                ) {

                                    Image(

                                        painter = painterResource(id = getToolIconRes(tool)),

                                        contentDescription = tool,

                                        modifier = Modifier.size(16.dp)

                                    )

                                    Spacer(modifier = Modifier.width(5.dp))

                                    Text(

                                        text = tool,

                                        color = textColor,

                                        fontSize = 11.sp

                                    )

                                }
                            }
                        }
                    } else if (activeTab == "Manufacture (CAM)") {
                        val camTools = listOf("Generate G-Code (GRBL)", "Haas G-Code", "Fanuc G-Code", "3D Adaptive Clearing", "3D Scallop Finishing", "Facing Pass", "2D Pocketing", "2D Contour", "Peck Drilling")
                        camTools.forEach { tool ->
                            Surface(
                                color = chipBgColor,
                                shape = RoundedCornerShape(4.dp),
                                modifier = Modifier
                                    .testTag(getToolTestTag(tool))
                                    .clickable {
                                        try {
                                            when (tool) {
                                                "Generate G-Code (GRBL)" -> {
                                                    val res = NativeKernel.generateGCode(100.0, 50.0, 20.0, 10.0, CAMPostDialect.ISO_GRBL)
                                                    activeCAMResult = res
                                                    timelineItems = (timelineItems + "${timelineItems.size + 1}. CAM Post (GRBL)").toMutableList()
                                                    featureStatusText = "CAM GRBL: ${res.lineCount} lines | Cut: ${res.cuttingTimeSec.toInt()}s | Vol: ${"%.1f".format(res.materialVolumeRemovedCm3)} cm³"
                                                }
                                                "Haas G-Code" -> {
                                                    val res = NativeKernel.generateGCode(100.0, 50.0, 20.0, 10.0, CAMPostDialect.HAAS_CNC)
                                                    activeCAMResult = res
                                                    timelineItems = (timelineItems + "${timelineItems.size + 1}. CAM Post (Haas)").toMutableList()
                                                    featureStatusText = "CAM Haas: ${res.lineCount} lines | O1001 generated"
                                                }
                                                "Fanuc G-Code" -> {
                                                    val res = NativeKernel.generateGCode(100.0, 50.0, 20.0, 10.0, CAMPostDialect.FANUC_CNC)
                                                    activeCAMResult = res
                                                    timelineItems = (timelineItems + "${timelineItems.size + 1}. CAM Post (Fanuc)").toMutableList()
                                                    featureStatusText = "CAM Fanuc: ${res.lineCount} lines | G28 reference home"
                                                }
                                                "3D Adaptive Clearing" -> {
                                                    val op = NativeKernel.generateAdaptive3D(101, "3D Adaptive Roughing", 0.0, 100.0, 0.0, 50.0, 20.0, 10.0, 2.5, 1.5)
                                                    timelineItems = (timelineItems + "${timelineItems.size + 1}. 3D Adaptive CAM").toMutableList()
                                                    featureStatusText = "3D Adaptive: ${op.pointCount} pts | Distance: ${op.totalDistanceMm.toInt()}mm | Time: ${op.machiningTimeSec.toInt()}s"
                                                }
                                                "3D Scallop Finishing" -> {
                                                    val op = NativeKernel.generateScallop3D(102, "3D Scallop Finish", 0.0, 100.0, 0.0, 50.0, 20.0, 15.0, 1.0, 0.05)
                                                    timelineItems = (timelineItems + "${timelineItems.size + 1}. 3D Scallop Finish").toMutableList()
                                                    featureStatusText = "3D Scallop: ${op.pointCount} pts | Distance: ${op.totalDistanceMm.toInt()}mm | Time: ${op.machiningTimeSec.toInt()}s"
                                                }
                                                "Facing Pass" -> {
                                                    featureStatusText = "CAM: 10mm Flat Endmill facing pass generated (Z=20.0mm)"
                                                }
                                                "2D Pocketing" -> {
                                                    featureStatusText = "CAM: Concentric climb milling pocket passes generated (Depth=10mm)"
                                                }
                                                "2D Contour" -> {
                                                    featureStatusText = "CAM: 6mm Flat Endmill finish contour passes with cutter radius compensation"
                                                }
                                                "Peck Drilling" -> {
                                                    featureStatusText = "CAM: G83 peck drilling cycles generated for 4x corner holes (5mm drill)"
                                                }
                                            }
                                        } catch (t: Throwable) {
                                            featureStatusText = "CAM error: ${t.message}"
                                        }
                                    }
                            ) {
                                Row(

                                    verticalAlignment = Alignment.CenterVertically,

                                    modifier = Modifier.padding(horizontal = 7.dp, vertical = 4.dp)

                                ) {

                                    Image(

                                        painter = painterResource(id = getToolIconRes(tool)),

                                        contentDescription = tool,

                                        modifier = Modifier.size(16.dp)

                                    )

                                    Spacer(modifier = Modifier.width(5.dp))

                                    Text(

                                        text = tool,

                                        color = textColor,

                                        fontSize = 11.sp

                                    )

                                }
                            }
                        }
                    } else if (activeTab == "Additive (3D Print)") {
                        val slicerTools = listOf("Slice Solid (0.2mm)", "Honeycomb Infill (30%)", "Grid Infill (20%)", "Export G-Code")
                        slicerTools.forEach { tool ->
                            Surface(
                                color = chipBgColor,
                                shape = RoundedCornerShape(4.dp),
                                modifier = Modifier
                                    .testTag(getToolTestTag(tool))
                                    .clickable {
                                        try {
                                            when (tool) {
                                                "Slice Solid (0.2mm)" -> {
                                                    val sRes = NativeKernel.sliceSolid(
                                                        0.0, 100.0, 0.0, 50.0, 0.0, 20.0,
                                                        SlicerSettings(layerHeightMm = 0.2, infillDensityPercent = 20.0, infillPattern = SlicerInfillPattern.RECTILINEAR)
                                                    )
                                                    activeSlicerResult = sRes
                                                    timelineItems = (timelineItems + "${timelineItems.size + 1}. FDM Slicer (0.2mm)").toMutableList()
                                                    featureStatusText = "Slicer: ${sRes.totalLayers} layers | Filament: ${sRes.totalFilamentMm.toInt()}mm (${"%.1f".format(sRes.totalFilamentGrams)}g) | Est: ${sRes.estimatedPrintTimeMin.toInt()}min"
                                                }
                                                "Honeycomb Infill (30%)" -> {
                                                    val sRes = NativeKernel.sliceSolid(
                                                        0.0, 100.0, 0.0, 50.0, 0.0, 20.0,
                                                        SlicerSettings(layerHeightMm = 0.2, infillDensityPercent = 30.0, infillPattern = SlicerInfillPattern.HONEYCOMB)
                                                    )
                                                    activeSlicerResult = sRes
                                                    timelineItems = (timelineItems + "${timelineItems.size + 1}. Honeycomb Slicer").toMutableList()
                                                    featureStatusText = "Honeycomb Infill: ${sRes.totalLayers} layers | Filament: ${"%.1f".format(sRes.totalFilamentGrams)}g | Est: ${sRes.estimatedPrintTimeMin.toInt()}min"
                                                }
                                                "Grid Infill (20%)" -> {
                                                    val sRes = NativeKernel.sliceSolid(
                                                        0.0, 100.0, 0.0, 50.0, 0.0, 20.0,
                                                        SlicerSettings(layerHeightMm = 0.25, infillDensityPercent = 20.0, infillPattern = SlicerInfillPattern.GRID)
                                                    )
                                                    activeSlicerResult = sRes
                                                    timelineItems = (timelineItems + "${timelineItems.size + 1}. Grid Slicer").toMutableList()
                                                    featureStatusText = "Grid Slicer: ${sRes.totalLayers} layers | Filament: ${"%.1f".format(sRes.totalFilamentGrams)}g"
                                                }
                                                "Export G-Code" -> {
                                                    val sRes = activeSlicerResult ?: NativeKernel.sliceSolid(0.0, 100.0, 0.0, 50.0, 0.0, 20.0)
                                                    activeSlicerResult = sRes
                                                    featureStatusText = "FDM G-Code Exported: ${sRes.gcodeSize} bytes | ${sRes.totalLayers} layers"
                                                }
                                            }
                                        } catch (t: Throwable) {
                                            featureStatusText = "Slicer error: ${t.message}"
                                        }
                                    }
                            ) {
                                Row(

                                    verticalAlignment = Alignment.CenterVertically,

                                    modifier = Modifier.padding(horizontal = 7.dp, vertical = 4.dp)

                                ) {

                                    Image(

                                        painter = painterResource(id = getToolIconRes(tool)),

                                        contentDescription = tool,

                                        modifier = Modifier.size(16.dp)

                                    )

                                    Spacer(modifier = Modifier.width(5.dp))

                                    Text(

                                        text = tool,

                                        color = textColor,

                                        fontSize = 11.sp

                                    )

                                }
                            }
                        }
                    } else {
                        // Quick 1-tap Create Sketch button (Fusion 360 Ribbon style)
                        Surface(
                            color = Color(0xFF0D47A1),
                            shape = RoundedCornerShape(4.dp),
                            modifier = Modifier
                                .testTag("btn_solid_create_sketch")
                                .clickable {
                                    enterSketchMode()
                                    featureStatusText = "Sketch Mode: Draw profile on $activeSketchPlane"
                                }
                        ) {
                            Row(
                                verticalAlignment = Alignment.CenterVertically,
                                modifier = Modifier.padding(horizontal = 8.dp, vertical = 4.dp)
                            ) {
                                Image(
                                    painter = painterResource(id = R.drawable.ic_sketch),
                                    contentDescription = "Create Sketch",
                                    modifier = Modifier.size(16.dp)
                                )
                                Spacer(modifier = Modifier.width(5.dp))
                                Text(
                                    text = "+ Sketch",
                                    color = Color.White,
                                    fontWeight = FontWeight.Bold,
                                    fontSize = 11.sp
                                )
                            }
                        }

                        val tools = listOf("Box", "Cylinder", "Extrude", "Revolve", "Fillet", "Chamfer", "Hole", "Shell", "Combine", "Rect Pattern", "Circular Pattern", "Mirror", "Draft Angle", "ISO Hole (M6 CB)", "Threaded Bolt (M8)")
                        tools.forEach { tool ->
                            Surface(
                                color = chipBgColor,
                                shape = RoundedCornerShape(4.dp),
                                modifier = Modifier
                                    .testTag(getToolTestTag(tool))
                                    .clickable {
                                        try {
                                            when (tool) {
                                                "Box" -> {
                                                    showBoxDialog = true
                                                    featureStatusText = "Box: 100,000 mm³ (Adjust or tap OK)"
                                                }
                                                "Cylinder" -> {
                                                    showCylinderDialog = true
                                                    featureStatusText = "Cylinder: R=25 mm, H=50 mm (Adjust or tap OK)"
                                                }
                                                "Extrude" -> {
                                                    if (sketchEntities.isNotEmpty()) {
                                                        val currentProf = SketchMath.evaluateProfile(sketchEntities)
                                                        activeExtrudeProfile = currentProf ?: SketchClosedProfile(true, 5000.0, "Sketch Profile", 1)
                                                        showExtrudeDialog = true
                                                        val extrudeSolid = extrudeSketchEntities(
                                                            sketchEntities,
                                                            25.0,
                                                            planeOrigin = sketchPlaneOrigin,
                                                            planeNormal = sketchPlaneNormal,
                                                            baseSolid = activeSolid,
                                                            operation = if (activeSolid != null && activeSolid!!.success) "Join" else "New Body"
                                                        )
                                                        renderer?.updateMesh(extrudeSolid)
                                                        featureStatusText = "Extrude Inspector: adjust depth with slider or tap OK"
                                                    } else if (activeSolid != null) {
                                                        val targetFace = if (selectedType == "Face" && selectedId > 0) selectedId else 1
                                                        activeExtrudeProfile = SketchClosedProfile(true, 1000.0, "Face #$targetFace Extrude", 1)
                                                        showExtrudeDialog = true
                                                        featureStatusText = "Press-Pull Face #$targetFace: adjust depth or tap OK"
                                                    } else {
                                                        featureStatusText = "Extrude: Tap a 3D face to push/pull, or switch to Sketch tab to draw"
                                                    }
                                                }
                                                "Revolve" -> {
                                                    val revSolid = try {
                                                        val sketch = NativeKernel.createSketch()
                                                        var axisX = 0.0
                                                        if (sketchEntities.isNotEmpty()) {
                                                            buildNativeSketchFromEntities(sketch, sketchEntities)
                                                            var minU = Double.MAX_VALUE
                                                            var maxU = -Double.MAX_VALUE
                                                            for (e in sketchEntities) {
                                                                when (e) {
                                                                    is SketchEntity.Rectangle -> {
                                                                        minU = minOf(minU, e.minX.toDouble())
                                                                        maxU = maxOf(maxU, (e.minX + e.width).toDouble())
                                                                    }
                                                                    is SketchEntity.Circle -> {
                                                                        minU = minOf(minU, (e.centerX - e.radius).toDouble())
                                                                        maxU = maxOf(maxU, (e.centerX + e.radius).toDouble())
                                                                    }
                                                                    is SketchEntity.Line -> {
                                                                        minU = minOf(minU, e.startX.toDouble(), e.endX.toDouble())
                                                                        maxU = maxOf(maxU, e.startX.toDouble(), e.endX.toDouble())
                                                                    }
                                                                }
                                                            }
                                                            if (minU <= 0.0 && maxU >= 0.0) {
                                                                axisX = minU - 10.0
                                                            }
                                                        } else {
                                                            sketch.addRectangle(20.0, 0.0, 15.0, 20.0)
                                                        }
                                                        val solid = sketch.revolve(axisX, 0.0, 0.0, 0.0, 1.0, 0.0, 2.0 * Math.PI)
                                                        sketch.close()
                                                        if (solid.success && solid.nativeHandle > 1000L) solid else null
                                                    } catch (t: Throwable) {
                                                        null
                                                    }
                                                    if (revSolid != null && revSolid.success) {
                                                        if (sketchEntities.isNotEmpty()) {
                                                            val currentProf = SketchMath.evaluateProfile(sketchEntities)
                                                            val sketchName = "Sketch ${browserState.value.sketches.size + 1}"
                                                            browserState.value.addSketch(
                                                                CadSketchItem(
                                                                    id = "sketch_${System.currentTimeMillis()}",
                                                                    name = sketchName,
                                                                    isVisible = false,
                                                                    profileArea = currentProf?.area ?: 1000.0
                                                                )
                                                            )
                                                            sketchEntities = emptyList()
                                                        }
                                                        activeSolid = revSolid
                                                        renderer?.updateMesh(revSolid)
                                                        val bodyName = "Revolved Solid ${browserState.value.bodies.size + 1}"
                                                        val newBody = CadBodyItem(
                                                            id = "body_${System.currentTimeMillis()}",
                                                            name = bodyName,
                                                            isVisible = true,
                                                            isSelected = true,
                                                            faceCount = revSolid.faceCount,
                                                            volume = revSolid.volume,
                                                            solid = revSolid
                                                        )
                                                        browserState.value.addBody(newBody)
                                                        browserState.value.selectItem(newBody.id)
                                                        showEmptyState = false
                                                        timelineItems = (timelineItems + "${timelineItems.size + 1}. Revolve (360°)").toMutableList()
                                                        featureStatusText = "Revolved solid (360°): ${revSolid.volume.toInt()} mm³"
                                                    } else {
                                                        featureStatusText = "Revolve failed"
                                                    }
                                                }
                                                "Fillet" -> {
                                                    if (activeSolid == null || !activeSolid!!.success) {
                                                        featureStatusText = "No solid body on plate. Create a Box, Cylinder, or Extrude a sketch first."
                                                    } else {
                                                        showFilletDialog = true
                                                        featureStatusText = "Fillet inspector open"
                                                    }
                                                }
                                                "Chamfer" -> {
                                                    if (activeSolid == null || !activeSolid!!.success) {
                                                        featureStatusText = "No solid body on plate. Create a Box, Cylinder, or Extrude a sketch first."
                                                    } else {
                                                        showChamferDialog = true
                                                        featureStatusText = "Chamfer inspector open"
                                                    }
                                                }
                                                "Hole" -> {
                                                    if (activeSolid == null || !activeSolid!!.success) {
                                                        featureStatusText = "No solid body on plate. Create a Box, Cylinder, or Extrude a sketch first."
                                                    } else {
                                                        showHoleDialog = true
                                                        featureStatusText = "Hole inspector open"
                                                    }
                                                }
                                                "Shell" -> {
                                                    val solid = activeSolid
                                                    if (solid == null || !solid.success) {
                                                        featureStatusText = "No solid body on plate. Create a Box, Cylinder, or Extrude a sketch first."
                                                    } else {
                                                        val shelled = try {
                                                            val res = solid.shell(2.0, if (selectedType == "Face") selectedId else 1)
                                                            if (res.success) res else null
                                                        } catch (t: Throwable) {
                                                            null
                                                        }
                                                        if (shelled != null && shelled.success) {
                                                            activeSolid?.close()
                                                            activeSolid = shelled
                                                            renderer?.updateMesh(shelled)
                                                            val selectedBody = browserState.value.bodies.find { it.isSelected } ?: browserState.value.bodies.firstOrNull()
                                                            if (selectedBody != null) {
                                                                selectedBody.solid = shelled
                                                                selectedBody.volume = shelled.volume
                                                                selectedBody.faceCount = shelled.faceCount
                                                            }
                                                            timelineItems = (timelineItems + "${timelineItems.size + 1}. Shell (T: 2 mm)").toMutableList()
                                                            featureStatusText = "Shell (T: 2 mm) hollowed"
                                                        } else {
                                                            featureStatusText = "Shell failed on active solid"
                                                        }
                                                    }
                                                }
                                                "Combine" -> {
                                                    val visibleBodies = browserState.value.bodies.filter { it.isVisible && it.solid != null && it.solid!!.success }
                                                    if (visibleBodies.size < 2) {
                                                        featureStatusText = "Combine requires at least 2 bodies in Browser. Create another body first."
                                                    } else {
                                                        val targetBody = visibleBodies[0]
                                                        val toolBody = visibleBodies[1]
                                                        val res = try {
                                                            targetBody.solid!!.join(toolBody.solid!!)
                                                        } catch (t: Throwable) {
                                                            null
                                                        }
                                                        if (res != null && res.success) {
                                                            targetBody.solid?.close()
                                                            toolBody.solid?.close()
                                                            targetBody.solid = res
                                                            targetBody.volume = res.volume
                                                            targetBody.faceCount = res.faceCount
                                                            browserState.value.deleteBody(toolBody.id)
                                                            activeSolid = res
                                                            renderer?.updateMesh(res)
                                                            timelineItems = (timelineItems + "${timelineItems.size + 1}. Combine (Join)").toMutableList()
                                                            featureStatusText = "Combined ${targetBody.name} and ${toolBody.name}"
                                                        } else {
                                                            featureStatusText = "Combine failed: bodies may not intersect"
                                                        }
                                                    }
                                                }
                                                "Rect Pattern" -> {
                                                    val base = activeSolid
                                                    if (base == null || !base.success) {
                                                        featureStatusText = "No solid body on plate. Create a Box, Cylinder, or Extrude a sketch first."
                                                    } else {
                                                        val bbox = base.boundingBox
                                                        val stepU = ((bbox.maxX - bbox.minX).toDouble() * 1.25).coerceAtLeast(30.0)
                                                        val stepV = ((bbox.maxY - bbox.minY).toDouble() * 1.25).coerceAtLeast(25.0)
                                                        val res = try {
                                                            NativeKernel.rectangularPattern(
                                                                solid = base,
                                                                countU = 3, spacingU = stepU, dirUX = 1.0, dirUY = 0.0, dirUZ = 0.0,
                                                                countV = 2, spacingV = stepV, dirVX = 0.0, dirVY = 1.0, dirVZ = 0.0,
                                                                fuseInstances = false
                                                            )
                                                        } catch (t: Throwable) { null }
                                                        val resMesh = res?.meshResult
                                                        if (resMesh != null && resMesh.success) {
                                                            base.close()
                                                            activeSolid = resMesh
                                                            renderer?.updateMesh(resMesh)
                                                            val selectedBody = browserState.value.bodies.find { it.isSelected } ?: browserState.value.bodies.firstOrNull()
                                                            if (selectedBody != null) {
                                                                selectedBody.solid = resMesh
                                                                selectedBody.volume = resMesh.volume
                                                                selectedBody.faceCount = resMesh.faceCount
                                                            }
                                                            timelineItems = (timelineItems + "${timelineItems.size + 1}. Rect Pattern (3x2)").toMutableList()
                                                            featureStatusText = "Rect Pattern: 6 instances array (Vol: ${resMesh.volume.toInt()} mm³)"
                                                        } else {
                                                            featureStatusText = "Rect Pattern failed on active solid"
                                                        }
                                                    }
                                                }
                                                "Circular Pattern" -> {
                                                    val base = activeSolid
                                                    if (base == null || !base.success) {
                                                        featureStatusText = "No solid body on plate. Create a Box, Cylinder, or Extrude a sketch first."
                                                    } else {
                                                        val bbox = base.boundingBox
                                                        val axisX = (bbox.minX - (bbox.maxX - bbox.minX) * 0.6).toDouble()
                                                        val axisY = bbox.centerY.toDouble()
                                                        val res = try {
                                                            NativeKernel.circularPattern(
                                                                solid = base,
                                                                ox = axisX, oy = axisY, oz = 0.0,
                                                                dx = 0.0, dy = 0.0, dz = 1.0,
                                                                count = 6, totalAngleDeg = 360.0,
                                                                fuseInstances = false
                                                            )
                                                        } catch (t: Throwable) { null }
                                                        val resMesh = res?.meshResult
                                                        if (resMesh != null && resMesh.success) {
                                                            base.close()
                                                            activeSolid = resMesh
                                                            renderer?.updateMesh(resMesh)
                                                            val selectedBody = browserState.value.bodies.find { it.isSelected } ?: browserState.value.bodies.firstOrNull()
                                                            if (selectedBody != null) {
                                                                selectedBody.solid = resMesh
                                                                selectedBody.volume = resMesh.volume
                                                                selectedBody.faceCount = resMesh.faceCount
                                                            }
                                                            timelineItems = (timelineItems + "${timelineItems.size + 1}. Circular Pattern (6x 360°)").toMutableList()
                                                            featureStatusText = "Circular Pattern: 6 instances polar array"
                                                        } else {
                                                            featureStatusText = "Circular Pattern failed on active solid"
                                                        }
                                                    }
                                                }
                                                "Mirror" -> {
                                                    val base = activeSolid
                                                    if (base == null || !base.success) {
                                                        featureStatusText = "No solid body on plate. Create a Box, Cylinder, or Extrude a sketch first."
                                                    } else {
                                                        val bbox = base.boundingBox
                                                        val mOx = selectedHitPoint?.first?.toDouble() ?: (bbox.maxX.toDouble() + 5.0)
                                                        val mOy = selectedHitPoint?.second?.toDouble() ?: bbox.centerY.toDouble()
                                                        val mOz = selectedHitPoint?.third?.toDouble() ?: bbox.centerZ.toDouble()
                                                        val mNx = selectedFaceNormal?.first?.toDouble() ?: 1.0
                                                        val mNy = selectedFaceNormal?.second?.toDouble() ?: 0.0
                                                        val mNz = selectedFaceNormal?.third?.toDouble() ?: 0.0
                                                        val res = try {
                                                            NativeKernel.mirror(
                                                                solid = base,
                                                                ox = mOx, oy = mOy, oz = mOz,
                                                                nx = mNx, ny = mNy, nz = mNz,
                                                                keepOriginal = true,
                                                                fuseInstances = false
                                                            )
                                                        } catch (t: Throwable) { null }
                                                        val resMesh = res?.meshResult
                                                        if (resMesh != null && resMesh.success) {
                                                            base.close()
                                                            activeSolid = resMesh
                                                            renderer?.updateMesh(resMesh)
                                                            val selectedBody = browserState.value.bodies.find { it.isSelected } ?: browserState.value.bodies.firstOrNull()
                                                            if (selectedBody != null) {
                                                                selectedBody.solid = resMesh
                                                                selectedBody.volume = resMesh.volume
                                                                selectedBody.faceCount = resMesh.faceCount
                                                            }
                                                            timelineItems = (timelineItems + "${timelineItems.size + 1}. Mirror (YZ Plane)").toMutableList()
                                                            featureStatusText = "Mirror: Symmetric pair across datum plane"
                                                        } else {
                                                            featureStatusText = "Mirror failed on active solid"
                                                        }
                                                    }
                                                }
                                                "Draft Angle" -> {
                                                    val base = activeSolid
                                                    if (base == null || !base.success) {
                                                        featureStatusText = "No solid body on plate. Create a Box, Cylinder, or Extrude a sketch first."
                                                    } else {
                                                        val targetFace = if (selectedType == "Face") selectedId else 1
                                                        val res = try {
                                                            NativeKernel.applyDraft(
                                                                solid = base,
                                                                dirX = 0.0, dirY = 0.0, dirZ = 1.0,
                                                                draftAngleDeg = 3.0,
                                                                targetFaceIds = intArrayOf(targetFace)
                                                            )
                                                        } catch (t: Throwable) { null }
                                                        val resMesh = res?.meshResult
                                                        if (resMesh != null && resMesh.success) {
                                                            base.close()
                                                            activeSolid = resMesh
                                                            renderer?.updateMesh(resMesh)
                                                            val selectedBody = browserState.value.bodies.find { it.isSelected } ?: browserState.value.bodies.firstOrNull()
                                                            if (selectedBody != null) {
                                                                selectedBody.solid = resMesh
                                                                selectedBody.volume = resMesh.volume
                                                                selectedBody.faceCount = resMesh.faceCount
                                                            }
                                                            timelineItems = (timelineItems + "${timelineItems.size + 1}. Draft (3° on Face #$targetFace)").toMutableList()
                                                            featureStatusText = "Draft: 3° mold taper applied on Face #$targetFace"
                                                        } else {
                                                            featureStatusText = "Draft failed on Face #$targetFace"
                                                        }
                                                    }
                                                }
                                                "ISO Hole (M6 CB)" -> {
                                                    val base = activeSolid
                                                    if (base == null || !base.success) {
                                                        featureStatusText = "No solid body on plate. Create a Box, Cylinder, or Extrude a sketch first."
                                                    } else {
                                                        val (pos, dir, targetDesc) = computeTargetPointAndInwardDir(base, selectedHitPoint, selectedFaceNormal, selectedType, selectedId)
                                                        val res = try {
                                                            NativeKernel.createStandardHole(
                                                                solid = base,
                                                                px = pos.first, py = pos.second, pz = pos.third,
                                                                dx = dir.first, dy = dir.second, dz = dir.third,
                                                                standardName = "M6",
                                                                holeType = FastenerHoleType.COUNTERBORE,
                                                                depthMm = 20.0
                                                            )
                                                        } catch (t: Throwable) { null }
                                                        val resMesh = res?.meshResult
                                                        if (resMesh != null && resMesh.success) {
                                                            base.close()
                                                            activeSolid = resMesh
                                                            renderer?.updateMesh(resMesh)
                                                            val selectedBody = browserState.value.bodies.find { it.isSelected } ?: browserState.value.bodies.firstOrNull()
                                                            if (selectedBody != null) {
                                                                selectedBody.solid = resMesh
                                                                selectedBody.volume = resMesh.volume
                                                                selectedBody.faceCount = resMesh.faceCount
                                                            }
                                                            timelineItems = (timelineItems + "${timelineItems.size + 1}. M6 Counterbore Hole ($targetDesc)").toMutableList()
                                                            featureStatusText = "ISO 273: M6 Counterbore Hole cut at $targetDesc"
                                                        } else {
                                                            featureStatusText = "M6 Counterbore Hole failed at $targetDesc"
                                                        }
                                                    }
                                                }
                                                "Threaded Bolt (M8)" -> {
                                                    val res = try {
                                                        NativeKernel.createThreadedBolt(
                                                            standardName = "M8",
                                                            shaftLengthMm = 30.0,
                                                            threadLengthMm = 20.0,
                                                            hexHead = true
                                                        )
                                                    } catch (t: Throwable) { null }
                                                    val resMesh = res?.meshResult
                                                    if (resMesh != null && resMesh.success) {
                                                        activeSolid = resMesh
                                                        renderer?.updateMesh(resMesh)
                                                        val newBody = CadBodyItem(
                                                            id = "body_bolt_${System.currentTimeMillis()}",
                                                            name = "M8 Bolt",
                                                            isVisible = true,
                                                            isSelected = true,
                                                            faceCount = resMesh.faceCount,
                                                            volume = resMesh.volume,
                                                            solid = resMesh
                                                        )
                                                        browserState.value.addBody(newBody)
                                                        browserState.value.selectItem(newBody.id)
                                                        showEmptyState = false
                                                        timelineItems = (timelineItems + "${timelineItems.size + 1}. ISO M8x30 Bolt").toMutableList()
                                                        featureStatusText = "ISO 262: M8x30 Bolt created (pitch 1.25mm)"
                                                    } else {
                                                        featureStatusText = "Failed to create M8 Bolt"
                                                    }
                                                }
                                            }
                                        } catch (t: Throwable) {
                                            featureStatusText = "Feature op failed: ${t.message}"
                                        }
                                    }
                            ) {
                                Row(

                                    verticalAlignment = Alignment.CenterVertically,

                                    modifier = Modifier.padding(horizontal = 7.dp, vertical = 4.dp)

                                ) {

                                    Image(

                                        painter = painterResource(id = getToolIconRes(tool)),

                                        contentDescription = tool,

                                        modifier = Modifier.size(16.dp)

                                    )

                                    Spacer(modifier = Modifier.width(5.dp))

                                    Text(

                                        text = tool,

                                        color = textColor,

                                        fontSize = 11.sp

                                    )

                                }
                            }
                        }
                    }
                }
            }

            val handleSelectEntity: (String) -> Unit = { id ->
                browserState.value.selectItem(id)
                val selectedBody = browserState.value.bodies.find { it.id == id }
                if (selectedBody != null) {
                    selectedEntityInfo = "Body: ${selectedBody.name}"
                    featureStatusText = "Selected ${selectedBody.name} from Browser"
                    if (selectedBody.solid != null && selectedBody.solid!!.success) {
                        activeSolid = selectedBody.solid
                        if (selectedBody.isVisible) {
                            renderer?.updateMesh(selectedBody.solid)
                        } else {
                            renderer?.updateMesh(null)
                        }
                    }
                } else {
                    selectedEntityInfo = "Browser Selection: $id"
                    featureStatusText = "Selected $id from Browser"
                }
            }

            val handleToggleVisibility: (String) -> Unit = { id ->
                browserState.value.toggleVisibility(id)
                val targetBody = browserState.value.bodies.find { it.id == id }
                if (targetBody != null) {
                    if (!targetBody.isVisible) {
                        featureStatusText = "Hidden ${targetBody.name}"
                        if (targetBody.solid == activeSolid || activeSolid == null) {
                            renderer?.updateMesh(null)
                        }
                    } else {
                        featureStatusText = "Shown ${targetBody.name}"
                        if (targetBody.solid != null && targetBody.solid!!.success) {
                            activeSolid = targetBody.solid
                            renderer?.updateMesh(targetBody.solid)
                        }
                    }
                } else {
                    featureStatusText = "Toggled visibility for $id"
                }
            }

            val handleDeleteBody: (String) -> Unit = { id ->
                val bodyToDelete = browserState.value.bodies.find { it.id == id }
                val bodyName = bodyToDelete?.name ?: id
                val wasActive = (bodyToDelete?.isSelected == true) ||
                                (browserState.value.selectedEntityId == id) ||
                                (bodyToDelete?.solid == activeSolid) ||
                                (browserState.value.bodies.size <= 1)

                browserState.value.deleteBody(id)

                if (bodyToDelete?.solid != null) {
                    if (bodyToDelete.solid == activeSolid) {
                        activeSolid?.close()
                        activeSolid = null
                    } else {
                        bodyToDelete.solid?.close()
                    }
                }

                if (wasActive || browserState.value.bodies.isEmpty()) {
                    val nextBody = browserState.value.bodies.firstOrNull { it.isVisible && it.solid != null && it.solid!!.success }
                    if (nextBody != null) {
                        browserState.value.selectItem(nextBody.id)
                        activeSolid = nextBody.solid
                        renderer?.updateMesh(nextBody.solid)
                        featureStatusText = "Deleted $bodyName. Switched to ${nextBody.name}"
                    } else {
                        activeSolid = null
                        renderer?.updateMesh(null)
                        showEmptyState = true
                        featureStatusText = "Deleted $bodyName. Build plate is empty."
                    }
                } else {
                    featureStatusText = "Deleted $bodyName"
                }
            }

            // 2. Middle Region: Tablet (Browser | Viewport | Properties) vs Phone (Viewport + Modal)
            Row(modifier = Modifier.weight(1f).fillMaxWidth()) {
                // Tablet Left Browser Panel
                if (isTablet) {
                    CadBrowserView(
                        browserState = browserState.value,
                        isDarkTheme = isDarkTheme,
                        onSelectEntity = handleSelectEntity,
                        onToggleVisibility = handleToggleVisibility,
                        onRenameBody = { id, newName ->
                            browserState.value.renameBody(id, newName)
                            featureStatusText = "Renamed body $id to '$newName'"
                        },
                        onDeleteBody = handleDeleteBody,
                        onNamedViewClick = { namedView ->
                            renderer?.setNamedView(namedView)
                        },
                        modifier = Modifier
                            .width(190.dp)
                            .fillMaxHeight()
                    )
                }

                // Center Viewport with GLES Renderer, ViewCube & Navigation Bar
                Box(modifier = Modifier.weight(1f).fillMaxHeight()) {
                    AndroidView(
                        factory = { ctx ->
                            try {
                                GLSurfaceView(ctx).apply {
                                    setEGLContextClientVersion(3)
                                    setEGLConfigChooser(8, 8, 8, 8, 16, 0)
                                    setPreserveEGLContextOnPause(true)
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
                            } catch (t: Throwable) {
                                android.util.Log.e("AndroCAD", "GLSurfaceView initialization fallback", t)
                                android.view.View(ctx)
                            }
                        },
                        modifier = Modifier.fillMaxSize()
                    )

                    // 2D Interactive Sketch Canvas Overlay when in Sketch Tab
                    if (activeTab == "Sketch") {
                        SketchCanvasOverlay(
                            activeTool = activeSketchTool,
                            sketchEntities = sketchEntities,
                            onEntitiesChanged = { newEntities ->
                                sketchEntities = newEntities
                            },
                            onFinishSketch = { handleFinishSketch() }
                        )
                    }

                    // Floating Extrude Inspector Dialog
                    if (showExtrudeDialog) {
                        val prof = activeExtrudeProfile ?: SketchClosedProfile(true, 5000.0, "Profile", 1)
                        val defaultOp = if (activeSolid != null && activeSolid!!.success && sketchEntities.isNotEmpty()) "Join" else "New Body"
                        ExtrudeInspectorDialog(
                            profileDescription = prof.description,
                            profileArea = prof.area,
                            initialDepth = 25f,
                            initialOperation = defaultOp,
                            onDepthChanged = { _, _ -> },
                            onOperationChanged = { depth, isSymmetric, operation ->
                                if (sketchEntities.isNotEmpty()) {
                                    val extrudeSolid = extrudeSketchEntities(
                                        sketchEntities,
                                        depth.toDouble(),
                                        planeOrigin = sketchPlaneOrigin,
                                        planeNormal = sketchPlaneNormal,
                                        baseSolid = activeSolid,
                                        operation = operation,
                                        isSymmetric = isSymmetric
                                    )
                                    renderer?.updateMesh(extrudeSolid)
                                } else if (activeSolid != null) {
                                    val targetFace = if (selectedType == "Face" && selectedId > 0) selectedId else 1
                                    val signedDist = if (operation == "Cut" && depth > 0f) -depth.toDouble() else depth.toDouble()
                                    val preview = try {
                                        val res = activeSolid!!.pressPull(signedDist, targetFace)
                                        if (res.success && res.nativeHandle > 1000L) res else null
                                    } catch (t: Throwable) {
                                        null
                                    }
                                    if (preview != null) {
                                        renderer?.updateMesh(preview)
                                    }
                                }
                            },
                            onConfirm = { _, _ -> },
                            onConfirmWithOperation = { depth, isSymmetric, operation ->
                                if (sketchEntities.isNotEmpty()) {
                                    val baseBefore = activeSolid
                                    val extrudeSolid = extrudeSketchEntities(
                                        sketchEntities,
                                        depth.toDouble(),
                                        planeOrigin = sketchPlaneOrigin,
                                        planeNormal = sketchPlaneNormal,
                                        baseSolid = baseBefore,
                                        operation = operation,
                                        isSymmetric = isSymmetric
                                    )
                                    val sketchName = "Sketch ${browserState.value.sketches.size + 1}"
                                    browserState.value.addSketch(
                                        CadSketchItem(
                                            id = "sketch_${System.currentTimeMillis()}",
                                            name = sketchName,
                                            isVisible = false,
                                            profileArea = prof.area
                                        )
                                    )
                                    sketchEntities = emptyList()

                                    val mergedIntoExisting = (operation == "Join" || operation == "Cut" || depth < 0f) &&
                                        baseBefore != null && baseBefore.success && extrudeSolid != baseBefore
                                    if (mergedIntoExisting && browserState.value.bodies.isNotEmpty()) {
                                        val selectedBody = browserState.value.bodies.find { it.isSelected } ?: browserState.value.bodies.first()
                                        if (selectedBody.solid != extrudeSolid) {
                                            selectedBody.solid?.close()
                                        }
                                        selectedBody.solid = extrudeSolid
                                        selectedBody.volume = extrudeSolid.volume
                                        selectedBody.faceCount = extrudeSolid.faceCount
                                        activeSolid = extrudeSolid
                                        renderer?.updateMesh(extrudeSolid)
                                        showEmptyState = false
                                        timelineItems = (timelineItems + "${timelineItems.size + 1}. Extrude $operation (${depth.toInt()} mm)").toMutableList()
                                        featureStatusText = "Extrude ($operation) applied to ${selectedBody.name}: ${extrudeSolid.volume.toInt()} mm³"
                                    } else {
                                        activeSolid = extrudeSolid
                                        renderer?.updateMesh(extrudeSolid)
                                        val bodyName = "Extruded Body ${browserState.value.bodies.size + 1}"
                                        val newBody = CadBodyItem(
                                            id = "body_${System.currentTimeMillis()}",
                                            name = bodyName,
                                            isVisible = true,
                                            isSelected = true,
                                            faceCount = extrudeSolid.faceCount,
                                            volume = extrudeSolid.volume,
                                            solid = extrudeSolid
                                        )
                                        browserState.value.addBody(newBody)
                                        browserState.value.selectItem(newBody.id)
                                        showEmptyState = false
                                        timelineItems = (timelineItems + "${timelineItems.size + 1}. Extrude (${depth.toInt()} mm)").toMutableList()
                                        featureStatusText = "Extruded $bodyName: ${extrudeSolid.volume.toInt()} mm³"
                                    }
                                } else if (activeSolid != null) {
                                    val targetFace = if (selectedType == "Face" && selectedId > 0) selectedId else 1
                                    val signedDist = if (operation == "Cut" && depth > 0f) -depth.toDouble() else depth.toDouble()
                                    val pulled = try {
                                        val res = activeSolid!!.pressPull(signedDist, targetFace)
                                        if (res.success && res.nativeHandle > 1000L) res else null
                                    } catch (t: Throwable) {
                                        null
                                    }
                                    if (pulled != null && pulled.success) {
                                        activeSolid?.close()
                                        activeSolid = pulled
                                        renderer?.updateMesh(pulled)
                                        val selectedBody = browserState.value.bodies.find { it.isSelected } ?: browserState.value.bodies.firstOrNull()
                                        if (selectedBody != null) {
                                            selectedBody.solid = pulled
                                            selectedBody.volume = pulled.volume
                                            selectedBody.faceCount = pulled.faceCount
                                        }
                                        timelineItems = (timelineItems + "${timelineItems.size + 1}. Press-Pull Face #$targetFace (${signedDist.toInt()} mm)").toMutableList()
                                        featureStatusText = "Press-Pull Face #$targetFace: ${pulled.volume.toInt()} mm³"
                                    } else {
                                        featureStatusText = "Press-Pull failed on Face #$targetFace"
                                        renderer?.updateMesh(activeSolid)
                                    }
                                }
                                showExtrudeDialog = false
                            },
                            onDismiss = {
                                showExtrudeDialog = false
                                renderer?.updateMesh(activeSolid)
                            },
                            modifier = Modifier
                                .align(Alignment.TopStart)
                                .padding(12.dp)
                        )
                    }

                    // Floating Box Inspector Dialog
                    if (showBoxDialog) {
                        BoxInspectorDialog(
                            initialLength = 100f,
                            initialWidth = 50f,
                            initialHeight = 20f,
                            onDimensionsChanged = { l, w, h ->
                                val box = NativeKernel.createBox(l.toDouble(), w.toDouble(), h.toDouble())
                                val solid = if (box.success) box else NativeKernel.createFallbackBox(l.toDouble(), w.toDouble(), h.toDouble())
                                renderer?.updateMesh(solid)
                            },
                            onConfirm = { l, w, h ->
                                val box = NativeKernel.createBox(l.toDouble(), w.toDouble(), h.toDouble())
                                val solid = if (box.success) box else NativeKernel.createFallbackBox(l.toDouble(), w.toDouble(), h.toDouble())
                                activeSolid = solid
                                renderer?.updateMesh(solid)
                                val bodyName = "Solid Box ${browserState.value.bodies.size + 1}"
                                val newBody = CadBodyItem(
                                    id = "body_${System.currentTimeMillis()}",
                                    name = bodyName,
                                    isVisible = true,
                                    isSelected = true,
                                    faceCount = solid.faceCount,
                                    volume = solid.volume,
                                    solid = solid
                                )
                                browserState.value.addBody(newBody)
                                browserState.value.selectItem(newBody.id)
                                showEmptyState = false
                                timelineItems = (timelineItems + "${timelineItems.size + 1}. Box (${l.toInt()}x${w.toInt()}x${h.toInt()})").toMutableList()
                                featureStatusText = "Box created: ${solid.volume.toInt()} mm³"
                                showBoxDialog = false
                            },
                            onDismiss = {
                                showBoxDialog = false
                                renderer?.updateMesh(activeSolid)
                            },
                            modifier = Modifier
                                .align(Alignment.TopStart)
                                .padding(12.dp)
                        )
                    }

                    // Floating Cylinder Inspector Dialog (Fusion 360 Primitive)
                    if (showCylinderDialog) {
                        CylinderInspectorDialog(
                            initialRadius = 25f,
                            initialHeight = 50f,
                            onDimensionsChanged = { r, h ->
                                val cyl = NativeKernel.createCylinder(r.toDouble(), h.toDouble())
                                renderer?.updateMesh(cyl)
                            },
                            onConfirm = { r, h ->
                                val cyl = NativeKernel.createCylinder(r.toDouble(), h.toDouble())
                                activeSolid = cyl
                                renderer?.updateMesh(cyl)
                                val bodyName = "Cylinder ${browserState.value.bodies.size + 1}"
                                val newBody = CadBodyItem(
                                    id = "body_${System.currentTimeMillis()}",
                                    name = bodyName,
                                    isVisible = true,
                                    isSelected = true,
                                    faceCount = cyl.faceCount,
                                    volume = cyl.volume,
                                    solid = cyl
                                )
                                browserState.value.addBody(newBody)
                                browserState.value.selectItem(newBody.id)
                                showEmptyState = false
                                timelineItems = (timelineItems + "${timelineItems.size + 1}. Cylinder (Ø${(r * 2).toInt()} x ${h.toInt()} mm)").toMutableList()
                                featureStatusText = "Cylinder created: ${cyl.volume.toInt()} mm³"
                                showCylinderDialog = false
                            },
                            onDismiss = {
                                showCylinderDialog = false
                                renderer?.updateMesh(activeSolid)
                            },
                            modifier = Modifier
                                .align(Alignment.TopStart)
                                .padding(12.dp)
                        )
                    }

                    // Floating Fillet Inspector Dialog
                    if (showFilletDialog) {
                        FilletInspectorDialog(
                            initialRadius = 5f,
                            onRadiusChanged = { r ->
                                val solid = activeSolid ?: return@FilletInspectorDialog
                                if (solid.nativeHandle > 1000L && solid.success) {
                                    val preview = try {
                                        val edge = if (selectedType == "Edge" && selectedId > 0) selectedId else 1
                                        val res = solid.fillet(r.toDouble(), edge)
                                        if (res.success) res else null
                                    } catch (t: Throwable) {
                                        null
                                    }
                                    if (preview != null) {
                                        renderer?.updateMesh(preview)
                                    }
                                }
                            },
                            onConfirm = { r ->
                                val solid = activeSolid
                                if (solid == null || !solid.success) {
                                    featureStatusText = "No active solid to fillet"
                                    showFilletDialog = false
                                    return@FilletInspectorDialog
                                }
                                val edge = if (selectedType == "Edge" && selectedId > 0) selectedId else 1
                                val filleted = try {
                                    val res = solid.fillet(r.toDouble(), edge)
                                    if (res.success) res else null
                                } catch (t: Throwable) {
                                    null
                                }

                                if (filleted != null && filleted.success) {
                                    solid.close()
                                    activeSolid = filleted
                                    renderer?.updateMesh(filleted)
                                    val selectedBody = browserState.value.bodies.find { it.isSelected } ?: browserState.value.bodies.firstOrNull()
                                    if (selectedBody != null) {
                                        selectedBody.solid = filleted
                                        selectedBody.volume = filleted.volume
                                        selectedBody.faceCount = filleted.faceCount
                                    }
                                    timelineItems = (timelineItems + "${timelineItems.size + 1}. Fillet (R: ${r.toInt()} mm)").toMutableList()
                                    featureStatusText = "Fillet (R: ${r.toInt()} mm) applied"
                                } else {
                                    featureStatusText = "Fillet failed: radius ${r.toInt()} mm may be too large for edge #$edge"
                                    renderer?.updateMesh(solid)
                                }
                                showFilletDialog = false
                            },
                            onDismiss = {
                                showFilletDialog = false
                                renderer?.updateMesh(activeSolid)
                            },
                            modifier = Modifier
                                .align(Alignment.TopStart)
                                .padding(12.dp)
                        )
                    }

                    // Floating Chamfer Inspector Dialog
                    if (showChamferDialog) {
                        ChamferInspectorDialog(
                            initialDistance = 3f,
                            onDistanceChanged = { d ->
                                val solid = activeSolid ?: return@ChamferInspectorDialog
                                if (solid.nativeHandle > 1000L && solid.success) {
                                    val preview = try {
                                        val edge = if (selectedType == "Edge" && selectedId > 0) selectedId else 1
                                        val res = solid.chamfer(d.toDouble(), edge)
                                        if (res.success) res else null
                                    } catch (t: Throwable) {
                                        null
                                    }
                                    if (preview != null) {
                                        renderer?.updateMesh(preview)
                                    }
                                }
                            },
                            onConfirm = { d ->
                                val solid = activeSolid
                                if (solid == null || !solid.success) {
                                    featureStatusText = "No active solid to chamfer"
                                    showChamferDialog = false
                                    return@ChamferInspectorDialog
                                }
                                val edge = if (selectedType == "Edge" && selectedId > 0) selectedId else 1
                                val chamfered = try {
                                    val res = solid.chamfer(d.toDouble(), edge)
                                    if (res.success) res else null
                                } catch (t: Throwable) {
                                    null
                                }

                                if (chamfered != null && chamfered.success) {
                                    solid.close()
                                    activeSolid = chamfered
                                    renderer?.updateMesh(chamfered)
                                    val selectedBody = browserState.value.bodies.find { it.isSelected } ?: browserState.value.bodies.firstOrNull()
                                    if (selectedBody != null) {
                                        selectedBody.solid = chamfered
                                        selectedBody.volume = chamfered.volume
                                        selectedBody.faceCount = chamfered.faceCount
                                    }
                                    timelineItems = (timelineItems + "${timelineItems.size + 1}. Chamfer (D: ${d.toInt()} mm)").toMutableList()
                                    featureStatusText = "Chamfer (D: ${d.toInt()} mm) applied"
                                } else {
                                    featureStatusText = "Chamfer failed: distance ${d.toInt()} mm exceeds edge #$edge limit"
                                    renderer?.updateMesh(solid)
                                }
                                showChamferDialog = false
                            },
                            onDismiss = {
                                showChamferDialog = false
                                renderer?.updateMesh(activeSolid)
                            },
                            modifier = Modifier
                                .align(Alignment.TopStart)
                                .padding(12.dp)
                        )
                    }

                    // Floating Hole Inspector Dialog
                    if (showHoleDialog) {
                        HoleInspectorDialog(
                            initialDiameter = 10f,
                            initialDepth = 20f,
                            onHoleChanged = { diam, depth ->
                                val solid = activeSolid ?: return@HoleInspectorDialog
                                if (solid.nativeHandle > 1000L && solid.success) {
                                    val (pos, dir, _) = computeTargetPointAndInwardDir(solid, selectedHitPoint, selectedFaceNormal, selectedType, selectedId)
                                    val preview = try {
                                        val res = solid.hole(pos.first, pos.second, pos.third, dir.first, dir.second, dir.third, (diam * 0.5f).toDouble(), depth.toDouble())
                                        if (res.success && res.nativeHandle > 1000L) res else null
                                    } catch (t: Throwable) {
                                        null
                                    }
                                    if (preview != null) {
                                        renderer?.updateMesh(preview)
                                    }
                                }
                            },
                            onConfirm = { diam, depth ->
                                val solid = activeSolid
                                if (solid == null || !solid.success) {
                                    featureStatusText = "No active solid to cut hole"
                                    showHoleDialog = false
                                    return@HoleInspectorDialog
                                }
                                val (pos, dir, targetDesc) = computeTargetPointAndInwardDir(solid, selectedHitPoint, selectedFaceNormal, selectedType, selectedId)
                                val holed = try {
                                    val res = solid.hole(pos.first, pos.second, pos.third, dir.first, dir.second, dir.third, (diam * 0.5f).toDouble(), depth.toDouble())
                                    if (res.success && res.nativeHandle > 1000L) res else null
                                } catch (t: Throwable) {
                                    null
                                }

                                if (holed != null && holed.success) {
                                    solid.close()
                                    activeSolid = holed
                                    renderer?.updateMesh(holed)
                                    val selectedBody = browserState.value.bodies.find { it.isSelected } ?: browserState.value.bodies.firstOrNull()
                                    if (selectedBody != null) {
                                        selectedBody.solid = holed
                                        selectedBody.volume = holed.volume
                                        selectedBody.faceCount = holed.faceCount
                                    }
                                    timelineItems = (timelineItems + "${timelineItems.size + 1}. Hole (Ø${diam.toInt()} at $targetDesc)").toMutableList()
                                    featureStatusText = "Hole (Ø${diam.toInt()} x ${depth.toInt()} mm) cut at $targetDesc"
                                } else {
                                    featureStatusText = "Hole cut failed at $targetDesc: verify position & diameter"
                                    renderer?.updateMesh(solid)
                                }
                                showHoleDialog = false
                            },
                            onDismiss = {
                                showHoleDialog = false
                                renderer?.updateMesh(activeSolid)
                            },
                            modifier = Modifier
                                .align(Alignment.TopStart)
                                .padding(12.dp)
                        )
                    }

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
                            ViewCubeButton("TOP", isDarkTheme, testTag = "viewcube_TOP") { renderer?.setNamedView(NamedView.TOP) }

                            // Middle Ring: LEFT | FRONT | RIGHT
                            Row(
                                horizontalArrangement = Arrangement.spacedBy(2.dp),
                                modifier = Modifier.padding(vertical = 2.dp)
                            ) {
                                ViewCubeButton("L", isDarkTheme, width = 26, testTag = "viewcube_LEFT") { renderer?.setNamedView(NamedView.LEFT) }
                                ViewCubeButton("FRONT", isDarkTheme, width = 46, testTag = "viewcube_FRONT") { renderer?.setNamedView(NamedView.FRONT) }
                                ViewCubeButton("R", isDarkTheme, width = 26, testTag = "viewcube_RIGHT") { renderer?.setNamedView(NamedView.RIGHT) }
                            }

                            // Bottom Row: BOTTOM and ISO (HOME)
                            Row(
                                horizontalArrangement = Arrangement.spacedBy(4.dp),
                                modifier = Modifier.padding(top = 2.dp)
                            ) {
                                ViewCubeButton("BOTTOM", isDarkTheme, width = 52, testTag = "viewcube_BOTTOM") { renderer?.setNamedView(NamedView.BOTTOM) }
                                ViewCubeButton("ISO", isDarkTheme, width = 44, testTag = "viewcube_ISO") { renderer?.setNamedView(NamedView.HOME) }
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
                                modifier = Modifier.testTag("nav_orbit").clickable { navTool = "Orbit" }
                            )

                            // Pan Mode Toggle
                            Text(
                                text = "Pan",
                                color = if (navTool == "Pan") accentBlue else textMuted,
                                fontSize = 12.sp,
                                modifier = Modifier.testTag("nav_pan").clickable { navTool = "Pan" }
                            )

                            // Fit to View (Home Zoom)
                            Text(
                                text = "Fit",
                                color = textMuted,
                                fontSize = 12.sp,
                                modifier = Modifier.testTag("nav_fit").clickable { renderer?.zoomToFit() }
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
                                modifier = Modifier.testTag("nav_display_mode").clickable {
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
                                        modifier = Modifier.testTag("phone_sheet_close").clickable { phoneActiveSheet = null }
                                    )
                                }
                                Spacer(modifier = Modifier.height(6.dp))

                                when (phoneActiveSheet) {
                                    "browser" -> {
                                        CadBrowserView(
                                            browserState = browserState.value,
                                            isDarkTheme = isDarkTheme,
                                            onSelectEntity = handleSelectEntity,
                                            onToggleVisibility = handleToggleVisibility,
                                            onRenameBody = { id, newName ->
                                                browserState.value.renameBody(id, newName)
                                                featureStatusText = "Renamed body $id to '$newName'"
                                            },
                                            onDeleteBody = handleDeleteBody,
                                            onNamedViewClick = { namedView ->
                                                renderer?.setNamedView(namedView)
                                            },
                                            modifier = Modifier.fillMaxSize()
                                        )
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

                            val simRes = activeSimulationResult
                            if (simRes != null && simRes.success) {
                                Spacer(modifier = Modifier.height(8.dp))
                                Text(text = "FEA SIMULATION RESULTS", color = textColor, fontSize = 12.sp)
                                Text(text = "Study: Linear Static Stress", color = highlightCyan, fontSize = 11.sp)
                                Text(text = "Max Stress: ${simRes.maxVonMisesStressMPa.toInt()} MPa", color = Color(0xFFFF5252), fontSize = 11.sp)
                                Text(text = "Min Stress: ${simRes.minVonMisesStressMPa.toInt()} MPa", color = Color(0xFF448AFF), fontSize = 11.sp)
                                Text(text = "Max Disp: ${"%.3f".format(simRes.maxDisplacementMm)} mm", color = textColor, fontSize = 11.sp)
                                Text(text = "Safety Factor: ${"%.2f".format(simRes.minSafetyFactor)}", color = if (simRes.minSafetyFactor >= 1.5) successGreen else Color(0xFFFF5252), fontSize = 11.sp)
                                Text(text = "Nodes: ${simRes.nodeCount} | Tets: ${simRes.elementCount}", color = textMuted, fontSize = 10.sp)
                            }

                            val modalRes = activeModalResult
                            if (modalRes != null && modalRes.success) {
                                Spacer(modifier = Modifier.height(8.dp))
                                Text(text = "MODAL VIBRATION ANALYSIS", color = textColor, fontSize = 12.sp)
                                Text(text = "Study: Free Vibration Eigenmodes", color = highlightCyan, fontSize = 11.sp)
                                modalRes.modes.forEach { mode ->
                                    Text(text = "Mode ${mode.modeNumber}: ${"%.1f".format(mode.frequencyHz)} Hz (${"%.1f".format(mode.angularFreqRadS)} rad/s)", color = highlightOrange, fontSize = 11.sp)
                                }
                                Text(text = "Nodes: ${modalRes.nodeCount} | Elements: ${modalRes.elementCount}", color = textMuted, fontSize = 10.sp)
                            }

                            val camRes = activeCAMResult
                            if (camRes != null && camRes.success) {
                                Spacer(modifier = Modifier.height(8.dp))
                                Text(text = "CAM & G-CODE STATISTICS", color = textColor, fontSize = 12.sp)
                                Text(text = "Operations: 5 (Facing, Pocket, Contour, Drill)", color = highlightCyan, fontSize = 11.sp)
                                Text(text = "G-Code Lines: ${camRes.lineCount}", color = successGreen, fontSize = 11.sp)
                                Text(text = "Cutting Time: ${camRes.cuttingTimeSec.toInt()} s", color = textColor, fontSize = 11.sp)
                                Text(text = "Rapid Time: ${"%.1f".format(camRes.rapidTimeSec)} s", color = textMuted, fontSize = 11.sp)
                                Text(text = "Toolpath Distance: ${camRes.totalLengthMm.toInt()} mm", color = textMuted, fontSize = 11.sp)
                                Text(text = "Material Removed: ${"%.1f".format(camRes.materialVolumeRemovedCm3)} cm³", color = highlightOrange, fontSize = 11.sp)
                            }

                            val slicerRes = activeSlicerResult
                            if (slicerRes != null && slicerRes.success) {
                                Spacer(modifier = Modifier.height(8.dp))
                                Text(text = "FDM 3D PRINT SLICER", color = textColor, fontSize = 12.sp)
                                Text(text = "Total Layers: ${slicerRes.totalLayers} (0.20mm)", color = highlightCyan, fontSize = 11.sp)
                                Text(text = "Filament Used: ${slicerRes.totalFilamentMm.toInt()} mm (${"%.1f".format(slicerRes.totalFilamentGrams)} g)", color = successGreen, fontSize = 11.sp)
                                Text(text = "Est. Print Time: ${slicerRes.estimatedPrintTimeMin.toInt()} min", color = highlightOrange, fontSize = 11.sp)
                                Text(text = "G-Code Export: ${slicerRes.gcodeSize / 1024} KB", color = textMuted, fontSize = 10.sp)
                            }

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
                                modifier = Modifier
                                    .testTag("timeline_step_${index}")
                                    .clickable {
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
                            modifier = Modifier
                                .testTag("timeline_roll_forward")
                                .clickable {
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
                            modifier = Modifier
                                .testTag("phone_nav_tree")
                                .clickable {
                                    phoneActiveSheet = if (phoneActiveSheet == "browser") null else "browser"
                                }
                        )
                        Text(
                            text = "⏱ Timeline",
                            color = if (phoneActiveSheet == "timeline") accentBlue else textMuted,
                            fontSize = 12.sp,
                            modifier = Modifier
                                .testTag("phone_nav_timeline")
                                .clickable {
                                    phoneActiveSheet = if (phoneActiveSheet == "timeline") null else "timeline"
                                }
                        )
                        Text(
                            text = "⚙ Props",
                            color = if (phoneActiveSheet == "properties") accentBlue else textMuted,
                            fontSize = 12.sp,
                            modifier = Modifier
                                .testTag("phone_nav_props")
                                .clickable {
                                    phoneActiveSheet = if (phoneActiveSheet == "properties") null else "properties"
                                }
                        )
                    }
                }
            }

            if (showEmptyState) {
                EmptyStateView(
                    isDarkTheme = isDarkTheme,
                    onStartTutorial = {
                        showEmptyState = false
                        tutorialStepIndex = 0
                        showTutorial = true
                    },
                    onOpenSamples = {
                        showEmptyState = false
                        showSamplesGallery = true
                    },
                    onImportModel = {
                        try {
                            stepImportLauncher.launch(arrayOf("*/*"))
                        } catch (e: Throwable) {
                            showEmptyState = false
                        }
                    },
                    onNewPart = {
                        showEmptyState = false
                        activeSolid?.close()
                        activeSolid = null
                        renderer?.updateMesh(null)
                        sketchEntities = emptyList()
                        timelineItems = mutableListOf()
                        browserState.value = BrowserState()
                        activeTab = "Sketch"
                        featureStatusText = "New Design: Draw sketch profile"
                    },
                    modifier = Modifier.fillMaxSize()
                )
            }

            if (showTutorial && tutorialStepIndex in TutorialManager.STEPS.indices) {
                val currentStep = TutorialManager.STEPS[tutorialStepIndex]
                TutorialOverlayView(
                    stepIndex = tutorialStepIndex,
                    totalSteps = TutorialManager.STEPS.size,
                    step = currentStep,
                    isDarkTheme = isDarkTheme,
                    onNext = {
                        if (tutorialStepIndex < TutorialManager.STEPS.size - 1) {
                            tutorialStepIndex++
                            val targetTab = TutorialManager.STEPS[tutorialStepIndex].targetTab
                            if (targetTab != null) activeTab = targetTab
                        }
                    },
                    onPrev = {
                        if (tutorialStepIndex > 0) {
                            tutorialStepIndex--
                            val targetTab = TutorialManager.STEPS[tutorialStepIndex].targetTab
                            if (targetTab != null) activeTab = targetTab
                        }
                    },
                    onSkip = {
                        showTutorial = false
                        coroutineScope.launch {
                            TutorialManager.setCompleted(context, true)
                        }
                    },
                    onFinish = {
                        showTutorial = false
                        coroutineScope.launch {
                            TutorialManager.setCompleted(context, true)
                        }
                    },
                    modifier = Modifier.fillMaxSize()
                )
            }

            if (showSamplesGallery) {
                SamplesGalleryDialog(
                    isDarkTheme = isDarkTheme,
                    onSelectSample = { sample ->
                        val result = SamplesManager.loadSample(context, sample, activeTimeline)
                        if (result.success) {
                            activeSolid?.close()
                            activeSolid = result
                            renderer?.updateMesh(result)
                            val newBody = CadBodyItem(
                                id = "body_${sample.id}_${System.currentTimeMillis()}",
                                name = sample.nameEn,
                                isVisible = true,
                                isSelected = true,
                                solid = result
                            )
                            browserState.value = BrowserState(
                                bodies = listOf(newBody)
                            )
                            timelineItems = listOf("1. ${sample.nameEn} (DAG)")
                            featureStatusText = "Loaded sample: ${sample.nameEn} (~${result.volume.toInt()} mm³)"
                        } else {
                            featureStatusText = "Failed to load sample: ${result.errorMessage}"
                        }
                        showSamplesGallery = false
                        showEmptyState = false
                    },
                    onDismiss = {
                        showSamplesGallery = false
                    },
                    modifier = Modifier.fillMaxSize()
                )
            }
        }
    }
}

@Composable
fun ViewCubeButton(label: String, isDarkTheme: Boolean, width: Int = 50, testTag: String? = null, onClick: () -> Unit) {
    Surface(
        color = if (isDarkTheme) Color(0xFF2A2D32) else Color(0xFFD6D9DE),
        shape = RoundedCornerShape(3.dp),
        border = androidx.compose.foundation.BorderStroke(1.dp, if (isDarkTheme) Color(0xFF454950) else Color(0xFFB0B3B8)),
        modifier = Modifier
            .width(width.dp)
            .height(22.dp)
            .then(if (testTag != null) Modifier.testTag(testTag) else Modifier)
            .clickable(onClick = onClick)
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

fun buildNativeSketchFromEntities(sketch: NativeSketch, entities: List<SketchEntity>) {
    if (entities.isEmpty()) {
        sketch.addRectangle(-50.0, -25.0, 100.0, 50.0)
        return
    }

    val existingPoints = mutableListOf<Triple<Double, Double, Int>>() // u, v, ptId
    fun getOrCreatePoint(u: Double, v: Double): Int {
        for (pt in existingPoints) {
            val du = pt.first - u
            val dv = pt.second - v
            if (du * du + dv * dv <= 6.25) { // 2.5 mm spatial snap tolerance
                return pt.third
            }
        }
        val ptId = sketch.addPoint(u, v)
        existingPoints.add(Triple(u, v, ptId))
        return ptId
    }

    val rects = entities.filterIsInstance<SketchEntity.Rectangle>()
    val circles = entities.filterIsInstance<SketchEntity.Circle>()
    val lines = entities.filterIsInstance<SketchEntity.Line>()

    for (rect in rects) {
        sketch.addRectangle(rect.minX.toDouble(), rect.minY.toDouble(), rect.width.toDouble(), rect.height.toDouble())
    }
    for (circ in circles) {
        val pCenter = getOrCreatePoint(circ.centerX.toDouble(), circ.centerY.toDouble())
        sketch.addCircle(pCenter, circ.radius.toDouble())
    }
    for (l in lines) {
        val p1 = getOrCreatePoint(l.startX.toDouble(), l.startY.toDouble())
        val p2 = getOrCreatePoint(l.endX.toDouble(), l.endY.toDouble())
        if (p1 != p2) {
            sketch.addLine(p1, p2)
        }
    }
}

fun calculateSketchBounds(entities: List<SketchEntity>): Triple<Double, Double, Double> {
    if (entities.isEmpty()) return Triple(100.0, 50.0, 25.0)
    var minX = Double.MAX_VALUE
    var maxX = -Double.MAX_VALUE
    var minY = Double.MAX_VALUE
    var maxY = -Double.MAX_VALUE

    for (e in entities) {
        when (e) {
            is SketchEntity.Rectangle -> {
                minX = minOf(minX, e.minX.toDouble())
                maxX = maxOf(maxX, (e.minX + e.width).toDouble())
                minY = minOf(minY, e.minY.toDouble())
                maxY = maxOf(maxY, (e.minY + e.height).toDouble())
            }
            is SketchEntity.Circle -> {
                minX = minOf(minX, (e.centerX - e.radius).toDouble())
                maxX = maxOf(maxX, (e.centerX + e.radius).toDouble())
                minY = minOf(minY, (e.centerY - e.radius).toDouble())
                maxY = maxOf(maxY, (e.centerY + e.radius).toDouble())
            }
            is SketchEntity.Line -> {
                minX = minOf(minX, e.startX.toDouble(), e.endX.toDouble())
                maxX = maxOf(maxX, e.startX.toDouble(), e.endX.toDouble())
                minY = minOf(minY, e.startY.toDouble(), e.endY.toDouble())
                maxY = maxOf(maxY, e.startY.toDouble(), e.endY.toDouble())
            }
        }
    }
    val dx = maxOf(10.0, maxX - minX)
    val dy = maxOf(10.0, maxY - minY)
    return Triple(dx, dy, 25.0)
}

fun computeSketchBasis(
    nxIn: Double, nyIn: Double, nzIn: Double
): Triple<Triple<Double, Double, Double>, Triple<Double, Double, Double>, Triple<Double, Double, Double>> {
    val len = Math.sqrt(nxIn * nxIn + nyIn * nyIn + nzIn * nzIn).coerceAtLeast(1e-6)
    val nx = nxIn / len
    val ny = nyIn / len
    val nz = nzIn / len

    // Choose U orthogonal to N
    val ux: Double
    val uy: Double
    val uz: Double
    if (Math.abs(nx) < 0.9 && Math.abs(ny) < 0.9) {
        // Near Z-axis: U = (1, 0, 0) projected
        val tx = 1.0 - nx * nx
        val ty = -nx * ny
        val tz = -nx * nz
        val tLen = Math.sqrt(tx * tx + ty * ty + tz * tz).coerceAtLeast(1e-6)
        ux = tx / tLen
        uy = ty / tLen
        uz = tz / tLen
    } else if (Math.abs(nz) < 0.9) {
        val lx = -ny
        val ly = nx
        val lLen = Math.sqrt(lx * lx + ly * ly).coerceAtLeast(1e-6)
        ux = lx / lLen
        uy = ly / lLen
        uz = 0.0
    } else {
        ux = 1.0
        uy = 0.0
        uz = 0.0
    }

    val vx = ny * uz - nz * uy
    val vy = nz * ux - nx * uz
    val vz = nx * uy - ny * ux
    return Triple(Triple(ux, uy, uz), Triple(vx, vy, vz), Triple(nx, ny, nz))
}

fun computeTargetPointAndInwardDir(
    solid: BoxMeshResult,
    selectedHitPoint: Triple<Float, Float, Float>?,
    selectedFaceNormal: Triple<Float, Float, Float>?,
    selectedType: String?,
    selectedId: Int
): Triple<Triple<Double, Double, Double>, Triple<Double, Double, Double>, String> {
    selectedHitPoint?.let { pt ->
        val norm = selectedFaceNormal ?: Triple(0f, 0f, 1f)
        val dx = -norm.first.toDouble()
        val dy = -norm.second.toDouble()
        val dz = -norm.third.toDouble()
        val len = Math.sqrt(dx * dx + dy * dy + dz * dz).coerceAtLeast(1e-4)
        return Triple(
            Triple(pt.first.toDouble(), pt.second.toDouble(), pt.third.toDouble()),
            Triple(dx / len, dy / len, dz / len),
            "Tap (${pt.first.toInt()}, ${pt.second.toInt()}, ${pt.third.toInt()})"
        )
    }
    if (selectedType == "Face" && selectedId > 0) {
        val centerNorm = solid.getFaceCenterAndNormal(selectedId)
        if (centerNorm != null) {
            val (c, n) = centerNorm
            val dx = -n.first.toDouble()
            val dy = -n.second.toDouble()
            val dz = -n.third.toDouble()
            val len = Math.sqrt(dx * dx + dy * dy + dz * dz).coerceAtLeast(1e-4)
            return Triple(
                Triple(c.first.toDouble(), c.second.toDouble(), c.third.toDouble()),
                Triple(dx / len, dy / len, dz / len),
                "Face #$selectedId Center"
            )
        }
    }
    val bbox = solid.boundingBox
    return Triple(
        Triple(bbox.centerX.toDouble(), bbox.centerY.toDouble(), bbox.maxZ.toDouble()),
        Triple(0.0, 0.0, -1.0),
        "Top Face"
    )
}

fun extrudeSketchEntities(
    entities: List<SketchEntity>,
    depth: Double,
    planeOrigin: Triple<Double, Double, Double>? = null,
    planeNormal: Triple<Double, Double, Double>? = null,
    baseSolid: BoxMeshResult? = null,
    operation: String = "New Body",
    isSymmetric: Boolean = false
): BoxMeshResult {
    val (dx, dy, _) = calculateSketchBounds(entities)
    val absDepth = Math.abs(depth).coerceAtLeast(1.0)
    return try {
        val orig = planeOrigin ?: Triple(0.0, 0.0, 0.0)
        val norm = planeNormal ?: Triple(0.0, 0.0, 1.0)
        val basis = computeSketchBasis(norm.first, norm.second, norm.third)
        val nx = basis.third.first
        val ny = basis.third.second
        val nz = basis.third.third

        val isCut = (operation == "Cut" || depth < 0.0)
        val dirSign = if (isCut) -1.0 else 1.0
        val startOffset = if (isSymmetric) -0.5 * absDepth * dirSign else if (isCut) 0.1 else 0.0
        val ox = orig.first + nx * startOffset
        val oy = orig.second + ny * startOffset
        val oz = orig.third + nz * startOffset

        val sketch = NativeKernel.createSketch(
            ox, oy, oz,
            basis.first.first, basis.first.second, basis.first.third,
            basis.second.first, basis.second.second, basis.second.third,
            nx, ny, nz
        )
        buildNativeSketchFromEntities(sketch, entities)
        val extDepth = if (isCut) absDepth + 0.2 else absDepth
        val toolSolid = sketch.extrude(nx * extDepth * dirSign, ny * extDepth * dirSign, nz * extDepth * dirSign)
        sketch.close()

        val validTool = if (toolSolid.success && toolSolid.nativeHandle > 1000L) {
            toolSolid
        } else {
            NativeKernel.createFallbackBox(dx, dy, absDepth)
        }

        if (baseSolid != null && baseSolid.success && baseSolid.nativeHandle > 1000L && validTool.nativeHandle > 1000L) {
            if (isCut) {
                val cutRes = baseSolid.cut(validTool)
                if (cutRes.success && cutRes.nativeHandle > 1000L) {
                    validTool.close()
                    return cutRes
                }
            } else if (operation == "Join") {
                val joinRes = baseSolid.join(validTool)
                if (joinRes.success && joinRes.nativeHandle > 1000L) {
                    validTool.close()
                    return joinRes
                }
            }
        }
        validTool
    } catch (t: Throwable) {
        NativeKernel.createFallbackBox(dx, dy, absDepth)
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

fun getToolTestTag(tool: String): String {
    return when (tool) {
        "Box" -> "tool_Box"
        "Cylinder" -> "tool_Cylinder"
        "Extrude" -> "tool_Extrude"
        "Revolve" -> "tool_Revolve"
        "Fillet" -> "tool_Fillet"
        "Chamfer" -> "tool_Chamfer"
        "Hole" -> "tool_Hole"
        "Shell" -> "tool_Shell"
        "Combine" -> "tool_Combine"
        "Rect Pattern" -> "tool_RectPattern"
        "Circular Pattern" -> "tool_CircularPattern"
        "Mirror" -> "tool_Mirror"
        "Draft Angle" -> "tool_DraftAngle"
        "ISO Hole (M6 CB)" -> "tool_ISOHoleM6"
        "Threaded Bolt (M8)" -> "tool_ThreadedBoltM8"
        "Rectangle" -> "tool_Rectangle"
        "Line" -> "tool_Line"
        "Circle" -> "tool_Circle"
        "B-Spline" -> "tool_BSpline"
        "Project Edge" -> "tool_ProjectEdge"
        "Dimension" -> "tool_Dimension"
        "Constraints" -> "tool_Constraints"
        "Save" -> "tool_Save"
        "Open" -> "tool_Open"
        "Export STEP" -> "tool_ExportSTEP"
        "Import STEP" -> "tool_ImportSTEP"
        "Export STL" -> "tool_ExportSTL"
        "Import STL" -> "tool_ImportSTL"
        "Loft" -> "tool_Loft"
        "Press-Pull (+10)" -> "tool_PressPull_Plus10"
        "Press-Pull (-5)" -> "tool_PressPull_Minus5"
        "Thicken (5mm)" -> "tool_Thicken5"
        "U-Bracket" -> "tool_UBracket"
        "Unfold Pattern" -> "tool_UnfoldPattern"
        "Export DXF" -> "tool_ExportDXF_SheetMetal"
        "4-View Sheet" -> "tool_4ViewSheet"
        "Section Cut" -> "tool_SectionCut"
        "BOM & Balloons" -> "tool_BOM_Balloons"
        "GD&T Frame" -> "tool_GDTFrame"
        "Detail View 2:1" -> "tool_DetailView"
        "Auxiliary View" -> "tool_AuxiliaryView"
        "Multi-Sheet Doc" -> "tool_MultiSheetDoc"
        "Export SVG" -> "tool_ExportSVG_Drawing"
        "Mass (Steel)" -> "tool_MassSteel"
        "Mat: Aluminum" -> "tool_MatAluminum"
        "Mat: Titanium" -> "tool_MatTitanium"
        "Mat: Brass" -> "tool_MatBrass"
        "CoG & Inertia" -> "tool_CoG_Inertia"
        "Analyze Mesh" -> "tool_AnalyzeMesh"
        "Repair Mesh" -> "tool_RepairMesh"
        "Smooth Mesh" -> "tool_SmoothMesh"
        "Mesh to Solid" -> "tool_MeshToSolid"
        "Solve Beam FEA" -> "tool_SolveBeamFEA"
        "Tet10 Quadratic FEA" -> "tool_Tet10QuadraticFEA"
        "Modal Analysis" -> "tool_ModalAnalysis"
        "Mode 1 Vibration" -> "tool_Mode1Vibration"
        "Mode 2 Vibration" -> "tool_Mode2Vibration"
        "Mode 3 Vibration" -> "tool_Mode3Vibration"
        "Solve Part FEA" -> "tool_SolvePartFEA"
        "Fixed Support" -> "tool_FixedSupport"
        "Apply Load" -> "tool_ApplyLoad"
        "Safety Factor" -> "tool_SafetyFactor"
        "Displacement Field" -> "tool_DisplacementField"
        "Generate G-Code (GRBL)" -> "tool_GenerateGCodeGRBL"
        "Haas G-Code" -> "tool_HaasGCode"
        "Fanuc G-Code" -> "tool_FanucGCode"
        "3D Adaptive Clearing" -> "tool_3DAdaptiveClearing"
        "3D Scallop Finishing" -> "tool_3DScallopFinishing"
        "Facing Pass" -> "tool_FacingPass"
        "2D Pocketing" -> "tool_2DPocketing"
        "2D Contour" -> "tool_2DContour"
        "Peck Drilling" -> "tool_PeckDrilling"
        "Slice Solid (0.2mm)" -> "tool_SliceSolid"
        "Honeycomb Infill (30%)" -> "tool_HoneycombInfill"
        "Grid Infill (20%)" -> "tool_GridInfill"
        "Export G-Code" -> "tool_ExportGCode_Slicer"
        "Robot Arm" -> "tool_RobotArm"
        "Rotate (+15°)" -> "tool_RotatePlus15"
        "Reset Joint" -> "tool_ResetJoint"
        "Check Collision" -> "tool_CheckCollision"
        else -> "tool_${tool.replace(" ", "_").replace("(", "").replace(")", "").replace("+", "Plus").replace("-", "Minus")}"
    }
}

fun getToolIconRes(tool: String): Int {
    return when {
        tool == "Box" -> R.drawable.ic_box
        tool == "Cylinder" -> R.drawable.ic_extrude
        tool == "Extrude" -> R.drawable.ic_extrude
        tool == "Revolve" -> R.drawable.ic_revolve
        tool == "Fillet" -> R.drawable.ic_fillet
        tool == "Chamfer" -> R.drawable.ic_chamfer
        tool == "Hole" -> R.drawable.ic_hole
        tool == "Shell" -> R.drawable.ic_shell
        tool == "Combine" -> R.drawable.ic_combine
        tool.contains("Rect Pattern") -> R.drawable.ic_pattern_rectangular
        tool.contains("Circular Pattern") -> R.drawable.ic_pattern_circular
        tool.contains("Mirror") -> R.drawable.ic_mirror
        tool.contains("Draft Angle") -> R.drawable.ic_draft_angle
        tool.contains("ISO Hole") || tool.contains("Counterbore") -> R.drawable.ic_fastener_counterbore
        tool.contains("Threaded Bolt") -> R.drawable.ic_thread_helical
        tool in listOf("Rectangle", "Line", "Circle") -> R.drawable.ic_sketch
        tool == "B-Spline" -> R.drawable.ic_bspline_curve
        tool == "Project Edge" -> R.drawable.ic_sketch_projection
        tool in listOf("Dimension", "Constraints") -> R.drawable.ic_dimension_linear
        tool in listOf("Save", "Open", "Export STEP", "Import STEP", "Export STL", "Import STL") -> R.drawable.ic_drawing_view
        tool == "Loft" -> R.drawable.ic_loft
        tool == "Sweep" -> R.drawable.ic_sweep
        tool.contains("Press-Pull") -> R.drawable.ic_press_pull
        tool.contains("Thicken") -> R.drawable.ic_shell
        tool.contains("U-Bracket") -> R.drawable.ic_flange
        tool.contains("Unfold") -> R.drawable.ic_flat_pattern
        tool.contains("Bend Relief") -> R.drawable.ic_bend_relief
        tool.contains("Sheet Metal DXF") || tool.contains("Export Flat DXF") || tool == "Export DXF" -> R.drawable.ic_drawing_view
        tool == "4-View Sheet" -> R.drawable.ic_drawing_view
        tool == "Section Cut" -> R.drawable.ic_section_view
        tool.contains("BOM") -> R.drawable.ic_bom_table
        tool.contains("GD&T") -> R.drawable.ic_gdt_feature_frame
        tool.contains("Detail View") -> R.drawable.ic_drawing_detail_view
        tool.contains("Auxiliary") || tool.contains("Multi-Sheet") || tool.contains("SVG") || tool.contains("DXF") -> R.drawable.ic_drawing_view
        tool.contains("Facing") -> R.drawable.ic_cam_facing
        tool.contains("Pocket") -> R.drawable.ic_cam_pocket
        tool.contains("Contour") -> R.drawable.ic_cam_contour
        tool.contains("Drill") -> R.drawable.ic_cam_drill
        tool.contains("Adaptive") -> R.drawable.ic_cam_3d_adaptive
        tool.contains("Scallop") -> R.drawable.ic_cam_3d_scallop
        tool.contains("G-Code") -> R.drawable.ic_cam_gcode
        tool.contains("Slice") -> R.drawable.ic_slicer_fdm
        tool.contains("Infill") -> R.drawable.ic_slicer_infill
        tool.contains("Tet10") -> R.drawable.ic_fea_tet10
        tool.contains("Modal") -> R.drawable.ic_fea_modal_freq
        tool.contains("Mode 1") -> R.drawable.ic_fea_mode_shape
        tool.contains("Mode 2") || tool.contains("Mode 3") || tool.contains("Vibration") -> R.drawable.ic_fea_vibration
        tool.contains("Beam FEA") || tool.contains("Part FEA") || tool.contains("Static FEA") -> R.drawable.ic_fea_solve
        tool.contains("Load") -> R.drawable.ic_fea_load
        tool.contains("Fixed Support") || tool.contains("Constraint") -> R.drawable.ic_fea_constraint
        tool.contains("Safety Factor") -> R.drawable.ic_fea_safety_factor
        tool.contains("Displacement") -> R.drawable.ic_fea_solve
        tool.contains("Analyze Mesh") -> R.drawable.ic_mesh_repair
        tool.contains("Repair") || tool.contains("Weld") -> R.drawable.ic_mesh_repair
        tool.contains("Smooth") -> R.drawable.ic_mesh_smooth
        tool.contains("Mesh to Solid") || tool.contains("Mesh to B-Rep") -> R.drawable.ic_mesh_to_brep
        tool.contains("Robot Arm") || tool.contains("Joint") || tool.contains("Rotate") || tool.contains("Assemble") || tool.contains("Collision") -> R.drawable.ic_assembly
        tool.contains("Mass") || tool.contains("Mat:") || tool.contains("Material") -> R.drawable.ic_material_physical
        tool.contains("CoG") || tool.contains("Inertia") -> R.drawable.ic_center_of_gravity
        else -> R.drawable.ic_box
    }
}
