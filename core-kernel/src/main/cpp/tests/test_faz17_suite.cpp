#include <iostream>
#include <cassert>
#include <cmath>
#include "Phase17Engine.h"

using namespace AndroCAD::Phase17;

void TestDomain1_HoekBrown() {
    std::cout << "[Test 1/10] Domain 1: Hoek-Brown Rock Mass Failure Criterion..." << std::endl;
    // Granodiorite: sigma_3 = 5.0 MPa, UCS = 120.0 MPa, GSI = 65, mi = 28, D = 0
    auto res = Phase17Engine::evaluateHoekBrown(5.0, 120.0, 65.0, 28.0, 0.0);
    assert(res.majorPrincipalStrengthMpa > 40.0 && res.majorPrincipalStrengthMpa < 150.0);
    assert(res.rockMassModulusGpa > 15.0);
    assert(res.rockMassStable == true);
    std::cout << "  -> Domain 1 Passed: Major strength = " << res.majorPrincipalStrengthMpa << " MPa, Modulus = " << res.rockMassModulusGpa << " GPa." << std::endl;
}

void TestDomain2_CsmCutterForces() {
    std::cout << "[Test 2/10] Domain 2: TBM Disc Cutter Rolling Indentation (CSM Model)..." << std::endl;
    // 17-inch cutter, tip = 13 mm, spacing = 85 mm, penetration = 6 mm, UCS = 110 MPa, shear = 14 MPa
    auto res = Phase17Engine::calculateCsmCutterForces(17.0, 13.0, 85.0, 6.0, 110.0, 14.0);
    assert(res.normalForceKn > 80.0 && res.normalForceKn <= 280.0);
    assert(res.rollingForceKn > 5.0 && res.rollingForceKn <= 45.0);
    assert(res.cutterLoadWithinBearingLimit == true);
    std::cout << "  -> Domain 2 Passed: Normal force = " << res.normalForceKn << " kN, Rolling = " << res.rollingForceKn << " kN." << std::endl;
}

void TestDomain3_SegmentalLining() {
    std::cout << "[Test 3/10] Domain 3: Precast Segmental Lining Ring Joint Mechanics..." << std::endl;
    // Metro tunnel: R = 3.2 m, depth = 22 m, ground stiffness = 85 MPa, t = 0.35 m, bolt preload = 120 kN
    auto res = Phase17Engine::evaluateSegmentalLining(3.2, 22.0, 85.0, 0.35, 120.0);
    assert(res.jointOpeningDisplacementMm < 2.0);
    assert(res.boltTensionStressMpa < 640.0);
    assert(res.liningIntegrityMaintained == true);
    std::cout << "  -> Domain 3 Passed: Bending moment = " << res.maxLiningBendingMomentKnM << " kNm, joint opening = " << res.jointOpeningDisplacementMm << " mm." << std::endl;
}

void TestDomain4_EpbChamberPressure() {
    std::cout << "[Test 4/10] Domain 4: EPB Chamber Pressure Balance & Face Stability..." << std::endl;
    // Crown depth = 18 m, water table = 3 m, density = 19 kN/m3, c = 15 kPa, phi = 28 deg
    auto res = Phase17Engine::evaluateEpbFacePressure(18.0, 3.0, 19.0, 15.0, 28.0);
    assert(res.requiredChamberPressureBar > 1.5 && res.requiredChamberPressureBar < 5.0);
    assert(res.faceStabilitySafetyFactor >= 1.05);
    assert(res.surfaceCollapseAvoided == true);
    std::cout << "  -> Domain 4 Passed: Required chamber pressure = " << res.requiredChamberPressureBar << " bar, SF = " << res.faceStabilitySafetyFactor << std::endl;
}

void TestDomain5_PeckSettlement() {
    std::cout << "[Test 5/10] Domain 5: Peck Greenfield Settlement Trough..." << std::endl;
    // Depth = 20 m, diameter = 6.5 m, volume loss = 0.8%, offset = 5 m
    auto res = Phase17Engine::calculatePeckSettlement(20.0, 6.5, 0.8, 5.0);
    assert(res.maxSettlementMm < 15.0);
    assert(res.buildingDamageCategory <= 1);
    assert(res.settlementWithinNegligibleLimit == true);
    std::cout << "  -> Domain 5 Passed: Max settlement = " << res.maxSettlementMm << " mm, width parameter i = " << res.troughWidthParameterMeters << " m." << std::endl;
}

