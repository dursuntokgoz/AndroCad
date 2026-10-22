// Copyright (c) 2026 dursuntokgoz. All rights reserved.
package com.androcad.app.ui

import androidx.compose.foundation.Image
import androidx.compose.foundation.background
import androidx.compose.foundation.border
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
fun ExtrudeInspectorDialog(
    profileDescription: String,
    profileArea: Double,
    initialDepth: Float = 25f,
    onDepthChanged: (depth: Float, isSymmetric: Boolean) -> Unit,
    onConfirm: (depth: Float, isSymmetric: Boolean) -> Unit,
    onDismiss: () -> Unit,
    modifier: Modifier = Modifier
) {
    var depth by remember { mutableStateOf(initialDepth) }
    var isSymmetric by remember { mutableStateOf(false) }

    LaunchedEffect(depth, isSymmetric) {
        onDepthChanged(depth, isSymmetric)
    }

    Surface(
        color = Color(0xEE1E2228),
        shape = RoundedCornerShape(12.dp),
        border = androidx.compose.foundation.BorderStroke(1.5.dp, Color(0xFF00A4E4)),
        modifier = modifier
            .width(280.dp)
            .testTag("extrude_inspector_card")
    ) {
        Column(
            modifier = Modifier
                .padding(10.dp)
                .verticalScroll(rememberScrollState())
        ) {
            // Header
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Row(verticalAlignment = Alignment.CenterVertically) {
                    Image(
                        painter = painterResource(id = R.drawable.ic_extrude),
                        contentDescription = "Extrude",
                        modifier = Modifier.size(20.dp)
                    )
                    Spacer(modifier = Modifier.width(8.dp))
                    Text(
                        text = "EXTRUDE",
                        color = Color.White,
                        fontSize = 13.sp,
                        fontWeight = FontWeight.Bold
                    )
                }
                Text(
                    text = "✕",
                    color = Color(0xFFAAAAAA),
                    fontSize = 14.sp,
                    modifier = Modifier
                        .testTag("btn_extrude_close")
                        .clickable { onDismiss() }
                        .padding(4.dp)
                )
            }

            Spacer(modifier = Modifier.height(6.dp))

            // Profile info
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Text(
                    text = "Profile",
                    color = Color(0xFFAAAAAA),
                    fontSize = 11.sp
                )
                Text(
                    text = profileDescription,
                    color = Color(0xFF00E5FF),
                    fontSize = 11.sp,
                    fontWeight = FontWeight.SemiBold,
                    modifier = Modifier.testTag("text_extrude_profile_info")
                )
            }

            Text(
                text = "Area: ${profileArea.toInt()} mm²",
                color = Color(0xFF888888),
                fontSize = 10.sp
            )

            Spacer(modifier = Modifier.height(6.dp))

            // Distance / Depth control
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Text(
                    text = "Distance",
                    color = Color(0xFFAAAAAA),
                    fontSize = 11.sp
                )
                Text(
                    text = "${depth.toInt()} mm",
                    color = Color(0xFF00A4E4),
                    fontSize = 13.sp,
                    fontWeight = FontWeight.Bold,
                    modifier = Modifier.testTag("text_extrude_depth")
                )
            }

            // Stepper buttons
            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(vertical = 2.dp),
                horizontalArrangement = Arrangement.spacedBy(6.dp)
            ) {
                listOf(-10f to "-10", -1f to "-1", 1f to "+1", 10f to "+10").forEach { (delta, label) ->
                    Surface(
                        color = Color(0xFF2C2F33),
                        shape = RoundedCornerShape(6.dp),
                        modifier = Modifier
                            .weight(1f)
                            .testTag("btn_extrude_step_${label.replace("+", "plus_").replace("-", "minus_")}")
                            .clickable {
                                depth = (depth + delta).coerceIn(2f, 150f)
                            }
                    ) {
                        Text(
                            text = label,
                            color = Color.White,
                            fontSize = 10.sp,
                            modifier = Modifier.padding(vertical = 4.dp),
                            textAlign = androidx.compose.ui.text.style.TextAlign.Center
                        )
                    }
                }
            }

            // Depth slider
            Slider(
                value = depth,
                onValueChange = { depth = it },
                valueRange = 2f..150f,
                steps = 0,
                colors = SliderDefaults.colors(
                    thumbColor = Color(0xFF00A4E4),
                    activeTrackColor = Color(0xFF00A4E4),
                    inactiveTrackColor = Color(0xFF454950)
                ),
                modifier = Modifier
                    .fillMaxWidth()
                    .testTag("slider_extrude_depth")
            )

            // Direction Toggle (One Side vs Symmetric)
            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(vertical = 4.dp),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Text(
                    text = "Direction",
                    color = Color(0xFFAAAAAA),
                    fontSize = 11.sp
                )
                Row(horizontalArrangement = Arrangement.spacedBy(4.dp)) {
                    Surface(
                        color = if (!isSymmetric) Color(0xFF00A4E4) else Color(0xFF2C2F33),
                        shape = RoundedCornerShape(4.dp),
                        modifier = Modifier
                            .testTag("btn_extrude_dir_one_side")
                            .clickable { isSymmetric = false }
                    ) {
                        Text(
                            text = "One Side",
                            color = Color.White,
                            fontSize = 10.sp,
                            modifier = Modifier.padding(horizontal = 6.dp, vertical = 3.dp)
                        )
                    }
                    Surface(
                        color = if (isSymmetric) Color(0xFF00A4E4) else Color(0xFF2C2F33),
                        shape = RoundedCornerShape(4.dp),
                        modifier = Modifier
                            .testTag("btn_extrude_dir_symmetric")
                            .clickable { isSymmetric = true }
                    ) {
                        Text(
                            text = "Symmetric",
                            color = Color.White,
                            fontSize = 10.sp,
                            modifier = Modifier.padding(horizontal = 6.dp, vertical = 3.dp)
                        )
                    }
                }
            }

            Spacer(modifier = Modifier.height(14.dp))

            // Action Buttons (Cancel / OK)
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.spacedBy(8.dp)
            ) {
                Surface(
                    color = Color(0xFF3B2A2A),
                    shape = RoundedCornerShape(6.dp),
                    modifier = Modifier
                        .weight(1f)
                        .testTag("btn_extrude_cancel")
                        .clickable { onDismiss() }
                ) {
                    Text(
                        text = "Cancel",
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
                    modifier = Modifier
                        .weight(1f)
                        .testTag("btn_extrude_ok")
                        .clickable { onConfirm(depth, isSymmetric) }
                ) {
                    Text(
                        text = "✓ OK",
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
