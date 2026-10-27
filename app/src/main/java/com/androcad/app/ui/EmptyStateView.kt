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
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.platform.testTag
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp

@Composable
fun EmptyStateView(
    isDarkTheme: Boolean,
    onStartTutorial: () -> Unit,
    onOpenSamples: () -> Unit,
    onImportModel: () -> Unit,
    onNewPart: () -> Unit,
    modifier: Modifier = Modifier
) {
    val isTr = LocalizationManager.currentLanguage.value == AppLanguage.TR
    val darkPanelColor = if (isDarkTheme) Color(0xFF1E2124) else Color(0xFFF0F2F5)
    val textColor = if (isDarkTheme) Color.White else Color(0xFF1F2937)
    val textMuted = if (isDarkTheme) Color(0xFFB0B3B8) else Color(0xFF5A626A)
    val accentBlue = Color(0xFF007ACC)
    val cardBgColor = if (isDarkTheme) Color(0xFF282B30) else Color(0xFFFFFFFF)
    val borderColor = if (isDarkTheme) Color(0xFF383C42) else Color(0xFFCBD0D6)

    Box(
        modifier = modifier
            .background(darkPanelColor.copy(alpha = 0.95f))
            .testTag("empty_state_screen")
            .padding(16.dp),
        contentAlignment = Alignment.Center
    ) {
        Card(
            modifier = Modifier
                .widthIn(max = 480.dp)
                .fillMaxWidth(),
            colors = CardDefaults.cardColors(containerColor = cardBgColor),
            shape = RoundedCornerShape(12.dp),
            border = androidx.compose.foundation.BorderStroke(1.dp, borderColor)
        ) {
            Column(
                modifier = Modifier
                    .padding(24.dp)
                    .verticalScroll(rememberScrollState()),
                horizontalAlignment = Alignment.CenterHorizontally
            ) {
                Text(
                    text = if (isTr) "AndroCAD'e Hoş Geldiniz" else "Welcome to AndroCAD",
                    color = textColor,
                    fontSize = 20.sp,
                    fontWeight = FontWeight.Bold,
                    textAlign = TextAlign.Center
                )
                Spacer(modifier = Modifier.height(8.dp))
                Text(
                    text = if (isTr) "Open CASCADE 8.0.1 ve PlaneGCS çözücülü mobil parametrik 3B CAD platformu." else "Mobile-native 3D Parametric CAD with Open CASCADE 8.0.1 & PlaneGCS solver.",
                    color = textMuted,
                    fontSize = 12.sp,
                    textAlign = TextAlign.Center
                )

                Spacer(modifier = Modifier.height(20.dp))

                // Action 1: Onboarding Tutorial
                EmptyStateActionButton(
                    title = if (isTr) "🎓 İnteraktif Rehberi Başlat" else "🎓 Start Interactive Tutorial",
                    description = if (isTr) "Görünüm kontrolü, eskiz çizimi ve 3B ekstrüzyonu 6 basit adımda öğrenin." else "Learn viewport navigation, sketching, and 3D extrusion in 6 simple steps.",
                    color = accentBlue,
                    testTag = "empty_state_btn_tutorial",
                    onClick = onStartTutorial
                )

                Spacer(modifier = Modifier.height(10.dp))

                // Action 2: Sample Designs Gallery
                EmptyStateActionButton(
                    title = if (isTr) "📦 Örnek Tasarımları Keşfet" else "📦 Explore Sample Designs",
                    description = if (isTr) "Hazır parametrik modelleri açın ve düzenleyin (L-Braket, Burç, Telefon Standı)." else "Open and edit genuine parametric models (L-Bracket, Bushing, Phone Stand).",
                    color = Color(0xFF2E7D32),
                    testTag = "empty_state_btn_samples",
                    onClick = onOpenSamples
                )

                Spacer(modifier = Modifier.height(10.dp))

                // Action 3: Import Model via SAF
                EmptyStateActionButton(
                    title = if (isTr) "📂 Dosya İçe Aktar (STEP / STL)" else "📂 Import File (STEP / STL)",
                    description = if (isTr) "SAF üzerinden cihazınızdan veya buluttan CAD parçalarını doğrudan yükleyin." else "Select a CAD model from device storage using Android Storage Access Framework.",
                    color = Color(0xFFE65100),
                    testTag = "empty_state_btn_import",
                    onClick = onImportModel
                )

                Spacer(modifier = Modifier.height(10.dp))

                // Action 4: Create Default Part
                Surface(
                    color = Color.Transparent,
                    shape = RoundedCornerShape(8.dp),
                    border = androidx.compose.foundation.BorderStroke(1.dp, borderColor),
                    modifier = Modifier
                        .fillMaxWidth()
                        .clickable(onClick = onNewPart)
                        .testTag("empty_state_btn_new_part")
                        .padding(vertical = 4.dp)
                ) {
                    Text(
                        text = if (isTr) "Varsayılan Katı Blokla Başla (100x50x20 mm) →" else "Start with Default Solid Block (100x50x20 mm) →",
                        color = textColor,
                        fontSize = 11.sp,
                        textAlign = TextAlign.Center,
                        modifier = Modifier.padding(10.dp)
                    )
                }
            }
        }
    }
}

@Composable
private fun EmptyStateActionButton(
    title: String,
    description: String,
    color: Color,
    testTag: String,
    onClick: () -> Unit
) {
    Surface(
        color = color.copy(alpha = 0.12f),
        shape = RoundedCornerShape(8.dp),
        border = androidx.compose.foundation.BorderStroke(1.dp, color.copy(alpha = 0.5f)),
        modifier = Modifier
            .fillMaxWidth()
            .clickable(onClick = onClick)
            .testTag(testTag)
    ) {
        Column(modifier = Modifier.padding(12.dp)) {
            Text(
                text = title,
                color = color,
                fontSize = 13.sp,
                fontWeight = FontWeight.SemiBold
            )
            Spacer(modifier = Modifier.height(2.dp))
            Text(
                text = description,
                color = Color.Gray,
                fontSize = 10.sp
            )
        }
    }
}
