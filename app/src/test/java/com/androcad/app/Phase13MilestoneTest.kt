package com.androcad.app

import org.junit.Assert.*
import org.junit.Test

class Phase13MilestoneTest {

    @Test
    fun testDomain1_BulbousBowWaveResistance() {
        val res = Phase13Features.optimizeHull(lengthM = 165.0, speedKnots = 21.0)
        assertTrue(res.valid)
        assertTrue(res.displacementTons > 15000.0)
        assertTrue(res.waveReductionPercent > 10.0)
    }

    @Test
    fun testDomain2_PropellerCavitation() {
        val res = Phase13Features.evaluatePropeller(diameterM = 5.2)
        assertTrue(res.safe)
        assertTrue(res.bar > 0.6)
        assertTrue(res.pressureKpa < 4.0)
    }

    @Test
    fun testDomain3_SeakeepingSlamming() {
        val res = Phase13Features.simulateSeakeeping(waveHeightM = 6.0)
        assertTrue(res.acceptable)
        assertTrue(res.pitchDeg < 5.0)
        assertTrue(res.slammingKpa > 200.0)
    }

    @Test
    fun testDomain4_HydrofoilPerformance() {
        val res = Phase13Features.analyzeHydrofoil(cruiseKnots = 38.0)
        assertTrue(res.stable)
        assertTrue(res.ldRatio > 12.0)
        assertTrue(res.subCavitating)
    }

    @Test
    fun testDomain5_ImoManeuverability() {
        val res = Phase13Features.verifyManeuvering(rudderAreaM2 = 22.5)
        assertTrue(res.imoCompliant)
        assertTrue(res.tacticalDiameterL < 5.0)
        assertTrue(res.steeringTorqueKnm > 200.0)
    }

    @Test
    fun testDomain6_SubmarineHullBuckling() {
        val res = Phase13Features.evaluateSubmarine(depthM = 400.0)
        assertTrue(res.safe)
        assertTrue(res.safetyFactor >= 2.0)
        assertTrue(res.collapsePressureMpa > 8.0)
    }

    @Test
    fun testDomain7_WaterjetPropulsion() {
        val res = Phase13Features.analyzeWaterjet(diameterM = 0.85)
        assertTrue(res.efficient)
        assertTrue(res.thrustKn > 50.0)
        assertTrue(res.efficiencyPercent > 80.0)
    }

    @Test
    fun testDomain8_MooringCatenary() {
        val res = Phase13Features.simulateMooring(depthM = 850.0)
        assertTrue(res.adequate)
        assertTrue(res.sfMbl >= 1.67)
        assertTrue(res.offsetM < 25.0)
    }

    @Test
    fun testDomain9_MarineRibletDrag() {
        val res = Phase13Features.evaluateRiblets(heightUm = 45.0)
        assertTrue(res.effective)
        assertTrue(res.dragReductionPercent > 5.0)
        assertTrue(res.bioFoulingInhibited)
    }

    @Test
    fun testDomain10_Marine2070Certification() {
        val cert = Phase13Features.certifyFaz13()
        assertEquals(2070, cert.totalMilestones)
        assertEquals(13, cert.totalPhases)
        assertEquals("v11.0.0", cert.semver)
        assertTrue(cert.zeroBroadStorage)
        assertTrue(cert.occt801)
        assertTrue(cert.buttonDeterminismVerified)
    }
}
