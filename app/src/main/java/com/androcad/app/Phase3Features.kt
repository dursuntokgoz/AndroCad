// Copyright (c) 2026 dursuntokgoz. All rights reserved.
package com.androcad.app

import kotlin.math.PI
import kotlin.math.abs
import kotlin.math.cbrt
import kotlin.math.cos
import kotlin.math.pow
import kotlin.math.round
import kotlin.math.sin
import kotlin.math.sqrt
import kotlin.math.tan

/**
 * Faz 3 (Milestones 21..70): Post-1.0 Hardening & Expansion Engine for Android Kotlin/Compose layer.
 */
object Phase3Features {

    val SUPPORTED_LOCALES = listOf("tr", "en", "de", "ja", "ar")

    private val LOCALIZED_TABS = mapOf(
        "tr" to mapOf("Solid" to "Katı", "Sketch" to "Eskiz", "Surface" to "Yüzey", "Simulation" to "Simülasyon", "CAM" to "Üretim", "Inspect" to "Denetim"),
        "en" to mapOf("Solid" to "Solid", "Sketch" to "Sketch", "Surface" to "Surface", "Simulation" to "Simulation", "CAM" to "CAM", "Inspect" to "Inspect"),
        "de" to mapOf("Solid" to "Volumen", "Sketch" to "Skizze", "Surface" to "Fläche", "Simulation" to "Simulation", "CAM" to "CAM", "Inspect" to "Prüfen"),
        "ja" to mapOf("Solid" to "ソリッド", "Sketch" to "スケッチ", "Surface" to "サーフェス", "Simulation" to "解析", "CAM" to "CAM", "Inspect" to "検査"),
        "ar" to mapOf("Solid" to "مجسم", "Sketch" to "رسم", "Surface" to "سطح", "Simulation" to "محاكاة", "CAM" to "تصنيع", "Inspect" to "فحص")
    )

    fun isRtlLocale(locale: String): Boolean = (locale == "ar")

    fun translateTab(locale: String, tabKey: String): String {
        return LOCALIZED_TABS[locale]?.get(tabKey) ?: tabKey
    }

    fun computeVariableFilletVolume(dx: Double, dy: Double, dz: Double, rStart: Double, rMid: Double, rEnd: Double): Double {
        val steps = 100
        var sumR2 = 0.0
        for (i in 0..steps) {
            val u = i.toDouble() / steps
            val rU = (1.0 - u) * (1.0 - u) * rStart + 2.0 * (1.0 - u) * u * rMid + u * u * rEnd
            val w = if (i == 0 || i == steps) 0.5 else 1.0
            sumR2 += w * rU * rU
        }
        val meanR2 = sumR2 / steps
        val removed = (1.0 - PI / 4.0) * meanR2 * dx
        return (dx * dy * dz - removed).coerceAtLeast(1.0)
    }

    fun computeAsymmetricChamferVolume(dx: Double, dy: Double, dz: Double, d1: Double, d2: Double, angleDeg: Double): Double {
        val effD2 = if (d2 > 0.0) d2 else d1 * tan(angleDeg * PI / 180.0)
        val removed = 0.5 * d1 * effD2 * dx
        return (dx * dy * dz - removed).coerceAtLeast(1.0)
    }

    fun computeEulerBucklingLoadN(lengthMm: Double, widthMm: Double, thicknessMm: Double, youngsModulusMPa: Double): Double {
        val inertia = (widthMm * thicknessMm.pow(3.0)) / 12.0
        return (PI * PI * youngsModulusMPa * inertia) / (lengthMm * lengthMm)
    }

    fun computeInvoluteGearPitchDiameter(moduleMm: Double, teeth: Int): Double = moduleMm * teeth

    fun computeIso286Fit(nominalDiaMm: Double, shaftZone: String): Triple<Double, Double, String> {
        val iUnit = 0.45 * cbrt(nominalDiaMm) + 0.001 * nominalDiaMm
        val it7 = round(16.0 * iUnit)
        val it6 = round(10.0 * iUnit)
        val holeLower = 0.0
        val holeUpper = it7
        val (shaftLower, shaftUpper) = when (shaftZone) {
            "g6" -> {
                val dev = round(-2.5 * nominalDiaMm.pow(0.34))
                Pair(dev - it6, dev)
            }
            "p6" -> {
                val dev = round(it7 + 4.0)
                Pair(dev, dev + it6)
            }
            else -> Pair(-it6, 0.0)
        }
        val minClearance = holeLower - shaftUpper
        val maxClearance = holeUpper - shaftLower
        val cls = when {
            minClearance >= 0.0 -> "Clearance Fit"
            maxClearance <= 0.0 -> "Interference Fit"
            else -> "Transition Fit"
        }
        return Triple(minClearance, maxClearance, cls)
    }

    fun verifyStylusJitterReduction(rawRms: Double, filteredRms: Double): Double {
        return 1.0 - (filteredRms / rawRms.coerceAtLeast(1e-6))
    }

    fun computeSimpTargetVolume(initialVol: Double, targetFraction: Double): Double {
        return initialVol * targetFraction.coerceIn(0.1, 0.9)
    }

    fun formatIgesHeader(partName: String): String {
        val sLine = ("AndroCAD IGES 5.3 Exporter - $partName").padEnd(72, ' ') + "S      1"
        val gLine = "1H,,1H;,7Hmodel.i,10HAndroCAD,6HOCCT78,32,38,6,308,15,,1.0,2,2HMM,1;".padEnd(72, ' ') + "G      1"
        return "$sLine\n$gLine"
    }
}
