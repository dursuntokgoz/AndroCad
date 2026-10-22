// Copyright (c) 2026 dursuntokgoz. All rights reserved.
package com.androcad.app.ui

import androidx.compose.foundation.Image
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.Slider
import androidx.compose.material3.SliderDefaults
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.platform.testTag
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.androcad.app.R

@Composable
fun BoxInspectorDialog(
    initialLength: Float = 100f,
    initialWidth: Float = 50f,
    initialHeight: Float = 20f,
    onDimensionsChanged: (l: Float, w: Float, h: Float) -> Unit,
    onConfirm: (l: Float, w: Float, h: Float) -> Unit,
    onDismiss: () -> Unit,
    modifier: Modifier = Modifier
) {
    var length by remember { mutableStateOf(initialLength) }
    var width by remember { mutableStateOf(initialWidth) }
    var height by remember { mutableStateOf(initialHeight) }

    LaunchedEffect(length, width, height) {
        onDimensionsChanged(length, width, height)
    }

    Surface(
        color = Color(0xEE1E2228),
        shape = RoundedCornerShape(12.dp),
        border = androidx.compose.foundation.BorderStroke(1.5.dp, Color(0xFF00A4E4)),
        modifier = modifier
            .width(280.dp)
            .testTag("box_inspector_card")
    ) {
        Column(modifier = Modifier.padding(10.dp).verticalScroll(rememberScrollState())) {
            // Header
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Row(verticalAlignment = Alignment.CenterVertically) {
                    Image(
                        painter = painterResource(id = R.drawable.ic_box),
                        contentDescription = "Box",
                        modifier = Modifier.size(18.dp)
                    )
                    Spacer(modifier = Modifier.width(8.dp))
                    Text("BOX (SOLID)", color = Color.White, fontSize = 13.sp, fontWeight = FontWeight.Bold)
                }
                Text(
                    text = "✕",
                    color = Color(0xFFAAAAAA),
                    fontSize = 14.sp,
                    modifier = Modifier.clickable { onDismiss() }.padding(4.dp)
                )
            }

            Spacer(modifier = Modifier.height(8.dp))

            // Length
            Text("Length (X): ${length.toInt()} mm", color = Color(0xFFAAAAAA), fontSize = 11.sp)
            Slider(
                value = length,
                onValueChange = { length = it },
                valueRange = 10f..200f,
                modifier = Modifier.fillMaxWidth().testTag("slider_box_length"),
                colors = SliderDefaults.colors(thumbColor = Color(0xFF00A4E4), activeTrackColor = Color(0xFF00A4E4))
            )

            // Width
            Text("Width (Y): ${width.toInt()} mm", color = Color(0xFFAAAAAA), fontSize = 11.sp)
            Slider(
                value = width,
                onValueChange = { width = it },
                valueRange = 10f..150f,
                modifier = Modifier.fillMaxWidth().testTag("slider_box_width"),
                colors = SliderDefaults.colors(thumbColor = Color(0xFF00A4E4), activeTrackColor = Color(0xFF00A4E4))
            )

            // Height
            Text("Height (Z): ${height.toInt()} mm", color = Color(0xFFAAAAAA), fontSize = 11.sp)
            Slider(
                value = height,
                onValueChange = { height = it },
                valueRange = 5f..100f,
                modifier = Modifier.fillMaxWidth().testTag("slider_box_height"),
                colors = SliderDefaults.colors(thumbColor = Color(0xFF00A4E4), activeTrackColor = Color(0xFF00A4E4))
            )

            Spacer(modifier = Modifier.height(10.dp))

            // Actions
            Row(modifier = Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                Surface(
                    color = Color(0xFF3B2A2A),
                    shape = RoundedCornerShape(6.dp),
                    modifier = Modifier.weight(1f).testTag("btn_box_cancel").clickable { onDismiss() }
                ) {
                    Text(
                        "Cancel",
                        color = Color(0xFFFF8A80),
                        fontSize = 11.sp,
                        fontWeight = FontWeight.SemiBold,
                        modifier = Modifier.padding(vertical = 6.dp),
                        textAlign = androidx.compose.ui.text.style.TextAlign.Center
                    )
                }
                Surface(
                    color = Color(0xFF00A4E4),
                    shape = RoundedCornerShape(6.dp),
                    modifier = Modifier.weight(1f).testTag("btn_box_ok").clickable { onConfirm(length, width, height) }
                ) {
                    Text(
                        "✓ OK",
                        color = Color.White,
                        fontSize = 11.sp,
                        fontWeight = FontWeight.Bold,
                        modifier = Modifier.padding(vertical = 6.dp),
                        textAlign = androidx.compose.ui.text.style.TextAlign.Center
                    )
                }
            }
        }
    }
}

