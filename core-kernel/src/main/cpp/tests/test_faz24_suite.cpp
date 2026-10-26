#include <iostream>
#include <cassert>
#include <cmath>
#include "Phase24Engine.h"

using namespace AndroCAD::Phase24;

int main() {
    std::cout << "============================================================" << std::endl;
    std::cout << " Starting AndroCAD Faz 24 Battery Physics Suite (M4071-M4270)" << std::endl;
    std::cout << "============================================================" << std::endl;

    // Test 1: DFN Electrochemistry
    std::cout << "[Test 1/10] Domain 1: Doyle-Fuller-Newman (DFN) Electrochemical Model..." << std::endl;
    auto dfn = Phase24Engine::evaluateDfnElectrochemistry(120.0, 150.0, 298.15, 500.0);
    assert(dfn.electrochemicalKineticsStable);
    assert(dfn.cellVoltageVolts >= 2.8 && dfn.cellVoltageVolts <= 4.25);
    assert(dfn.internalResistanceMilliOhms < 3.0);
    std::cout << "  -> Domain 1 Passed: Cell V = " << dfn.cellVoltageVolts << " V, SoC = " << dfn.stateOfChargePercent << "%, R_int = " << dfn.internalResistanceMilliOhms << " mOhm." << std::endl;

    // Test 2: Thermal Runaway Kinetics
    std::cout << "[Test 2/10] Domain 2: Thermal Runaway Exothermic Degradation..." << std::endl;
    auto runaway = Phase24Engine::evaluateThermalRunaway(45.0, 1.0, 150.0, 2.2);
    assert(runaway.thermalRunawayContained);
    assert(runaway.peakTemperatureCelsius <= 800.0);
    assert(runaway.gasGenerationVolumeLiters < 100.0);
    std::cout << "  -> Domain 2 Passed: Peak T = " << runaway.peakTemperatureCelsius << " C, Gas = " << runaway.gasGenerationVolumeLiters << " L, Heat = " << runaway.maxHeatGenerationRateWatts << " W." << std::endl;

    // Test 3: Pack Crashworthiness
    std::cout << "[Test 3/10] Domain 3: Pack Crashworthiness & Multi-Cell Extrusions..." << std::endl;
    auto crash = Phase24Engine::evaluatePackCrash(32.0, 2200.0, 180.0, 240.0);
    assert(crash.unEceR100CrashApproved);
    assert(crash.maxCellIntrusionDistanceMm < 30.0);
    assert(crash.plasticDeformationMarginMm >= 30.0);
    std::cout << "  -> Domain 3 Passed: Intrusion = " << crash.maxCellIntrusionDistanceMm << " mm, Energy = " << crash.energyAbsorptionKiloJoules << " kJ." << std::endl;

    // Test 4: Venting Gas Dynamics
    std::cout << "[Test 4/10] Domain 4: Venting Gas Dynamics & Burst Disc Relief..." << std::endl;
    auto vent = Phase24Engine::evaluateVentingGas(12.0, 850.0, 25.0, 0.032);
    assert(vent.safeOverpressureReliefAchieved);
    assert(vent.burstDiscRupturePressureBar <= 8.0);
    assert(vent.ventThroatMachNumber == 1.0);
    std::cout << "  -> Domain 4 Passed: Sonic vel = " << vent.ventingGasVelocityMs << " m/s, Rupture P = " << vent.burstDiscRupturePressureBar << " bar." << std::endl;

    // Test 5: Thermal Barrier Propagation Arrest
    std::cout << "[Test 5/10] Domain 5: Aerogel Inter-Cell Thermal Barrier..." << std::endl;
    auto barrier = Phase24Engine::evaluateThermalBarrier(750.0, 2.0, 0.018, 120.0);
    assert(barrier.cascadingPropagationPrevented);
    assert(barrier.maxAdjacentCellTemperatureC < 65.0);
    assert(barrier.aerogelThermalConductivityWMK <= 0.020);
    std::cout << "  -> Domain 5 Passed: Adj Cell T = " << barrier.maxAdjacentCellTemperatureC << " C, Attenuation = " << barrier.heatFluxAttenuationPercent << "%." << std::endl;

    // Test 6: Cold Plate Cooling
    std::cout << "[Test 6/10] Domain 6: Serpentine Cold Plate Micro-Channel Cooling..." << std::endl;
    auto cold = Phase24Engine::evaluateColdPlateCooling(15.0, 1800.0, 1.8, 4.5, 0.0028);
    assert(cold.optimalThermalUniformityAchieved);
    assert(cold.coolantPressureDropKpa <= 25.0);
    assert(cold.moduleTempNonUniformityDeltaC <= 2.5);
    std::cout << "  -> Domain 6 Passed: Delta P = " << cold.coolantPressureDropKpa << " kPa, Uniformity delta T = " << cold.moduleTempNonUniformityDeltaC << " C." << std::endl;

    // Test 7: Busbar Joint Contact Resistance
    std::cout << "[Test 7/10] Domain 7: High-Frequency Laser Welded Busbar Tabs..." << std::endl;
    auto busbar = Phase24Engine::evaluateBusbarJoint(350.0, 48.0, 98.0, 40.0);
    assert(busbar.highConductivityJointCertified);
    assert(busbar.jointContactResistanceMicroOhms <= 1.2);
    assert(busbar.maxJointTemperatureRiseK < 15.0);
    std::cout << "  -> Domain 7 Passed: Joint R = " << busbar.jointContactResistanceMicroOhms << " uOhm, Joule heat = " << busbar.jouleHeatingDissipationWatts << " W." << std::endl;

    // Test 8: High-Voltage Pyro-Fuse
    std::cout << "[Test 8/10] Domain 8: High-Voltage Battery Disconnect Unit (BDU) Pyro-Fuse..." << std::endl;
    auto fuse = Phase24Engine::evaluatePyroFuse(25.0, 8.5, 800.0, 0.5);
    assert(fuse.highVoltageIsolationGuaranteed);
    assert(fuse.clearingTimeMilliseconds < 1.5);
    assert(fuse.arcVoltageVolts > 800.0);
    std::cout << "  -> Domain 8 Passed: Clearing time = " << fuse.clearingTimeMilliseconds << " ms, Arc V = " << fuse.arcVoltageVolts << " V." << std::endl;

    // Test 9: Swelling & Breathing Stresses
    std::cout << "[Test 9/10] Domain 9: Cyclic Cell Swelling & End-Plate Deflection..." << std::endl;
    auto swell = Phase24Engine::evaluateSwellingBreathing(1.0, 800.0, 78.5, 15.0);
    assert(swell.mechanicalIntegrityPreserved);
    assert(swell.cellSwellingPressureMpa <= 0.85);
    assert(swell.tieRodTensileStressMpa <= 180.0);
    std::cout << "  -> Domain 9 Passed: Swell P = " << swell.cellSwellingPressureMpa << " MPa, Tie rod stress = " << swell.tieRodTensileStressMpa << " MPa." << std::endl;

    // Test 10: EIS & Acoustic Diagnostics
    std::cout << "[Test 10/10] Domain 10: In-Situ EIS & Ultrasonic Wave SOH Diagnostic..." << std::endl;
    auto eis = Phase24Engine::evaluateEisDiagnostic(1000.0, 0.85, -0.42, 2450.0);
    assert(eis.un38_3ComplianceCertified);
    assert(eis.stateOfHealthPercent >= 80.0);
    assert(eis.chargeTransferResistanceMilliOhms < 1.5);
    std::cout << "  -> Domain 10 Passed: SOH = " << eis.stateOfHealthPercent << "%, R_ct = " << eis.chargeTransferResistanceMilliOhms << " mOhm, ToF = " << eis.ultrasonicTimeOfFlightNs << " ns." << std::endl;

    std::cout << "============================================================" << std::endl;
    std::cout << " All 10/10 Faz 24 Battery Physics Domains Passed Cleanly!   " << std::endl;
    std::cout << "============================================================" << std::endl;
    return 0;
}
