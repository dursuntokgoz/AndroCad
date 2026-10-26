#include <iostream>
#include <cassert>
#include <cmath>
#include <vector>
#include "Phase18Engine.h"

using namespace AndroCAD::Phase18;

void TestDomain1_SwissLeverEscapement() {
    std::cout << "[Test 1/10] Domain 1: Swiss Lever Escapement Kinematic Geometry..." << std::endl;
    // 15 teeth, impulse angle = 32 deg, lock angle = 2.0 deg, fork length = 3.2 mm
    auto res = Phase18Engine::evaluateSwissLeverEscapement(15.0, 32.0, 2.0, 3.2);
    assert(res.liftAngleDegrees >= 48.0);
    assert(res.lockAngleDegrees >= 1.5 && res.lockAngleDegrees <= 3.0);
    assert(res.escapementOperatingReliably == true);
    std::cout << "  -> Domain 1 Passed: Lift angle = " << res.liftAngleDegrees << " deg, Lock = " << res.lockAngleDegrees << " deg." << std::endl;
}

void TestDomain2_HairspringIsochronism() {
    std::cout << "[Test 2/10] Domain 2: Balance Hairspring Isochronism & Breguet Overcoil..." << std::endl;
    // Silinvar spring: b = 0.12 mm, h = 0.035 mm, L = 125 mm, I = 12.0 mg*cm2, E = 170 GPa
    auto res = Phase18Engine::calculateHairspringOscillation(0.12, 0.035, 125.0, 12.0, 170.0);
    assert(res.naturalFrequencyHz >= 2.5 && res.naturalFrequencyHz <= 5.0);
    assert(res.isochronismErrorSecondsPerDay <= 2.0);
    assert(res.meetsCoscIsochronism == true);
    std::cout << "  -> Domain 2 Passed: Beat frequency = " << res.naturalFrequencyHz << " Hz (28,800 vph), Isochronism error = " << res.isochronismErrorSecondsPerDay << " s/d." << std::endl;
}

void TestDomain3_GearTrainMeshing() {
    std::cout << "[Test 3/10] Domain 3: Cycloidal Gear Train Meshing & Transmission..." << std::endl;
    // Barrel 84, Center 12/80, Third 10/75, Fourth 10/80, Escape 10, Barrel torque = 12.0 mNm
    auto res = Phase18Engine::calculateGearTrainMeshing(84, 12, 80, 10, 75, 10, 80, 10, 12.0);
    assert(res.totalGearRatio >= 3000.0);
    assert(res.escapeWheelTorqueMicroNm >= 1.5);
    assert(res.smoothContinuousTransmission == true);
    std::cout << "  -> Domain 3 Passed: Total gear ratio = " << res.totalGearRatio << ":1, Escape torque = " << res.escapeWheelTorqueMicroNm << " uNm." << std::endl;
}

void TestDomain4_MainspringBarrel() {
    std::cout << "[Test 4/10] Domain 4: Mainspring Barrel Torque & Power Reserve..." << std::endl;
    // Nivaflex blade: e = 0.11 mm, b = 1.25 mm, L = 380 mm, Rb = 5.2 mm, Ra = 1.1 mm
    auto res = Phase18Engine::evaluateMainspringPowerReserve(0.11, 1.25, 380.0, 5.2, 1.1);
    assert(res.powerReserveHours >= 48.0);
    assert(res.powerReserveExceeds48Hours == true);
    std::cout << "  -> Domain 4 Passed: Power reserve = " << res.powerReserveHours << " hours, Active coils = " << res.activeCoilCount << std::endl;
}

void TestDomain5_TourbillonDynamics() {
    std::cout << "[Test 5/10] Domain 5: Tourbillon Positional Error Cancellation..." << std::endl;
    // Titanium cage: m = 0.28 g, r = 6.5 mm, period = 60 s, uncompensated drift = 7.5 s/day
    auto res = Phase18Engine::evaluateTourbillonCancellation(0.28, 6.5, 60.0, 7.5);
    assert(res.tourbillonCompensatedDriftSecPerDay <= 1.20);
    assert(res.achievesSuperiorRateStability == true);
    std::cout << "  -> Domain 5 Passed: Tourbillon compensated drift = " << res.tourbillonCompensatedDriftSecPerDay << " s/day (88% cancellation)." << std::endl;
}

void TestDomain6_AutomaticWinding() {
    std::cout << "[Test 6/10] Domain 6: Automatic Winding Rotor Kinematics..." << std::endl;
    // Heavy tungsten rotor: m = 2.8 g, r_ecc = 12.0 mm, freq = 1.2 Hz, ratio = 145:1
    auto res = Phase18Engine::evaluateAutomaticWinding(2.8, 12.0, 1.2, 145.0);
    assert(res.windingTorqueDeliveredMilliNm >= 8.0);
    assert(res.windingEfficiencyPercent >= 75.0);
    assert(res.bidirectionalWindingEffective == true);
    std::cout << "  -> Domain 6 Passed: Winding torque = " << res.windingTorqueDeliveredMilliNm << " mNm, Efficiency = " << res.windingEfficiencyPercent << "%." << std::endl;
}

