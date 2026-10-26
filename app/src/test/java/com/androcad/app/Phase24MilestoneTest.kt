package com.androcad.app

import org.junit.Assert.*
import org.junit.Test

class Phase24MilestoneTest {

    @Test
    fun testDomain1_DfnElectrochemistry() {
        val res = Phase24Features.evaluateDfnElectrochemistry(120.0, 150.0, 298.15, 500.0)
        assertTrue(res.stable)
        assertTrue(res.cellVoltageV in 2.8..4.25)
        assertTrue(res.internalResistanceMOhm < 3.0)
    }

    @Test
    fun testDomain2_ThermalRunaway() {
        val res = Phase24Features.evaluateThermalRunaway(45.0, 1.0, 2.2)
        assertTrue(res.contained)
        assertTrue(res.peakTemperatureC <= 800.0)
        assertTrue(res.gasVolumeLiters < 120.0)
    }

    @Test
    fun testDomain3_PackCrash() {
        val res = Phase24Features.evaluatePackCrash(32.0, 2200.0, 180.0)
        assertTrue(res.approved)
        assertTrue(res.maxCellIntrusionMm < 30.0)
        assertTrue(res.plasticMarginMm >= 30.0)
    }

    @Test
    fun testDomain4_VentingGas() {
        val res = Phase24Features.evaluateVentingGas(850.0, 0.032)
        assertTrue(res.safeRelief)
        assertTrue(res.rupturePressureBar <= 8.0)
        assertEquals(1.0, res.throatMach, 0.001)
    }

    @Test
    fun testDomain5_ThermalBarrier() {
        val res = Phase24Features.evaluateThermalBarrier(750.0, 2.0, 0.018)
        assertTrue(res.propagationPrevented)
        assertTrue(res.maxAdjacentCellTempC < 65.0)
    }

    @Test
    fun testDomain6_ColdPlateCooling() {
        val res = Phase24Features.evaluateColdPlateCooling(15.0, 1800.0)
        assertTrue(res.optimalUniformity)
        assertTrue(res.coolantPressureDropKpa <= 25.0)
        assertTrue(res.tempUniformityDeltaC <= 2.5)
    }

    @Test
    fun testDomain7_BusbarJoint() {
        val res = Phase24Features.evaluateBusbarJoint(350.0, 48.0)
        assertTrue(res.certified)
        assertTrue(res.jointResistanceMicroOhm <= 1.2)
        assertTrue(res.jointTempRiseK < 15.0)
    }

    @Test
    fun testDomain8_PyroFuse() {
        val res = Phase24Features.evaluatePyroFuse(25.0, 800.0)
        assertTrue(res.isolationGuaranteed)
        assertTrue(res.clearingTimeMs < 1.5)
    }

    @Test
    fun testDomain9_SwellingBreathing() {
        val res = Phase24Features.evaluateSwelling(1.0, 800.0, 78.5)
        assertTrue(res.integrityPreserved)
        assertTrue(res.swellingPressureMpa <= 0.85)
        assertTrue(res.tieRodStressMpa <= 180.0)
    }

    @Test
    fun testDomain10_EisDiagnostic() {
        val res = Phase24Features.evaluateEisDiagnostic(2450.0)
        assertTrue(res.certified)
        assertTrue(res.sohPercent >= 80.0)
        assertTrue(res.chargeTransferMOhm < 1.5)
    }
}
