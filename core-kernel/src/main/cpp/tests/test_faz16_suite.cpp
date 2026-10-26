#include <iostream>
#include <cassert>
#include <cmath>
#include "Phase16Engine.h"

using namespace AndroCAD::Phase16;

void TestDomain1_GisFieldStress() {
    std::cout << "[Test 1/10] Domain 1: GIS Electric Field Stress & Dielectric Breakdown..." << std::endl;
    // 145 kV GIS, inner r = 50 mm, outer R = 150 mm, gas pressure = 0.55 MPa
    auto res = Phase16Engine::evaluateGisElectricField(145.0, 50.0, 150.0, 0.55);
    assert(res.schwaigerFactor > 0.35);
    assert(res.dielectricSafetyMargin >= 1.25);
    assert(res.withstandsLightningImpulse == true);
    std::cout << "  -> Domain 1 Passed: Max E-field = " << res.maxElectricFieldKvm << " kV/m, Schwaiger = " << res.schwaigerFactor << std::endl;
}

void TestDomain2_ArcPlasma() {
    std::cout << "[Test 2/10] Domain 2: Arc Plasma Magnetohydrodynamics & Quenching..." << std::endl;
    // 40,000 A arc, 45 mm length, blast pressure = 0.85 MPa, sonic speed = 340 m/s
    auto res = Phase16Engine::simulateArcPlasma(40000.0, 45.0, 0.85, 340.0);
    assert(res.centralArcTemperatureKelvin > 10000.0 && res.centralArcTemperatureKelvin <= 22000.0);
    assert(res.arcQuenchedSuccessfully == true);
    std::cout << "  -> Domain 2 Passed: Arc temp = " << res.centralArcTemperatureKelvin << " K, voltage = " << res.arcVoltageVolts << " V." << std::endl;
}

void TestDomain3_TrvInterruption() {
    std::cout << "[Test 3/10] Domain 3: Transient Recovery Voltage (TRV) Interruption..." << std::endl;
    // 63 kA short-circuit, L = 0.25 mH, C = 4.5 nF, 50 Hz
    auto res = Phase16Engine::evaluateTrvInterruption(63.0, 0.25, 4.5, 50.0);
    assert(res.peakTrvKv > 150.0 && res.peakTrvKv < 300.0);
    assert(res.timeToPeakMicroseconds >= 20.0);
    assert(res.meetsIec62271_100 == true);
    std::cout << "  -> Domain 3 Passed: Peak TRV = " << res.peakTrvKv << " kV, RRRV = " << res.rrrvKvPerMicrosecond << " kV/us." << std::endl;
}

void TestDomain4_EcoGasMixture() {
    std::cout << "[Test 4/10] Domain 4: SF6 Alternative Gas Mixture (C4-FN / CO2 / O2)..." << std::endl;
    // 5% C4-FN, 85% CO2, 10% O2 at 0.70 MPa
    auto res = Phase16Engine::evaluateEcoGasMixture(5.0, 85.0, 10.0, 0.70);
    assert(res.dielectricStrengthRatioToSf6 >= 0.80);
    assert(res.globalWarmingPotentialGwp < 1000.0);
    assert(res.compliantWithEuFgasRegulation == true);
    std::cout << "  -> Domain 4 Passed: Dielectric ratio = " << res.dielectricStrengthRatioToSf6 << "x SF6, GWP = " << res.globalWarmingPotentialGwp << std::endl;
}

void TestDomain5_VftoDynamics() {
    std::cout << "[Test 5/10] Domain 5: Very Fast Transient Overvoltage (VFTO)..." << std::endl;
    // 145 kV strike, Z0 = 65 ohms, bus length = 25 m, R_spark = 5 ohms
    auto res = Phase16Engine::calculateVftoWavefront(145.0, 65.0, 25.0, 5.0);
    assert(res.wavefrontRiseTimeNanoseconds >= 3.0 && res.wavefrontRiseTimeNanoseconds <= 20.0);
    assert(res.peakVftoPu <= 2.2);
    assert(res.spacerInsulatorProtected == true);
    std::cout << "  -> Domain 5 Passed: Peak VFTO = " << res.peakVftoPu << " p.u., rise time = " << res.wavefrontRiseTimeNanoseconds << " ns." << std::endl;
}

