package com.androcad.app

import org.junit.Assert.*
import org.junit.Test

class Phase27MilestoneTest {

    @Test
    fun testDomain1_BoomBuckling() {
        val res = Phase27Features.evaluateBoomBuckling(105.0, 610.0, 25.0, 14500.0)
        assertTrue(res.certified)
        assertTrue(res.safetyFactor >= 2.0)
        assertTrue(res.eulerBucklingKn > 29000.0)
    }

    @Test
    fun testDomain2_FinalDrive() {
        val res = Phase27Features.evaluateFinalDrive(45.0)
        assertTrue(res.certified)
        assertTrue(res.outputTorqueKnm >= 1200.0)
        assertTrue(res.pittingSafetySh >= 1.30)
    }

    @Test
    fun testDomain3_WireRope() {
        val res = Phase27Features.evaluateWireRope(85.0, 2800.0)
        assertTrue(res.satisfied)
        assertTrue(res.dToDRatio >= 28.0)
        assertTrue(res.bendingCycles >= 50000.0)
    }

    @Test
    fun testDomain4_TubularJoint() {
        val res = Phase27Features.evaluateTubularJointScf()
        assertTrue(res.compliant)
        assertTrue(res.scf <= 2.85)
        assertTrue(res.fatigueLifeHours >= 25000.0)
    }

    @Test
    fun testDomain5_BucketDigging() {
        val res = Phase27Features.evaluateBucketDigging(65.0, 320.0, 4.2, 0.95)
        assertTrue(res.targetAchieved)
        assertTrue(res.pullForceKn >= 2400.0)
        assertTrue(res.bucketVolumeM3 >= 60.0)
    }

    @Test
    fun testDomain6_WalkingCam() {
        val res = Phase27Features.evaluateWalkingCam(1200.0, 38000.0, 3.8, 18.0)
        assertTrue(res.verified)
        assertTrue(res.groundPressureKpa <= 185.0)
        assertTrue(res.strideM >= 2.0)
    }

    @Test
    fun testDomain7_DrumBrake() {
        val res = Phase27Features.evaluateDrumBrake(4.5)
        assertTrue(res.approved)
        assertTrue(res.peakTempC < 350.0)
    }

    @Test
    fun testDomain8_SlewMachinery() {
        val res = Phase27Features.evaluateSlewMachinery()
        assertTrue(res.verified)
        assertTrue(res.slewAlphaRadS2 >= 0.025)
    }

    @Test
    fun testDomain9_GuyWire() {
        val res = Phase27Features.evaluateGuyWire(520.0)
        assertTrue(res.stable)
        assertTrue(res.pretensionKn >= 450.0)
        assertTrue(res.safetyFactor >= 3.0)
    }

    @Test
    fun testDomain10_CenterPin() {
        val res = Phase27Features.evaluateCenterPin()
        assertTrue(res.certified)
        assertTrue(res.contactPressureMpa < 45.0)
        assertTrue(res.shearStressMpa < 85.0)
    }
}
