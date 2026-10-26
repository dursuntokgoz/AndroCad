package com.androcad.app

import org.junit.Assert.*
import org.junit.Test

class Phase22MilestoneTest {

    @Test
    fun testDomain1_BemAerodynamics() {
        val res = Phase22Features.evaluateBemAerodynamics(10.5, 75.0, 55.0, 12.0, 1.2, 2.4, 5.8, 0.008)
        assertTrue(res.converged)
        assertTrue(res.prandtlTipLossFactor > 0.2)
        assertTrue(res.sectionalThrustForceNPerM > 100.0)
    }

    @Test
    fun testDomain2_PitchBearing() {
        val res = Phase22Features.evaluatePitchBearing(60.0, 120, 45.0, 8500.0, 450.0, 220.0)
        assertTrue(res.certified)
        assertTrue(res.maxHertzContactStressMpa < 2500.0)
        assertTrue(res.racewayFatigueLifeHours >= 175200.0)
    }

    @Test
    fun testDomain3_PlanetaryGearbox() {
        val res = Phase22Features.evaluatePlanetaryGearbox(3200.0, 25, 45, 115, 5, 550.0, 20.0)
        assertTrue(res.certified)
        assertTrue(res.pittingSafetySh >= 1.25)
        assertTrue(res.bendingSafetySf >= 1.40)
    }

    @Test
    fun testDomain4_AeroelasticFlutter() {
        val res = Phase22Features.evaluateAeroelasticFlutter(75.0, 4.5e8, 3.2e7, 180.0, 240.0, 8.2)
        assertTrue(res.flutterMarginSafe)
        assertTrue(res.frequencySeparationRatio >= 2.5)
        assertTrue(res.flutterOnsetWindSpeedMs > 35.0)
    }

    @Test
    fun testDomain5_PmGenerator() {
        val res = Phase22Features.evaluatePmGenerator(5.0, 12.0, 3.2, 1.8, 64.0, 1.25)
        assertTrue(res.directDriveCompliant)
        assertTrue(res.electricalEfficiencyPercent >= 95.0)
        assertTrue(res.coggingTorquePercent <= 1.5)
    }

    @Test
    fun testDomain6_TowerViv() {
        val res = Phase22Features.evaluateTowerViv(3.8, 5.6, 120.0, 0.28, 0.02)
        assertTrue(res.resonanceSuppressed)
        assertTrue(res.tmdDampingRatio >= 0.05)
    }

    @Test
    fun testDomain7_TrailingEdgeNoise() {
        val res = Phase22Features.evaluateTrailingEdgeNoise(78.0, 1.4, 25.0, 40.0, 80.0)
        assertTrue(res.compliant)
        assertTrue(res.noiseReductionDeltaDba >= 3.0)
    }

    @Test
    fun testDomain8_PitchActuation() {
        val res = Phase22Features.evaluatePitchActuation(45.0, 18.0, 1.2e5, 12.0, 8.5)
        assertTrue(res.ipcApproved)
        assertTrue(res.maxPitchRateDegPerSec >= 8.0)
    }

    @Test
    fun testDomain9_OffshoreMonopile() {
        val res = Phase22Features.evaluateOffshoreMonopile(8.0, 75.0, 35.0, 2500.0, 65000.0, 34.0)
        assertTrue(res.geotechnicalIntegrityMet)
        assertTrue(res.accumulatedTiltDeg < 0.25)
    }

    @Test
    fun testDomain10_BladeFatigue() {
        val res = Phase22Features.evaluateBladeFatigue(1200.0, 850.0, 0.045, 650.0, 1e7)
        assertTrue(res.iec61400Passed)
        assertTrue(res.minerDamageIndexD < 0.85)
    }
}
