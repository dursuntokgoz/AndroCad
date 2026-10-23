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
fun CylinderInspectorDialog(
    initialRadius: Float = 25f,
    initialHeight: Float = 50f,
    onDimensionsChanged: (radius: Float, height: Float) -> Unit,
    onConfirm: (radius: Float, height: Float) -> Unit,
    onDismiss: () -> Unit,
    modifier: Modifier = Modifier
) {
    var radius by remember { mutableStateOf(initialRadius) }
    var height by remember { mutableStateOf(initialHeight) }

    LaunchedEffect(radius, height) {
        onDimensionsChanged(radius, height)
    }

    Surface(
        color = Color(0xEE1E2228),
        shape = RoundedCornerShape(12.dp),
        border = androidx.compose.foundation.BorderStroke(1.5.dp, Color(0xFF00A4E4)),
        modifier = modifier
            .width(280.dp)
            .testTag("cylinder_inspector_card")
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
                        painter = painterResource(id = R.drawable.ic_extrude),
                        contentDescription = "Cylinder",
                        modifier = Modifier.size(18.dp)
                    )
                    Spacer(modifier = Modifier.width(8.dp))
                    Text("CYLINDER (SOLID)", color = Color.White, fontSize = 13.sp, fontWeight = FontWeight.Bold)
                }
                Text(
                    text = "✕",
                    color = Color(0xFFAAAAAA),
                    fontSize = 14.sp,
                    modifier = Modifier.clickable { onDismiss() }.padding(4.dp)
                )
            }

            Spacer(modifier = Modifier.height(8.dp))

            // Radius
            Text("Radius (R): ${radius.toInt()} mm (Ø ${(radius * 2).toInt()} mm)", color = Color(0xFFAAAAAA), fontSize = 11.sp)
            Slider(
                value = radius,
                onValueChange = { radius = it },
                valueRange = 5f..100f,
                modifier = Modifier.fillMaxWidth().testTag("slider_cylinder_radius"),
                colors = SliderDefaults.colors(thumbColor = Color(0xFF00A4E4), activeTrackColor = Color(0xFF00A4E4))
            )

            // Height
            Text("Height (Z): ${height.toInt()} mm", color = Color(0xFFAAAAAA), fontSize = 11.sp)
            Slider(
                value = height,
                onValueChange = { height = it },
                valueRange = 5f..200f,
                modifier = Modifier.fillMaxWidth().testTag("slider_cylinder_height"),
                colors = SliderDefaults.colors(thumbColor = Color(0xFF00A4E4), activeTrackColor = Color(0xFF00A4E4))
            )

            Spacer(modifier = Modifier.height(10.dp))

            // Actions
            Row(modifier = Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                Surface(
                    color = Color(0xFF3B2A2A),
                    shape = RoundedCornerShape(6.dp),
                    modifier = Modifier.weight(1f).testTag("btn_cylinder_cancel").clickable { onDismiss() }
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
                    modifier = Modifier.weight(1f).testTag("btn_cylinder_ok").clickable { onConfirm(radius, height) }
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
    var isFirst by remember { mutableStateOf(true) }

    LaunchedEffect(radius) {
        if (isFirst) {
            isFirst = false
        } else {
            onRadiusChanged(radius)
        }
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
    var isFirst by remember { mutableStateOf(true) }

    LaunchedEffect(distance) {
        if (isFirst) {
            isFirst = false
        } else {
            onDistanceChanged(distance)
        }
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
    var isFirst by remember { mutableStateOf(true) }

    LaunchedEffect(diameter, depth) {
        if (isFirst) {
            isFirst = false
        } else {
            onHoleChanged(diameter, depth)
        }
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

@Composable
fun RevolveInspectorDialog(
    initialAngleDeg: Float = 360f,
    onRevolveChanged: (angleDeg: Float, operation: String) -> Unit,
    onConfirm: (angleDeg: Float, operation: String) -> Unit,
    onDismiss: () -> Unit,
    modifier: Modifier = Modifier
) {
    var angleDeg by remember { mutableStateOf(initialAngleDeg) }
    var operation by remember { mutableStateOf("New Body") }
    var isFirst by remember { mutableStateOf(true) }

    LaunchedEffect(angleDeg, operation) {
        if (isFirst) {
            isFirst = false
        } else {
            onRevolveChanged(angleDeg, operation)
        }
    }

    Surface(
        color = Color(0xEE1E2228),
        shape = RoundedCornerShape(12.dp),
        border = androidx.compose.foundation.BorderStroke(1.5.dp, Color(0xFF00A4E4)),
        modifier = modifier
            .width(280.dp)
            .testTag("revolve_inspector_card")
    ) {
        Column(modifier = Modifier.padding(10.dp).verticalScroll(rememberScrollState())) {
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Row(verticalAlignment = Alignment.CenterVertically) {
                    Image(
                        painter = painterResource(id = R.drawable.ic_revolve),
                        contentDescription = "Revolve",
                        modifier = Modifier.size(18.dp)
                    )
                    Spacer(modifier = Modifier.width(8.dp))
                    Text("REVOLVE", color = Color.White, fontSize = 13.sp, fontWeight = FontWeight.Bold)
                }
                Text("✕", color = Color(0xFFAAAAAA), fontSize = 14.sp, modifier = Modifier.clickable { onDismiss() }.padding(4.dp))
            }

            Spacer(modifier = Modifier.height(8.dp))
            Text("Angle: ${angleDeg.toInt()}°", color = Color(0xFFAAAAAA), fontSize = 11.sp)
            Slider(
                value = angleDeg,
                onValueChange = { angleDeg = it },
                valueRange = 15f..360f,
                modifier = Modifier.fillMaxWidth().testTag("slider_revolve_angle"),
                colors = SliderDefaults.colors(thumbColor = Color(0xFF00A4E4), activeTrackColor = Color(0xFF00A4E4))
            )

            Text("Operation:", color = Color(0xFFAAAAAA), fontSize = 11.sp)
            Spacer(modifier = Modifier.height(4.dp))
            Row(modifier = Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.spacedBy(6.dp)) {
                listOf("New Body", "Join", "Cut").forEach { op ->
                    val selected = (operation == op)
                    Surface(
                        color = if (selected) Color(0xFF007ACC) else Color(0xFF2C2F33),
                        shape = RoundedCornerShape(6.dp),
                        modifier = Modifier
                            .weight(1f)
                            .testTag("btn_revolve_op_${op.replace(" ", "_")}")
                            .clickable { operation = op }
                    ) {
                        Text(
                            text = op,
                            color = Color.White,
                            fontSize = 10.sp,
                            modifier = Modifier.padding(vertical = 5.dp),
                            textAlign = androidx.compose.ui.text.style.TextAlign.Center
                        )
                    }
                }
            }

            Spacer(modifier = Modifier.height(10.dp))
            Row(modifier = Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                Surface(
                    color = Color(0xFF3B2A2A),
                    shape = RoundedCornerShape(6.dp),
                    modifier = Modifier.weight(1f).testTag("btn_revolve_cancel").clickable { onDismiss() }
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
                    modifier = Modifier.weight(1f).testTag("btn_revolve_ok").clickable { onConfirm(angleDeg, operation) }
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
fun ShellInspectorDialog(
    initialThickness: Float = 2f,
    onThicknessChanged: (t: Float) -> Unit,
    onConfirm: (t: Float) -> Unit,
    onDismiss: () -> Unit,
    modifier: Modifier = Modifier
) {
    var thickness by remember { mutableStateOf(initialThickness) }
    var isFirst by remember { mutableStateOf(true) }

    LaunchedEffect(thickness) {
        if (isFirst) {
            isFirst = false
        } else {
            onThicknessChanged(thickness)
        }
    }

    Surface(
        color = Color(0xEE1E2228),
        shape = RoundedCornerShape(12.dp),
        border = androidx.compose.foundation.BorderStroke(1.5.dp, Color(0xFF00A4E4)),
        modifier = modifier
            .width(280.dp)
            .testTag("shell_inspector_card")
    ) {
        Column(modifier = Modifier.padding(10.dp).verticalScroll(rememberScrollState())) {
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Row(verticalAlignment = Alignment.CenterVertically) {
                    Image(
                        painter = painterResource(id = R.drawable.ic_shell),
                        contentDescription = "Shell",
                        modifier = Modifier.size(18.dp)
                    )
                    Spacer(modifier = Modifier.width(8.dp))
                    Text("SHELL (HOLLOW)", color = Color.White, fontSize = 13.sp, fontWeight = FontWeight.Bold)
                }
                Text("✕", color = Color(0xFFAAAAAA), fontSize = 14.sp, modifier = Modifier.clickable { onDismiss() }.padding(4.dp))
            }

            Spacer(modifier = Modifier.height(10.dp))
            Text("Wall Thickness: ${"%.1f".format(thickness)} mm", color = Color(0xFFAAAAAA), fontSize = 11.sp)
            Slider(
                value = thickness,
                onValueChange = { thickness = it },
                valueRange = 0.5f..15f,
                modifier = Modifier.fillMaxWidth().testTag("slider_shell_thickness"),
                colors = SliderDefaults.colors(thumbColor = Color(0xFF00A4E4), activeTrackColor = Color(0xFF00A4E4))
            )

            Spacer(modifier = Modifier.height(10.dp))
            Row(modifier = Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                Surface(
                    color = Color(0xFF3B2A2A),
                    shape = RoundedCornerShape(6.dp),
                    modifier = Modifier.weight(1f).testTag("btn_shell_cancel").clickable { onDismiss() }
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
                    modifier = Modifier.weight(1f).testTag("btn_shell_ok").clickable { onConfirm(thickness) }
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
fun CombineInspectorDialog(
    initialOperation: String = "Join",
    onConfirm: (operation: String, keepToolBody: Boolean) -> Unit,
    onDismiss: () -> Unit,
    modifier: Modifier = Modifier
) {
    var operation by remember { mutableStateOf(initialOperation) }
    var keepTool by remember { mutableStateOf(false) }

    Surface(
        color = Color(0xEE1E2228),
        shape = RoundedCornerShape(12.dp),
        border = androidx.compose.foundation.BorderStroke(1.5.dp, Color(0xFF00A4E4)),
        modifier = modifier
            .width(280.dp)
            .testTag("combine_inspector_card")
    ) {
        Column(modifier = Modifier.padding(10.dp).verticalScroll(rememberScrollState())) {
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Row(verticalAlignment = Alignment.CenterVertically) {
                    Image(
                        painter = painterResource(id = R.drawable.ic_combine),
                        contentDescription = "Combine",
                        modifier = Modifier.size(18.dp)
                    )
                    Spacer(modifier = Modifier.width(8.dp))
                    Text("COMBINE (BOOLEAN)", color = Color.White, fontSize = 13.sp, fontWeight = FontWeight.Bold)
                }
                Text("✕", color = Color(0xFFAAAAAA), fontSize = 14.sp, modifier = Modifier.clickable { onDismiss() }.padding(4.dp))
            }

            Spacer(modifier = Modifier.height(10.dp))
            Text("Boolean Operation:", color = Color(0xFFAAAAAA), fontSize = 11.sp)
            Spacer(modifier = Modifier.height(6.dp))
            Row(modifier = Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.spacedBy(6.dp)) {
                listOf("Join", "Cut", "Intersect").forEach { op ->
                    val selected = (operation == op)
                    Surface(
                        color = if (selected) {
                            when (op) {
                                "Cut" -> Color(0xFFD32F2F)
                                "Intersect" -> Color(0xFFFF8F00)
                                else -> Color(0xFF007ACC)
                            }
                        } else Color(0xFF2C2F33),
                        shape = RoundedCornerShape(6.dp),
                        modifier = Modifier
                            .weight(1f)
                            .testTag("btn_combine_op_$op")
                            .clickable { operation = op }
                    ) {
                        Text(
                            text = op,
                            color = Color.White,
                            fontSize = 11.sp,
                            fontWeight = if (selected) FontWeight.Bold else FontWeight.Normal,
                            modifier = Modifier.padding(vertical = 6.dp),
                            textAlign = androidx.compose.ui.text.style.TextAlign.Center
                        )
                    }
                }
            }

            Spacer(modifier = Modifier.height(8.dp))
            Surface(
                color = if (keepTool) Color(0xFF007ACC) else Color(0xFF2C2F33),
                shape = RoundedCornerShape(6.dp),
                modifier = Modifier
                    .fillMaxWidth()
                    .testTag("btn_combine_keep_tool")
                    .clickable { keepTool = !keepTool }
            ) {
                Text(
                    text = if (keepTool) "✓ Keep Tool Body" else "Keep Tool Body: Off",
                    color = Color.White,
                    fontSize = 11.sp,
                    modifier = Modifier.padding(vertical = 5.dp, horizontal = 8.dp)
                )
            }

            Spacer(modifier = Modifier.height(10.dp))
            Row(modifier = Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                Surface(
                    color = Color(0xFF3B2A2A),
                    shape = RoundedCornerShape(6.dp),
                    modifier = Modifier.weight(1f).testTag("btn_combine_cancel").clickable { onDismiss() }
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
                    modifier = Modifier.weight(1f).testTag("btn_combine_ok").clickable { onConfirm(operation, keepTool) }
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
fun TransformInspectorDialog(
    onTransformChanged: (tx: Float, ty: Float, tz: Float, rz: Float, scale: Float) -> Unit,
    onConfirm: (tx: Float, ty: Float, tz: Float, rz: Float, scale: Float) -> Unit,
    onDismiss: () -> Unit,
    modifier: Modifier = Modifier
) {
    var tx by remember { mutableStateOf(0f) }
    var ty by remember { mutableStateOf(0f) }
    var tz by remember { mutableStateOf(0f) }
    var rz by remember { mutableStateOf(0f) }
    var scale by remember { mutableStateOf(1f) }
    var isFirst by remember { mutableStateOf(true) }

    LaunchedEffect(tx, ty, tz, rz, scale) {
        if (isFirst) {
            isFirst = false
        } else {
            onTransformChanged(tx, ty, tz, rz, scale)
        }
    }

    Surface(
        color = Color(0xEE1E2228),
        shape = RoundedCornerShape(12.dp),
        border = androidx.compose.foundation.BorderStroke(1.5.dp, Color(0xFF00A4E4)),
        modifier = modifier
            .width(280.dp)
            .testTag("transform_inspector_card")
    ) {
        Column(modifier = Modifier.padding(10.dp).verticalScroll(rememberScrollState())) {
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Text("MOVE / ROTATE / SCALE", color = Color.White, fontSize = 13.sp, fontWeight = FontWeight.Bold)
                Text("✕", color = Color(0xFFAAAAAA), fontSize = 14.sp, modifier = Modifier.clickable { onDismiss() }.padding(4.dp))
            }

            Spacer(modifier = Modifier.height(6.dp))
            Text("Move X: ${tx.toInt()} mm | Move Y: ${ty.toInt()} mm", color = Color(0xFFAAAAAA), fontSize = 10.sp)
            Slider(
                value = tx,
                onValueChange = { tx = it },
                valueRange = -100f..100f,
                modifier = Modifier.fillMaxWidth().testTag("slider_transform_tx"),
                colors = SliderDefaults.colors(thumbColor = Color(0xFF00A4E4), activeTrackColor = Color(0xFF00A4E4))
            )
            Slider(
                value = ty,
                onValueChange = { ty = it },
                valueRange = -100f..100f,
                modifier = Modifier.fillMaxWidth().testTag("slider_transform_ty"),
                colors = SliderDefaults.colors(thumbColor = Color(0xFF00A4E4), activeTrackColor = Color(0xFF00A4E4))
            )

            Text("Move Z: ${tz.toInt()} mm | Rotate Z: ${rz.toInt()}°", color = Color(0xFFAAAAAA), fontSize = 10.sp)
            Slider(
                value = tz,
                onValueChange = { tz = it },
                valueRange = -100f..100f,
                modifier = Modifier.fillMaxWidth().testTag("slider_transform_tz"),
                colors = SliderDefaults.colors(thumbColor = Color(0xFF00A4E4), activeTrackColor = Color(0xFF00A4E4))
            )
            Slider(
                value = rz,
                onValueChange = { rz = it },
                valueRange = -180f..180f,
                modifier = Modifier.fillMaxWidth().testTag("slider_transform_rz"),
                colors = SliderDefaults.colors(thumbColor = Color(0xFF00A4E4), activeTrackColor = Color(0xFF00A4E4))
            )

            Text("Scale Factor: ${"%.2f".format(scale)}x", color = Color(0xFFAAAAAA), fontSize = 10.sp)
            Slider(
                value = scale,
                onValueChange = { scale = it },
                valueRange = 0.25f..2.5f,
                modifier = Modifier.fillMaxWidth().testTag("slider_transform_scale"),
                colors = SliderDefaults.colors(thumbColor = Color(0xFF00A4E4), activeTrackColor = Color(0xFF00A4E4))
            )

            Spacer(modifier = Modifier.height(8.dp))
            Row(modifier = Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                Surface(
                    color = Color(0xFF3B2A2A),
                    shape = RoundedCornerShape(6.dp),
                    modifier = Modifier.weight(1f).testTag("btn_transform_cancel").clickable { onDismiss() }
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
                    modifier = Modifier.weight(1f).testTag("btn_transform_ok").clickable { onConfirm(tx, ty, tz, rz, scale) }
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
fun SphereInspectorDialog(
    initialRadius: Float = 30f,
    onRadiusChanged: (r: Float) -> Unit,
    onConfirm: (r: Float, operation: String) -> Unit,
    onDismiss: () -> Unit,
    modifier: Modifier = Modifier
) {
    var radius by remember { mutableStateOf(initialRadius) }
    var operation by remember { mutableStateOf("New Body") }

    LaunchedEffect(radius) {
        onRadiusChanged(radius)
    }

    Surface(
        color = Color(0xEE1E2228),
        shape = RoundedCornerShape(12.dp),
        border = androidx.compose.foundation.BorderStroke(1.5.dp, Color(0xFF00A4E4)),
        modifier = modifier
            .width(280.dp)
            .testTag("sphere_inspector_card")
    ) {
        Column(modifier = Modifier.padding(10.dp).verticalScroll(rememberScrollState())) {
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Text("SPHERE (SOLID)", color = Color.White, fontSize = 13.sp, fontWeight = FontWeight.Bold)
                Text("✕", color = Color(0xFFAAAAAA), fontSize = 14.sp, modifier = Modifier.clickable { onDismiss() }.padding(4.dp))
            }

            Spacer(modifier = Modifier.height(8.dp))
            Text("Radius (R): ${radius.toInt()} mm (Ø ${(radius * 2).toInt()} mm)", color = Color(0xFFAAAAAA), fontSize = 11.sp)
            Slider(
                value = radius,
                onValueChange = { radius = it },
                valueRange = 5f..100f,
                modifier = Modifier.fillMaxWidth().testTag("slider_sphere_radius"),
                colors = SliderDefaults.colors(thumbColor = Color(0xFF00A4E4), activeTrackColor = Color(0xFF00A4E4))
            )

            Text("Operation:", color = Color(0xFFAAAAAA), fontSize = 11.sp)
            Spacer(modifier = Modifier.height(4.dp))
            Row(modifier = Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.spacedBy(6.dp)) {
                listOf("New Body", "Join", "Cut").forEach { op ->
                    val selected = (operation == op)
                    Surface(
                        color = if (selected) Color(0xFF007ACC) else Color(0xFF2C2F33),
                        shape = RoundedCornerShape(6.dp),
                        modifier = Modifier
                            .weight(1f)
                            .testTag("btn_sphere_op_${op.replace(" ", "_")}")
                            .clickable { operation = op }
                    ) {
                        Text(
                            text = op,
                            color = Color.White,
                            fontSize = 10.sp,
                            modifier = Modifier.padding(vertical = 5.dp),
                            textAlign = androidx.compose.ui.text.style.TextAlign.Center
                        )
                    }
                }
            }

            Spacer(modifier = Modifier.height(10.dp))
            Row(modifier = Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                Surface(
                    color = Color(0xFF3B2A2A),
                    shape = RoundedCornerShape(6.dp),
                    modifier = Modifier.weight(1f).testTag("btn_sphere_cancel").clickable { onDismiss() }
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
                    modifier = Modifier.weight(1f).testTag("btn_sphere_ok").clickable { onConfirm(radius, operation) }
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
