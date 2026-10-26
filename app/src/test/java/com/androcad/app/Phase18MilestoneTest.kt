package com.androcad.app

import org.junit.Assert.*
import org.junit.Test

class Phase18MilestoneTest {

    @Test
    fun testDomain1_SwissLeverEscapement() {
        val res = Phase18Features.evaluateSwissLeverEscapement(15.0, 32.0, 2.0, 3.2)
        assertTrue(res.liftAngleDegrees >= 48.0)
        assertTrue(res.lockAngleDegrees in 1.5..3.0)
        assertTrue(res.escapementOperatingReliably)
    }

    @Test
    fun testDomain2_HairspringIsochronism() {
        val res = Phase18Features.calculateHairspringOscillation(0.12, 0.035, 125.0, 12.0, 170.0)
        assertTrue(res.naturalFrequencyHz in 2.5..5.0)
        assertTrue(res.isochronismErrorSecondsPerDay <= 2.0)
        assertTrue(res.meetsCoscIsochronism)
    }

    @Test
    fun testDomain3_GearTrainMeshing() {
        val res = Phase18Features.calculateGearTrainMeshing(84, 12, 80, 10, 75, 10, 80, 10, 12.0)
        assertTrue(res.totalGearRatio >= 3000.0)
        assertTrue(res.escapeWheelTorqueMicroNm >= 1.5)
        assertTrue(res.smoothContinuousTransmission)
    }

    @Test
    fun testDomain4_MainspringBarrel() {
        val res = Phase18Features.evaluateMainspringPowerReserve(0.11, 1.25, 380.0, 5.2, 1.1)
        assertTrue(res.powerReserveHours >= 48.0)
        assertTrue(res.powerReserveExceeds48Hours)
    }

    @Test
    fun testDomain5_TourbillonDynamics() {
        val res = Phase18Features.evaluateTourbillonCancellation(0.28, 6.5, 60.0, 7.5)
        assertTrue(res.tourbillonCompensatedDriftSecPerDay <= 1.20)
        assertTrue(res.achievesSuperiorRateStability)
    }

    @Test
    fun testDomain6_AutomaticWinding() {
        val res = Phase18Features.evaluateAutomaticWinding(2.8, 12.0, 1.2, 145.0)
        assertTrue(res.windingTorqueDeliveredMilliNm >= 8.0)
        assertTrue(res.bidirectionalWindingEffective)
    }

    @Test
    fun testDomain7_ChronographReset() {
        val res = Phase18Features.evaluateChronographReset(1.8, 2.2, 0.15)
        assertTrue(res.heartCamResetVelocityMs >= 1.0)
        assertTrue(res.instantaneousZeroReset)
    }

    @Test
    fun testDomain8_BalancePoising() {
        val res = Phase18Features.evaluateBalanceWheelPoising(9.8, 0.8, 0.4, 0.8, 4.5)
        assertTrue(res.poisingErrorMicrogramsMm < 15.0)
        assertTrue(res.positionalRateVariationSecPerDay < 1.0)
        assertTrue(res.perfectlyPoisedBalance)
    }

    @Test
    fun testDomain9_JewelBearingShock() {
        val res = Phase18Features.evaluateJewelBearingShock(0.08, 1.2, 5000.0, 180.0)
        assertTrue(res.incablocAxialDeflectionMicrons <= 45.0)
        assertTrue(res.withstandsIso1413DropTest)
    }

    @Test
    fun testDomain10_CoscCertification() {
        val rates = listOf(1.2, 2.1, -0.8, 0.5, 1.8)
        val res = Phase18Features.evaluateCoscCompliance(rates, 1.5, 0.2)
        assertTrue(res.meanDailyRateSecondsPerDay in -4.0..6.0)
        assertTrue(res.meanVariationInRatesSecPerDay <= 2.0)
        assertTrue(res.maxRateDifferenceBetweenPositionsSecPerDay <= 10.0)
        assertTrue(res.certifiedChronometerGrade)
    }
}
