#include <iostream>
#include <cassert>
#include <cmath>
#include "Phase26Engine.h"

using namespace AndroCAD::Phase26;

int main() {
    std::cout << "============================================================" << std::endl;
    std::cout << " Starting AndroCAD Faz 26 Deep-Sea Submersible Suite        " << std::endl;
    std::cout << "============================================================" << std::endl;

    // Test 1: Titanium Pressure Sphere
    std::cout << "[Test 1/10] Domain 1: Titanium Grade 5 Pressure Sphere Buckling..." << std::endl;
    auto tiSphere = Phase26Engine::evaluateTitaniumSphere(2.1, 11000.0, 830.0, 0.0015);
    assert(tiSphere.fullOceanDepthCertified);
    assert(tiSphere.asmePvhoSafetyFactor >= 1.50);
    assert(tiSphere.membraneStressMpa < 830.0 * 0.70);
    std::cout << "  -> Domain 1 Passed: Pressure = " << tiSphere.hydrostaticPressureMpa << " MPa, Buckling P = " << tiSphere.bifurcationBucklingPressureMpa << " MPa, SF = " << tiSphere.asmePvhoSafetyFactor << std::endl;

    // Test 2: Acrylic Viewport
    std::cout << "[Test 2/10] Domain 2: Truncated Conical Acrylic Viewport..." << std::endl;
    auto viewport = Phase26Engine::evaluateAcrylicViewport(200.0, 11000.0, 90.0, 10000.0);
    assert(viewport.asmePvho1ViewportApproved);
    assert(viewport.seatBendingStressMpa <= 55.0);
    assert(viewport.viscoelasticCreepDeflectionMm < 1.5);
    std::cout << "  -> Domain 2 Passed: Thickness = " << viewport.thicknessMm << " mm, Creep = " << viewport.viscoelasticCreepDeflectionMm << " mm." << std::endl;

    // Test 3: Syntactic Buoyancy Foam
    std::cout << "[Test 3/10] Domain 3: Syntactic Buoyancy Foam Hydrostatic Collapse..." << std::endl;
    auto foam = Phase26Engine::evaluateSyntacticFoam(8.5, 11000.0, 0.68);
    assert(foam.deepSeaBuoyancyCertified);
    assert(foam.hydrostaticCollapsePressureMpa >= 160.0);
    assert(foam.waterAbsorptionPercent < 0.5);
    std::cout << "  -> Domain 3 Passed: Density = " << foam.foamDensityKgM3 << " kg/m3, Lift = " << foam.netPositiveBuoyancyKn << " kN, Collapse P = " << foam.hydrostaticCollapsePressureMpa << " MPa." << std::endl;

    // Test 4: PBOF Subsea Enclosure
    std::cout << "[Test 4/10] Domain 4: Pressure-Balanced Oil-Filled (PBOF) Electronics..." << std::endl;
    auto pbof = Phase26Engine::evaluatePbofEnclosure(114.5, 45.0, 1.5);
    assert(pbof.pressureBalancedSealed);
    assert(pbof.differentialPressureBar <= 0.35);
    assert(pbof.dielectricBreakdownKv >= 35.0);
    std::cout << "  -> Domain 4 Passed: Delta P = " << pbof.differentialPressureBar << " bar, Dielectric = " << pbof.dielectricBreakdownKv << " kV/mm." << std::endl;

    // Test 5: Subsea Hydraulic Manipulator
    std::cout << "[Test 5/10] Domain 5: 7-DOF Spatial Subsea Hydraulic Manipulator..." << std::endl;
    auto manip = Phase26Engine::evaluateSubseaManipulator(1.0, 210.0, 150.0);
    assert(manip.sevenDofTeleoperationVerified);
    assert(manip.payloadCapacityKg >= 150.0);
    assert(manip.jointPositionErrorMm < 5.0);
    std::cout << "  -> Domain 5 Passed: Capacity = " << manip.payloadCapacityKg << " kg at " << manip.maxReachMeters << " m reach." << std::endl;

    // Test 6: High-Pressure Penetrator
    std::cout << "[Test 6/10] Domain 6: Ceramic-Sealed High-Pressure Penetrators..." << std::endl;
    auto penetrator = Phase26Engine::evaluateHighPressurePenetrator(125.0, 24, 6.0);
    assert(penetrator.hermeticFeedthroughCertified);
    assert(penetrator.heliumLeakRateMbarLPerS < 1e-9);
    assert(penetrator.electricalInsulationResistanceGigaOhms >= 5.0);
    std::cout << "  -> Domain 6 Passed: Helium leak = " << penetrator.heliumLeakRateMbarLPerS << " mbar*L/s, R_insul = " << penetrator.electricalInsulationResistanceGigaOhms << " GigaOhm." << std::endl;

    // Test 7: Life Support LiOH Scrubber
    std::cout << "[Test 7/10] Domain 7: Life Support Oxygen & LiOH CO2 Scrubber..." << std::endl;
    auto lss = Phase26Engine::evaluateLifeSupport(3, 4.8, 18.0, 1.5);
    assert(lss.lifeSupportBudgetCompliant);
    assert(lss.missionEnduranceHours >= 72.0);
    assert(lss.cabinCo2PartialPressureMbar < 5.0);
    std::cout << "  -> Domain 7 Passed: Endurance = " << lss.missionEnduranceHours << " hrs, CO2 = " << lss.cabinCo2PartialPressureMbar << " mbar." << std::endl;

    // Test 8: Emergency Ascent Ballast
    std::cout << "[Test 8/10] Domain 8: Drop-Weight Emergency Ascent Ballast Release..." << std::endl;
    auto ballast = Phase26Engine::evaluateAscentBallast(12500.0, 14.5, 850.0);
    assert(ballast.failsafeEmergencyAscentGuaranteed);
    assert(ballast.terminalAscentSpeedMs >= 1.2);
    assert(ballast.electromagneticHoldingForceKn >= 15.0);
    std::cout << "  -> Domain 8 Passed: Ascent speed = " << ballast.terminalAscentSpeedMs << " m/s, Holding force = " << ballast.electromagneticHoldingForceKn << " kN." << std::endl;

    // Test 9: Submersible Magnetic Thruster
    std::cout << "[Test 9/10] Domain 9: Ducted Kort Nozzle Magnetic Coupling Thruster..." << std::endl;
    auto thruster = Phase26Engine::evaluateSubmersibleThruster(5.0, 320.0, 4.5);
    assert(thruster.magneticDriveImmersionVerified);
    assert(thruster.bollardThrustNewtons >= 1800.0);
    assert(thruster.magneticCouplingBreakawayTorqueNm > 60.0);
    std::cout << "  -> Domain 9 Passed: Thrust = " << thruster.bollardThrustNewtons << " N, Breakaway torque = " << thruster.magneticCouplingBreakawayTorqueNm << " Nm." << std::endl;

    // Test 10: USBL Acoustic Telemetry
    std::cout << "[Test 10/10] Domain 10: Deep-Sea USBL Acoustic Positioning & Telemetry..." << std::endl;
    auto usbl = Phase26Engine::evaluateUsblTelemetry(10920.0, 12.5, 1500.0);
    assert(usbl.fullOceanDepthNavigationLocked);
    assert(usbl.slantRangeMeters >= 11000.0);
    assert(usbl.positioningAccuracyMeters < 25.0);
    std::cout << "  -> Domain 10 Passed: Slant range = " << usbl.slantRangeMeters << " m, Latency = " << usbl.acousticOneWayLatencySeconds << " s, Accuracy = " << usbl.positioningAccuracyMeters << " m." << std::endl;

    std::cout << "============================================================" << std::endl;
    std::cout << " All 10/10 Faz 26 Deep-Sea Domains Passed Cleanly!          " << std::endl;
    std::cout << "============================================================" << std::endl;
    return 0;
}
