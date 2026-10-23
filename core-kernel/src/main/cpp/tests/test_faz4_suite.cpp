// Copyright (c) 2026 dursuntokgoz. All rights reserved.
#include "Phase4Engine.h"
#include <cassert>
#include <cmath>
#include <iostream>

using namespace androcad;

int main() {
    std::cout << "[Faz4SuiteTest] Starting 200-Milestone Faz 4 & 3D Picking Verification Suite..." << std::endl;

    // 1. Domain 1 (M71 - M90): Top Tap Must Select Top Edge & Top Face, Never Bottom Edge #1/#2/#9/#10
    {
        // Tap near top-front edge (50, 50, 20) on a 100x50x20 box
        auto topFrontPick = Phase4Engine::Verify3DPickingAndTargetResolution(
            100.0, 50.0, 20.0,
            50.0, 50.0, 20.0,
            0.0, 0.0, 1.0,
            true
        );
        assert(topFrontPick.passed);
        assert(topFrontPick.resolvedEdgeId == 12);
        assert(topFrontPick.resolvedFaceId == 6);
        assert(topFrontPick.filletChamferTargetCode == 12);
        assert(topFrontPick.topTapSelectedTopEntity);
        assert(topFrontPick.bottomEdgeAvoidedOnTopTap);

        // Tap near top-back edge (50, 0, 20) -> must select Edge #11 (top-back), NOT Edge #9 (bottom-back) or Edge #1
        auto topBackPick = Phase4Engine::Verify3DPickingAndTargetResolution(
            100.0, 50.0, 20.0,
            50.0, 0.0, 20.0,
            0.0, 0.0, 1.0,
            true
        );
        assert(topBackPick.passed);
        assert(topBackPick.resolvedEdgeId == 11);
        assert(topBackPick.bottomEdgeAvoidedOnTopTap);

        // Tap at center of Top Face (50, 25, 20) away from edges (>12mm) -> must resolve to Face #6 loop (-1006)
        auto topCenterFacePick = Phase4Engine::Verify3DPickingAndTargetResolution(
            100.0, 50.0, 20.0,
            50.0, 25.0, 20.0,
            0.0, 0.0, 1.0,
            false
        );
        assert(topCenterFacePick.resolvedFaceId == 6);
        assert(topCenterFacePick.filletChamferTargetCode == -1006);

        // Audit all toolbar buttons
        auto btnRep = Phase4Engine::AuditAll85ToolbarButtons();
        assert(btnRep.allPassed);
        assert(btnRep.totalButtonsVerified >= 85);
    }

    // 2. Domain 2 (M91 - M110): Conics, Helix & Sketch Doctor
    {
        auto conic = Phase4Engine::EvaluateConicSection(40.0, 20.0, 0.35);
        assert(conic.isClosed);
        assert(conic.arcLengthMm > 180.0);
        assert(conic.enclosedAreaMm2 > 2500.0);

        auto helix = Phase4Engine::GenerateHelicalWire(15.0, 15.0, 4.0, 40.0);
        assert(helix.valid);
        assert(std::abs(helix.turns - 10.0) < 1e-6);
        assert(helix.totalWireLengthMm > 900.0);

        auto heal = Phase4Engine::HealSketchProfile(4, 2, 1, 0.05);
        assert(heal.isWatertightLoop);
        assert(heal.closedGaps == 4);
    }

    // 3. Domain 3 (M111 - M130): Cylindrical Emboss & Snap-Fit
    {
        auto emboss = Phase4Engine::ComputeCylindricalEmboss(30.0, 400.0, 2.5, false);
        assert(emboss.valid);
        assert(emboss.deltaVolumeMm3 > 1000.0);

        auto deboss = Phase4Engine::ComputeCylindricalEmboss(30.0, 400.0, 2.5, true);
        assert(deboss.valid);
        assert(deboss.deltaVolumeMm3 < 0.0);

        auto snap = Phase4Engine::AnalyzeCantileverSnapFit(22.0, 2.0, 8.0, 1.0, 2300.0, 65.0);
        assert(snap.safeAgainstYield);
        assert(snap.matingForceN > 0.5);
        assert(snap.retentionForceN > snap.matingForceN);
    }

    // 4. Domain 4 (M131 - M150): Catmull-Clark Subdivision & RANSAC Point Cloud Fit
    {
        auto subd = Phase4Engine::EvaluateCatmullClarkCage(12, 2);
        assert(subd.limitSurfacePatches == 192);
        assert(subd.g2ContinuousAtExtraordinaryCheck);

        auto ransac = Phase4Engine::FitPointCloudPrimitive(5000, 0.05, "Cylinder");
        assert(ransac.converged);
        assert(ransac.inlierCount > 4000);
    }

    // 5. Domain 5 (M151 - M170): Sheet Metal Nesting, Weldments & ASME Pressure Vessel
    {
        auto nest = Phase4Engine::OptimizeSheetNesting(1000.0, 2000.0, {150000.0, 220000.0, 180000.0, 250000.0}, 1.5);
        assert(nest.allPartsPlaced);
        assert(nest.utilizationRatio > 0.35);

        auto weld = Phase4Engine::ComputeWeldmentMember("ISO_RHS_80x40x4", 896.0, 1200.0, 45.0, 45.0);
        assert(weld.massKg > 8.0);

        auto vessel = Phase4Engine::AnalyzeAsmePressureVessel(2.0, 300.0, 138.0, 0.85);
        assert(vessel.codeCompliant);
        assert(vessel.requiredShellThicknessMm > 4.0);
    }

    // 6. Domain 6 (M171 - M190): Planar 3-Link Inverse Kinematics & Tolerance Stackup
    {
        auto ik = Phase4Engine::SolvePlanar3LinkIK(60.0, 50.0, 25.0, 90.0, 40.0, 0.25);
        assert(ik.converged);
        assert(ik.endEffectorErrorMm < 1e-3);

        auto tol = Phase4Engine::AnalyzeToleranceStackup(
            {50.0, -20.0, -28.0},
            {1.0, 1.0, 1.0},
            {0.10, 0.08, 0.08},
            1.40, 2.60
        );
        assert(std::abs(tol.nominalGapMm - 2.0) < 1e-6);
        assert(tol.rssToleranceMm < (tol.worstCaseMaxMm - tol.nominalGapMm));
        assert(tol.sixSigmaCapable);
    }

    // 7. Domain 7 (M191 - M210): Goodman Fatigue, Conjugate Heat, Pipe Flow & Injection Molding
    {
        auto fat = Phase4Engine::EvaluateGoodmanFatigue(130.0, -50.0, 500.0, 340.0, 5.0e5);
        assert(fat.infiniteLife);
        assert(fat.goodmanSafetyFactor > 1.2);

        auto therm = Phase4Engine::SolveConjugateHeatTransfer(45.0, 0.08, 25.0, 2.5, 0.85);
        assert(therm.maxTemperatureC > 25.0);
        assert(therm.convectionCoeffWPerM2K > 10.0);

        auto flow = Phase4Engine::SolveDarcyWeisbachPipeFlow(10.0, 25.0, 0.045, 60.0, 998.0, 1.0e-3);
        assert(flow.isTurbulent);
        assert(flow.pressureDropPa > 100.0);

        auto mold = Phase4Engine::SimulateInjectionMolding(35.0, 2.5, 80.0, 45.0);
        assert(mold.coolingTimeSec > 2.0);
        assert(mold.clampTonnageRequired > 10.0);
    }

    // 8. Domain 8 (M211 - M230): 5-Axis Swarf, 4-Axis Wire EDM & TPMS Implicit Field
    {
        auto swarf = Phase4Engine::GenerateSimultaneous5AxisSwarf(100.0, 30.0, 6.0, 20.0, 1.5);
        assert(swarf.totalPostBlocks >= 1000);
        assert(swarf.scallopHeightMm < 0.1);

        auto wedm = Phase4Engine::Generate4AxisWireEdmPath(40.0, 8.0, 0.25, 3);
        assert(wedm.roughingPlusSkimPasses == 4);
        assert(wedm.surfaceFinishRaUm < 0.8);

        double tpmsVal = Phase4Engine::EvaluateTpmsImplicitField("SchwarzP", 5.0, 5.0, 5.0, 10.0);
        assert(std::abs(tpmsVal - (-3.0)) < 1e-6);
    }

    // 9. Domain 9 (M231 - M250): GD&T True Position Bonus, DFM Check & Should-Cost LCA
    {
        double totalPosTol = Phase4Engine::ComputeGdtTruePositionBonusMm(10.0, 10.15, 0.25, 0.0, 0.10, true, true);
        assert(std::abs(totalPosTol - 0.25) < 1e-6);

        auto dfm = Phase4Engine::RunAutomatedDfmCheck(2.0, 4.0, 1.2, 0);
        assert(dfm.readyForProduction);
        assert(dfm.manufacturabilityScore == 100.0);

        auto cost = Phase4Engine::EstimateManufacturingCostAndLca(1.2, 0.75, 18.0, 65.0, "Aluminum");
        assert(cost.unitTotalCostUsd > 20.0);
        assert(cost.carbonFootprintKgCo2e > 1.0);
    }

    // 10. Domain 10 (M251 - M270): SSAO, AI CAD Intent & Provenance Watermark + All 200 Milestones
    {
        double ssao = Phase4Engine::ComputeSsaoOcclusionFactor(100.0, {99.2, 99.4, 100.3, 100.1}, 2.0, 0.1);
        assert(ssao >= 0.15 && ssao < 1.0);

        std::string intent = Phase4Engine::ParseNaturalLanguageCadIntent("Apply 5mm fillet radius on top edge");
        assert(intent.find("ACTION:FILLET") != std::string::npos);

        std::string wm = Phase4Engine::ComputeSolidProvenanceWatermark(100000.0, 6, 12, "AndroCAD-Faz4");
        assert(wm.rfind("ACWM-", 0) == 0);

        auto all200 = Phase4Engine::ExecuteAllFaz4Milestones();
        assert(all200.size() == 200);
        for (size_t i = 0; i < all200.size(); ++i) {
            assert(all200[i].milestoneNumber == static_cast<int32_t>(71 + i));
            assert(all200[i].verified);
        }
    }

    std::cout << "[Faz4SuiteTest] All 200 Faz 4 Milestones (M71..M270) and 3D Picking Verification PASSED!" << std::endl;
    return 0;
}
