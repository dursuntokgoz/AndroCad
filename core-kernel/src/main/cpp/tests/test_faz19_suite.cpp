#include <iostream>
#include <cassert>
#include <cmath>
#include "Phase19Engine.h"

using namespace AndroCAD::Phase19;

void TestDomain1_CrashBoxCrushing() {
    std::cout << "[Test 1/10] Domain 1: Frontal Crash Box Progressive Folding..." << std::endl;
    // Outer width = 80 mm, thickness = 2.0 mm, stroke = 140 mm, yield = 340 MPa, mass = 450 kg, v = 6.0 m/s (component sled test)
    auto res = Phase19Engine::evaluateCrashBoxCrushing(80.0, 2.0, 140.0, 340.0, 450.0, 6.0);
    assert(res.crushForceEfficiencyPercent >= 70.0);
    assert(res.meanCrushForceKn > 40.0);
    assert(res.stableAxialFolding == true);
    std::cout << "  -> Domain 1 Passed: Mean force = " << res.meanCrushForceKn << " kN, CFE = " << res.crushForceEfficiencyPercent << "%." << std::endl;
}

void TestDomain2_JohnsonCookStress() {
    std::cout << "[Test 2/10] Domain 2: Johnson-Cook Dynamic Plasticity (AHSS)..." << std::endl;
    // DP800 steel: eps = 0.15, strain_rate = 500 s^-1, T = 350 K, A = 520, B = 750, n = 0.35, C = 0.015, m = 1.05
    auto res = Phase19Engine::evaluateJohnsonCookStress(0.15, 500.0, 350.0, 520.0, 750.0, 0.35, 0.015, 1.05);
    assert(res.dynamicFlowStressMpa >= 520.0 && res.dynamicFlowStressMpa < 1600.0);
    assert(res.strainRateEnhancementRatio > 1.0);
    assert(res.ahssYieldCriterionMet == true);
    std::cout << "  -> Domain 2 Passed: Dynamic flow stress = " << res.dynamicFlowStressMpa << " MPa, Strain rate bonus = " << res.strainRateEnhancementRatio << std::endl;
}

void TestDomain3_AirbagInflation() {
    std::cout << "[Test 3/10] Domain 3: Airbag Inflator Gas Dynamics..." << std::endl;
    // Driver airbag: 60 L, 55 g gas, burn time = 28 ms, vent = 25 mm
    auto res = Phase19Engine::simulateAirbagInflation(60.0, 55.0, 28.0, 25.0);
    assert(res.peakBagPressureKpa >= 25.0 && res.peakBagPressureKpa <= 48.0);
    assert(res.timeToFullInflationMs < 40.0);
    assert(res.safeDeploymentCompliance == true);
    std::cout << "  -> Domain 3 Passed: Peak pressure = " << res.peakBagPressureKpa << " kPa, Inflation time = " << res.timeToFullInflationMs << " ms." << std::endl;
}

void TestDomain4_SideImpactIntrusion() {
    std::cout << "[Test 4/10] Domain 4: B-Pillar Side Impact Intrusion (FMVSS 214)..." << std::endl;
    // Barrier mass = 1350 kg, 50 km/h, boron steel t = 2.2 mm, door beam Z = 15 cm3, baseline = 220 mm
    auto res = Phase19Engine::evaluateBPillarIntrusion(1350.0, 50.0, 2.2, 15.0, 220.0);
    assert(res.residualCabinSurvivalSpaceMm >= 140.0);
    assert(res.maxBPillarIntrusionVelocityMs < 9.0);
    assert(res.meetsFmvss214SideImpact == true);
    std::cout << "  -> Domain 4 Passed: Survival space = " << res.residualCabinSurvivalSpaceMm << " mm, Intrusion velocity = " << res.maxBPillarIntrusionVelocityMs << " m/s." << std::endl;
}

void TestDomain5_BiomechanicalInjury() {
    std::cout << "[Test 5/10] Domain 5: Biomechanical Head (HIC15) & Chest Injury..." << std::endl;
    // 15 ms window: mean accel = 52g, peak accel = 68g, chest displ = 34 mm, velocity = 2.2 m/s
    auto res = Phase19Engine::calculateInjuryCriteria(10.0, 25.0, 52.0, 68.0, 34.0, 2.2);
    assert(res.headInjuryCriterionHic15 < 700.0);
    assert(res.chestDeflectionMm < 42.0);
    assert(res.euroNcapFiveStarCompliant == true);
    std::cout << "  -> Domain 5 Passed: HIC15 = " << res.headInjuryCriterionHic15 << " (<700), Chest deflection = " << res.chestDeflectionMm << " mm (Euro NCAP 5-Star)." << std::endl;
}

