// Copyright (c) 2026 dursuntokgoz. All rights reserved.
package com.androcad.app

import com.androcad.kernel.NativeKernel
import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test
import kotlin.math.abs

class Phase4MilestoneTest {

    @Test
    fun testTopTapSelectsTopEdgeAndNeverBottomEdge() {
        val box = NativeKernel.createBox(100.0, 50.0, 20.0)
        assertTrue(box.success)

        // Tap near top-front edge (50, 0, 20) -> must resolve to a top edge (id 9..12 at z=20), NEVER bottom edge (1..4 at z=0)
        val topFrontHit = Triple(50f, 0f, 20f)
        val resolvedTopFrontEdge = box.resolveTargetEdgeId("Face", 6, topFrontHit)
        assertTrue(
            "Top tap at (50,0,20) must select top edge (9..12), got Edge #$resolvedTopFrontEdge",
            resolvedTopFrontEdge in 9..12
        )

        // Tap near top-back edge (50, 50, 20) -> must resolve to a top edge (9..12 at z=20)
        val topBackHit = Triple(50f, 50f, 20f)
        val resolvedTopBackEdge = box.resolveTargetEdgeId("Face", 6, topBackHit)
        assertTrue(
            "Top tap at (50,50,20) must select top edge (9..12), got Edge #$resolvedTopBackEdge",
            resolvedTopBackEdge in 9..12
        )

        // Tap at interior of top face (50, 25, 20) -> resolveTargetFaceId must return Face #6 (+Z top face)
        val topCenterHit = Triple(50f, 25f, 20f)
        val topNormal = Triple(0f, 0f, 1f)
        val resolvedTopFace = box.resolveTargetFaceId("Face", 0, topCenterHit, topNormal)
        assertEquals(6, resolvedTopFace)

        // Fillet/Chamfer target at interior of top face (50, 25, 20) (>12mm from edges) -> must return -1006 (Face #6 perimeter loop)
        val loopTarget = box.resolveFilletChamferTarget("Face", 6, topCenterHit, topNormal)
        assertEquals(-1006, loopTarget)

        // Fillet/Chamfer target near top edge (50, 2, 20) (<=12mm from top-front edge) -> must return top edge id (9..12)
        val nearTopEdgeHit = Triple(50f, 2f, 20f)
        val edgeTarget = box.resolveFilletChamferTarget("Face", 6, nearTopEdgeHit, topNormal)
        assertTrue("Near-edge tap on top face must return top edge 9..12, got $edgeTarget", edgeTarget in 9..12)
    }

    @Test
    fun testZUpCameraOrthogonalityAtAllNamedViews() {
        val views = listOf(
            Pair(45f, 30f),   // HOME / ISO
            Pair(0f, 90f),    // TOP (+90 deg)
            Pair(0f, -90f),   // BOTTOM (-90 deg)
            Pair(0f, 0f),     // FRONT
            Pair(90f, 0f),    // RIGHT
            Pair(-90f, 0f)    // LEFT
        )
        for ((azim, elev) in views) {
            val (f, u) = Phase4Features.computeZUpCameraVectors(azim, elev)
            val fLen = kotlin.math.sqrt((f.first * f.first + f.second * f.second + f.third * f.third).toDouble())
            val uLen = kotlin.math.sqrt((u.first * u.first + u.second * u.second + u.third * u.third).toDouble())
            val dot = f.first * u.first + f.second * u.second + f.third * u.third
            assertEquals("Forward vector must be unit length at ($azim, $elev)", 1.0, fLen, 1e-5)
            assertEquals("Up vector must be unit length at ($azim, $elev)", 1.0, uLen, 1e-5)
            assertEquals("Forward and Up vectors must be orthogonal at ($azim, $elev)", 0.0, dot.toDouble(), 1e-5)
        }
    }

    @Test
    fun testAll200Faz4DomainsAndEngineeringFormulas() {
        val domains = Phase4Features.getAllDomains()
        assertEquals(10, domains.size)
        assertEquals(71, domains.first().startMilestone)
        assertEquals(270, domains.last().endMilestone)
        val totalMilestones = domains.sumOf { it.endMilestone - it.startMilestone + 1 }
        assertEquals(200, totalMilestones)

        val (wireLen, helixAngle) = Phase4Features.computeHelixWireLength(20.0, 5.0, 50.0)
        assertTrue(wireLen > 1250.0)
        assertTrue(helixAngle > 2.0 && helixAngle < 5.0)

        val strainRatio = Phase4Features.computeSnapFitStrainRatio(20.0, 2.0, 1.0, 2400.0, 65.0)
        assertTrue(strainRatio < 1.0)

        val asmeThick = Phase4Features.computeAsmeVesselThicknessMm(2.5, 250.0, 138.0, 0.85)
        assertTrue(asmeThick > 5.0 && asmeThick < 6.0)

        val (rss, cpk) = Phase4Features.computeRssToleranceAndCpk(listOf(0.10, 0.08, 0.08), 0.35)
        assertTrue(rss < 0.26)
        assertTrue(cpk > 1.33)

        val fos = Phase4Features.computeGoodmanFatigueFos(130.0, -50.0, 500.0)
        assertTrue(fos > 1.2)

        val gyroidOrigin = Phase4Features.evaluateTpmsGyroid(0.0, 0.0, 0.0, 10.0)
        assertEquals(0.0, gyroidOrigin, 1e-6)

        val bonusPos = Phase4Features.computeMmcBonusPositionTolerance(10.0, 10.14, 0.25, 0.10)
        assertEquals(0.24, bonusPos, 1e-6)
    }
}