@Composable
fun FilletInspectorDialog(
    initialRadius: Float = 5f,
    onRadiusChanged: (r: Float) -> Unit,
    onConfirm: (r: Float) -> Unit,
    onDismiss: () -> Unit,
    modifier: Modifier = Modifier
) {
    var radius by remember { mutableStateOf(initialRadius) }

    LaunchedEffect(radius) {
        onRadiusChanged(radius)
    }

    Surface(
        color = Color(0xEE1E2228),
        shape = RoundedCornerShape(12.dp),
        border = androidx.compose.foundation.BorderStroke(1.5.dp, Color(0xFF00A4E4)),
        modifier = modifier
            .width(280.dp)
            .testTag("fillet_inspector_card")
    ) {
        Column(modifier = Modifier.padding(10.dp).verticalScroll(rememberScrollState())) {
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Row(verticalAlignment = Alignment.CenterVertically) {
                    Image(
                        painter = painterResource(id = R.drawable.ic_fillet),
                        contentDescription = "Fillet",
                        modifier = Modifier.size(18.dp)
                    )
                    Spacer(modifier = Modifier.width(8.dp))
                    Text("FILLET (RADIUS)", color = Color.White, fontSize = 13.sp, fontWeight = FontWeight.Bold)
                }
                Text("✕", color = Color(0xFFAAAAAA), fontSize = 14.sp, modifier = Modifier.clickable { onDismiss() }.padding(4.dp))
            }

            Spacer(modifier = Modifier.height(10.dp))
            Text("Radius: ${radius.toInt()} mm", color = Color(0xFFAAAAAA), fontSize = 11.sp)
            Slider(
                value = radius,
                onValueChange = { radius = it },
                valueRange = 1f..25f,
                modifier = Modifier.fillMaxWidth().testTag("slider_fillet_radius"),
                colors = SliderDefaults.colors(thumbColor = Color(0xFF00A4E4), activeTrackColor = Color(0xFF00A4E4))
            )

            Spacer(modifier = Modifier.height(10.dp))
            Row(modifier = Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                Surface(
                    color = Color(0xFF3B2A2A),
                    shape = RoundedCornerShape(6.dp),
                    modifier = Modifier.weight(1f).testTag("btn_fillet_cancel").clickable { onDismiss() }
                ) {
                    Text(
                        "Cancel",
                        color = Color(0xFFFF8A80),
                        fontSize = 11.sp,
                        fontWeight = FontWeight.SemiBold,
                        modifier = Modifier.padding(vertical = 6.dp),
                        textAlign = androidx.compose.ui.text.style.TextAlign.Center
                    )
                }
                Surface(
                    color = Color(0xFF00A4E4),
                    shape = RoundedCornerShape(6.dp),
                    modifier = Modifier.weight(1f).testTag("btn_fillet_ok").clickable { onConfirm(radius) }
                ) {
                    Text(
                        "✓ OK",
                        color = Color.White,
                        fontSize = 11.sp,
                        fontWeight = FontWeight.Bold,
                        modifier = Modifier.padding(vertical = 6.dp),
                        textAlign = androidx.compose.ui.text.style.TextAlign.Center
                    )
                }
            }
        }
    }
}

@Composable
fun ChamferInspectorDialog(
    initialDistance: Float = 3f,
    onDistanceChanged: (d: Float) -> Unit,
    onConfirm: (d: Float) -> Unit,
    onDismiss: () -> Unit,
    modifier: Modifier = Modifier
) {
    var distance by remember { mutableStateOf(initialDistance) }

    LaunchedEffect(distance) {
        onDistanceChanged(distance)
    }

    Surface(
        color = Color(0xEE1E2228),
        shape = RoundedCornerShape(12.dp),
        border = androidx.compose.foundation.BorderStroke(1.5.dp, Color(0xFF00A4E4)),
        modifier = modifier
            .width(280.dp)
            .testTag("chamfer_inspector_card")
    ) {
        Column(modifier = Modifier.padding(10.dp).verticalScroll(rememberScrollState())) {
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Row(verticalAlignment = Alignment.CenterVertically) {
                    Image(
                        painter = painterResource(id = R.drawable.ic_chamfer),
                        contentDescription = "Chamfer",
                        modifier = Modifier.size(18.dp)
                    )
                    Spacer(modifier = Modifier.width(8.dp))
                    Text("CHAMFER", color = Color.White, fontSize = 13.sp, fontWeight = FontWeight.Bold)
                }
                Text("✕", color = Color(0xFFAAAAAA), fontSize = 14.sp, modifier = Modifier.clickable { onDismiss() }.padding(4.dp))
            }

            Spacer(modifier = Modifier.height(10.dp))
            Text("Distance: ${distance.toInt()} mm", color = Color(0xFFAAAAAA), fontSize = 11.sp)
            Slider(
                value = distance,
                onValueChange = { distance = it },
                valueRange = 1f..20f,
                modifier = Modifier.fillMaxWidth().testTag("slider_chamfer_distance"),
                colors = SliderDefaults.colors(thumbColor = Color(0xFF00A4E4), activeTrackColor = Color(0xFF00A4E4))
            )

            Spacer(modifier = Modifier.height(10.dp))
            Row(modifier = Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                Surface(
                    color = Color(0xFF3B2A2A),
                    shape = RoundedCornerShape(6.dp),
                    modifier = Modifier.weight(1f).testTag("btn_chamfer_cancel").clickable { onDismiss() }
                ) {
                    Text(
                        "Cancel",
                        color = Color(0xFFFF8A80),
                        fontSize = 11.sp,
                        fontWeight = FontWeight.SemiBold,
                        modifier = Modifier.padding(vertical = 6.dp),
                        textAlign = androidx.compose.ui.text.style.TextAlign.Center
                    )
                }
                Surface(
                    color = Color(0xFF00A4E4),
                    shape = RoundedCornerShape(6.dp),
                    modifier = Modifier.weight(1f).testTag("btn_chamfer_ok").clickable { onConfirm(distance) }
                ) {
                    Text(
                        "✓ OK",
                        color = Color.White,
                        fontSize = 11.sp,
                        fontWeight = FontWeight.Bold,
                        modifier = Modifier.padding(vertical = 6.dp),
                        textAlign = androidx.compose.ui.text.style.TextAlign.Center
                    )
                }
            }
        }
    }
}