void TestDomain6_CcmConvergence() {
    std::cout << "[Test 6/10] Domain 6: Convergence-Confinement Method (CCM)..." << std::endl;
    // Tunnel R = 4.0 m, in-situ stress = 8.0 MPa, c = 2.5 MPa, phi = 32 deg, E = 8.0 GPa
    auto res = Phase17Engine::evaluateCcmConvergence(4.0, 8.0, 2.5, 32.0, 8.0);
    assert(res.wallConvergencePercent < 1.5);
    assert(res.plasticRadiusMeters < 10.0);
    assert(res.groundConfinementAdequate == true);
    std::cout << "  -> Domain 6 Passed: Convergence = " << res.wallConvergencePercent << "%, plastic radius = " << res.plasticRadiusMeters << " m." << std::endl;
}

void TestDomain7_TbmPowerTorque() {
    std::cout << "[Test 7/10] Domain 7: TBM Cutterhead Power & Torque Kinematics..." << std::endl;
    // TBM diameter = 6.5 m, 42 cutters, penetration = 5.5 mm/rev, RPM = 7.0, UCS = 90 MPa
    auto res = Phase17Engine::calculateTbmPowerTorque(6.5, 42, 5.5, 7.0, 90.0);
    assert(res.cutterheadPowerKw <= 4500.0);
    assert(res.requiredTorqueKnM <= 6500.0);
    assert(res.drivePowerSufficient == true);
    std::cout << "  -> Domain 7 Passed: Torque = " << res.requiredTorqueKnM << " kNm, Power = " << res.cutterheadPowerKw << " kW, Advance = " << res.advanceRateMetersPerHour << " m/h." << std::endl;
}

void TestDomain8_TailGrout() {
    std::cout << "[Test 8/10] Domain 8: Annular Gap Tail Grout Solidification..." << std::endl;
    // Lining R = 3.2 m, excavated R = 3.32 m, lining weight = 125 kN/m, water density = 10 kN/m3, pore press = 1.8 bar
    auto res = Phase17Engine::evaluateTailGrouting(3.2, 3.32, 125.0, 10.0, 1.8);
    assert(res.groutVolumeRequiredM3 > 2.0 && res.groutVolumeRequiredM3 < 5.0);
    assert(res.netTunnelBuoyancyKnPerMeter <= 15.0);
    assert(res.preventsTunnelFloatation == true);
    std::cout << "  -> Domain 8 Passed: Grout volume = " << res.groutVolumeRequiredM3 << " m3/m, Net buoyancy = " << res.netTunnelBuoyancyKnPerMeter << " kN/m." << std::endl;
}

void TestDomain9_ShieldJammingRisk() {
    std::cout << "[Test 9/10] Domain 9: Squeezing Ground Shield Jamming Risk..." << std::endl;
    // R = 3.5 m, overburden = 12.0 MPa, competence factor = 0.85, shield L = 9.0 m, taper = 35 mm, thrust = 45000 kN
    auto res = Phase17Engine::evaluateShieldJammingRisk(3.5, 12.0, 0.85, 9.0, 35.0, 45000.0);
    assert(res.thrustMarginPercent >= 25.0);
    assert(res.shieldFreeFromJamming == true);
    std::cout << "  -> Domain 9 Passed: Ground closure = " << res.radialGroundClosureMm << " mm, thrust margin = " << res.thrustMarginPercent << "%." << std::endl;
}

void TestDomain10_TunnelSeismic() {
    std::cout << "[Test 10/10] Domain 10: Tunnel Seismic Dynamic Shear Strain..." << std::endl;
    // PGA = 0.35g, shear wave Cs = 650 m/s, lining R = 3.2 m, E = 35 GPa
    auto res = Phase17Engine::evaluateTunnelSeismicResponse(0.35, 650.0, 3.2, 35.0);
    assert(res.liningShearStrainMicrostrain < res.concreteTensileStrainLimit);
    assert(res.earthquakeResistantLining == true);
    std::cout << "  -> Domain 10 Passed: Seismic shear strain = " << res.liningShearStrainMicrostrain << " microstrain (<150 limit)." << std::endl;
}

int main() {
    std::cout << "============================================================" << std::endl;
    std::cout << " Starting AndroCAD Faz 17 Geotechnical Suite (M2671-M2870)  " << std::endl;
    std::cout << "============================================================" << std::endl;

    TestDomain1_HoekBrown();
    TestDomain2_CsmCutterForces();
    TestDomain3_SegmentalLining();
    TestDomain4_EpbChamberPressure();
    TestDomain5_PeckSettlement();
    TestDomain6_CcmConvergence();
    TestDomain7_TbmPowerTorque();
    TestDomain8_TailGrout();
    TestDomain9_ShieldJammingRisk();
    TestDomain10_TunnelSeismic();

    std::cout << "============================================================" << std::endl;
    std::cout << " All 10/10 Faz 17 Tunneling Domains Passed Successfully!    " << std::endl;
    std::cout << "============================================================" << std::endl;
    return 0;
}
