// Copyright (c) 2026 dursuntokgoz. All rights reserved.
package com.androcad.app.tutorial

import androidx.activity.compose.BackHandler
import androidx.compose.animation.AnimatedVisibility
import androidx.compose.animation.fadeIn
import androidx.compose.animation.fadeOut
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.layout.widthIn
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
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import java.util.Locale

@Composable
fun TutorialOverlayView(
    stepIndex: Int,
    totalSteps: Int,
    step: TutorialStep,
    isDarkTheme: Boolean,
    onNext: () -> Unit,
    onPrev: () -> Unit,
    onSkip: () -> Unit,
    onFinish: () -> Unit,
    modifier: Modifier = Modifier
) {
    // Intercept back gesture to dismiss tutorial cleanly
    BackHandler(enabled = true) {
        onSkip()
    }

    val isTurkish = Locale.getDefault().language.lowercase().startsWith("tr")
    val title = if (isTurkish) step.titleTr else step.titleEn
    val description = if (isTurkish) step.descriptionTr else step.descriptionEn
    val skipText = if (isTurkish) "Öğreticiyi Geç" else "Skip Tutorial"
    val prevText = if (isTurkish) "Geri" else "Back"
    val nextText = if (isTurkish) "Sonraki" else "Next"
    val finishText = if (isTurkish) "Tasarım Alanına Başla" else "Get Started"
    val stepLabel = if (isTurkish) "Adım ${stepIndex + 1} / $totalSteps" else "Step ${stepIndex + 1} of $totalSteps"
    val focusHint = if (isTurkish) "Hedef Eleman: [${step.targetTag}]" else "Target Control: [${step.targetTag}]"

    val cardBgColor = if (isDarkTheme) Color(0xFF1E2228) else Color(0xFFFFFFFF)
    val textColor = if (isDarkTheme) Color(0xFFF1F5F9) else Color(0xFF0F172A)
    val textMuted = if (isDarkTheme) Color(0xFF94A3B8) else Color(0xFF64748B)
    val borderColor = if (isDarkTheme) Color(0xFF334155) else Color(0xFFE2E8F0)
    val accentBlue = Color(0xFF0284C7)
    val scrimColor = Color(0x77000000)

    Box(
        modifier = modifier
            .fillMaxSize()
            .background(scrimColor)
            .testTag("tutorial_overlay")
            .semantics { contentDescription = "Onboarding Tutorial Overlay" }
            .clickable(onClick = { /* prevent click through */ }),
        contentAlignment = Alignment.Center
    ) {
        Surface(
            color = cardBgColor,
            shape = RoundedCornerShape(16.dp),
            shadowElevation = 12.dp,
            border = androidx.compose.foundation.BorderStroke(1.dp, borderColor),
            modifier = Modifier
                .padding(20.dp)
                .widthIn(max = 520.dp)
                .fillMaxWidth()
        ) {
            Column(
                modifier = Modifier.padding(24.dp)
            ) {
                // Header with step badge and Skip button
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.SpaceBetween,
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    Surface(
                        color = accentBlue.copy(alpha = 0.15f),
                        shape = RoundedCornerShape(20.dp),
                        modifier = Modifier.testTag("tutorial_step_indicator")
                    ) {
                        Text(
                            text = stepLabel,
                            color = accentBlue,
                            fontWeight = FontWeight.SemiBold,
                            fontSize = 12.sp,
                            modifier = Modifier
                                .padding(horizontal = 10.dp, vertical = 4.dp)
                                .semantics { contentDescription = stepLabel }
                        )
                    }

                    OutlinedButton(
                        onClick = onSkip,
                        modifier = Modifier
                            .testTag("tutorial_btn_skip")
                            .semantics { contentDescription = skipText },
                        border = androidx.compose.foundation.BorderStroke(1.dp, borderColor),
                        shape = RoundedCornerShape(8.dp)
                    ) {
                        Text(
                            text = skipText,
                            color = textMuted,
                            fontSize = 12.sp,
                            fontWeight = FontWeight.Medium
                        )
                    }
                }

                Spacer(modifier = Modifier.height(16.dp))

                // Title
                Text(
                    text = title,
                    color = textColor,
                    fontWeight = FontWeight.Bold,
                    fontSize = 20.sp
                )

                Spacer(modifier = Modifier.height(10.dp))

                // Description
                Text(
                    text = description,
                    color = if (isDarkTheme) Color(0xFFCBD5E1) else Color(0xFF334155),
                    fontSize = 14.sp,
                    lineHeight = 20.sp
                )

                Spacer(modifier = Modifier.height(12.dp))

                // Target element tag pill
                Surface(
                    color = if (isDarkTheme) Color(0xFF0F172A) else Color(0xFFF8FAFC),
                    shape = RoundedCornerShape(6.dp),
                    border = androidx.compose.foundation.BorderStroke(1.dp, borderColor)
                ) {
                    Text(
                        text = focusHint,
                        color = accentBlue,
                        fontSize = 11.sp,
                        fontFamily = androidx.compose.ui.text.font.FontFamily.Monospace,
                        modifier = Modifier.padding(horizontal = 8.dp, vertical = 4.dp)
                    )
                }

                Spacer(modifier = Modifier.height(24.dp))

                // Navigation buttons
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.SpaceBetween,
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    if (stepIndex > 0) {
                        OutlinedButton(
                            onClick = onPrev,
                            modifier = Modifier
                                .testTag("tutorial_btn_prev")
                                .semantics { contentDescription = prevText },
                            shape = RoundedCornerShape(8.dp)
                        ) {
                            Text(text = "← $prevText", color = textColor, fontSize = 13.sp)
                        }
                    } else {
                        Spacer(modifier = Modifier.width(1.dp))
                    }

                    if (stepIndex < totalSteps - 1) {
                        Button(
                            onClick = onNext,
                            colors = ButtonDefaults.buttonColors(containerColor = accentBlue),
                            shape = RoundedCornerShape(8.dp),
                            modifier = Modifier
                                .testTag("tutorial_btn_next")
                                .semantics { contentDescription = nextText }
                        ) {
                            Text(text = "$nextText →", color = Color.White, fontSize = 13.sp, fontWeight = FontWeight.SemiBold)
                        }
                    } else {
                        Button(
                            onClick = onFinish,
                            colors = ButtonDefaults.buttonColors(containerColor = Color(0xFF16A34A)),
                            shape = RoundedCornerShape(8.dp),
                            modifier = Modifier
                                .testTag("tutorial_btn_finish")
                                .semantics { contentDescription = finishText }
                        ) {
                            Text(text = "✓ $finishText", color = Color.White, fontSize = 13.sp, fontWeight = FontWeight.Bold)
                        }
                    }
                }
            }
        }
    }
}
