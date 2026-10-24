// Copyright (c) 2026 dursuntokgoz. All rights reserved.
package com.androcad.app

import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test
import kotlin.math.abs

class Phase5MilestoneTest {

    @Test
    fun testPhase5DomainStructure() {
        val domains = Phase5Features.getAllDomains()
        assertEquals(10, domains.size)
        assertEquals(271, Phase5Features.FAZ5_START_MILESTONE)
        assertEquals(470, Phase5Features.FAZ5_END_MILESTONE)
        assertEquals(200, Phase5Features.FAZ5_TOTAL_MILESTONES)
        assertEquals(470, Phase5Features.getFullSystemCertifiedMilestoneCount())
        assertTrue(domains.all { it.verified })
    }

    @Test
    fun testDomain1DirectModelingPushPull() {
        val delta = Phase5Features.computePushPullVolumeDelta(2500.0, 10.0)
        assertEquals(25000.0, delta, 0.001)
    }

    @Test
    fun testDomain2BSplineKnotCount() {
        // Degree 3 with 8 poles has 8 + 3 + 1 = 12 knots
        val knotCount = Phase5Features.computeKnotCount(3, 8)
        assertEquals(12, knotCount)
    }

    @Test
    fun testDomain3VariableFilletInterpolation() {
        val midRadius = Phase5Features.computeVariableRadius(2.0, 8.0, 0.5)
        assertEquals(5.0, midRadius, 0.001)
    }

    @Test
    fun testDomain4CurvatureAnalysis() {
        val (mean, gaussian) = Phase5Features.computeCurvatures(50.0, 50.0)
        assertEquals(0.02, mean, 0.001)
        assertEquals(0.0004, gaussian, 0.00001)
    }

    @Test
    fun testDomain5SheetMetalBendAllowance() {
        // 90 degree bend with R=2mm, T=2mm, K=0.44
        val allowance = Phase5Features.computeBendAllowance(90.0, 2.0, 2.0, 0.44)
        assertTrue(allowance > 4.5 && allowance < 4.6)
    }

    @Test
    fun testDomain6RaySnappingDistance() {
        assertTrue(Phase5Features.isWithinSnapThreshold(100.0, 100.0, 105.0, 105.0, 12.0))
        assertFalse(Phase5Features.isWithinSnapThreshold(100.0, 100.0, 120.0, 120.0, 12.0))
    }

    @Test
    fun testDomain7KinematicJointAngleClamping() {
        assertEquals(45.0, Phase5Features.clampJointAngle(45.0, -180.0, 180.0), 0.001)
        assertEquals(180.0, Phase5Features.clampJointAngle(210.0, -180.0, 180.0), 0.001)
        assertEquals(-180.0, Phase5Features.clampJointAngle(-200.0, -180.0, 180.0), 0.001)
    }

    @Test
    fun testDomain8TopologyOptimizationSIMP() {
        val baseE = 70.0 // GPa (Aluminum)
        val penalizedE = Phase5Features.computeSimpModulus(baseE, 0.5, 3.0)
        assertEquals(70.0 * 0.125, penalizedE, 0.001)
    }

    @Test
    fun testDomain9AdaptiveScallopStepover() {
        // Ball end mill R=5mm, cusp=0.01mm
        val stepover = Phase5Features.computeScallopStepover(5.0, 0.01)
        assertTrue(stepover > 0.6 && stepover < 0.7)
    }
}