@Composable
fun HoleInspectorDialog(
    initialDiameter: Float = 10f,
    initialDepth: Float = 20f,
    onHoleChanged: (diam: Float, depth: Float) -> Unit,
    onConfirm: (diam: Float, depth: Float) -> Unit,
    onDismiss: () -> Unit,
    modifier: Modifier = Modifier
) {
    var diameter by remember { mutableStateOf(initialDiameter) }
    var depth by remember { mutableStateOf(initialDepth) }

    LaunchedEffect(diameter, depth) {
        onHoleChanged(diameter, depth)
    }

    Surface(
        color = Color(0xEE1E2228),
        shape = RoundedCornerShape(12.dp),
        border = androidx.compose.foundation.BorderStroke(1.5.dp, Color(0xFF00A4E4)),
        modifier = modifier
            .width(280.dp)
            .testTag("hole_inspector_card")
    ) {
        Column(modifier = Modifier.padding(10.dp).verticalScroll(rememberScrollState())) {
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Row(verticalAlignment = Alignment.CenterVertically) {
                    Image(
                        painter = painterResource(id = R.drawable.ic_hole),
                        contentDescription = "Hole",
                        modifier = Modifier.size(18.dp)
                    )
                    Spacer(modifier = Modifier.width(8.dp))
                    Text("HOLE CUTTER", color = Color.White, fontSize = 13.sp, fontWeight = FontWeight.Bold)
                }
                Text("✕", color = Color(0xFFAAAAAA), fontSize = 14.sp, modifier = Modifier.clickable { onDismiss() }.padding(4.dp))
            }

            Spacer(modifier = Modifier.height(10.dp))
            Text("Diameter (Ø): ${diameter.toInt()} mm", color = Color(0xFFAAAAAA), fontSize = 11.sp)
            Slider(
                value = diameter,
                onValueChange = { diameter = it },
                valueRange = 2f..40f,
                modifier = Modifier.fillMaxWidth().testTag("slider_hole_diameter"),
                colors = SliderDefaults.colors(thumbColor = Color(0xFF00A4E4), activeTrackColor = Color(0xFF00A4E4))
            )

            Text("Depth: ${depth.toInt()} mm", color = Color(0xFFAAAAAA), fontSize = 11.sp)
            Slider(
                value = depth,
                onValueChange = { depth = it },
                valueRange = 5f..50f,
                modifier = Modifier.fillMaxWidth().testTag("slider_hole_depth"),
                colors = SliderDefaults.colors(thumbColor = Color(0xFF00A4E4), activeTrackColor = Color(0xFF00A4E4))
            )

            Spacer(modifier = Modifier.height(10.dp))
            Row(modifier = Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                Surface(
                    color = Color(0xFF3B2A2A),
                    shape = RoundedCornerShape(6.dp),
                    modifier = Modifier.weight(1f).testTag("btn_hole_cancel").clickable { onDismiss() }
                ) {
                    Text(
                        "Cancel",
                        color = Color(0xFFFF8A80),
                        fontSize = 11.sp,
                        fontWeight = FontWeight.SemiBold,
                        modifier = Modifier.padding(vertical = 6.dp),
                        textAlign = androidx.compose.ui.text.style.TextAlign.Center
                    )
                }
                Surface(
                    color = Color(0xFF00A4E4),
                    shape = RoundedCornerShape(6.dp),
                    modifier = Modifier.weight(1f).testTag("btn_hole_ok").clickable { onConfirm(diameter, depth) }
                ) {
                    Text(
                        "✓ OK",
                        color = Color.White,
                        fontSize = 11.sp,
                        fontWeight = FontWeight.Bold,
                        modifier = Modifier.padding(vertical = 6.dp),
                        textAlign = androidx.compose.ui.text.style.TextAlign.Center
                    )
                }
            }
        }
    }
}
