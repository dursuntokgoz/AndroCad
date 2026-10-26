#include "Phase13Engine.h"
#include <cassert>
#include <iostream>
#include <vector>
#include <cmath>

using namespace AndroCAD::Phase13;

void TestDomain1_BulbousBowWaveResistance() {
    std::cout << "[Test 1/10] Domain 1: Hull Form Fairing & Bulbous Bow..." << std::endl;
    HullFormParameters p;
    p.lengthWaterlineM = 165.0;
    p.speedKnots = 21.0;

    auto res = OptimizeBulbousBowHull(p);
    assert(res.hullFairingValid);
    assert(res.displacementTons > 15000.0);
    assert(res.froudeNumber > 0.2 && res.froudeNumber < 0.35);
    assert(res.waveResistanceReductionPercent > 10.0);
    assert(res.intactStabilityCompliant);
    std::cout << "  -> Domain 1 Passed: Bulbous bow wave drag reduction = " << res.waveResistanceReductionPercent << "%." << std::endl;
}

void TestDomain2_PropellerCavitation() {
    std::cout << "[Test 2/10] Domain 2: Propeller Blade Cavitation & Keller Criteria..." << std::endl;
    PropellerCavitationParameters p;
    p.diameterM = 5.2;

    auto res = EvaluatePropellerCavitation(p);
    assert(res.cavitationSafe);
    assert(res.bladeAreaRatioBar > 0.6);
    assert(res.pressureFluctuationKpa < 4.0);
    std::cout << "  -> Domain 2 Passed: Propeller BAR = " << res.bladeAreaRatioBar << ", pressure fluctuation = " << res.pressureFluctuationKpa << " kPa." << std::endl;
}

void TestDomain3_SeakeepingSlamming() {
    std::cout << "[Test 3/10] Domain 3: Seakeeping Motion (RAO) & Wave Slamming..." << std::endl;
    SeakeepingParameters p;
    p.significantWaveHeightM = 6.0;

    auto res = SimulateShipSeakeeping(p);
    assert(res.seakeepingAcceptable);
    assert(res.peakPitchAngleDeg < 5.0);
    assert(res.bowSlammingPressureKpa > 200.0);
    assert(res.greenWaterOnDeckAvoided);
    std::cout << "  -> Domain 3 Passed: Sea State 6 pitch angle = " << res.peakPitchAngleDeg << " deg." << std::endl;
}

void TestDomain4_HydrofoilPerformance() {
    std::cout << "[Test 4/10] Domain 4: Hydrofoil Lift-to-Drag & Foil-Borne Ride..." << std::endl;
    HydrofoilParameters p;
    p.cruiseSpeedKnots = 38.0;

    auto res = AnalyzeHydrofoilPerformance(p);
    assert(res.foilBorneStable);
    assert(res.foilLiftToDragRatio > 12.0);
    assert(res.subCavitatingFoilProfile);
    std::cout << "  -> Domain 4 Passed: Hydrofoil L/D = " << res.foilLiftToDragRatio << " at 38 knots." << std::endl;
}

void TestDomain5_ImoManeuverability() {
    std::cout << "[Test 5/10] Domain 5: Rudder Torque & IMO Ship Maneuverability..." << std::endl;
    RudderManeuveringParameters p;
    p.rudderAreaM2 = 22.5;

    auto res = VerifyImoManeuverability(p);
    assert(res.imoStandardSatisfied);
    assert(res.tacticalDiameterShipLengths < 5.0);
    assert(res.yawCheckingAbilityOvershootDeg < 10.0);
    std::cout << "  -> Domain 5 Passed: IMO tactical diameter = " << res.tacticalDiameterShipLengths << " ship lengths (<5.0 L)." << std::endl;
}

