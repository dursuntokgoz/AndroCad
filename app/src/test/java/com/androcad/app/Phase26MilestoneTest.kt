package com.androcad.app

import org.junit.Assert.*
import org.junit.Test

class Phase26MilestoneTest {

    @Test
    fun testDomain1_TitaniumSphere() {
        val res = Phase26Features.evaluateTitaniumSphere(2.1, 11000.0)
        assertTrue(res.certified)
        assertTrue(res.safetyFactor >= 1.50)
        assertTrue(res.pressureMpa >= 114.5)
    }

    @Test
    fun testDomain2_AcrylicViewport() {
        val res = Phase26Features.evaluateAcrylicViewport(200.0)
        assertTrue(res.approved)
        assertTrue(res.thicknessMm >= 250.0)
        assertTrue(res.creepDeflectionMm < 1.5)
    }

    @Test
    fun testDomain3_SyntacticFoam() {
        val res = Phase26Features.evaluateSyntacticFoam(8.5)
        assertTrue(res.certified)
        assertTrue(res.collapsePressureMpa >= 160.0)
        assertTrue(res.netLiftKn > 30.0)
    }

    @Test
    fun testDomain4_PbofEnclosure() {
        val res = Phase26Features.evaluatePbofEnclosure()
        assertTrue(res.sealed)
        assertTrue(res.deltaPBar <= 0.35)
        assertTrue(res.dielectricKv >= 35.0)
    }

    @Test
    fun testDomain5_Manipulator() {
        val res = Phase26Features.evaluateSubseaManipulator(150.0)
        assertTrue(res.verified)
        assertTrue(res.payloadKg >= 150.0)
        assertTrue(res.reachM >= 1.5)
    }

    @Test
    fun testDomain6_Penetrator() {
        val res = Phase26Features.evaluateHighPressurePenetrator()
        assertTrue(res.certified)
        assertTrue(res.heliumLeakRate < 1e-9)
        assertTrue(res.insulationResistanceGOhm >= 5.0)
    }

    @Test
    fun testDomain7_LifeSupport() {
        val res = Phase26Features.evaluateLifeSupport(3, 18.0)
        assertTrue(res.compliant)
        assertTrue(res.enduranceHours >= 72.0)
        assertTrue(res.co2Mbar < 5.0)
    }

    @Test
    fun testDomain8_AscentBallast() {
        val res = Phase26Features.evaluateAscentBallast()
        assertTrue(res.guaranteed)
        assertTrue(res.ascentSpeedMs >= 1.2)
        assertTrue(res.holdingForceKn >= 15.0)
    }

    @Test
    fun testDomain9_Thruster() {
        val res = Phase26Features.evaluateSubmersibleThruster(5.0)
        assertTrue(res.verified)
        assertTrue(res.thrustN >= 1800.0)
        assertTrue(res.breakawayTorqueNm > 60.0)
    }

    @Test
    fun testDomain10_UsblTelemetry() {
        val res = Phase26Features.evaluateUsblTelemetry(10920.0)
        assertTrue(res.locked)
        assertTrue(res.slantRangeM >= 11000.0)
        assertTrue(res.accuracyM < 25.0)
    }
}
