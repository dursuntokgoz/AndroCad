package com.androcad.app

import org.junit.Assert.*
import org.junit.Test

class Phase19MilestoneTest {

    @Test
    fun testDomain1_CrashBoxCrushing() {
        val res = Phase19Features.evaluateCrashBoxCrushing(80.0, 2.0, 140.0, 340.0, 450.0, 6.0)
        assertTrue(res.crushForceEfficiencyPercent >= 70.0)
        assertTrue(res.meanCrushForceKn > 40.0)
        assertTrue(res.stableAxialFolding)
    }

    @Test
    fun testDomain2_JohnsonCookStress() {
        val res = Phase19Features.evaluateJohnsonCookStress(0.15, 500.0, 350.0, 520.0, 750.0, 0.35, 0.015, 1.05)
        assertTrue(res.dynamicFlowStressMpa in 520.0..1600.0)
        assertTrue(res.strainRateEnhancementRatio > 1.0)
        assertTrue(res.ahssYieldCriterionMet)
    }

    @Test
    fun testDomain3_AirbagInflation() {
        val res = Phase19Features.simulateAirbagInflation(60.0, 55.0, 28.0, 25.0)
        assertTrue(res.peakBagPressureKpa in 25.0..48.0)
        assertTrue(res.timeToFullInflationMs < 40.0)
        assertTrue(res.safeDeploymentCompliance)
    }

    @Test
    fun testDomain4_SideImpactIntrusion() {
        val res = Phase19Features.evaluateBPillarIntrusion(1350.0, 50.0, 2.2, 15.0, 220.0)
        assertTrue(res.residualCabinSurvivalSpaceMm >= 140.0)
        assertTrue(res.maxBPillarIntrusionVelocityMs < 9.0)
        assertTrue(res.meetsFmvss214SideImpact)
    }

    @Test
    fun testDomain5_BiomechanicalInjury() {
        val res = Phase19Features.calculateInjuryCriteria(10.0, 25.0, 52.0, 68.0, 34.0, 2.2)
        assertTrue(res.headInjuryCriterionHic15 < 700.0)
        assertTrue(res.chestDeflectionMm < 42.0)
        assertTrue(res.euroNcapFiveStarCompliant)
    }

    @Test
    fun testDomain6_SpotWeldFailure() {
        val res = Phase19Features.evaluateSpotWeldIntegrity(2.5, 3.8, 1.4, 6.0, 600.0)
        assertTrue(res.mixedModeDamageIndex <= 1.0)
        assertTrue(res.spotWeldMaintainsStructuralIntegrity)
    }

    @Test
    fun testDomain7_BatteryPoleCrash() {
        val res = Phase19Features.evaluateBatteryPoleCrash(35.0, 4.5, 85.0, 75.0)
        assertTrue(res.remainingClearanceBufferMm >= 15.0)
        assertEquals(0.0, res.cellDeformationStrainPercent, 0.001)
        assertTrue(res.preventsBatteryThermalRunaway)
    }

    @Test
    fun testDomain8_PedestrianImpact() {
        val res = Phase19Features.evaluatePedestrianImpact(40.0, 25.0, 28.0, 0.8)
        assertTrue(res.headformHicScore < 1000.0)
        assertTrue(res.legformBendingAngleDegrees <= 19.0)
        assertTrue(res.compliesWithGtr9PedestrianSafety)
    }

    @Test
    fun testDomain9_SeatbeltPretensioner() {
        val res = Phase19Features.evaluateSeatbeltPretensioner(32.0, 78.0, 95.0, 22.0)
        assertTrue(res.shoulderBeltPlateauForceKn in 3.5..5.0)
        assertTrue(res.pelvisDisplacementMm < 120.0)
        assertTrue(res.limitsChestOverload)
    }

    @Test
    fun testDomain10_RcarBumperCrash() {
        val res = Phase19Features.evaluateRcarLowSpeedBumper(1550.0, 15.0, 180.0, 90.0)
        assertEquals(0.0, res.chassisRailResidualDeformationMm, 0.001)
        assertTrue(res.estimatedRepairCostEuro < 1000.0)
        assertTrue(res.zeroStructuralChassisDamage)
    }
}
