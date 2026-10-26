#include <iostream>
#include <cassert>
#include <cmath>
#include "Phase20Engine.h"

using namespace AndroCAD::Phase20;

void TestDomain1_InducerCavitation() {
    std::cout << "[Test 1/10] Domain 1: Cryogenic Inducer Cavitation & Suction Speed..." << std::endl;
    // Flow = 0.085 m3/s, 32,000 RPM, NPSH = 18.0 m, tip = 140 mm, hub = 55 mm
    auto res = Phase20Engine::evaluateInducerCavitation(0.085, 32000.0, 18.0, 140.0, 55.0);
    assert(res.suctionSpecificSpeedNss >= 25000.0);
    assert(res.inducerHeadRiseM > 100.0);
    assert(res.cavitationFreeOperation == true);
    std::cout << "  -> Domain 1 Passed: Nss = " << res.suctionSpecificSpeedNss << ", Inducer head rise = " << res.inducerHeadRiseM << " m." << std::endl;
}

void TestDomain2_ImpellerStress() {
    std::cout << "[Test 2/10] Domain 2: High-Speed Impeller Centrifugal Stresses (FSI)..." << std::endl;
    // Outer diameter = 185 mm, 32,000 RPM, Ti-6Al-4V density = 4430 kg/m3, yield = 880 MPa, p = 180 bar
    auto res = Phase20Engine::evaluateImpellerStress(185.0, 32000.0, 4430.0, 880.0, 180.0);
    assert(res.burstMarginFactor >= 1.25);
    assert(res.maxTipSpeedMs < 650.0);
    assert(res.impellerStructuralIntegrityMet == true);
    std::cout << "  -> Domain 2 Passed: Tip speed = " << res.maxTipSpeedMs << " m/s, Burst margin = " << res.burstMarginFactor << std::endl;
}

void TestDomain3_RegenCooling() {
    std::cout << "[Test 3/10] Domain 3: Regenerative Cooling Channel Bartz Heat Flux..." << std::endl;
    // Chamber p = 100 bar, throat d = 160 mm, c* = 1780 m/s, coolant = 45 kg/s, channel = 1.8x4.5 mm, CuCrZr = 320 W/m-K
    auto res = Phase20Engine::calculateRegenCooling(100.0, 160.0, 1780.0, 45.0, 1.8, 4.5, 320.0);
    assert(res.hotWallTempK <= 850.0);
    assert(res.peakHeatFluxMwM2 <= 75.0);
    assert(res.chamberWallThermalMarginSufficient == true);
    std::cout << "  -> Domain 3 Passed: Peak heat flux = " << res.peakHeatFluxMwM2 << " MW/m2, Hot-wall temp = " << res.hotWallTempK << " K." << std::endl;
}

void TestDomain4_AcousticDamping() {
    std::cout << "[Test 4/10] Domain 4: Acoustic Cavity & Baffle Combustion Damping..." << std::endl;
    // Chamber d = 340 mm, c = 1150 m/s, aperture = 14 mm, depth = 38 mm, vol = 33 cm3 (tuned to 1T mode)
    auto res = Phase20Engine::evaluateAcousticDamping(340.0, 1150.0, 14.0, 38.0, 33.0);
    assert(res.firstTangentialFrequencyHz > 1500.0);
    assert(res.acousticAbsorptionCoeff >= 0.80);
    assert(res.preventsHighFrequencyChugging == true);
    std::cout << "  -> Domain 4 Passed: 1T acoustic freq = " << res.firstTangentialFrequencyHz << " Hz, Cavity absorption = " << res.acousticAbsorptionCoeff << std::endl;
}

void TestDomain5_TurbineBlisk() {
    std::cout << "[Test 5/10] Domain 5: Turbine Blisk Creep-Fatigue & Gas Expansion..." << std::endl;
    // Inlet T = 920 K, p = 140 bar, PR = 1.85, mass flow = 22 kg/s, r = 105 mm, 32,000 RPM
    auto res = Phase20Engine::evaluateTurbineBlisk(920.0, 140.0, 1.85, 22.0, 105.0, 32000.0);
    assert(res.shaftPowerDeliveredKw >= 2000.0);
    assert(res.bladeRootCentrifugalStressMpa < 520.0);
    assert(res.turbineBliskWithinCreepLimit == true);
    std::cout << "  -> Domain 5 Passed: Shaft power = " << res.shaftPowerDeliveredKw << " kW, Root stress = " << res.bladeRootCentrifugalStressMpa << " MPa." << std::endl;
}