void TestDomain6_VacuumArc() {
    std::cout << "[Test 6/10] Domain 6: Vacuum Interrupter AMF/RMF Arc Control..." << std::endl;
    // 40 kA interruption, contact diameter = 80 mm, turns = 2, gap = 12 mm
    auto res = Phase16Engine::evaluateVacuumArc(40.0, 80.0, 2.0, 12.0);
    assert(res.axialMagneticFieldTesla >= res.criticalAmfThresholdTesla);
    assert(res.avoidsAnodeSpotMelting == true);
    std::cout << "  -> Domain 6 Passed: Axial B-field = " << res.axialMagneticFieldTesla << " T (Threshold: " << res.criticalAmfThresholdTesla << " T)." << std::endl;
}

void TestDomain7_BusbarForces() {
    std::cout << "[Test 7/10] Domain 7: Busbar Electrodynamic Short-Circuit Forces..." << std::endl;
    // 63 kA RMS, phase spacing = 450 mm, span = 1200 mm, Z = 45 cm3
    auto res = Phase16Engine::calculateBusbarForces(63.0, 450.0, 1200.0, 45.0);
    assert(res.peakShortCircuitCurrentKa >= 150.0);
    assert(res.busbarBendingStressMpa < 220.0);
    assert(res.withstands63KaShortCircuit == true);
    std::cout << "  -> Domain 7 Passed: Peak force = " << res.peakPeakForceKnPerMeter << " kN/m, stress = " << res.busbarBendingStressMpa << " MPa." << std::endl;
}

void TestDomain8_BushingStress() {
    std::cout << "[Test 8/10] Domain 8: Capacitive Graded Bushing & Partial Discharge..." << std::endl;
    // 145 kV rated, 35 condenser layers, length = 1800 mm, eps_r = 3.6
    auto res = Phase16Engine::evaluateBushingStress(145.0, 35.0, 1800.0, 3.6);
    assert(res.partialDischargePicoCoulombs < 5.0);
    assert(res.meetsIec60137 == true);
    std::cout << "  -> Domain 8 Passed: Partial discharge = " << res.partialDischargePicoCoulombs << " pC (<5 pC limit)." << std::endl;
}

void TestDomain9_SwitchgearKinematics() {
    std::cout << "[Test 9/10] Domain 9: Operating Mechanism Kinematics & Opening Time..." << std::endl;
    // Spring preload = 8500 N, moving mass = 28 kg, stroke = 120 mm, damping = 0.15
    auto res = Phase16Engine::simulateOperatingMechanism(8500.0, 28.0, 120.0, 0.15);
    assert(res.totalOpeningTimeMilliseconds < 45.0);
    assert(res.contactSeparationVelocityMs >= 3.0);
    assert(res.highSpeedInterruptionMet == true);
    std::cout << "  -> Domain 9 Passed: Opening time = " << res.totalOpeningTimeMilliseconds << " ms, velocity = " << res.contactSeparationVelocityMs << " m/s." << std::endl;
}

void TestDomain10_ArcFlashHazard() {
    std::cout << "[Test 10/10] Domain 10: Arc Flash Blast Pressure & Thermal Energy..." << std::endl;
    // Fault current = 31.5 kA, duration = 0.08 s, distance = 610 mm, enclosure = 1000 mm
    auto res = Phase16Engine::evaluateArcFlashHazard(31.5, 0.08, 610.0, 1000.0);
    assert(res.incidentEnergyCalCm2 <= 25.0);
    assert(res.peakBlastPressureKpa < 80.0);
    assert(res.internalArcClassifiedIac == true);
    std::cout << "  -> Domain 10 Passed: Incident energy = " << res.incidentEnergyCalCm2 << " cal/cm2, PPE Category = " << res.ppeCategoryRequired << std::endl;
}

int main() {
    std::cout << "============================================================" << std::endl;
    std::cout << " Starting AndroCAD Faz 16 Switchgear Suite (M2471-M2670)    " << std::endl;
    std::cout << "============================================================" << std::endl;

    TestDomain1_GisFieldStress();
    TestDomain2_ArcPlasma();
    TestDomain3_TrvInterruption();
    TestDomain4_EcoGasMixture();
    TestDomain5_VftoDynamics();
    TestDomain6_VacuumArc();
    TestDomain7_BusbarForces();
    TestDomain8_BushingStress();
    TestDomain9_SwitchgearKinematics();
    TestDomain10_ArcFlashHazard();

    std::cout << "============================================================" << std::endl;
    std::cout << " All 10/10 Faz 16 Switchgear Domains Passed Successfully!   " << std::endl;
    std::cout << "============================================================" << std::endl;
    return 0;
}
