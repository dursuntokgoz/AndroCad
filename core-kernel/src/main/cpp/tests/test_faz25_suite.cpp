#include <iostream>
#include <cassert>
#include <cmath>
#include "Phase25Engine.h"

using namespace AndroCAD::Phase25;

int main() {
    std::cout << "============================================================" << std::endl;
    std::cout << " Starting AndroCAD Faz 25 Railway Dynamics Suite (M4271-M4470)" << std::endl;
    std::cout << "============================================================" << std::endl;

    // Test 1: Kalker Contact Mechanics
    std::cout << "[Test 1/10] Domain 1: Kalker Wheel-Rail Creep & Hertz Stress..." << std::endl;
    auto kalker = Phase25Engine::evaluateKalkerContact(85.0, 460.0, 300.0, 0.0015, 0.0008, 0.02);
    assert(kalker.adhesionLimitSatisfied);
    assert(kalker.peakHertzContactStressMpa < 1500.0);
    assert(kalker.contactEllipseSemiAxisAMm > 0.0);
    std::cout << "  -> Domain 1 Passed: Hertz P0 = " << kalker.peakHertzContactStressMpa << " MPa, a = " << kalker.contactEllipseSemiAxisAMm << " mm, b = " << kalker.contactEllipseSemiAxisBMm << " mm." << std::endl;

    // Test 2: Hunting Stability
    std::cout << "[Test 2/10] Domain 2: High-Speed Bogie Hunting Stability & Klingel..." << std::endl;
    auto hunting = Phase25Engine::evaluateHuntingStability(350.0, 0.05, 1650.0, 3200.0, 1435.0);
    assert(hunting.highSpeedHuntingSuppressed);
    assert(hunting.criticalHuntingSpeedKmh >= 380.0);
    assert(hunting.hopfBifurcationDampingRatio >= 0.05);
    std::cout << "  -> Domain 2 Passed: V_crit = " << hunting.criticalHuntingSpeedKmh << " km/h, Klingel f = " << hunting.klingelFrequencyHz << " Hz, Damping = " << hunting.hopfBifurcationDampingRatio << std::endl;

    // Test 3: Primary Suspension
    std::cout << "[Test 3/10] Domain 3: Primary Coil Spring & Axlebox Guiding..." << std::endl;
    auto primary = Phase25Engine::evaluatePrimarySuspension(170.0, 42.0, 175.0, 5, 25000.0);
    assert(primary.primaryIsolationCompliant);
    assert(primary.verticalDeflectionMm >= 25.0 && primary.verticalDeflectionMm <= 65.0);
    std::cout << "  -> Domain 3 Passed: Deflection = " << primary.verticalDeflectionMm << " mm, K_x = " << primary.longitudinalStiffnessKnMm << " kN/mm." << std::endl;

    // Test 4: Secondary Air Spring
    std::cout << "[Test 4/10] Domain 4: Secondary Pneumatic Air Spring & Leveling..." << std::endl;
    auto air = Phase25Engine::evaluateSecondaryAirSpring(38000.0, 580.0, 60.0, 4.8);
    assert(air.secondarySuspensionCertified);
    assert(air.verticalStiffnessKnM >= 250.0 && air.verticalStiffnessKnM <= 650.0);
    assert(air.carbodyLevelingResponseTimeS < 2.0);
    std::cout << "  -> Domain 4 Passed: K_z = " << air.verticalStiffnessKnM << " kN/m, K_y = " << air.lateralStiffnessKnM << " kN/m, Leveling = " << air.carbodyLevelingResponseTimeS << " s." << std::endl;

    // Test 5: Anti-Yaw Damper
    std::cout << "[Test 5/10] Domain 5: Anti-Yaw Hydraulic Damper (Schlingerdämpfer)..." << std::endl;
    auto yaw = Phase25Engine::evaluateAntiYawDamper(0.08, 16.5, 0.10);
    assert(yaw.bogieBogieHuntingPrevented);
    assert(yaw.yawStabilityMarginFactor >= 1.35);
    assert(yaw.blowOffDampingForceKn >= 12.0);
    std::cout << "  -> Domain 5 Passed: Blow-off F = " << yaw.blowOffDampingForceKn << " kN, Margin = " << yaw.yawStabilityMarginFactor << std::endl;

    // Test 6: Bogie Frame Fatigue (Dang Van)
    std::cout << "[Test 6/10] Domain 6: Welded Bogie Frame Multiaxial Dang Van Fatigue..." << std::endl;
    auto frame = Phase25Engine::evaluateBogieFrameFatigue(160.0, 48.0, 32.0, 355.0);
    assert(frame.en13749FrameStructuralApproved);
    assert(frame.dangVanFatigueLimitRatio <= 0.85);
    std::cout << "  -> Domain 6 Passed: Dang Van ratio = " << frame.dangVanFatigueLimitRatio << ", Tresca = " << frame.maxTrescaStressMpa << " MPa." << std::endl;

    // Test 7: Disc Brakes Thermal Dissipation
    std::cout << "[Test 7/10] Domain 7: High-Speed Ventilated Disc Brake Energy..." << std::endl;
    auto brake = Phase25Engine::evaluateDiscBrake(350.0, 14500.0, 640.0, 0.35);
    assert(brake.thermalCrackingPrevented);
    assert(brake.peakDiscTemperatureC <= 650.0);
    assert(brake.emergencyBrakingDistanceM < 4500.0);
    std::cout << "  -> Domain 7 Passed: Stop dist = " << brake.emergencyBrakingDistanceM << " m, Peak T = " << brake.peakDiscTemperatureC << " C." << std::endl;

    // Test 8: Nadal Derailment Criterion
    std::cout << "[Test 8/10] Domain 8: Flange Climb Derailment (Nadal Y/Q)..." << std::endl;
    auto nadal = Phase25Engine::evaluateNadalDerailment(42.0, 85.0, 70.0, 0.32);
    assert(nadal.en14363RunningSafetyVerified);
    assert(nadal.nadalDerailmentRatioYQ <= 0.80);
    std::cout << "  -> Domain 8 Passed: Y/Q = " << nadal.nadalDerailmentRatioYQ << ", Flange angle = " << nadal.flangeContactAngleDegrees << " deg." << std::endl;

    // Test 9: Active Traction Drive
    std::cout << "[Test 9/10] Domain 9: Fully Suspended Traction Motor & Hollow Cardan..." << std::endl;
    auto drive = Phase25Engine::evaluateTractionDrive(1850.0, 4200.0, 15.0, 140.0);
    assert(drive.fullySuspendedDriveApproved);
    assert(drive.cardanShaftMisalignmentDeg <= 2.5);
    assert(drive.hollowShaftTorsionalStressMpa < 120.0);
    std::cout << "  -> Domain 9 Passed: Misalignment = " << drive.cardanShaftMisalignmentDeg << " deg, Shaft stress = " << drive.hollowShaftTorsionalStressMpa << " MPa." << std::endl;

    // Test 10: Sperling Ride Index
    std::cout << "[Test 10/10] Domain 10: Ride Comfort Quality (Sperling Wz & EN 14363)..." << std::endl;
    auto ride = Phase25Engine::evaluateSperlingRideIndex(350.0, 1.0, 38000.0);
    assert(ride.superiorRideQualityCertified);
    assert(ride.verticalSperlingIndexWz <= 2.2);
    assert(ride.lateralSperlingIndexWz <= 2.2);
    std::cout << "  -> Domain 10 Passed: Wz_vert = " << ride.verticalSperlingIndexWz << ", Wz_lat = " << ride.lateralSperlingIndexWz << " (Superior)." << std::endl;

    std::cout << "============================================================" << std::endl;
    std::cout << " All 10/10 Faz 25 Railway Dynamics Domains Passed Cleanly!  " << std::endl;
    std::cout << "============================================================" << std::endl;
    return 0;
}