void TestDomain6_InjectorAtomization() {
    std::cout << "[Test 6/10] Domain 6: Coaxial Swirl Injector Atomization (SMD D32)..." << std::endl;
    // LOX = 1.2 kg/s, CH4 = 0.45 kg/s, rho_lox = 1140 kg/m3, rho_ch4 = 422 kg/m3, d = 4.2 mm, deltaP = 15 bar
    auto res = Phase20Engine::evaluateInjectorAtomization(1.2, 0.45, 1140.0, 422.0, 4.2, 15.0);
    assert(res.sauterMeanDiameterMicrons <= 45.0);
    assert(res.mixingEfficiencyPercent >= 97.0);
    assert(res.optimalDropletAtomization == true);
    std::cout << "  -> Domain 6 Passed: Droplet SMD D32 = " << res.sauterMeanDiameterMicrons << " um, Mixing eff = " << res.mixingEfficiencyPercent << "%." << std::endl;
}

void TestDomain7_CryoBearing() {
    std::cout << "[Test 7/10] Domain 7: Cryogenic Hydrostatic & Hybrid Foil Bearings..." << std::endl;
    // Shaft d = 60 mm, L = 45 mm, clearance = 35 um, viscosity = 1.8e-4 Pa*s (LOX), 32,000 RPM, load = 3500 N
    auto res = Phase20Engine::evaluateCryoBearing(60.0, 45.0, 35.0, 1.8e-4, 32000.0, 3500.0);
    assert(res.fluidFilmThicknessMicrons >= 8.0);
    assert(res.directStiffnessNPerM >= 1e7);
    assert(res.stableRotorDynamicOperation == true);
    std::cout << "  -> Domain 7 Passed: Film thickness = " << res.fluidFilmThicknessMicrons << " um, Direct stiffness = " << res.directStiffnessNPerM << " N/m." << std::endl;
}

void TestDomain8_BellNozzleContour() {
    std::cout << "[Test 8/10] Domain 8: De Laval Supersonic Bell Nozzle Rao Contour..." << std::endl;
    // Throat d = 160 mm, area ratio = 38.0, gamma = 1.22, P_amb = 1.013 bar, Pc = 220 bar (full-flow staged combustion)
    auto res = Phase20Engine::calculateBellNozzleContour(160.0, 38.0, 1.22, 1.013, 220.0);
    assert(res.areaExpansionRatio == 38.0);
    assert(res.exitMachNumber > 4.0);
    assert(res.flowSeparationAvoided == true);
    std::cout << "  -> Domain 8 Passed: Exit Mach = " << res.exitMachNumber << ", Separation margin verified (Summerfield)." << std::endl;
}

void TestDomain9_CryoChilldown() {
    std::cout << "[Test 9/10] Domain 9: Cryogenic Chilldown Two-Phase Boiling..." << std::endl;
    // Pipe d = 75 mm, L = 6.0 m, t = 2.5 mm, LOX = 12 kg/s, T_initial = 293 K
    auto res = Phase20Engine::simulateCryoChilldown(75.0, 6.0, 2.5, 12.0, 293.0);
    assert(res.filmBoilingDurationSeconds < 30.0);
    assert(res.peakWaterHammerPressureBar < 25.0);
    assert(res.rapidCryoChilldownSuccessful == true);
    std::cout << "  -> Domain 9 Passed: Chilldown time = " << res.filmBoilingDurationSeconds << " s, Water hammer = " << res.peakWaterHammerPressureBar << " bar." << std::endl;
}

void TestDomain10_TvcActuator() {
    std::cout << "[Test 10/10] Domain 10: TVC Electro-Mechanical Actuator Kinematics..." << std::endl;
    // Engine mass = 1200 kg, I = 850 kg*m2, max deflection = 7.5 deg, slew = 20 deg/s, arm = 0.85 m
    auto res = Phase20Engine::evaluateTvcActuator(1200.0, 850.0, 7.5, 20.0, 0.85);
    assert(res.frequencyBandwidthHz >= 25.0);
    assert(res.slewRateDegreesPerSecond >= 20.0);
    assert(res.tvcFlightControlMeetsAs9100 == true);
    std::cout << "  -> Domain 10 Passed: Bandwidth = " << res.frequencyBandwidthHz << " Hz (>25 Hz), Actuator force = " << res.dynamicActuatorForceKn << " kN." << std::endl;
}

int main() {
    std::cout << "============================================================" << std::endl;
    std::cout << " Starting AndroCAD Faz 20 Rocket Turbopump Suite (M3271-3470)" << std::endl;
    std::cout << "============================================================" << std::endl;

    TestDomain1_InducerCavitation();
    TestDomain2_ImpellerStress();
    TestDomain3_RegenCooling();
    TestDomain4_AcousticDamping();
    TestDomain5_TurbineBlisk();
    TestDomain6_InjectorAtomization();
    TestDomain7_CryoBearing();
    TestDomain8_BellNozzleContour();
    TestDomain9_CryoChilldown();
    TestDomain10_TvcActuator();

    std::cout << "============================================================" << std::endl;
    std::cout << " All 10/10 Faz 20 Rocket Turbopump Domains Passed!          " << std::endl;
    std::cout << "============================================================" << std::endl;
    return 0;
}
