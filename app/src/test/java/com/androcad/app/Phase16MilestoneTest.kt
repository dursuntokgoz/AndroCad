package com.androcad.app

import org.junit.Assert.*
import org.junit.Test

class Phase16MilestoneTest {

    @Test
    fun testDomain1_GisFieldStress() {
        val res = Phase16Features.evaluateGisElectricField(145.0, 50.0, 150.0, 0.55)
        assertTrue(res.schwaigerFactor > 0.35)
        assertTrue(res.dielectricSafetyMargin >= 1.25)
        assertTrue(res.withstandsLightningImpulse)
    }

    @Test
    fun testDomain2_ArcPlasma() {
        val res = Phase16Features.simulateArcPlasma(40000.0, 45.0, 0.85, 340.0)
        assertTrue(res.centralArcTemperatureKelvin in 10000.0..22000.0)
        assertTrue(res.arcQuenchedSuccessfully)
    }

    @Test
    fun testDomain3_TrvInterruption() {
        val res = Phase16Features.evaluateTrvInterruption(63.0, 0.25, 4.5, 50.0)
        assertTrue(res.peakTrvKv in 150.0..300.0)
        assertTrue(res.timeToPeakMicroseconds >= 20.0)
        assertTrue(res.meetsIec62271_100)
    }

    @Test
    fun testDomain4_EcoGasMixture() {
        val res = Phase16Features.evaluateEcoGasMixture(5.0, 85.0, 10.0, 0.70)
        assertTrue(res.dielectricStrengthRatioToSf6 >= 0.80)
        assertTrue(res.globalWarmingPotentialGwp < 1000.0)
        assertTrue(res.compliantWithEuFgasRegulation)
    }

    @Test
    fun testDomain5_VftoDynamics() {
        val res = Phase16Features.calculateVftoWavefront(145.0, 65.0, 25.0, 5.0)
        assertTrue(res.wavefrontRiseTimeNanoseconds in 3.0..20.0)
        assertTrue(res.peakVftoPu <= 2.2)
        assertTrue(res.spacerInsulatorProtected)
    }

    @Test
    fun testDomain6_VacuumArc() {
        val res = Phase16Features.evaluateVacuumArc(40.0, 80.0, 2.0, 12.0)
        assertTrue(res.axialMagneticFieldTesla >= res.criticalAmfThresholdTesla)
        assertTrue(res.avoidsAnodeSpotMelting)
    }

    @Test
    fun testDomain7_BusbarForces() {
        val res = Phase16Features.calculateBusbarForces(63.0, 450.0, 1200.0, 45.0)
        assertTrue(res.peakShortCircuitCurrentKa >= 150.0)
        assertTrue(res.busbarBendingStressMpa < 220.0)
        assertTrue(res.withstands63KaShortCircuit)
    }

    @Test
    fun testDomain8_BushingStress() {
        val res = Phase16Features.evaluateBushingStress(145.0, 35.0, 1800.0, 3.6)
        assertTrue(res.partialDischargePicoCoulombs < 5.0)
        assertTrue(res.meetsIec60137)
    }

    @Test
    fun testDomain9_SwitchgearKinematics() {
        val res = Phase16Features.simulateOperatingMechanism(8500.0, 28.0, 120.0, 0.15)
        assertTrue(res.totalOpeningTimeMilliseconds < 45.0)
        assertTrue(res.contactSeparationVelocityMs >= 3.0)
        assertTrue(res.highSpeedInterruptionMet)
    }

    @Test
    fun testDomain10_ArcFlashHazard() {
        val res = Phase16Features.evaluateArcFlashHazard(31.5, 0.08, 610.0, 1000.0)
        assertTrue(res.incidentEnergyCalCm2 <= 25.0)
        assertTrue(res.peakBlastPressureKpa < 80.0)
        assertTrue(res.internalArcClassifiedIac)
    }
}
