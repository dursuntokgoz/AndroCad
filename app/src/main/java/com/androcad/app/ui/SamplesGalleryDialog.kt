// Copyright (c) 2026 dursuntokgoz. All rights reserved.
package com.androcad.app.ui

import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.layout.widthIn
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.Button
import androidx.compose.material3.ButtonDefaults
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.platform.testTag
import androidx.compose.ui.semantics.contentDescription
import androidx.compose.ui.semantics.semantics
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.androcad.app.samples.SampleDesign
import com.androcad.app.samples.SamplesManager
import java.util.Locale

@Composable
fun SamplesGalleryDialog(
    isDarkTheme: Boolean,
    onSelectSample: (SampleDesign) -> Unit,
    onDismiss: () -> Unit,
    modifier: Modifier = Modifier
) {
    val isTurkish = Locale.getDefault().language.lowercase().startsWith("tr")
    val title = if (isTurkish) "Örnek CAD Tasarımları" else "Sample CAD Designs"
    val subtitle = if (isTurkish) {
        "AndroCAD'in parametrik özelliklerini keşfetmek için hazır özgün modellerden birini açın."
    } else {
        "Open an original sample design to explore AndroCAD's parametric modeling features."
    }
    val closeText = if (isTurkish) "Kapat" else "Close"
    val openText = if (isTurkish) "Modeli Aç" else "Open Model"

    val dialogBgColor = if (isDarkTheme) Color(0xFF1E2228) else Color(0xFFFFFFFF)
    val cardBgColor = if (isDarkTheme) Color(0xFF282C34) else Color(0xFFF1F5F9)
    val textColor = if (isDarkTheme) Color(0xFFF1F5F9) else Color(0xFF0F172A)
    val textMuted = if (isDarkTheme) Color(0xFF94A3B8) else Color(0xFF64748B)
    val borderColor = if (isDarkTheme) Color(0xFF3B4252) else Color(0xFFCBD5E1)
    val accentBlue = Color(0xFF0284C7)
    val scrimColor = Color(0x88000000)

    Box(
        modifier = modifier
            .fillMaxSize()
            .background(scrimColor)
            .testTag("samples_gallery_dialog")
            .semantics { contentDescription = "Samples Gallery Dialog" }
            .clickable(onClick = onDismiss),
        contentAlignment = Alignment.Center
    ) {
        Surface(
            color = dialogBgColor,
            shape = RoundedCornerShape(16.dp),
            shadowElevation = 16.dp,
            border = androidx.compose.foundation.BorderStroke(1.dp, borderColor),
            modifier = Modifier
                .padding(16.dp)
                .widthIn(max = 680.dp)
                .fillMaxWidth()
                .clickable(enabled = false, onClick = {}) // block background dismiss
        ) {
            Column(
                modifier = Modifier.padding(20.dp)
            ) {
                // Header
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.SpaceBetween,
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    Column {
                        Text(
                            text = title,
                            color = textColor,
                            fontWeight = FontWeight.Bold,
                            fontSize = 20.sp
                        )
                        Spacer(modifier = Modifier.height(4.dp))
                        Text(
                            text = subtitle,
                            color = textMuted,
                            fontSize = 12.sp
                        )
                    }

                    OutlinedButton(
                        onClick = onDismiss,
                        modifier = Modifier
                            .testTag("samples_btn_close")
                            .semantics { contentDescription = closeText },
                        border = androidx.compose.foundation.BorderStroke(1.dp, borderColor),
                        shape = RoundedCornerShape(8.dp)
                    ) {
                        Text(text = closeText, color = textColor, fontSize = 12.sp)
                    }
                }

                Spacer(modifier = Modifier.height(16.dp))

                // Samples list
                Column(
                    modifier = Modifier.verticalScroll(rememberScrollState()),
                    verticalArrangement = Arrangement.spacedBy(12.dp)
                ) {
                    SamplesManager.SAMPLES.forEach { sample ->
                        val sampleTitle = if (isTurkish) sample.nameTr else sample.nameEn
                        val sampleDesc = if (isTurkish) sample.descriptionTr else sample.descriptionEn

                        Surface(
                            color = cardBgColor,
                            shape = RoundedCornerShape(10.dp),
                            border = androidx.compose.foundation.BorderStroke(1.dp, borderColor),
                            modifier = Modifier.fillMaxWidth()
                        ) {
                            Row(
                                modifier = Modifier
                                    .padding(14.dp)
                                    .fillMaxWidth(),
                                verticalAlignment = Alignment.CenterVertically
                            ) {
                                // Thumbnail badge / placeholder
                                Surface(
                                    color = accentBlue.copy(alpha = 0.2f),
                                    shape = RoundedCornerShape(8.dp),
                                    modifier = Modifier.size(54.dp)
                                ) {
                                    Box(contentAlignment = Alignment.Center) {
                                        Text(
                                            text = "CAD",
                                            color = accentBlue,
                                            fontWeight = FontWeight.Bold,
                                            fontSize = 13.sp
                                        )
                                    }
                                }

                                Spacer(modifier = Modifier.width(14.dp))

                                Column(modifier = Modifier.weight(1f)) {
                                    Text(
                                        text = sampleTitle,
                                        color = textColor,
                                        fontWeight = FontWeight.SemiBold,
                                        fontSize = 15.sp
                                    )
                                    Spacer(modifier = Modifier.height(3.dp))
                                    Text(
                                        text = sampleDesc,
                                        color = textMuted,
                                        fontSize = 12.sp,
                                        lineHeight = 16.sp
                                    )
                                    Spacer(modifier = Modifier.height(2.dp))
                                    Text(
                                        text = "Vol: ~${sample.nominalVolumeMm3.toInt()} mm³ | Parametric DAG",
                                        color = accentBlue,
                                        fontSize = 10.sp,
                                        fontFamily = androidx.compose.ui.text.font.FontFamily.Monospace
                                    )
                                }

                                Spacer(modifier = Modifier.width(10.dp))

                                Button(
                                    onClick = { onSelectSample(sample) },
                                    colors = ButtonDefaults.buttonColors(containerColor = accentBlue),
                                    shape = RoundedCornerShape(8.dp),
                                    modifier = Modifier
                                        .testTag(sample.testTag)
                                        .semantics { contentDescription = "$openText $sampleTitle" }
                                ) {
                                    Text(
                                        text = openText,
                                        color = Color.White,
                                        fontSize = 12.sp,
                                        fontWeight = FontWeight.SemiBold
                                    )
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