void TestDomain7_ChronographReset() {
    std::cout << "[Test 7/10] Domain 7: Chronograph Clutch & Heart Cam Reset..." << std::endl;
    // Hammer spring = 1.8 N, cam radius = 2.2 mm, clutch clearance = 0.15 mm
    auto res = Phase18Engine::evaluateChronographReset(1.8, 2.2, 0.15);
    assert(res.heartCamResetVelocityMs >= 1.0);
    assert(res.instantaneousZeroReset == true);
    std::cout << "  -> Domain 7 Passed: Reset hammer velocity = " << res.heartCamResetVelocityMs << " m/s, Clutch delay = " << res.horizontalClutchEngagementDelayMs << " ms." << std::endl;
}

void TestDomain8_BalancePoising() {
    std::cout << "[Test 8/10] Domain 8: Balance Wheel Inertia & Poising Error..." << std::endl;
    // Glucydur balance: OD = 9.8 mm, width = 0.8 mm, thick = 0.4 mm, unbalance = 0.8 ug at 4.5 mm
    auto res = Phase18Engine::evaluateBalanceWheelPoising(9.8, 0.8, 0.4, 0.8, 4.5);
    assert(res.poisingErrorMicrogramsMm < 15.0);
    assert(res.positionalRateVariationSecPerDay < 1.0);
    assert(res.perfectlyPoisedBalance == true);
    std::cout << "  -> Domain 8 Passed: Moment of inertia = " << res.momentOfInertiaMgCm2 << " mg*cm2, Poise error = " << res.poisingErrorMicrogramsMm << " ug*mm." << std::endl;
}

void TestDomain9_JewelBearingShock() {
    std::cout << "[Test 9/10] Domain 9: Jewel Bearing Friction & Incabloc Shock..." << std::endl;
    // Pivot d = 0.08 mm, load = 1.2 mN, shock = 5000g, Incabloc spring = 180 N/mm
    auto res = Phase18Engine::evaluateJewelBearingShock(0.08, 1.2, 5000.0, 180.0);
    assert(res.incablocAxialDeflectionMicrons <= 45.0);
    assert(res.withstandsIso1413DropTest == true);
    std::cout << "  -> Domain 9 Passed: Incabloc deflection = " << res.incablocAxialDeflectionMicrons << " um, ISO 1413 drop shock = " << res.maxImpactShockGWithstood << "g." << std::endl;
}

void TestDomain10_CoscCertification() {
    std::cout << "[Test 10/10] Domain 10: COSC Chronometer Multi-Position Certification..." << std::endl;
    // 5 positions rates: +1.2, +2.1, -0.8, +0.5, +1.8 s/day; Hot (38C) = +1.5, Cold (8C) = +0.2
    std::vector<double> rates = {1.2, 2.1, -0.8, 0.5, 1.8};
    auto res = Phase18Engine::evaluateCoscCompliance(rates, 1.5, 0.2);
    assert(res.meanDailyRateSecondsPerDay >= -4.0 && res.meanDailyRateSecondsPerDay <= +6.0);
    assert(res.meanVariationInRatesSecPerDay <= 2.0);
    assert(res.maxRateDifferenceBetweenPositionsSecPerDay <= 10.0);
    assert(res.thermalCoefficientSecPerDayPerCelsius <= 0.60);
    assert(res.certifiedChronometerGrade == true);
    std::cout << "  -> Domain 10 Passed: Mean rate = " << res.meanDailyRateSecondsPerDay << " s/d, Max pos diff = " << res.maxRateDifferenceBetweenPositionsSecPerDay << " s/d (COSC Certified)." << std::endl;
}

int main() {
    std::cout << "============================================================" << std::endl;
    std::cout << " Starting AndroCAD Faz 18 Horology Suite (M2871-M3070)      " << std::endl;
    std::cout << "============================================================" << std::endl;

    TestDomain1_SwissLeverEscapement();
    TestDomain2_HairspringIsochronism();
    TestDomain3_GearTrainMeshing();
    TestDomain4_MainspringBarrel();
    TestDomain5_TourbillonDynamics();
    TestDomain6_AutomaticWinding();
    TestDomain7_ChronographReset();
    TestDomain8_BalancePoising();
    TestDomain9_JewelBearingShock();
    TestDomain10_CoscCertification();

    std::cout << "============================================================" << std::endl;
    std::cout << " All 10/10 Faz 18 Horology Domains Passed Successfully!     " << std::endl;
    std::cout << "============================================================" << std::endl;
    return 0;
}