void TestDomain6_SubmarineHullBuckling() {
    std::cout << "[Test 6/10] Domain 6: Submarine Pressure Hull Ring-Stiffened Buckling..." << std::endl;
    SubmarineHullParameters p;
    p.testDepthMeters = 400.0;

    auto res = EvaluateSubmarineHullBuckling(p);
    assert(res.bucklingSafe);
    assert(res.interframeBucklingSafetyFactor >= 2.0);
    assert(res.collapsePressureMpa > 8.0);
    std::cout << "  -> Domain 6 Passed: Submarine interframe buckling SF = " << res.interframeBucklingSafetyFactor << " at 400m depth." << std::endl;
}

void TestDomain7_WaterjetPropulsion() {
    std::cout << "[Test 7/10] Domain 7: Waterjet Propulsion Pump Impeller..." << std::endl;
    WaterjetPropulsionParameters p;
    p.impellerDiameterM = 0.85;

    auto res = AnalyzeWaterjetPump(p);
    assert(res.waterjetPumpEfficient);
    assert(res.grossThrustKiloNewtons > 50.0);
    assert(res.pumpEfficiencyPercent > 80.0);
    std::cout << "  -> Domain 7 Passed: Waterjet thrust = " << res.grossThrustKiloNewtons << " kN, efficiency = " << res.pumpEfficiencyPercent << "%." << std::endl;
}

void TestDomain8_MooringCatenary() {
    std::cout << "[Test 8/10] Domain 8: Mooring Line Catenary Tension & Platform Drift..." << std::endl;
    MooringCatenaryParameters p;
    p.waterDepthM = 850.0;

    auto res = SimulateMooringCatenary(p);
    assert(res.mooringAdequate);
    assert(res.safetyFactorMbl >= 1.67);
    assert(res.platformHorizontalOffsetM < 25.0);
    std::cout << "  -> Domain 8 Passed: Mooring SF MBL = " << res.safetyFactorMbl << " (> 1.67 per API RP 2SK)." << std::endl;
}

void TestDomain9_MarineRibletDrag() {
    std::cout << "[Test 9/10] Domain 9: Marine Anti-Fouling Micro-Texture Riblets..." << std::endl;
    MarineRibletParameters p;
    p.ribletHeightMicrons = 45.0;

    auto res = EvaluateRibletDragReduction(p);
    assert(res.dragReductionEffective);
    assert(res.skinFrictionDragReductionPercent > 5.0);
    assert(res.bioFoulingBarnacleAdhesionInhibited);
    std::cout << "  -> Domain 9 Passed: Skin friction drag reduction = " << res.skinFrictionDragReductionPercent << "%." << std::endl;
}

void TestDomain10_Marine2070Certification() {
    std::cout << "[Test 10/10] Domain 10: DNV / ABS Class & 2,070 Milestone Certification..." << std::endl;
    auto cert = CertifyAndroCadPhase13Platform();
    assert(cert.totalMilestonesCompleted == 2070);
    assert(cert.totalPhases == 13);
    assert(cert.semverVersion == "v11.0.0");
    assert(cert.zeroBroadStoragePermissions);
    assert(cert.occt801Integrated);
    assert(cert.buttonDeterminismVerified);
    assert(cert.allTestsGreen);
    std::cout << "  -> Domain 10 Passed: 2,070 milestones certified marine-class for Release v11.0.0." << std::endl;
}

int main() {
    std::cout << "============================================================" << std::endl;
    std::cout << " Starting AndroCAD Faz 13 Marine Hydro Suite (M1871-M2070) " << std::endl;
    std::cout << "============================================================" << std::endl;

    TestDomain1_BulbousBowWaveResistance();
    TestDomain2_PropellerCavitation();
    TestDomain3_SeakeepingSlamming();
    TestDomain4_HydrofoilPerformance();
    TestDomain5_ImoManeuverability();
    TestDomain6_SubmarineHullBuckling();
    TestDomain7_WaterjetPropulsion();
    TestDomain8_MooringCatenary();
    TestDomain9_MarineRibletDrag();
    TestDomain10_Marine2070Certification();

    std::cout << "============================================================" << std::endl;
    std::cout << " All 10/10 Faz 13 Marine Domains Passed Successfully!       " << std::endl;
    std::cout << "============================================================" << std::endl;
    return 0;
}
