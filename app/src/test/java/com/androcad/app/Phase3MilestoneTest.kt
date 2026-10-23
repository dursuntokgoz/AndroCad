// Copyright (c) 2026 dursuntokgoz. All rights reserved.
package com.androcad.app

import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test

class Phase3MilestoneTest {

    @Test
    fun testPhase3VariableFilletAndAsymmetricChamferMath() {
        val volFillet = Phase3Features.computeVariableFilletVolume(100.0, 50.0, 30.0, 2.0, 5.0, 8.0)
        assertTrue(volFillet < 150000.0 && volFillet > 140000.0)

        val volChamfer = Phase3Features.computeAsymmetricChamferVolume(100.0, 50.0, 30.0, 4.0, 8.0, 45.0)
        assertEquals(150000.0 - 1600.0, volChamfer, 0.01)
    }

    @Test
    fun testPhase3LocalizationAndRtlSupport() {
        assertEquals(5, Phase3Features.SUPPORTED_LOCALES.size)
        assertEquals("Katı", Phase3Features.translateTab("tr", "Solid"))
        assertEquals("Volumen", Phase3Features.translateTab("de", "Solid"))
        assertEquals("ソリッド", Phase3Features.translateTab("ja", "Solid"))
        assertEquals("مجسم", Phase3Features.translateTab("ar", "Solid"))
        assertTrue(Phase3Features.isRtlLocale("ar"))
        assertFalse(Phase3Features.isRtlLocale("tr"))
    }

    @Test
    fun testPhase3Iso286FitsAndGearPitch() {
        val (minClr, maxClr, clsClr) = Phase3Features.computeIso286Fit(25.0, "g6")
        assertTrue(minClr >= 0.0)
        assertTrue(maxClr > minClr)
        assertEquals("Clearance Fit", clsClr)

        val (_, maxInt, clsInt) = Phase3Features.computeIso286Fit(40.0, "p6")
        assertTrue(maxInt <= 0.0)
        assertEquals("Interference Fit", clsInt)

        assertEquals(60.0, Phase3Features.computeInvoluteGearPitchDiameter(2.5, 24), 1e-6)
    }

    @Test
    fun testPhase3BucklingStylusSimpAndIges() {
        val pCr = Phase3Features.computeEulerBucklingLoadN(200.0, 20.0, 4.0, 210000.0)
        assertTrue(pCr > 5000.0)

        val jitterRed = Phase3Features.verifyStylusJitterReduction(0.48, 0.09)
        assertTrue(jitterRed >= 0.75)

        val simpVol = Phase3Features.computeSimpTargetVolume(100000.0, 0.40)
        assertEquals(40000.0, simpVol, 0.01)

        val igesHdr = Phase3Features.formatIgesHeader("Bracket")
        assertTrue(igesHdr.contains("S      1"))
        assertTrue(igesHdr.contains("G      1"))
    }
}
