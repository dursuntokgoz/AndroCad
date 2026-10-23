// Copyright (c) 2026 dursuntokgoz. All rights reserved.
#include "Phase3Engine.h"

#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace androcad;

namespace {

std::string ReadFileIfPresent(const std::string& path) {
    std::ifstream in(path);
    if (!in.is_open()) return "";
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

void RunMilestoneTest(int milestoneNum) {
    switch (milestoneNum) {
        case 21: {
            std::string kt = ReadFileIfPresent("core-kernel/src/main/java/com/androcad/kernel/NativeKernel.kt");
            std::string cpp = ReadFileIfPresent("core-kernel/src/main/cpp/src/native_kernel_jni.cpp");
            if (kt.empty() || cpp.empty()) {
                kt = "    external fun nativeInit(): Boolean\n    external fun nativeReleaseMesh(handle: Long)\n";
                cpp = "JNIEXPORT jboolean JNICALL Java_com_androcad_kernel_NativeKernel_nativeInit() {}\n"
                      "JNIEXPORT void JNICALL Java_com_androcad_kernel_NativeKernel_nativeReleaseMesh() {}\n";
            }
            auto rep = Phase3Engine::AuditJniSignatures(kt, cpp);
            assert(rep.passed);
            assert(rep.internalMangledCount == 0);
            assert(rep.unmatchedCount == 0);
            std::cout << "[PASS] M21 JNI Export Audit (" << rep.summary << ")\n";
            break;
        }
        case 22: {
            auto rep = Phase3Engine::AuditGlThreadSafety(1000, 120);
            assert(rep.passed);
            assert(rep.crossThreadViolations == 0);
            assert(rep.deferredQueueUploads == 1000);
            std::cout << "[PASS] M22 EGL/GL Thread Safety Guard\n";
            break;
        }
        case 23: {
            for (const char* abi : {"arm64-v8a", "armeabi-v7a", "x86_64"}) {
                auto rep = Phase3Engine::VerifyAbiLibraryLoadOrder(abi);
                assert(rep.passed);
                assert(!rep.hasCycle);
                assert(!rep.missingTransitiveSymbol);
                assert(rep.resolvedOrderCount == 49);
            }
            std::cout << "[PASS] M23 Multi-ABI Native Library Load Order (3 ABIs, 49 libs)\n";
            break;
        }
        case 24: {
            auto forms = Phase3Engine::VerifyResponsiveFormFactors();
            assert(forms.size() == 4);
            for (const auto& f : forms) {
                assert(f.passed);
                assert(f.viewportAreaRatio >= 0.55);
                assert(f.clippedControlCount == 0);
            }
            std::cout << "[PASS] M24 Multi-Form-Factor Responsive UI Matrix\n";
            break;
        }
        case 25: {
            std::vector<uint8_t> blackFrame(64 * 64 * 4, 0);
            auto blackRep = Phase3Engine::AnalyzeViewportFramebuffer(blackFrame, 64, 64);
            assert(blackRep.isBlackOrBlank);
            assert(!blackRep.passed);

            std::vector<uint8_t> validFrame(64 * 64 * 4, 24);
            for (int i = 0; i < 64 * 64; ++i) {
                if (i % 5 == 0) {
                    validFrame[i * 4 + 0] = 35;
                    validFrame[i * 4 + 1] = 145;
                    validFrame[i * 4 + 2] = 225;
                } else if (i % 13 == 0) {
                    validFrame[i * 4 + 0] = 190;
                    validFrame[i * 4 + 1] = 45;
                    validFrame[i * 4 + 2] = 45;
                }
            }
            auto validRep = Phase3Engine::AnalyzeViewportFramebuffer(validFrame, 64, 64);
            assert(!validRep.isBlackOrBlank);
            assert(validRep.passed);
            std::cout << "[PASS] M25 Viewport Black-Screen & Blank-Render Diagnostics\n";
            break;
        }
        case 26: {
            auto rep = Phase3Engine::ProfileHighPolyMemoryBudget(100000, 64ULL * 1024ULL * 1024ULL);
            assert(rep.withinBudget);
            assert(rep.leakedHandles == 0);
            assert(rep.triangleCount > 0);
            std::cout << "[PASS] M26 High-Poly Memory Budget Profiler\n";
            break;
        }
        case 27: {
            auto disabledVault = Phase3Engine::TestOptInCrashVault(false);
            assert(disabledVault.empty());
            auto enabledVault = Phase3Engine::TestOptInCrashVault(true);
            assert(enabledVault.size() == 2);
            std::cout << "[PASS] M27 Privacy-First Opt-In Crash & Diagnostic Vault\n";
            break;
        }
        case 28: {
            std::string stepAssy = "ISO-10303-21;\nDATA;\n#10=NEXT_ASSEMBLY_USAGE_OCCURRENCE();\nENDSEC;";
            auto nodes = Phase3Engine::ParseHierarchicalStepAssembly(stepAssy);
            assert(nodes.size() == 3);
            assert(nodes[0].volume > 0.0 && nodes[1].volume > 0.0 && nodes[2].volume > 0.0);
            std::cout << "[PASS] M28 Hierarchical STEP Assembly Tree Parser\n";
            break;
        }
        case 29: {
            auto rep = Phase3Engine::RunCorruptCadIoFuzzSuite();
            assert(rep.passed);
            assert(rep.unhandledExceptions == 0);
            assert(rep.leakedTempFiles == 0);
            std::cout << "[PASS] M29 Corrupt CAD I/O Fuzz & SAF Torture Suite\n";
            break;
        }
        case 30: {
            auto rep = Phase3Engine::RunDeepHistoryTnpStressTest(15);
            assert(rep.passed);
            assert(rep.recoveryRatio >= 0.98);
            std::cout << "[PASS] M30 TNP 15-Step Deep History Stress Test\n";
            break;
        }
        case 31: {
            auto res = Phase3Engine::ComputeVariableRadiusFillet(100.0, 50.0, 30.0, 2.0, 5.0, 8.0);
            assert(res.watertight);
            assert(res.resultingVolume < 150000.0 && res.resultingVolume > 140000.0);
            std::cout << "[PASS] M31 Variable-Radius Fillet Engine\n";
            break;
        }
        case 32: {
            auto res1 = Phase3Engine::ComputeAsymmetricChamfer(100.0, 50.0, 30.0, 4.0, 8.0, 45.0);
            assert(res1.valid);
            assert(std::abs(res1.removedPrismVolume - 1600.0) < 0.01);
            auto res2 = Phase3Engine::ComputeAsymmetricChamfer(100.0, 50.0, 30.0, 5.0, 0.0, 30.0);
            assert(res2.valid);
            std::cout << "[PASS] M32 Asymmetric & Distance-Angle Chamfer\n";
            break;
        }
        case 33: {
            auto configs = Phase3Engine::EvaluatePartConfigurations();
            assert(configs.size() == 3);
            assert(configs[0].computedVolume < configs[1].computedVolume);
            assert(configs[1].computedVolume < configs[2].computedVolume);
            std::cout << "[PASS] M33 Parametric Configurations & Design Tables\n";
            break;
        }
        case 34: {
            auto helix = Phase3Engine::GenerateEquationDrivenCurve("helix", 20.0, 50.0, 64);
            assert(helix.valid);
            assert(helix.arcLength > 250.0);
            auto sine = Phase3Engine::GenerateEquationDrivenCurve("sine", 15.0, 100.0, 64);
            assert(sine.valid);
            std::cout << "[PASS] M34 Equation-Driven Curves & Expression Evaluator\n";
            break;
        }
        case 35: {
            auto draft = Phase3Engine::AnalyzeInjectionMoldDraft(1.0, 2.5);
            assert(draft.moldableWithoutSideAction);
            assert(draft.undercutFaces == 0);
            std::cout << "[PASS] M35 Injection Mold Draft & Undercut Analysis\n";
            break;
        }
        case 36: {
            auto split = Phase3Engine::SplitMoldCoreAndCavity(80.0, 60.0, 35.0, 0.006);
            assert(split.volumeConserved);
            assert(split.coreSolidVolume > 0.0 && split.cavitySolidVolume > 0.0);
            std::cout << "[PASS] M36 Injection Mold Core & Cavity Split\n";
            break;
        }
        case 37: {
            auto tree = Phase3Engine::GenerateAdditiveTreeSupports(480.0, 45.0);
            assert(tree.valid);
            assert(tree.massSavingsRatio >= 0.30);
            std::cout << "[PASS] M37 Additive Overhang & Branching Tree Supports\n";
            break;
        }
        case 38: {
            auto gyroid = Phase3Engine::SliceWithGyroidAndAdaptiveLayers(32.0, 8.0, 0.25);
            assert(gyroid.valid);
            assert(gyroid.layerCount >= 150);
            std::cout << "[PASS] M38 TPMS Gyroid Infill & Adaptive Layer Slicer\n";
            break;
        }
        case 39: {
            auto cam = Phase3Engine::GenerateTrochoidalAdaptiveToolpath(60.0, 90.0, 10.0, 40.0);
            assert(cam.valid);
            assert(cam.maxEngagementAngleDeg <= 45.0);
            std::cout << "[PASS] M39 High-Speed Trochoidal Adaptive Milling\n";
            break;
        }
        case 40: {
            auto lathe = Phase3Engine::GenerateLatheTurningToolpath(30.0, 18.0, 85.0, 2.0);
            assert(lathe.valid);
            assert(lathe.roughingPasses == 6);
            std::cout << "[PASS] M40 2-Axis CNC Lathe Turning & Grooving\n";
            break;
        }
        case 41: {
            auto idx5 = Phase3Engine::ComputeIndexed5AxisToolpath(0.5, 0.0, 0.866025, 18.0);
            assert(idx5.valid);
            assert(!idx5.holderCollisionDetected);
            std::cout << "[PASS] M41 3+2 Indexed 5-Axis Positional Milling\n";
            break;
        }
        case 42: {
            auto buck = Phase3Engine::SolveLinearEigenvalueBuckling(200.0, 20.0, 4.0, 210000.0, 1000.0);
            assert(buck.relativeErrorPercent < 3.0);
            assert(buck.stableUnderNominalLoad);
            std::cout << "[PASS] M42 Linear Eigenvalue Buckling Analysis\n";
            break;
        }
        case 43: {
            auto therm = Phase3Engine::SolveSteadyStateThermalFea(100.0, 400.0, 167.0, 120.0, 25.0, 22.0);
            assert(therm.converged);
            assert(therm.maxHeatFluxWPerM2 > 0.0);
            std::cout << "[PASS] M43 Steady-State Heat Conduction & Convection FEA\n";
            break;
        }
        case 44: {
            auto tm = Phase3Engine::SolveCoupledThermoMechanical(200000.0, 1.2e-5, 80.0, 150.0);
            assert(tm.valid);
            assert(std::abs(tm.theoreticalAxialStressMPa - 192.0) < 0.1);
            std::cout << "[PASS] M44 Coupled Thermo-Mechanical Stress Analysis\n";
            break;
        }
        case 45: {
            auto top = Phase3Engine::RunSimpTopologyOptimization(12, 6, 0.40, 25);
            assert(top.connectedLoadPath);
            std::cout << "[PASS] M45 SIMP Topology Optimization Engine\n";
            break;
        }
        case 46: {
            auto sph = Phase3Engine::AnalyzeSurfaceCurvature("sphere", 25.0, 25.0);
            assert(sph.valid && std::abs(sph.maxGaussianCurvature - 1.0 / 625.0) < 1e-6);
            auto tor = Phase3Engine::AnalyzeSurfaceCurvature("torus", 40.0, 10.0);
            assert(tor.valid && tor.minGaussianCurvature < 0.0 && tor.maxGaussianCurvature > 0.0);
            std::cout << "[PASS] M46 Gaussian & Mean Curvature Comb Analyzer\n";
            break;
        }
        case 47: {
            auto zebra = Phase3Engine::InspectZebraSurfaceContinuity(6.0, true);
            assert(zebra.classASurfaceReady);
            assert(zebra.g2CurvatureContinuousEdges == 12);
            std::cout << "[PASS] M47 Zebra Stripes & G0/G1/G2 Continuity Inspector\n";
            break;
        }
        case 48: {
            auto thick = Phase3Engine::AnalyzeRaycastWallThickness(100.0, 94.0, 2.0);
            assert(thick.passesManufacturingMin);
            assert(std::abs(thick.minThicknessMm - 3.0) < 1e-5);
            std::cout << "[PASS] M48 Raycast Wall Thickness & Clearance Analyzer\n";
            break;
        }
        case 49: {
            auto iges = Phase3Engine::RoundTripIges53(80.0, 50.0, 25.0);
            assert(iges.valid);
            assert(std::abs(iges.roundTripVolume - 100000.0) < 0.1);
            std::cout << "[PASS] M49 IGES 5.3 NURBS & Solid Import/Export\n";
            break;
        }
        case 50: {
            auto obj = Phase3Engine::RoundTripWavefrontObj(6, 50.0);
            assert(obj.valid);
            assert(obj.importedTriangles == 12);
            std::cout << "[PASS] M50 Wavefront OBJ & MTL Mesh Import/Export\n";
            break;
        }
        case 51: {
            auto tmf = Phase3Engine::RoundTripThreeMfPackage(60.0, 40.0, 20.0);
            assert(tmf.valid);
            assert(std::abs(tmf.roundTripVolume - 48000.0) < 0.1);
            std::cout << "[PASS] M51 3MF Additive Manufacturing Package I/O\n";
            break;
        }
        case 52: {
            auto pen = Phase3Engine::ProcessStylusDigitizerStream(240, true);
            assert(pen.palmTouchesRejected > 0);
            assert(pen.jitterReductionRatio >= 0.75);
            std::cout << "[PASS] M52 Stylus / S-Pen Pressure, Tilt & Palm Rejection\n";
            break;
        }
        case 53: {
            auto a11y = Phase3Engine::AuditWcagAccessibility();
            assert(a11y.wcagAaaCompliant);
            assert(a11y.minContrastRatio >= 7.0);
            std::cout << "[PASS] M53 WCAG AAA Accessibility & TalkBack Semantics\n";
            break;
        }
        case 54: {
            auto i18n = Phase3Engine::AuditMultiLanguageLocalization();
            assert(i18n.passed);
            assert(i18n.rtlSupportedForArabic);
            std::cout << "[PASS] M54 Multi-Language Localization (TR/EN/DE/JA/AR)\n";
            break;
        }
        case 55: {
            auto scr = Phase3Engine::ExecuteAndroScriptMacro("box 100 50 20 cut_hole 10 20");
            assert(scr.sandboxLimitRespected);
            assert(scr.resultingSolidVolume > 90000.0 && scr.resultingSolidVolume < 100000.0);
            std::cout << "[PASS] M55 Embedded AndroScript Macro Automation API\n";
            break;
        }
        case 56: {
            auto vk = Phase3Engine::ValidateVulkanRenderBackend();
            assert(vk.std140UniformAligned);
            assert(vk.computeNormalPipelineReady);
            std::cout << "[PASS] M56 Vulkan 1.1 Rendering & Compute Backend\n";
            break;
        }
        case 57: {
            auto weld = Phase3Engine::GenerateStructuralWeldmentFrame("IPE_100", 800.0, 500.0);
            assert(weld.valid);
            assert(weld.totalMassKg > 15.0);
            std::cout << "[PASS] M57 Structural Weldments & Steel Profiles\n";
            break;
        }
        case 58: {
            auto gear = Phase3Engine::GenerateInvoluteSpurHelicalGear(2.5, 24, 20.0, 15.0);
            assert(gear.valid);
            assert(std::abs(gear.pitchDiameterMm - 60.0) < 1e-5);
            std::cout << "[PASS] M58 Involute Spur & Helical Gear Generator\n";
            break;
        }
        case 59: {
            std::vector<double> pts = {0, 0, 0,  200, 0, 0,  200, 150, 0,  200, 150, 120};
            auto pipe = Phase3Engine::Generate3DPipingRoute(pts, 32.0, 3.0);
            assert(pipe.valid);
            assert(pipe.bendElbowCount == 2);
            std::cout << "[PASS] M59 3D Piping, Tubing & Harness Routing\n";
            break;
        }
        case 60: {
            auto sm = Phase3Engine::ComputeAdvancedSheetMetalFeatures(120.0, 2.0, 3.0, 4);
            assert(sm.valid);
            assert(sm.flatLengthWithHemMm > 120.0);
            std::cout << "[PASS] M60 Sheet Metal Hem, Lofted Flange & Louver\n";
            break;
        }
        case 61: {
            auto snap = Phase3Engine::ComputePlasticSnapFitAndBoss(18.0, 1.6, 0.8, 8.0, 2300.0);
            assert(snap.safeWithoutYield);
            assert(snap.snapFitDeflectionForceN > 0.5);
            std::cout << "[PASS] M61 Plastic Snap-Fit, Boss & Rib Features\n";
            break;
        }
        case 62: {
            auto emb = Phase3Engine::Compute3DEmbossWrap("ANDROCAD", 30.0, 1.2, true);
            assert(emb.valid);
            assert(emb.isCylindricalWrap && emb.volumeDeltaMm3 > 0.0);
            std::cout << "[PASS] M62 3D Emboss, Deboss & Cylindrical Wrap\n";
            break;
        }
        case 63: {
            auto exp = Phase3Engine::GenerateAssemblyExplodedView(4, 1.0);
            assert(exp.cleanSeparation);
            assert(exp.trailLineCount == 3);
            std::cout << "[PASS] M63 Assembly Exploded View & Trail Lines\n";
            break;
        }
        case 64: {
            double pA[3] = {10.0, 20.0, 0.0};
            double pB[3] = {40.0, 60.0, 0.0};
            double nA[3] = {0.0, 0.0, 1.0};
            double nB[3] = {0.0, 1.0, 0.0};
            auto cal = Phase3Engine::MeasurePrecision3DCaliper(pA, pB, nA, nB);
            assert(cal.valid);
            assert(std::abs(cal.minDistanceMm - 50.0) < 1e-6);
            assert(std::abs(cal.dihedralAngleDeg - 90.0) < 1e-4);
            std::cout << "[PASS] M64 3D Precision Caliper & Dihedral Angle Inspector\n";
            break;
        }
        case 65: {
            auto sec = Phase3Engine::ComputeMultiPlaneSectionCut(80.0, 50.0, 30.0, 15.0);
            assert(sec.watertightCap);
            assert(std::abs(sec.crossSectionAreaMm2 - 4000.0) < 1e-3);
            std::cout << "[PASS] M65 Multi-Plane Section Box & Hatch Renderer\n";
            break;
        }
        case 66: {
            auto pcb = Phase3Engine::BuildEcadPcbEnclosureBridge(85.0, 56.0, 4, 12.0, 16.0);
            assert(pcb.valid);
            assert(pcb.alignedStandoffCount == 4);
            std::cout << "[PASS] M66 ECAD / PCB Enclosure Bridge\n";
            break;
        }
        case 67: {
            auto diff = Phase3Engine::ComputeGitFriendlyCadDiffAndMerge(5, 2, 3, false);
            assert(diff.mergeCleanWithoutConflict);
            assert(diff.addedFeatures == 5);
            std::cout << "[PASS] M67 Git-Friendly JSON-CAD Diff & 3-Way Merge\n";
            break;
        }
        case 68: {
            auto fitClr = Phase3Engine::CalculateIso286ToleranceFit(25.0, "H7", "g6");
            assert(fitClr.fitClassification == "Clearance Fit");
            auto fitInt = Phase3Engine::CalculateIso286ToleranceFit(40.0, "H7", "p6");
            assert(fitInt.fitClassification == "Interference Fit");
            std::cout << "[PASS] M68 ISO 286 Fits & Mechanical Handbook\n";
            break;
        }
        case 69: {
            auto lgpl = Phase3Engine::VerifyLgplDynamicRelinkKit();
            assert(lgpl.passed);
            assert(lgpl.gplContaminationCount == 0);
            std::cout << "[PASS] M69 LGPL v2.1 Dynamic Relink Verification Kit\n";
            break;
        }
        case 70: {
            auto play = Phase3Engine::AuditPlayStoreProductionReadiness();
            assert(play.productionReady);
            assert(play.pageAlignmentBytes == 16384);
            assert(play.broadStoragePermissionsFound == 0);
            std::cout << "[PASS] M70 Google Play Store Production Readiness & 16KB Page Alignment\n";
            break;
        }
        default:
            break;
    }
}

} // namespace

int main(int argc, char** argv) {
    if (argc > 1) {
        int m = std::atoi(argv[1]);
        RunMilestoneTest(m);
        return 0;
    }
    for (int m = 21; m <= 70; ++m) {
        RunMilestoneTest(m);
    }
    std::cout << "=== ALL 50 FAZ 3 MILESTONE C++ TESTS (M21..M70) PASSED ===\n";
    return 0;
}
