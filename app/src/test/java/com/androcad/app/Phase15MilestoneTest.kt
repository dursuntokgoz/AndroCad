package com.androcad.app

import org.junit.Assert.*
import org.junit.Test

class Phase15MilestoneTest {

    @Test
    fun testDomain1_RPVStress() {
        val res = Phase15Features.calculateRPVStress(15.5, 2200.0, 220.0, 180.0)
        assertTrue(res.hoopStressMpa in 150.0..180.0)
        assertTrue(res.longitudinalStressMpa in 70.0..90.0)
        assertTrue(res.safetyFactor >= 1.15)
        assertTrue(res.meetsAsmeSectionIII)
    }

    @Test
    fun testDomain2_ThermalCreep() {
        val res = Phase15Features.evaluateThermalCreep(120.0, 550.0, 80000.0, 1.0e-5, 4.0)
        assertTrue(res.withinCreepLimit)
        assertTrue(res.totalCreepStrainPercent < 1.0)
        assertTrue(res.relaxedStressMpa <= 120.0)
    }

    @Test
    fun testDomain3_RadiationEmbrittlement() {
        val res = Phase15Features.evaluateNeutronEmbrittlement(3.5, 0.08, 0.65, -20.0)
        assertTrue(res.deltaRtNdtCelsius in 40.0..150.0)
        assertTrue(res.adjustedRtNdtCelsius < 132.0)
        assertTrue(res.upperShelfEnergyJoules >= 68.0)
        assertTrue(res.acceptableCharpyToughness)
    }

    @Test
    fun testDomain4_ShieldingAttenuation() {
        val res = Phase15Features.calculateShieldingAttenuation(160.0, 0.12, 0.8, 50000.0)
        assertTrue(res.exitDoseRateMilliSvPerHour < 0.02)
        assertTrue(res.attenuationDecades > 5.0)
        assertTrue(res.meetsAlaraStandard)
    }

    @Test
    fun testDomain5_CrdmScram() {
        val res = Phase15Features.simulateCrdmScram(120.0, 3.8, 720.0, 0.45, 600.0)
        assertTrue(res.scramTimeSeconds in 0.50..2.50)
        assertTrue(res.fastScramCompliant)
    }

    @Test
    fun testDomain6_DnbrMargin() {
        val res = Phase15Features.evaluateThermalHydraulicDnbr(3500.0, 15.5, 1250.0, 1200.0)
        assertTrue(res.dnbr >= 1.30)
        assertTrue(res.noNucleateBoilingDeparture)
    }

    @Test
    fun testDomain7_SeismicSseResponse() {
        val res = Phase15Features.evaluateSeismicResponse(45000.0, 85000.0, 0.35, 0.05)
        assertTrue(res.naturalFrequencyHz in 4.0..15.0)
        assertTrue(res.maxDisplacementMm < 25.0)
        assertTrue(res.meetsSseMargin)
    }

    @Test
    fun testDomain8_ShroudFiv() {
        val res = Phase15Features.evaluateShroudVibration(4.2, 3400.0, 12.5, 0.20)
        assertTrue(res.vortexSheddingFrequencyHz < 1.0)
        assertTrue(res.lockInResonanceAvoided)
    }

    @Test
    fun testDomain9_ZircaloyOxidation() {
        val res = Phase15Features.evaluateZircaloyOxidation(1050.0, 120.0, 15.0)
        assertTrue(res.compliesWith10Cfr5046)
        assertTrue(res.equivalentCladdingReactedPercent < 17.0)
        assertTrue(res.oxideLayerThicknessMicrons in 15.0..80.0)
    }

    @Test
    fun testDomain10_ContainmentTendonCapacity() {
        val res = Phase15Features.evaluateContainmentTendonLoss(1100.0, 120.0, 45.0, 22.0, 1.2)
        assertTrue(res.prestressLossPercent < 22.0)
        assertTrue(res.ultimateBurstPressureMpa >= 0.90)
        assertTrue(res.maintainsContainmentIntegrity)
    }
}
