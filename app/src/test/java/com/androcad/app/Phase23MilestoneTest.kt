package com.androcad.app

import org.junit.Assert.*
import org.junit.Test

class Phase23MilestoneTest {

    @Test
    fun testDomain1_Whiffletree() {
        val res = Phase23Features.evaluateWhiffletree(4.0, 0.25, 6200.0, 27, 60.0)
        assertTrue(res.diffractionLimited)
        assertTrue(res.rmsSurfaceDeformationNm <= 15.0)
        assertEquals(27, res.supportPointsCount)
    }

    @Test
    fun testDomain2_ActiveOptics() {
        val res = Phase23Features.evaluateActiveOptics(85.0, 100.0)
        assertTrue(res.certified)
        assertTrue(res.residualWavefrontRmsNm <= 10.0)
        assertTrue(res.strokeResolutionNm < 2.0)
    }

    @Test
    fun testDomain3_ZernikeAberrations() {
        val res = Phase23Features.evaluateZernikeAberrations(632.8, 0.025, 0.018, 0.012, 0.008)
        assertTrue(res.marechalCriterionMet)
        assertTrue(res.strehlRatio >= 0.80)
        assertTrue(res.totalRmsNm < 45.0)
    }

    @Test
    fun testDomain4_Hexapod() {
        val res = Phase23Features.evaluateHexapodKinematics(450.0, 320.0)
        assertTrue(res.verified)
        assertTrue(res.maxAxialPosErrorUm < 0.5)
        assertTrue(res.maxAngularPosErrorUrad < 1.0)
    }

    @Test
    fun testDomain5_ZerodurThermals() {
        val res = Phase23Features.evaluateZerodurThermals(4.0, 0.25, 0.5, 0.01)
        assertTrue(res.figureNegligible)
        assertTrue(res.thermalDeformationPeakNm < 5.0)
    }

    @Test
    fun testDomain6_SpiderDiffraction() {
        val res = Phase23Features.evaluateSpiderDiffraction(4.0, 1.8, 80.0, 12.0)
        assertTrue(res.compliant)
        assertTrue(res.firstEigenFreqHz >= 18.0)
    }

    @Test
    fun testDomain7_CryoDewar() {
        val res = Phase23Features.evaluateCryoDewar(77.0, 295.0, 6, 85.0, 45.0, 30.0)
        assertTrue(res.budgetSatisfied)
        assertTrue(res.totalHeatLoadMw < 1200.0)
    }

    @Test
    fun testDomain8_FastSteeringMirror() {
        val res = Phase23Features.evaluateFastSteeringMirror(0.00035, 420.0, 18.5, 1.4)
        assertTrue(res.jitterSuppressed)
        assertTrue(res.bandwidthHz >= 200.0)
        assertTrue(res.strokeArcsec >= 120.0)
    }

    @Test
    fun testDomain9_HydrostaticBearing() {
        val res = Phase23Features.evaluateHydrostaticBearing(0.25, 450.0, 0.068)
        assertTrue(res.trackingSmooth)
        assertTrue(res.oilFilmThicknessUm >= 35.0)
        assertTrue(res.frictionCoefficient < 0.0005)
    }

    @Test
    fun testDomain10_DomeSeeing() {
        val res = Phase23Features.evaluateDomeSeeing(0.4, 3.5)
        assertTrue(res.subArcsecondAchieved)
        assertTrue(res.seeingAngleArcsec < 0.85)
        assertTrue(res.friedParameterR0Cm >= 12.0)
    }
}
