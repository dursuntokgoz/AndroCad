// Copyright (c) 2026 dursuntokgoz. All rights reserved.
package com.androcad.app

import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test
import kotlin.math.abs

class Phase6MilestoneTest {

    @Test
    fun testPhase6DomainStructure() {
        val domains = Phase6Features.getAllDomains()
        assertEquals(10, domains.size)
        assertEquals(471, Phase6Features.FAZ6_START_MILESTONE)
        assertEquals(670, Phase6Features.FAZ6_END_MILESTONE)
        assertEquals(200, Phase6Features.FAZ6_TOTAL_MILESTONES)
        assertEquals(670, Phase6Features.getFullSystemCertifiedMilestoneCount())
        assertTrue(domains.all { it.verified })
    }

    @Test
    fun testDomain1SynchronousOffsetVolume() {
        val vol = Phase6Features.computeSynchronousOffsetVolume(4, 2500.0, 5.0)
        assertEquals(50000.0, vol, 0.001)
    }

    @Test
    fun testDomain2InvoluteGearPitchDiameter() {
        val d = Phase6Features.computePitchDiameter(24, 2.5)
        assertEquals(60.0, d, 0.001)
    }

    @Test
    fun testDomain3AsymmetricFilletBlendWidth() {
        val width = Phase6Features.computeEllipticalBlendWidth(3.0, 4.0)
        assertEquals(5.0, width, 0.001)
    }

    @Test
    fun testDomain4RmsCurvature() {
        val rms = Phase6Features.computeRmsCurvature(0.03, 0.04)
        assertEquals(0.035355, rms, 0.0001)
    }

    @Test
    fun testDomain5AdaptiveKFactor() {
        val k1 = Phase6Features.computeAdaptiveKFactor(0.5, 2.0) // ratio 0.25 < 1.0
        assertTrue(k1 >= 0.33 && k1 <= 0.44)
        val k2 = Phase6Features.computeAdaptiveKFactor(4.0, 2.0) // ratio 2.0 >= 1.0
        assertEquals(0.44, k2, 0.001)
    }

    @Test
    fun testDomain6RayPickTolerance() {
        assertTrue(Phase6Features.isRayPickWithinTolerance(0.002, 0.005))
        assertFalse(Phase6Features.isRayPickWithinTolerance(0.008, 0.005))
    }

    @Test
    fun testDomain7RssToleranceStackup() {
        // sqrt(0.03^2 + 0.04^2) = 0.05
        val rss = Phase6Features.computeRssTolerance(listOf(0.03, 0.04))
        assertEquals(0.05, rss, 0.001)
    }

    @Test
    fun testDomain8Tet10NodeCountEstimation() {
        val nodes = Phase6Features.estimateTet10Nodes(15000)
        assertEquals(60000, nodes)
    }

    @Test
    fun testDomain9TheoreticalSurfaceRoughness() {
        val ra = Phase6Features.computeTheoreticalRa(0.004) // 4 microns cusp
        assertEquals(1.0, ra, 0.001) // 1.0 micrometer Ra
    }
}
