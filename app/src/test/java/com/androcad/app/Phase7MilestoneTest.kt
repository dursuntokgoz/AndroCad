package com.androcad.app

import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test

class Phase7MilestoneTest {

    @Test
    fun testDomain1_TopologySynthesis() {
        val result = Phase7Features.runTopologySynthesis(
            targetVolumeFraction = 0.35,
            evolutionaryRate = 0.05,
            voxelGridSize = 5000L
        )
        assertTrue("Topology synthesis should converge", result.converged)
        assertTrue("Iterations should be > 0", result.iterationsRun > 0)
        assertEquals(0.35, result.finalVolumeFraction, 0.01)
        assertTrue("Compliance should be positive", result.finalCompliance > 0.0)
    }

    @Test
    fun testDomain2_CoupledMultiphysics() {
        val result = Phase7Features.solveCoupledThermalStructural()
        assertTrue("System should be thermally stable", result.isStable)
        assertTrue("Max temperature should exceed ambient", result.maxTempC > 20.0)
        assertTrue("Coupled safety factor should exceed 1.2", result.coupledSafetyFactor >= 1.2)
    }

    @Test
    fun testDomain4_DeterministicSubPixelPick() {
        val hit = Phase7Features.performDeterministicSubPixelPick(150.0f, 250.0f, "EDGE")
        assertTrue("Ray pick should hit entity", hit.hasHit)
        assertEquals(42L, hit.entityId)
        assertEquals("EDGE", hit.entityType)
        assertTrue("Sub-pixel tolerance should be under 1px", hit.screenDistancePx < 1.0)
    }

    @Test
    fun testDomain5_ProgressiveDieDesign() {
        val res = Phase7Features.designProgressiveDie()
        assertTrue("Die layout should be feasible", res.isFeasible)
        assertEquals(6, res.stationCount)
        assertTrue("Nesting efficiency should be > 30%", res.blankNestingEfficiency > 30.0)
    }

    @Test
    fun testDomain10_DigitalTwinSync() {
        val sync = Phase7Features.syncDigitalTwin("DMG-MORI-5X-CNC-04", "TURBINE-09")
        assertTrue("Digital twin should be synchronized", sync.isSynchronized)
        assertTrue("Sync latency should be sub-10ms", sync.latencyMs < 10.0)
        assertTrue("Hash should contain STEP AP242", sync.stepAP242Hash.contains("STEP-AP242-PMI"))
    }
}
