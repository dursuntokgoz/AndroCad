package com.androcad.app

import org.junit.Assert.assertTrue
import org.junit.Test

class Phase29MilestoneTest {

    @Test
    fun testDomain1ThermalBudget() {
        val res = Phase29Features.calculateCryostatThermalBudget()
        assertTrue(res.message, res.passed)
    }

    @Test
    fun testDomain2CoaxThermal() {
        val res = Phase29Features.evaluateCoaxialCabling()
        assertTrue(res.message, res.passed)
    }

    @Test
    fun testDomain3DilutionCooling() {
        val res = Phase29Features.calculateDilutionCoolingPower()
        assertTrue(res.message, res.passed)
    }

    @Test
    fun testDomain4CavityResonator() {
        val res = Phase29Features.evaluateCavityResonator()
        assertTrue(res.message, res.passed)
    }

    @Test
    fun testDomain5MagneticShielding() {
        val res = Phase29Features.calculateMagneticShielding()
        assertTrue(res.message, res.passed)
    }

    @Test
    fun testDomain6CryoTruss() {
        val res = Phase29Features.evaluateCryogenicTruss()
        assertTrue(res.message, res.passed)
    }

    @Test
    fun testDomain7PurcellFilter() {
        val res = Phase29Features.evaluatePurcellFilter()
        assertTrue(res.message, res.passed)
    }

    @Test
    fun testDomain8CryoHemt() {
        val res = Phase29Features.evaluateCryoHemt()
        assertTrue(res.message, res.passed)
    }

    @Test
    fun testDomain9TwpaAmplifier() {
        val res = Phase29Features.evaluateTwpaParametric()
        assertTrue(res.message, res.passed)
    }

    @Test
    fun testDomain10CryoRadiation() {
        val res = Phase29Features.calculateCryoRadiationShield()
        assertTrue(res.message, res.passed)
    }
}
