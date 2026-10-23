// Copyright (c) 2026 dursuntokgoz. All rights reserved.
package com.androcad.app.ui

import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.platform.testTag
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.androcad.app.model.BrowserState
import com.androcad.app.renderer.NamedView

@Composable
fun CadBrowserView(
    browserState: BrowserState,
    isDarkTheme: Boolean,
    onSelectEntity: (String) -> Unit,
    onToggleVisibility: (String) -> Unit,
    onRenameBody: (String, String) -> Unit,
    onDeleteBody: (String) -> Unit,
    onNamedViewClick: (NamedView) -> Unit,
    modifier: Modifier = Modifier
) {
    val darkPanelColor = if (isDarkTheme) Color(0xFF1E2124) else Color(0xFFF0F2F5)
    val textColor = if (isDarkTheme) Color.White else Color(0xFF1F2937)
    val textMuted = if (isDarkTheme) Color(0xFFB0B3B8) else Color(0xFF5A626A)
    val borderColor = if (isDarkTheme) Color(0xFF2B2D31) else Color(0xFFCBD0D6)
    val accentBlue = Color(0xFF007ACC)
    val highlightCyan = Color(0xFF00A4E4)
    val chipBgColor = if (isDarkTheme) Color(0xFF2F3136) else Color(0xFFE5E7EB)

    var renameTargetId by remember { mutableStateOf<String?>(null) }
    var renameText by remember { mutableStateOf("") }

    Column(
        modifier = modifier
            .background(darkPanelColor)
            .border(width = 1.dp, color = borderColor)
            .padding(8.dp)
            .verticalScroll(rememberScrollState())
    ) {
        // Header
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .clickable { browserState.isExpanded = !browserState.isExpanded }
                .testTag("browser_header"),
            verticalAlignment = Alignment.CenterVertically,
            horizontalArrangement = Arrangement.SpaceBetween
        ) {
            Text(
                text = "📁 BROWSER",
                color = textColor,
                fontSize = 13.sp,
                fontWeight = FontWeight.Bold
            )
            Text(
                text = if (browserState.isExpanded) "▼" else "▶",
                color = textMuted,
                fontSize = 11.sp
            )
        }

        if (browserState.isExpanded) {
            Spacer(modifier = Modifier.height(8.dp))

            // 1. Document Settings
            BrowserSectionHeader(
                title = "Document Settings",
                isExpanded = browserState.isDocSettingsExpanded,
                testTag = "browser_section_doc_settings",
                textColor = textColor,
                textMuted = textMuted,
                onClick = { browserState.isDocSettingsExpanded = !browserState.isDocSettingsExpanded }
            )
            if (browserState.isDocSettingsExpanded) {
                Column(modifier = Modifier.padding(start = 12.dp, top = 2.dp, bottom = 4.dp)) {
                    Text(text = "• Units: Millimeters (mm)", color = textMuted, fontSize = 11.sp)
                    Text(text = "• Linear Tol: 0.01 mm", color = textMuted, fontSize = 11.sp)
                    Text(text = "• Angular Tol: 0.5°", color = textMuted, fontSize = 11.sp)
                }
            }

            // 2. Named Views
            BrowserSectionHeader(
                title = "Named Views",
                isExpanded = browserState.isNamedViewsExpanded,
                testTag = "browser_section_named_views",
                textColor = textColor,
                textMuted = textMuted,
                onClick = { browserState.isNamedViewsExpanded = !browserState.isNamedViewsExpanded }
            )
            if (browserState.isNamedViewsExpanded) {
                Column(modifier = Modifier.padding(start = 12.dp, top = 2.dp, bottom = 4.dp)) {
                    val views = listOf(
                        "Top View" to NamedView.TOP,
                        "Front View" to NamedView.FRONT,
                        "Right View" to NamedView.RIGHT,
                        "Home (Iso)" to NamedView.HOME
                    )
                    views.forEach { (label, namedView) ->
                        Text(
                            text = "• $label",
                            color = accentBlue,
                            fontSize = 11.sp,
                            modifier = Modifier
                                .padding(vertical = 2.dp)
                                .clickable { onNamedViewClick(namedView) }
                        )
                    }
                }
            }

            // 3. Origin
            BrowserSectionHeader(
                title = "Origin",
                isExpanded = browserState.isOriginExpanded,
                testTag = "browser_section_origin",
                textColor = textColor,
                textMuted = textMuted,
                onClick = { browserState.isOriginExpanded = !browserState.isOriginExpanded }
            )
            if (browserState.isOriginExpanded) {
                Column(modifier = Modifier.padding(start = 12.dp, top = 2.dp, bottom = 4.dp)) {
                    OriginPlaneRow(
                        name = "XY Plane",
                        isVisible = browserState.originXyVisible,
                        testTag = "browser_item_origin_xy",
                        textColor = textColor,
                        onToggle = { browserState.originXyVisible = !browserState.originXyVisible }
                    )
                    OriginPlaneRow(
                        name = "XZ Plane",
                        isVisible = browserState.originXzVisible,
                        testTag = "browser_item_origin_xz",
                        textColor = textColor,
                        onToggle = { browserState.originXzVisible = !browserState.originXzVisible }
                    )
                    OriginPlaneRow(
                        name = "YZ Plane",
                        isVisible = browserState.originYzVisible,
                        testTag = "browser_item_origin_yz",
                        textColor = textColor,
                        onToggle = { browserState.originYzVisible = !browserState.originYzVisible }
                    )
                }
            }

            // 4. Bodies
            BrowserSectionHeader(
                title = "Bodies (${browserState.bodies.size})",
                isExpanded = browserState.isBodiesExpanded,
                testTag = "browser_section_bodies",
                textColor = textColor,
                textMuted = textMuted,
                onClick = { browserState.isBodiesExpanded = !browserState.isBodiesExpanded }
            )
            if (browserState.isBodiesExpanded) {
                Column(modifier = Modifier.padding(start = 12.dp, top = 2.dp, bottom = 4.dp)) {
                    if (browserState.bodies.isEmpty()) {
                        Text(text = "• (No active bodies)", color = textMuted, fontSize = 11.sp)
                    } else {
                        browserState.bodies.forEach { body ->
                            Row(
                                modifier = Modifier
                                    .fillMaxWidth()
                                    .padding(vertical = 2.dp)
                                    .background(
                                        if (body.isSelected) accentBlue.copy(alpha = 0.2f) else Color.Transparent,
                                        shape = RoundedCornerShape(4.dp)
                                    )
                                    .padding(horizontal = 4.dp, vertical = 2.dp),
                                verticalAlignment = Alignment.CenterVertically
                            ) {
                                // Visibility Eye
                                Text(
                                    text = if (body.isVisible) "👁" else "🚫",
                                    fontSize = 11.sp,
                                    modifier = Modifier
                                        .clickable { onToggleVisibility(body.id) }
                                        .testTag("browser_vis_${body.id}")
                                        .padding(end = 6.dp)
                                )
                                // Body Name (Selection)
                                Text(
                                    text = body.name,
                                    color = if (body.isSelected) highlightCyan else textColor,
                                    fontSize = 11.sp,
                                    fontWeight = if (body.isSelected) FontWeight.Bold else FontWeight.Normal,
                                    modifier = Modifier
                                        .weight(1f)
                                        .clickable { onSelectEntity(body.id) }
                                        .testTag("browser_item_${body.id}")
                                )
                                // Rename
                                Text(
                                    text = "✎",
                                    color = textMuted,
                                    fontSize = 10.sp,
                                    modifier = Modifier
                                        .padding(horizontal = 4.dp)
                                        .clickable {
                                            renameTargetId = body.id
                                            renameText = body.name
                                        }
                                        .testTag("browser_rename_${body.id}")
                                )
                                // Delete
                                Text(
                                    text = "✕",
                                    color = Color(0xFFFF5252),
                                    fontSize = 10.sp,
                                    modifier = Modifier
                                        .padding(horizontal = 4.dp)
                                        .clickable { onDeleteBody(body.id) }
                                        .testTag("browser_delete_${body.id}")
                                )
                            }
                        }
                    }
                }
            }

            // 5. Sketches
            BrowserSectionHeader(
                title = "Sketches (${browserState.sketches.size})",
                isExpanded = browserState.isSketchesExpanded,
                testTag = "browser_section_sketches",
                textColor = textColor,
                textMuted = textMuted,
                onClick = { browserState.isSketchesExpanded = !browserState.isSketchesExpanded }
            )
            if (browserState.isSketchesExpanded) {
                Column(modifier = Modifier.padding(start = 12.dp, top = 2.dp, bottom = 4.dp)) {
                    if (browserState.sketches.isEmpty()) {
                        Text(text = "• (No active sketches)", color = textMuted, fontSize = 11.sp)
                    } else {
                        browserState.sketches.forEach { sketch ->
                            Row(
                                modifier = Modifier
                                    .fillMaxWidth()
                                    .padding(vertical = 2.dp)
                                    .padding(horizontal = 4.dp, vertical = 2.dp),
                                verticalAlignment = Alignment.CenterVertically
                            ) {
                                Text(
                                    text = if (sketch.isVisible) "👁" else "🚫",
                                    fontSize = 11.sp,
                                    modifier = Modifier
                                        .clickable { sketch.isVisible = !sketch.isVisible }
                                        .testTag("browser_vis_${sketch.id}")
                                        .padding(end = 6.dp)
                                )
                                Text(
                                    text = sketch.name,
                                    color = textColor,
                                    fontSize = 11.sp,
                                    modifier = Modifier
                                        .weight(1f)
                                        .clickable { onSelectEntity(sketch.id) }
                                        .testTag("browser_item_${sketch.id}")
                                )
                            }
                        }
                    }
                }
            }

            // 6. Construction
            BrowserSectionHeader(
                title = "Construction",
                isExpanded = browserState.isConstructionExpanded,
                testTag = "browser_section_construction",
                textColor = textColor,
                textMuted = textMuted,
                onClick = { browserState.isConstructionExpanded = !browserState.isConstructionExpanded }
            )
            if (browserState.isConstructionExpanded) {
                Column(modifier = Modifier.padding(start = 12.dp, top = 2.dp, bottom = 4.dp)) {
                    Text(text = "• Plane 1 (Offset 10mm)", color = textMuted, fontSize = 11.sp)
                    Text(text = "• Axis 1 (Centerline)", color = textMuted, fontSize = 11.sp)
                }
            }
        }
    }

    // Rename Dialog
    if (renameTargetId != null) {
        AlertDialog(
            onDismissRequest = { renameTargetId = null },
            title = { Text("Rename Body", fontSize = 14.sp) },
            text = {
                OutlinedTextField(
                    value = renameText,
                    onValueChange = { renameText = it },
                    singleLine = true,
                    label = { Text("Name") }
                )
            },
            confirmButton = {
                TextButton(onClick = {
                    val id = renameTargetId
                    if (id != null && renameText.isNotBlank()) {
                        onRenameBody(id, renameText.trim())
                    }
                    renameTargetId = null
                }) {
                    Text("OK")
                }
            },
            dismissButton = {
                TextButton(onClick = { renameTargetId = null }) {
                    Text("Cancel")
                }
            }
        )
    }
}

@Composable
private fun BrowserSectionHeader(
    title: String,
    isExpanded: Boolean,
    testTag: String,
    textColor: Color,
    textMuted: Color,
    onClick: () -> Unit
) {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .clickable(onClick = onClick)
            .testTag(testTag)
            .padding(vertical = 3.dp),
        verticalAlignment = Alignment.CenterVertically,
        horizontalArrangement = Arrangement.SpaceBetween
    ) {
        Text(
            text = "${if (isExpanded) "▼" else "▶"} $title",
            color = textColor,
            fontSize = 11.sp,
            fontWeight = FontWeight.SemiBold
        )
    }
}

@Composable
private fun OriginPlaneRow(
    name: String,
    isVisible: Boolean,
    testTag: String,
    textColor: Color,
    onToggle: () -> Unit
) {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .padding(vertical = 1.dp)
            .clickable(onClick = onToggle)
            .testTag(testTag),
        verticalAlignment = Alignment.CenterVertically
    ) {
        Text(
            text = if (isVisible) "👁" else "🚫",
            fontSize = 10.sp,
            modifier = Modifier.padding(end = 4.dp)
        )
        Text(text = name, color = textColor, fontSize = 10.sp)
    }
}
