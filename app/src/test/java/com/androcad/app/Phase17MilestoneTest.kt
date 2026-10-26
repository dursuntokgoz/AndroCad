package com.androcad.app

import org.junit.Assert.*
import org.junit.Test

class Phase17MilestoneTest {

    @Test
    fun testDomain1_HoekBrown() {
        val res = Phase17Features.evaluateHoekBrown(5.0, 120.0, 65.0, 28.0, 0.0)
        assertTrue(res.majorPrincipalStrengthMpa in 40.0..150.0)
        assertTrue(res.rockMassModulusGpa > 15.0)
        assertTrue(res.rockMassStable)
    }

    @Test
    fun testDomain2_CsmCutterForces() {
        val res = Phase17Features.calculateCsmCutterForces(17.0, 13.0, 85.0, 6.0, 110.0, 14.0)
        assertTrue(res.normalForceKn in 80.0..280.0)
        assertTrue(res.rollingForceKn in 5.0..45.0)
        assertTrue(res.cutterLoadWithinBearingLimit)
    }

    @Test
    fun testDomain3_SegmentalLining() {
        val res = Phase17Features.evaluateSegmentalLining(3.2, 22.0, 85.0, 0.35, 120.0)
        assertTrue(res.jointOpeningDisplacementMm < 2.0)
        assertTrue(res.boltTensionStressMpa < 640.0)
        assertTrue(res.liningIntegrityMaintained)
    }

    @Test
    fun testDomain4_EpbChamberPressure() {
        val res = Phase17Features.evaluateEpbFacePressure(18.0, 3.0, 19.0, 15.0, 28.0)
        assertTrue(res.requiredChamberPressureBar in 1.5..5.0)
        assertTrue(res.faceStabilitySafetyFactor >= 1.05)
        assertTrue(res.surfaceCollapseAvoided)
    }

    @Test
    fun testDomain5_PeckSettlement() {
        val res = Phase17Features.calculatePeckSettlement(20.0, 6.5, 0.8, 5.0)
        assertTrue(res.maxSettlementMm < 15.0)
        assertTrue(res.buildingDamageCategory <= 1)
        assertTrue(res.settlementWithinNegligibleLimit)
    }

    @Test
    fun testDomain6_CcmConvergence() {
        val res = Phase17Features.evaluateCcmConvergence(4.0, 8.0, 2.5, 32.0, 8.0)
        assertTrue(res.wallConvergencePercent < 1.5)
        assertTrue(res.plasticRadiusMeters < 10.0)
        assertTrue(res.groundConfinementAdequate)
    }

    @Test
    fun testDomain7_TbmPowerTorque() {
        val res = Phase17Features.calculateTbmPowerTorque(6.5, 42, 5.5, 7.0, 90.0)
        assertTrue(res.cutterheadPowerKw <= 4500.0)
        assertTrue(res.requiredTorqueKnM <= 6500.0)
        assertTrue(res.drivePowerSufficient)
    }

    @Test
    fun testDomain8_TailGrout() {
        val res = Phase17Features.evaluateTailGrouting(3.2, 3.32, 125.0, 10.0, 1.8)
        assertTrue(res.groutVolumeRequiredM3 in 2.0..5.0)
        assertTrue(res.netTunnelBuoyancyKnPerMeter <= 15.0)
        assertTrue(res.preventsTunnelFloatation)
    }

    @Test
    fun testDomain9_ShieldJammingRisk() {
        val res = Phase17Features.evaluateShieldJammingRisk(3.5, 12.0, 0.85, 9.0, 35.0, 45000.0)
        assertTrue(res.thrustMarginPercent >= 25.0)
        assertTrue(res.shieldFreeFromJamming)
    }

    @Test
    fun testDomain10_TunnelSeismic() {
        val res = Phase17Features.evaluateTunnelSeismicResponse(0.35, 650.0, 3.2, 35.0)
        assertTrue(res.liningShearStrainMicrostrain < res.concreteTensileStrainLimit)
        assertTrue(res.earthquakeResistantLining)
    }
}
