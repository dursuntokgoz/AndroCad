package com.androcad.app

import org.junit.Assert.*
import org.junit.Test

class Phase25MilestoneTest {

    @Test
    fun testDomain1_KalkerContact() {
        val res = Phase25Features.evaluateKalkerContact(85.0, 460.0)
        assertTrue(res.adhesionSatisfied)
        assertTrue(res.peakHertzMpa < 1500.0)
        assertTrue(res.aMm > 0.0)
    }

    @Test
    fun testDomain2_HuntingStability() {
        val res = Phase25Features.evaluateHuntingStability(350.0, 0.05)
        assertTrue(res.huntingSuppressed)
        assertTrue(res.vCritKmh >= 380.0)
        assertTrue(res.dampingRatio >= 0.05)
    }

    @Test
    fun testDomain3_PrimarySuspension() {
        val res = Phase25Features.evaluatePrimarySuspension(170.0, 42.0, 175.0, 5)
        assertTrue(res.compliant)
        assertTrue(res.verticalDeflectionMm in 25.0..65.0)
    }

    @Test
    fun testDomain4_AirSpring() {
        val res = Phase25Features.evaluateSecondaryAirSpring(4.8, 60.0)
        assertTrue(res.certified)
        assertTrue(res.verticalStiffnessKnM in 250.0..650.0)
    }

    @Test
    fun testDomain5_AntiYawDamper() {
        val res = Phase25Features.evaluateAntiYawDamper(16.5)
        assertTrue(res.prevented)
        assertTrue(res.marginFactor >= 1.35)
    }

    @Test
    fun testDomain6_BogieFrameFatigue() {
        val res = Phase25Features.evaluateBogieFrameFatigue(160.0, 48.0, 32.0)
        assertTrue(res.approved)
        assertTrue(res.dangVanRatio <= 0.85)
    }

    @Test
    fun testDomain7_DiscBrake() {
        val res = Phase25Features.evaluateDiscBrake(350.0, 14500.0)
        assertTrue(res.prevented)
        assertTrue(res.stoppingDistanceM < 4500.0)
        assertTrue(res.peakDiscTempC <= 650.0)
    }

    @Test
    fun testDomain8_NadalDerailment() {
        val res = Phase25Features.evaluateNadalDerailment(42.0, 85.0, 70.0)
        assertTrue(res.runningSafetyVerified)
        assertTrue(res.yqRatio <= 0.80)
    }

    @Test
    fun testDomain9_TractionDrive() {
        val res = Phase25Features.evaluateTractionDrive(1850.0, 15.0)
        assertTrue(res.approved)
        assertTrue(res.misalignmentDeg <= 2.5)
        assertTrue(res.shaftStressMpa < 120.0)
    }

    @Test
    fun testDomain10_SperlingRideIndex() {
        val res = Phase25Features.evaluateSperlingRideIndex()
        assertTrue(res.certified)
        assertTrue(res.wzVert <= 2.2)
        assertTrue(res.wzLat <= 2.2)
    }
}
