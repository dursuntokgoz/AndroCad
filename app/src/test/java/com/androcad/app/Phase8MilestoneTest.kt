package com.androcad.app

import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test

class Phase8MilestoneTest {

    @Test
    fun testDomain1_SpatialRayPickingDeterminism() {
        val pick = Phase8Features.verifyDeterministicRayPick(
            rayOriginZ = 100.0,
            rayDirZ = -1.0,
            boxZTop = 20.0,
            boxZBottom = 0.0
        )
        assertTrue("Ray should hit model", pick.hit)
        assertTrue("Top tap must hit top face", pick.isTopFace)
        assertEquals(6, pick.targetEntityId)
        assertEquals("FACE", pick.entityType)
        assertEquals(20.0, pick.hitZ, 1e-4)
        assertFalse("Front face must not be occluded", pick.isOccluded)
    }

    @Test
    fun testDomain2_MoldToolingFeasibility() {
        val mold = Phase8Features.analyzeMoldCavity(partHeightMm = 40.0, draftAngleDeg = 2.5)
        assertTrue("Mold extraction should be feasible", mold.extractionFeasible)
        assertFalse("Adequate draft angle must have no undercuts", mold.hasUndercuts)
        assertEquals(20.0, mold.partingPlaneZ, 1e-4)
        assertTrue("Ejector pins must be >= 6", mold.ejectorPinCount >= 6)
    }

    @Test
    fun testDomain3_AIGenerativeSynthesis() {
        val gen = Phase8Features.synthesizeAIGenerativeDAG("Aero bracket with 4 mounting holes")
        assertTrue("Synthesis should succeed", gen.successful)
        assertTrue("Mass reduction should exceed 35%", gen.massReductionPercent > 35.0)
        assertTrue("Stiffness ratio should exceed 100", gen.stiffnessRatio > 100.0)
        assertEquals(14, gen.featureCount)
    }

    @Test
    fun testDomain5_TubeHydroforming() {
        val hydro = Phase8Features.simulateHydroforming(1100.0)
        assertTrue("Hydroforming should be successful", hydro.successful)
        assertTrue("Tube expansion should be > 20%", hydro.maxExpansionPercent > 20.0)
        assertTrue("Wall thickness should be > 1.2mm", hydro.minWallThicknessMm > 1.2)
    }

    @Test
    fun testDomain9_DigitalTwinMachineStatus() {
        val twin = Phase8Features.monitorDigitalTwin(vibrationRms = 1.2)
        assertTrue("Telemetry must be synced", twin.synced)
        assertFalse("Normal vibration must not trigger chatter", twin.chatterDetected)
        assertTrue("Edge latency must be under 5ms", twin.latencyMs < 5.0)
        assertEquals("OPTIMAL_CONTINUOUS_MILLING", twin.status)
    }
}
