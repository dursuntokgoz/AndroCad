// Copyright (c) 2026 dursuntokgoz. All rights reserved.
package com.androcad.app.ui

import androidx.compose.foundation.Canvas
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.clickable
import androidx.compose.foundation.gestures.detectDragGestures
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.geometry.CornerRadius
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.geometry.Size
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.Path
import androidx.compose.ui.graphics.PathEffect
import androidx.compose.ui.graphics.drawscope.DrawScope
import androidx.compose.ui.graphics.drawscope.Stroke
import androidx.compose.ui.input.pointer.pointerInput
import androidx.compose.ui.platform.LocalDensity
import androidx.compose.ui.platform.testTag
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.androcad.app.model.SketchClosedProfile
import com.androcad.app.model.SketchEntity
import com.androcad.app.model.SketchMath
import kotlin.math.abs
import kotlin.math.hypot
import kotlin.math.max
import kotlin.math.min

@Composable
fun SketchCanvasOverlay(
    activeTool: String,
    sketchEntities: List<SketchEntity>,
    onEntitiesChanged: (List<SketchEntity>) -> Unit,
    onFinishSketch: () -> Unit,
    modifier: Modifier = Modifier
) {
    var isSnapEnabled by remember { mutableStateOf(true) }
    val gridSpacingMm = 10f
    val canvasScale = 2.0f // 2.0 pixels per mm

    var dragStart by remember { mutableStateOf<Offset?>(null) }
    var dragCurrent by remember { mutableStateOf<Offset?>(null) }
    var isDragging by remember { mutableStateOf(false) }
    var lastPolylinePoint by remember { mutableStateOf<Pair<Float, Float>?>(null) }

    LaunchedEffect(activeTool) {
        lastPolylinePoint = null
    }

    LaunchedEffect(sketchEntities.size) {
        if (sketchEntities.isEmpty()) {
            lastPolylinePoint = null
        }
    }

    val currentProfile = remember(sketchEntities) {
        SketchMath.evaluateProfile(sketchEntities)
    }

    BoxWithConstraints(
        modifier = modifier
            .fillMaxSize()
            .testTag("sketch_canvas_overlay")
    ) {
        val widthPx = constraints.maxWidth.toFloat()
        val heightPx = constraints.maxHeight.toFloat()
        val centerX = widthPx / 2f
        val centerY = heightPx / 2f

        fun toMmX(px: Float): Float = (px - centerX) / canvasScale
        fun toMmY(py: Float): Float = -(py - centerY) / canvasScale
        fun toPxX(mmX: Float): Float = centerX + mmX * canvasScale
        fun toPxY(mmY: Float): Float = centerY - mmY * canvasScale

        val density = LocalDensity.current

        fun findNearbyVertex(xMm: Float, yMm: Float, entities: List<SketchEntity>, toleranceMm: Float = 10.0f): Pair<Float, Float>? {
            var closestDist = Float.MAX_VALUE
            var closestVertex: Pair<Float, Float>? = null

            fun checkPt(vx: Float, vy: Float) {
                val d = hypot(xMm - vx, yMm - vy)
                if (d <= toleranceMm && d < closestDist) {
                    closestDist = d
                    closestVertex = Pair(vx, vy)
                }
            }

            for (e in entities) {
                when (e) {
                    is SketchEntity.Line -> {
                        checkPt(e.startX, e.startY)
                        checkPt(e.endX, e.endY)
                    }
                    is SketchEntity.Rectangle -> {
                        checkPt(e.minX, e.minY)
                        checkPt(e.minX + e.width, e.minY)
                        checkPt(e.minX + e.width, e.minY + e.height)
                        checkPt(e.minX, e.minY + e.height)
                    }
                    is SketchEntity.Circle -> {
                        checkPt(e.centerX, e.centerY)
                    }
                }
            }
            return closestVertex
        }

        // Interactive Canvas
        Canvas(
            modifier = Modifier
                .fillMaxSize()
                .pointerInput(activeTool, isSnapEnabled, sketchEntities) {
                    detectDragGestures(
                        onDragStart = { offset ->
                            dragStart = offset
                            dragCurrent = offset
                            isDragging = true
                        },
                        onDrag = { change, dragAmount ->
                            change.consume()
                            dragCurrent = (dragCurrent ?: dragStart ?: change.position) + dragAmount
                        },
                        onDragEnd = {
                            val start = dragStart
                            val current = dragCurrent
                            if (start != null && current != null) {
                                var sX = toMmX(start.x)
                                var sY = toMmY(start.y)
                                var eX = toMmX(current.x)
                                var eY = toMmY(current.y)

                                val snapStart = if (isSnapEnabled) findNearbyVertex(sX, sY, sketchEntities) else null
                                if (snapStart != null) {
                                    sX = snapStart.first
                                    sY = snapStart.second
                                } else if (isSnapEnabled) {
                                    sX = SketchMath.snapToGrid(sX, gridSpacingMm)
                                    sY = SketchMath.snapToGrid(sY, gridSpacingMm)
                                }

                                val snapEnd = if (isSnapEnabled) findNearbyVertex(eX, eY, sketchEntities) else null
                                if (snapEnd != null) {
                                    eX = snapEnd.first
                                    eY = snapEnd.second
                                } else if (isSnapEnabled) {
                                    eX = SketchMath.snapToGrid(eX, gridSpacingMm)
                                    eY = SketchMath.snapToGrid(eY, gridSpacingMm)
                                }

                                when (activeTool) {
                                    "Rectangle" -> {
                                        val minX = min(sX, eX)
                                        val minY = min(sY, eY)
                                        val w = abs(eX - sX)
                                        val h = abs(eY - sY)
                                        if (w >= 4f && h >= 4f) {
                                            val rect = SketchEntity.Rectangle(
                                                id = "rect_${System.currentTimeMillis()}",
                                                minX = minX,
                                                minY = minY,
                                                width = w,
                                                height = h
                                            )
                                            onEntitiesChanged(sketchEntities + rect)
                                        }
                                    }
                                    "Circle" -> {
                                        val r = hypot(eX - sX, eY - sY)
                                        if (r >= 3f) {
                                            val circ = SketchEntity.Circle(
                                                id = "circ_${System.currentTimeMillis()}",
                                                centerX = sX,
                                                centerY = sY,
                                                radius = r
                                            )
                                            onEntitiesChanged(sketchEntities + circ)
                                        }
                                    }
                                    "Line", "B-Spline" -> {
                                        val length = hypot(eX - sX, eY - sY)
                                        if (length < 3f) {
                                            // Tap to draw / Polyline chaining mode
                                            val prev = lastPolylinePoint
                                            if (prev != null) {
                                                val distFromPrev = hypot(sX - prev.first, sY - prev.second)
                                                if (distFromPrev >= 3f) {
                                                    val line = SketchEntity.Line(
                                                        id = "line_${System.currentTimeMillis()}",
                                                        startX = prev.first,
                                                        startY = prev.second,
                                                        endX = sX,
                                                        endY = sY
                                                    )
                                                    onEntitiesChanged(sketchEntities + line)
                                                    val closeTarget = findNearbyVertex(sX, sY, sketchEntities, toleranceMm = 10f)
                                                    if (closeTarget != null && closeTarget != prev) {
                                                        lastPolylinePoint = null
                                                    } else {
                                                        lastPolylinePoint = Pair(sX, sY)
                                                    }
                                                }
                                            } else {
                                                lastPolylinePoint = Pair(sX, sY)
                                            }
                                        } else {
                                            // Drag to draw line
                                            val line = SketchEntity.Line(
                                                id = "line_${System.currentTimeMillis()}",
                                                startX = sX,
                                                startY = sY,
                                                endX = eX,
                                                endY = eY
                                            )
                                            onEntitiesChanged(sketchEntities + line)
                                            lastPolylinePoint = Pair(eX, eY)
                                        }
                                    }
                                    "Constraints" -> {
                                        if (sketchEntities.isNotEmpty()) {
                                            val snapped = sketchEntities.map { ent ->
                                                when (ent) {
                                                    is SketchEntity.Line -> {
                                                        val dx = abs(ent.endX - ent.startX)
                                                        val dy = abs(ent.endY - ent.startY)
                                                        if (dx < 5f) ent.copy(endX = ent.startX)
                                                        else if (dy < 5f) ent.copy(endY = ent.startY)
                                                        else ent
                                                    }
                                                    else -> ent
                                                }
                                            }
                                            onEntitiesChanged(snapped)
                                        }
                                    }
                                }
                            }
                            dragStart = null
                            dragCurrent = null
                            isDragging = false
                        },
                        onDragCancel = {
                            dragStart = null
                            dragCurrent = null
                            isDragging = false
                        }
                    )
                }
        ) {
            // 1. Draw Grid
            val gridSpacingPx = gridSpacingMm * canvasScale
            val startGridX = (centerX % gridSpacingPx)
            var curX = startGridX
            while (curX < size.width) {
                val isMajor = abs((curX - centerX) % (gridSpacingPx * 5)) < 1f
                drawLine(
                    color = if (isMajor) Color(0x33888888) else Color(0x18888888),
                    start = Offset(curX, 0f),
                    end = Offset(curX, size.height),
                    strokeWidth = if (isMajor) 1.5f else 1f
                )
                curX += gridSpacingPx
            }

            val startGridY = (centerY % gridSpacingPx)
            var curY = startGridY
            while (curY < size.height) {
                val isMajor = abs((curY - centerY) % (gridSpacingPx * 5)) < 1f
                drawLine(
                    color = if (isMajor) Color(0x33888888) else Color(0x18888888),
                    start = Offset(0f, curY),
                    end = Offset(size.width, curY),
                    strokeWidth = if (isMajor) 1.5f else 1f
                )
                curY += gridSpacingPx
            }

            // 2. Draw Coordinate Origin Axes (Red = X, Green = Y)
            // X-Axis
            drawLine(
                color = Color(0xBBF44336),
                start = Offset(0f, centerY),
                end = Offset(size.width, centerY),
                strokeWidth = 2f
            )
            // Y-Axis
            drawLine(
                color = Color(0xBB4CAF50),
                start = Offset(centerX, 0f),
                end = Offset(centerX, size.height),
                strokeWidth = 2f
            )
            // Origin point
            drawCircle(color = Color(0xFF00A4E4), radius = 5f, center = Offset(centerX, centerY))
            drawCircle(color = Color.White, radius = 2.5f, center = Offset(centerX, centerY))

            // 3. Render Committed Sketch Entities
            val strokeColor = Color(0xFF00A4E4)
            val fillColor = Color(0x3300A4E4)
            val handleColor = Color.White

            for (entity in sketchEntities) {
                when (entity) {
                    is SketchEntity.Rectangle -> {
                        val pxL = toPxX(entity.minX)
                        val pxR = toPxX(entity.minX + entity.width)
                        val pxT = toPxY(entity.minY + entity.height)
                        val pxB = toPxY(entity.minY)

                        val left = min(pxL, pxR)
                        val top = min(pxT, pxB)
                        val width = abs(pxR - pxL)
                        val height = abs(pxB - pxT)

                        // Translucent closed profile fill
                        drawRect(
                            color = fillColor,
                            topLeft = Offset(left, top),
                            size = Size(width, height)
                        )
                        // Solid cyan outline
                        drawRect(
                            color = strokeColor,
                            topLeft = Offset(left, top),
                            size = Size(width, height),
                            style = Stroke(width = 3.5f)
                        )
                        // 4 Corner vertex handles
                        listOf(
                            Offset(left, top),
                            Offset(left + width, top),
                            Offset(left + width, top + height),
                            Offset(left, top + height)
                        ).forEach { pt ->
                            drawCircle(color = strokeColor, radius = 5f, center = pt)
                            drawCircle(color = handleColor, radius = 3f, center = pt)
                        }
                    }
                    is SketchEntity.Circle -> {
                        val cX = toPxX(entity.centerX)
                        val cY = toPxY(entity.centerY)
                        val rPx = entity.radius * canvasScale

                        // Translucent closed profile fill
                        drawCircle(
                            color = fillColor,
                            radius = rPx,
                            center = Offset(cX, cY)
                        )
                        // Solid outline
                        drawCircle(
                            color = strokeColor,
                            radius = rPx,
                            center = Offset(cX, cY),
                            style = Stroke(width = 3.5f)
                        )
                        // Center crosshair
                        drawLine(
                            color = strokeColor,
                            start = Offset(cX - 8f, cY),
                            end = Offset(cX + 8f, cY),
                            strokeWidth = 2f
                        )
                        drawLine(
                            color = strokeColor,
                            start = Offset(cX, cY - 8f),
                            end = Offset(cX, cY + 8f),
                            strokeWidth = 2f
                        )
                    }
                    is SketchEntity.Line -> {
                        val p1 = Offset(toPxX(entity.startX), toPxY(entity.startY))
                        val p2 = Offset(toPxX(entity.endX), toPxY(entity.endY))

                        drawLine(
                            color = strokeColor,
                            start = p1,
                            end = p2,
                            strokeWidth = 3.5f
                        )
                        drawCircle(color = strokeColor, radius = 4f, center = p1)
                        drawCircle(color = handleColor, radius = 2.5f, center = p1)
                        drawCircle(color = strokeColor, radius = 4f, center = p2)
                        drawCircle(color = handleColor, radius = 2.5f, center = p2)
                    }
                }
            }

            // Active Polyline Node Indicator
            if (activeTool == "Line") {
                lastPolylinePoint?.let { pt ->
                    val nodePx = Offset(toPxX(pt.first), toPxY(pt.second))
                    drawCircle(color = Color(0xFFFFB300), radius = 8f, center = nodePx, style = Stroke(width = 2.5f))
                    drawCircle(color = Color(0xFFFFD54F), radius = 4f, center = nodePx)
                }
            }

            // 4. Live Rubber-Band In-Progress Drawing Preview
            if (isDragging && dragStart != null && dragCurrent != null) {
                val start = dragStart!!
                val current = dragCurrent!!
                var sX = toMmX(start.x)
                var sY = toMmY(start.y)
                var eX = toMmX(current.x)
                var eY = toMmY(current.y)

                val snapStartHover = if (isSnapEnabled) findNearbyVertex(sX, sY, sketchEntities) else null
                if (snapStartHover != null) {
                    sX = snapStartHover.first
                    sY = snapStartHover.second
                } else if (isSnapEnabled) {
                    sX = SketchMath.snapToGrid(sX, gridSpacingMm)
                    sY = SketchMath.snapToGrid(sY, gridSpacingMm)
                }

                val snapEndHover = if (isSnapEnabled) findNearbyVertex(eX, eY, sketchEntities) else null
                if (snapEndHover != null) {
                    eX = snapEndHover.first
                    eY = snapEndHover.second
                } else if (isSnapEnabled) {
                    eX = SketchMath.snapToGrid(eX, gridSpacingMm)
                    eY = SketchMath.snapToGrid(eY, gridSpacingMm)
                }

                val previewStartPx = Offset(toPxX(sX), toPxY(sY))
                val previewCurrentPx = Offset(toPxX(eX), toPxY(eY))
                val dashEffect = PathEffect.dashPathEffect(floatArrayOf(12f, 8f), 0f)

                if (snapEndHover != null) {
                    drawCircle(Color(0xFFFFB300), radius = 12f, center = previewCurrentPx, style = Stroke(width = 3f))
                    drawCircle(Color(0xFFFFD54F), radius = 5f, center = previewCurrentPx)
                }

                when (activeTool) {
                    "Rectangle" -> {
                        val left = min(previewStartPx.x, previewCurrentPx.x)
                        val top = min(previewStartPx.y, previewCurrentPx.y)
                        val w = abs(previewCurrentPx.x - previewStartPx.x)
                        val h = abs(previewCurrentPx.y - previewStartPx.y)

                        drawRect(
                            color = Color(0x2200A4E4),
                            topLeft = Offset(left, top),
                            size = Size(w, h)
                        )
                        drawRect(
                            color = Color(0xFF00E5FF),
                            topLeft = Offset(left, top),
                            size = Size(w, h),
                            style = Stroke(width = 3f, pathEffect = dashEffect)
                        )
                    }
                    "Circle" -> {
                        val rPx = hypot(eX - sX, eY - sY) * canvasScale
                        drawCircle(
                            color = Color(0x2200A4E4),
                            radius = rPx,
                            center = previewStartPx
                        )
                        drawCircle(
                            color = Color(0xFF00E5FF),
                            radius = rPx,
                            center = previewStartPx,
                            style = Stroke(width = 3f, pathEffect = dashEffect)
                        )
                        drawLine(
                            color = Color(0xAAFFFFFF),
                            start = previewStartPx,
                            end = previewCurrentPx,
                            strokeWidth = 2f
                        )
                    }
                    "Line" -> {
                        drawLine(
                            color = Color(0xFF00E5FF),
                            start = previewStartPx,
                            end = previewCurrentPx,
                            strokeWidth = 3f,
                            pathEffect = dashEffect
                        )
                    }
                }
            }
        }

        // Live Floating Dimension Callout during Touch Drag
        if (isDragging && dragStart != null && dragCurrent != null) {
            val cur = dragCurrent!!
            val st = dragStart!!
            val sX = toMmX(st.x)
            val sY = toMmY(st.y)
            var eX = toMmX(cur.x)
            var eY = toMmY(cur.y)

            val snapEnd = if (isSnapEnabled) findNearbyVertex(eX, eY, sketchEntities) else null
            if (snapEnd != null) {
                eX = snapEnd.first
                eY = snapEnd.second
            } else if (isSnapEnabled) {
                eX = SketchMath.snapToGrid(eX, gridSpacingMm)
                eY = SketchMath.snapToGrid(eY, gridSpacingMm)
            }

            val dimText = when (activeTool) {
                "Rectangle" -> "${abs(eX - sX).toInt()} × ${abs(eY - sY).toInt()} mm"
                "Circle" -> "R: ${hypot(eX - sX, eY - sY).toInt()} mm (Ø ${(hypot(eX - sX, eY - sY) * 2).toInt()} mm)"
                "Line" -> "L: ${hypot(eX - sX, eY - sY).toInt()} mm"
                else -> ""
            }
            if (dimText.isNotEmpty()) {
                val pillX = (cur.x - 60f).coerceIn(10f, widthPx - 140f)
                val pillY = (cur.y - 50f).coerceIn(40f, heightPx - 70f)
                Surface(
                    color = Color(0xEE1E2228),
                    shape = RoundedCornerShape(8.dp),
                    border = androidx.compose.foundation.BorderStroke(1.dp, Color(0xFF00E5FF)),
                    modifier = Modifier
                        .offset(x = (pillX / density.density).dp, y = (pillY / density.density).dp)
                ) {
                    Text(
                        text = dimText,
                        color = Color(0xFF00E5FF),
                        fontSize = 11.sp,
                        fontWeight = FontWeight.Bold,
                        modifier = Modifier.padding(horizontal = 8.dp, vertical = 4.dp)
                    )
                }
            }
        }

        // Top Status Badge (Fusion 360 Style Profile Indicator)
        Surface(
            color = Color(0xEE1E2228),
            shape = RoundedCornerShape(8.dp),
            border = androidx.compose.foundation.BorderStroke(1.dp, Color(0xFF00A4E4)),
            modifier = Modifier
                .align(Alignment.TopStart)
                .padding(12.dp)
                .testTag("sketch_status_badge")
        ) {
            Row(
                modifier = Modifier.padding(horizontal = 10.dp, vertical = 6.dp),
                verticalAlignment = Alignment.CenterVertically
            ) {
                val isClosed = currentProfile?.isClosed == true
                Surface(
                    color = if (isClosed) Color(0xFF4CAF50) else Color(0xFFFF9800),
                    shape = RoundedCornerShape(4.dp),
                    modifier = Modifier.size(8.dp)
                ) {}
                Spacer(modifier = Modifier.width(8.dp))
                Text(
                    text = if (isClosed) {
                        "✓ ${currentProfile?.description ?: "Closed Profile"} • Ready to Extrude"
                    } else if (sketchEntities.isNotEmpty()) {
                        "Drawing... (${sketchEntities.size} entities, open)"
                    } else {
                        "Sketch Mode: Draw with 1 finger (${activeTool})"
                    },
                    color = Color.White,
                    fontSize = 11.sp
                )
            }
        }

        // Bottom HUD Control Bar
        Surface(
            color = Color(0xEE1A1C20),
            shape = RoundedCornerShape(20.dp),
            border = androidx.compose.foundation.BorderStroke(1.dp, Color(0xFF454950)),
            modifier = Modifier
                .align(Alignment.BottomStart)
                .padding(start = 12.dp, bottom = 48.dp)
        ) {
            Row(
                modifier = Modifier.padding(horizontal = 10.dp, vertical = 6.dp),
                verticalAlignment = Alignment.CenterVertically,
                horizontalArrangement = Arrangement.spacedBy(8.dp)
            ) {
                // Snap Toggle
                Surface(
                    color = if (isSnapEnabled) Color(0xFF00A4E4) else Color(0xFF2C2F33),
                    shape = RoundedCornerShape(12.dp),
                    modifier = Modifier
                        .testTag("btn_sketch_snap")
                        .clickable { isSnapEnabled = !isSnapEnabled }
                ) {
                    Text(
                        text = if (isSnapEnabled) "Snap: 10mm" else "Snap: Off",
                        color = Color.White,
                        fontSize = 10.sp,
                        modifier = Modifier.padding(horizontal = 8.dp, vertical = 4.dp)
                    )
                }

                // Undo Button
                if (sketchEntities.isNotEmpty()) {
                    Surface(
                        color = Color(0xFF2C2F33),
                        shape = RoundedCornerShape(12.dp),
                        modifier = Modifier
                            .testTag("btn_sketch_undo")
                            .clickable {
                                onEntitiesChanged(sketchEntities.dropLast(1))
                            }
                    ) {
                        Text(
                            text = "Undo",
                            color = Color.White,
                            fontSize = 10.sp,
                            modifier = Modifier.padding(horizontal = 8.dp, vertical = 4.dp)
                        )
                    }

                    // Clear Button
                    Surface(
                        color = Color(0xFF3B2A2A),
                        shape = RoundedCornerShape(12.dp),
                        modifier = Modifier
                            .testTag("btn_sketch_clear")
                            .clickable {
                                onEntitiesChanged(emptyList())
                            }
                    ) {
                        Text(
                            text = "Clear",
                            color = Color(0xFFFF8A80),
                            fontSize = 10.sp,
                            modifier = Modifier.padding(horizontal = 8.dp, vertical = 4.dp)
                        )
                    }
                }

                // Instant Preset Shape (100x50 mm) for rapid 1-tap sketching and testing
                Surface(
                    color = Color(0xFF233644),
                    shape = RoundedCornerShape(12.dp),
                    modifier = Modifier
                        .testTag("btn_sketch_preset")
                        .clickable {
                            val preset = SketchEntity.Rectangle(
                                id = "rect_preset_${System.currentTimeMillis()}",
                                minX = -50f,
                                minY = -25f,
                                width = 100f,
                                height = 50f
                            )
                            onEntitiesChanged(sketchEntities + preset)
                        }
                ) {
                    Text(
                        text = "+ 100x50 mm",
                        color = Color(0xFF80D8FF),
                        fontSize = 10.sp,
                        modifier = Modifier.padding(horizontal = 8.dp, vertical = 4.dp)
                    )
                }

                // HUD Finish Sketch Button
                Surface(
                    color = Color(0xFF4CAF50),
                    shape = RoundedCornerShape(12.dp),
                    modifier = Modifier
                        .testTag("btn_sketch_finish_hud")
                        .clickable { onFinishSketch() }
                ) {
                    Text(
                        text = "Finish Sketch ✓",
                        color = Color.White,
                        fontSize = 10.sp,
                        modifier = Modifier.padding(horizontal = 10.dp, vertical = 4.dp)
                    )
                }
            }
        }

        // Primary Direct Extrude Button when Profile is Closed (Fusion 360 Fast-Action)
        if (currentProfile?.isClosed == true) {
            Surface(
                color = Color(0xFF007ACC),
                shape = RoundedCornerShape(16.dp),
                border = androidx.compose.foundation.BorderStroke(1.5.dp, Color(0xFF00A4E4)),
                modifier = Modifier
                    .align(Alignment.BottomEnd)
                    .padding(end = 16.dp, bottom = 48.dp)
                    .testTag("btn_sketch_extrude_direct")
                    .clickable { onFinishSketch() }
            ) {
                Row(
                    modifier = Modifier.padding(horizontal = 14.dp, vertical = 8.dp),
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    Text(
                        text = "Katılaştır (Extrude) ⬆",
                        color = Color.White,
                        fontSize = 12.sp,
                        fontWeight = FontWeight.Bold
                    )
                }
            }
        }
    }
}