void TestDomain6_SpotWeldFailure() {
    std::cout << "[Test 6/10] Domain 6: Spot Weld Mixed-Mode Failure (AWS D8.1M)..." << std::endl;
    // Tension = 2.5 kN, shear = 3.8 kN, t = 1.4 mm, nugget = 6.0 mm, tensile = 600 MPa
    auto res = Phase19Engine::evaluateSpotWeldIntegrity(2.5, 3.8, 1.4, 6.0, 600.0);
    assert(res.mixedModeDamageIndex <= 1.0);
    assert(res.spotWeldMaintainsStructuralIntegrity == true);
    std::cout << "  -> Domain 6 Passed: Mixed-mode index = " << res.mixedModeDamageIndex << " (<= 1.0 limit), Shear stress = " << res.nuggetShearStressMpa << " MPa." << std::endl;
}

void TestDomain7_BatteryPoleCrash() {
    std::cout << "[Test 7/10] Domain 7: EV Battery Pack Pole Impact Protection..." << std::endl;
    // 32 km/h pole: energy = 35 kJ, rocker t = 4.5 mm, internal honeycomb = 85 mm, clearance = 75 mm
    auto res = Phase19Engine::evaluateBatteryPoleCrash(35.0, 4.5, 85.0, 75.0);
    assert(res.remainingClearanceBufferMm >= 15.0);
    assert(res.cellDeformationStrainPercent == 0.0);
    assert(res.preventsBatteryThermalRunaway == true);
    std::cout << "  -> Domain 7 Passed: Intrusion = " << res.intrusionDepthMm << " mm, Remaining buffer = " << res.remainingClearanceBufferMm << " mm." << std::endl;
}

void TestDomain8_PedestrianImpact() {
    std::cout << "[Test 8/10] Domain 8: Pedestrian Protection & Pop-Up Bonnet (GTR 9)..." << std::endl;
    // Impact = 40 km/h, under clearance = 25 mm, lift time = 28 ms, stiffness = 0.8 kN/mm
    auto res = Phase19Engine::evaluatePedestrianImpact(40.0, 25.0, 28.0, 0.8);
    assert(res.headformHicScore < 1000.0);
    assert(res.legformBendingAngleDegrees <= 19.0);
    assert(res.compliesWithGtr9PedestrianSafety == true);
    std::cout << "  -> Domain 8 Passed: Headform HIC = " << res.headformHicScore << " (<1000), FlexPLI angle = " << res.legformBendingAngleDegrees << " deg." << std::endl;
}

void TestDomain9_SeatbeltPretensioner() {
    std::cout << "[Test 9/10] Domain 9: Seatbelt Pretensioner & Load Limiter..." << std::endl;
    // Pulse = 32g, occupant = 78 kg, torsion bar torque = 95 Nm, spool radius = 22 mm
    auto res = Phase19Engine::evaluateSeatbeltPretensioner(32.0, 78.0, 95.0, 22.0);
    assert(res.shoulderBeltPlateauForceKn >= 3.5 && res.shoulderBeltPlateauForceKn <= 5.0);
    assert(res.pelvisDisplacementMm < 120.0);
    assert(res.limitsChestOverload == true);
    std::cout << "  -> Domain 9 Passed: Belt plateau force = " << res.shoulderBeltPlateauForceKn << " kN, Retraction time = " << res.webbingRetractionTimeMs << " ms." << std::endl;
}

void TestDomain10_RcarBumperCrash() {
    std::cout << "[Test 10/10] Domain 10: RCAR Low-Speed Insurance Bumper Crash..." << std::endl;
    // Vehicle = 1550 kg, v = 15 km/h, bumper strength = 180 kN, collapse length = 90 mm
    auto res = Phase19Engine::evaluateRcarLowSpeedBumper(1550.0, 15.0, 180.0, 90.0);
    assert(res.chassisRailResidualDeformationMm == 0.0);
    assert(res.estimatedRepairCostEuro < 1000.0);
    assert(res.zeroStructuralChassisDamage == true);
    std::cout << "  -> Domain 10 Passed: Chassis deformation = " << res.chassisRailResidualDeformationMm << " mm, Repair cost = " << res.estimatedRepairCostEuro << " EUR." << std::endl;
}

int main() {
    std::cout << "============================================================" << std::endl;
    std::cout << " Starting AndroCAD Faz 19 Crashworthiness Suite (M3071-3270)" << std::endl;
    std::cout << "============================================================" << std::endl;

    TestDomain1_CrashBoxCrushing();
    TestDomain2_JohnsonCookStress();
    TestDomain3_AirbagInflation();
    TestDomain4_SideImpactIntrusion();
    TestDomain5_BiomechanicalInjury();
    TestDomain6_SpotWeldFailure();
    TestDomain7_BatteryPoleCrash();
    TestDomain8_PedestrianImpact();
    TestDomain9_SeatbeltPretensioner();
    TestDomain10_RcarBumperCrash();

    std::cout << "============================================================" << std::endl;
    std::cout << " All 10/10 Faz 19 Crash Domains Passed Successfully!        " << std::endl;
    std::cout << "============================================================" << std::endl;
    return 0;
}
