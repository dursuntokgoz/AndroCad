package com.androcad.app

import org.junit.Assert.*
import org.junit.Test

class Phase20MilestoneTest {

    @Test
    fun testDomain1_InducerCavitation() {
        val res = Phase20Features.evaluateInducerCavitation(0.085, 32000.0, 18.0, 140.0, 55.0)
        assertTrue(res.suctionSpecificSpeedNss >= 25000.0)
        assertTrue(res.inducerHeadRiseM > 100.0)
        assertTrue(res.cavitationFreeOperation)
    }

    @Test
    fun testDomain2_ImpellerStress() {
        val res = Phase20Features.evaluateImpellerStress(185.0, 32000.0, 4430.0, 880.0, 180.0)
        assertTrue(res.burstMarginFactor >= 1.25)
        assertTrue(res.maxTipSpeedMs < 650.0)
        assertTrue(res.impellerStructuralIntegrityMet)
    }

    @Test
    fun testDomain3_RegenCooling() {
        val res = Phase20Features.calculateRegenCooling(100.0, 160.0, 1780.0, 45.0, 1.8, 4.5, 320.0)
        assertTrue(res.hotWallTempK <= 850.0)
        assertTrue(res.peakHeatFluxMwM2 <= 75.0)
        assertTrue(res.chamberWallThermalMarginSufficient)
    }

    @Test
    fun testDomain4_AcousticDamping() {
        val res = Phase20Features.evaluateAcousticDamping(340.0, 1150.0, 14.0, 38.0, 33.0)
        assertTrue(res.firstTangentialFrequencyHz > 1500.0)
        assertTrue(res.acousticAbsorptionCoeff >= 0.80)
        assertTrue(res.preventsHighFrequencyChugging)
    }

    @Test
    fun testDomain5_TurbineBlisk() {
        val res = Phase20Features.evaluateTurbineBlisk(920.0, 140.0, 1.85, 22.0, 105.0, 32000.0)
        assertTrue(res.shaftPowerDeliveredKw >= 2000.0)
        assertTrue(res.bladeRootCentrifugalStressMpa < 520.0)
        assertTrue(res.turbineBliskWithinCreepLimit)
    }

    @Test
    fun testDomain6_InjectorAtomization() {
        val res = Phase20Features.evaluateInjectorAtomization(1.2, 0.45, 1140.0, 422.0, 4.2, 15.0)
        assertTrue(res.sauterMeanDiameterMicrons <= 45.0)
        assertTrue(res.mixingEfficiencyPercent >= 97.0)
        assertTrue(res.optimalDropletAtomization)
    }

    @Test
    fun testDomain7_CryoBearing() {
        val res = Phase20Features.evaluateCryoBearing(60.0, 45.0, 35.0, 1.8e-4, 32000.0, 3500.0)
        assertTrue(res.fluidFilmThicknessMicrons >= 8.0)
        assertTrue(res.directStiffnessNPerM >= 1e7)
        assertTrue(res.stableRotorDynamicOperation)
    }

    @Test
    fun testDomain8_BellNozzleContour() {
        val res = Phase20Features.calculateBellNozzleContour(160.0, 38.0, 1.22, 1.013, 220.0)
        assertEquals(38.0, res.areaExpansionRatio, 0.001)
        assertTrue(res.exitMachNumber > 4.0)
        assertTrue(res.flowSeparationAvoided)
    }

    @Test
    fun testDomain9_CryoChilldown() {
        val res = Phase20Features.simulateCryoChilldown(75.0, 6.0, 2.5, 12.0, 293.0)
        assertTrue(res.filmBoilingDurationSeconds < 30.0)
        assertTrue(res.peakWaterHammerPressureBar < 25.0)
        assertTrue(res.rapidCryoChilldownSuccessful)
    }

    @Test
    fun testDomain10_TvcActuator() {
        val res = Phase20Features.evaluateTvcActuator(1200.0, 850.0, 7.5, 20.0, 0.85)
        assertTrue(res.frequencyBandwidthHz >= 25.0)
        assertTrue(res.slewRateDegreesPerSecond >= 20.0)
        assertTrue(res.tvcFlightControlMeetsAs9100)
    }
}
