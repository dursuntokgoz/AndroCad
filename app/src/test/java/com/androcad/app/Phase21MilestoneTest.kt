package com.androcad.app

import org.junit.Assert.*
import org.junit.Test

class Phase21MilestoneTest {

    @Test
    fun testDomain1_CapillaryPressure() {
        val res = Phase21Features.evaluateYoungLaplace(0.0728, 45.0, 100.0, 50.0, 1000.0)
        assertTrue(res.youngLaplacePressurePa > 500.0)
        assertTrue(res.meniscusRadiusOfCurvatureMicrons > 0.0)
        assertTrue(res.capillaryRiseHeightMm > 100.0)
        assertTrue(res.spontaneousCapillaryFilling)
    }

    @Test
    fun testDomain2_DropletGeneration() {
        val res = Phase21Features.evaluateDropletGeneration(120.0, 20.0, 0.005, 0.005, 50.0)
        assertTrue(res.capillaryNumberCa in 0.005..0.25)
        assertTrue(res.dropletVolumePicoliters > 10.0)
        assertTrue(res.generationFrequencyHz > 10.0)
        assertTrue(res.monodisperseDropletRegime)
    }

    @Test
    fun testDomain3_DeanFocusing() {
        val res = Phase21Features.evaluateDeanFocusing(0.45, 200.0, 50.0, 15.0, 12.0, 0.001, 1000.0)
        assertTrue(res.reynoldsNumberRe > 10.0)
        assertTrue(res.deanNumberDe in 1.0..60.0)
        assertTrue(res.inertialLiftForcePicoNewtons > 1.0)
        assertTrue(res.singleStreamCellFocusingAchieved)
    }

    @Test
    fun testDomain4_ElectroOsmoticFlow() {
        val res = Phase21Features.evaluateElectroOsmoticFlow(25000.0, -45.0, 78.4, 0.001, 0.01)
        assertTrue(abs(res.eofVelocityMmPerS) > 0.05)
        assertTrue(res.debyeScreeningLengthNanometers in 1.0..50.0)
        assertTrue(res.plugFlowProfileMaintained)
    }

    @Test
    fun testDomain5_MicroDiffusion() {
        val res = Phase21Features.evaluateMicroDiffusion(2.5, 100.0, 4.5e-10)
        assertTrue(res.pecletNumberPe > 100.0)
        assertTrue(res.mixingLengthMm > 10.0)
        assertTrue(res.completeMolecularMixing)
    }

    @Test
    fun testDomain6_QuakeValve() {
        val res = Phase21Features.evaluateQuakeValve(150.0, 30.0, 100.0, 2.2)
        assertTrue(res.membraneDeflectionMicrons >= 25.0)
        assertTrue(res.closingResponseTimeMs < 20.0)
        assertTrue(res.hermeticMicrochannelSealing)
    }

    @Test
    fun testDomain7_EwodActuation() {
        val res = Phase21Features.evaluateEwodActuation(55.0, 115.0, 2.0, 2.8, 0.035)
        assertTrue(res.dynamicContactAngleDegrees < 115.0)
        assertTrue(res.electrowettingDrivingForceMicroNewtons >= 5.0)
        assertTrue(res.reliableDigitalDropletTransport)
    }

    @Test
    fun testDomain8_CfPcrThermal() {
        val res = Phase21Features.evaluateCfPcrThermal(1.2, 12.0, 20.0, 25.0, 35.0)
        assertTrue(res.denaturationTimeSeconds >= 3.0)
        assertTrue(res.annealingTimeSeconds >= 5.0)
        assertTrue(res.extensionTimeSeconds >= 5.0)
        assertTrue(res.dnaAmplificationSuccessful)
    }

    @Test
    fun testDomain9_SawSorting() {
        val res = Phase21Features.evaluateSawSorting(13.5, 1.5, 7.5, 1080.0, 1000.0, 1490.0)
        assertTrue(res.acousticRadiationForcePicoNewtons >= 10.0)
        assertTrue(res.standingWaveWavelengthMicrons > 50.0)
        assertTrue(res.highThroughputCellSeparation)
    }

    @Test
    fun testDomain10_OrganChipShear() {
        val res = Phase21Features.evaluateOrganChipShear(2.5, 400.0, 100.0, 0.0035, 8.0)
        assertTrue(res.wallShearStressDyneCm2 in 10.0..25.0)
        assertTrue(res.cyclicMechanicalStrainPercent <= 12.0)
        assertTrue(res.physiologicalEndothelialPhenotype)
    }
}
