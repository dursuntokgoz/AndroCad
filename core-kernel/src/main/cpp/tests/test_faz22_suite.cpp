#include <iostream>
#include <cassert>
#include <cmath>
#include "Phase22Engine.h"

using namespace AndroCAD::Phase22;

int main() {
    std::cout << "============================================================" << std::endl;
    std::cout << " Starting AndroCAD Faz 22 Wind Turbine Suite (M3671-M3870)  " << std::endl;
    std::cout << "============================================================" << std::endl;

    // Test 1: BEM Aerodynamics
    std::cout << "[Test 1/10] Domain 1: Blade Element Momentum (BEM) Aerodynamics..." << std::endl;
    auto bem = Phase22Engine::evaluateBemAerodynamics(10.5, 75.0, 55.0, 12.0, 1.2, 2.4, 5.8, 0.008);
    assert(bem.aerodynamicEquilibriumConverged);
    assert(bem.prandtlTipLossFactor > 0.2);
    assert(bem.sectionalThrustForceNPerM > 100.0);
    std::cout << "  -> Domain 1 Passed: Cl = " << bem.liftCoefficientCl << ", Prandtl F = " << bem.prandtlTipLossFactor << std::endl;

    // Test 2: Pitch Bearing Raceway Stress
    std::cout << "[Test 2/10] Domain 2: Pitch Slewing Bearing Hertzian Stresses..." << std::endl;
    auto bearing = Phase22Engine::evaluatePitchBearing(60.0, 120, 45.0, 8500.0, 450.0, 220.0);
    assert(bearing.hertzianStressWithinAllowable);
    assert(bearing.fatigueLifeExceeds20Years);
    assert(bearing.maxHertzContactStressMpa < 2500.0);
    std::cout << "  -> Domain 2 Passed: Hertz stress = " << bearing.maxHertzContactStressMpa << " MPa, L10h = " << bearing.racewayFatigueLifeL10hHours << " hrs." << std::endl;

    // Test 3: Planetary Gearbox ISO 6336
    std::cout << "[Test 3/10] Domain 3: Multi-Stage Planetary Gearbox Dynamics..." << std::endl;
    auto gearbox = Phase22Engine::evaluatePlanetaryGearbox(3200.0, 25, 45, 115, 5, 550.0, 20.0);
    assert(gearbox.gearboxReliabilityCertified);
    assert(gearbox.iso6336PittingSafetyFactorSh >= 1.25);
    assert(gearbox.iso6336BendingSafetyFactorSf >= 1.40);
    std::cout << "  -> Domain 3 Passed: Ratio = " << gearbox.gearRatio << ", Sh = " << gearbox.iso6336PittingSafetyFactorSh << ", Sf = " << gearbox.iso6336BendingSafetyFactorSf << std::endl;

    // Test 4: Aeroelastic Flutter
    std::cout << "[Test 4/10] Domain 4: Aeroelastic Blade Flutter Margin..." << std::endl;
    auto flutter = Phase22Engine::evaluateAeroelasticFlutter(75.0, 4.5e8, 3.2e7, 180.0, 240.0, 8.2);
    assert(flutter.aeroelasticStabilityMarginSufficient);
    assert(flutter.frequencySeparationRatio >= 2.5);
    assert(flutter.flutterOnsetWindSpeedMs > 35.0);
    std::cout << "  -> Domain 4 Passed: Flap f = " << flutter.firstFlapwiseFrequencyHz << " Hz, Torsion f = " << flutter.firstTorsionalFrequencyHz << " Hz, Sep ratio = " << flutter.frequencySeparationRatio << std::endl;

    // Test 5: PM Direct-Drive Generator
    std::cout << "[Test 5/10] Domain 5: PM Direct-Drive Generator & Cogging Torque..." << std::endl;
    auto gen = Phase22Engine::evaluatePmGenerator(5.0, 12.0, 3.2, 1.8, 64, 1.25);
    assert(gen.directDriveComplianceMet);
    assert(gen.electricalEfficiencyPercent >= 95.0);
    assert(gen.coggingTorquePeakPercent <= 1.5);
    std::cout << "  -> Domain 5 Passed: Torque = " << gen.electromagneticTorqueKnm << " kNm, Eff = " << gen.electricalEfficiencyPercent << "%" << std::endl;

    // Test 6: Tower VIV & Tuned Mass Damper
    std::cout << "[Test 6/10] Domain 6: Tubular Tower Vortex-Induced Vibration & TMD..." << std::endl;
    auto viv = Phase22Engine::evaluateTowerViv(3.8, 5.6, 120.0, 0.28, 0.02);
    assert(viv.lockInResonanceSuppressed);
    assert(viv.tunedMassDamperDampingRatio >= 0.05);
    std::cout << "  -> Domain 6 Passed: St = " << viv.strouhalNumber << ", TMD damping = " << viv.tunedMassDamperDampingRatio << std::endl;

    // Test 7: Trailing-Edge Serrations
    std::cout << "[Test 7/10] Domain 7: Serrated Trailing-Edge Aero-Acoustic Attenuation..." << std::endl;
    auto noise = Phase22Engine::evaluateTrailingEdgeNoise(78.0, 1.4, 25.0, 40.0, 80.0);
    assert(noise.acousticEmissionsCompliant);
    assert(noise.noiseReductionDeltaDba >= 3.0);
    std::cout << "  -> Domain 7 Passed: Delta SPL = " << noise.noiseReductionDeltaDba << " dBA (Baseline: " << noise.baselineNoiseSplDba << " dBA, Serrated: " << noise.serratedNoiseSplDba << " dBA)" << std::endl;

    // Test 8: Pitch Actuation
    std::cout << "[Test 8/10] Domain 8: Independent Pitch Control (IPC) Actuation..." << std::endl;
    auto pitch = Phase22Engine::evaluatePitchActuation(45.0, 18.0, 1.2e5, 12.0, 8.5);
    assert(pitch.independentPitchControlApproved);
    assert(pitch.maxPitchRateDegPerSec >= 8.0);
    std::cout << "  -> Domain 8 Passed: Pitch rate = " << pitch.maxPitchRateDegPerSec << " deg/s, Power = " << pitch.motorPowerRequirementKw << " kW" << std::endl;

    // Test 9: Offshore Monopile Soil-Structure
    std::cout << "[Test 9/10] Domain 9: Offshore Monopile Non-Linear Soil P-Y Deflection..." << std::endl;
    auto monopile = Phase22Engine::evaluateOffshoreMonopile(8.0, 75.0, 35.0, 2500.0, 65000.0, 34.0);
    assert(monopile.geotechnicalIntegritySatisfied);
    assert(monopile.accumulatedLongTermTiltDeg < 0.25);
    std::cout << "  -> Domain 9 Passed: Mudline disp = " << monopile.mudlineLateralDisplacementMm << " mm, Tilt = " << monopile.accumulatedLongTermTiltDeg << " deg" << std::endl;

    // Test 10: Blade Fatigue IEC 61400-23
    std::cout << "[Test 10/10] Domain 10: Full-Span Composite Blade Fatigue Damage Index..." << std::endl;
    auto fatigue = Phase22Engine::evaluateBladeFatigue(1200.0, 850.0, 0.045, 650.0, 1e7);
    assert(fatigue.iec61400_23CertificationPassed);
    assert(fatigue.minerCumulativeDamageIndexD < 0.85);
    std::cout << "  -> Domain 10 Passed: Miner D = " << fatigue.minerCumulativeDamageIndexD << ", Remaining life = " << fatigue.remainingFatigueLifeYears << " yrs" << std::endl;

    std::cout << "============================================================" << std::endl;
    std::cout << " All 10/10 Faz 22 Wind Turbine Domains Passed Successfully! " << std::endl;
    std::cout << "============================================================" << std::endl;
    return 0;
}
