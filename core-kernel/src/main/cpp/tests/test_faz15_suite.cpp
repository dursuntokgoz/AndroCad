#include <iostream>
#include <cassert>
#include <cmath>
#include "Phase15Engine.h"

using namespace AndroCAD::Phase15;

void TestDomain1_RPVStress() {
    std::cout << "[Test 1/10] Domain 1: RPV ASME Section III Stress Analysis..." << std::endl;
    // PWR RPV: internal pressure = 15.5 MPa, inner radius = 2200 mm, thickness = 220 mm, allowable = 180 MPa
    auto res = Phase15Engine::calculateRPVStress(15.5, 2200.0, 220.0, 180.0);
    assert(res.hoopStressMpa > 150.0 && res.hoopStressMpa < 180.0);
    assert(res.longitudinalStressMpa > 70.0 && res.longitudinalStressMpa < 90.0);
    assert(res.vonMisesStressMpa > 130.0 && res.vonMisesStressMpa < 160.0);
    assert(res.safetyFactor >= 1.15);
    std::cout << "  -> Domain 1 Passed: Hoop stress = " << res.hoopStressMpa << " MPa, SF = " << res.safetyFactor << std::endl;
}

void TestDomain2_ThermalCreep() {
    std::cout << "[Test 2/10] Domain 2: High-Temperature Thermal Creep Relaxation..." << std::endl;
    // 550 K (277 C), 120 MPa, 80,000 hours
    auto res = Phase15Engine::evaluateThermalCreep(120.0, 550.0, 80000.0, 1.0e-5, 4.0);
    assert(res.withinCreepLimit == true);
    assert(res.totalCreepStrainPercent < 1.0);
    assert(res.relaxedStressMpa <= 120.0);
    std::cout << "  -> Domain 2 Passed: Total creep strain = " << res.totalCreepStrainPercent << "% (<1.0%)." << std::endl;
}

void TestDomain3_RadiationEmbrittlement() {
    std::cout << "[Test 3/10] Domain 3: Fast Neutron Fluence Embrittlement..." << std::endl;
    // Fluence 3.5 x 10^19 n/cm2, Cu 0.08%, Ni 0.65%, initial RT_NDT = -20 C
    auto res = Phase15Engine::evaluateNeutronEmbrittlement(3.5, 0.08, 0.65, -20.0);
    assert(res.deltaRtNdtCelsius > 40.0 && res.deltaRtNdtCelsius < 150.0);
    assert(res.adjustedRtNdtCelsius < 132.0);
    assert(res.upperShelfEnergyJoules >= 68.0);
    assert(res.acceptableCharpyToughness == true);
    std::cout << "  -> Domain 3 Passed: Adjusted RT_NDT = " << res.adjustedRtNdtCelsius << " C, USE = " << res.upperShelfEnergyJoules << " J." << std::endl;
}

void TestDomain4_ShieldingAttenuation() {
    std::cout << "[Test 4/10] Domain 4: Gamma-Ray Shielding Attenuation & Buildup..." << std::endl;
    // Heavy concrete 160 cm, mu = 0.12 cm^-1, A = 0.8, incident = 50,000 mSv/hr
    auto res = Phase15Engine::calculateShieldingAttenuation(160.0, 0.12, 0.8, 50000.0);
    assert(res.exitDoseRateMilliSvPerHour < 0.02);
    assert(res.attenuationDecades > 5.0);
    assert(res.meetsAlaraStandard == true);
    std::cout << "  -> Domain 4 Passed: Exit dose rate = " << res.exitDoseRateMilliSvPerHour << " mSv/h (<0.02 ALARA limit)." << std::endl;
}

void TestDomain5_CrdmScram() {
    std::cout << "[Test 5/10] Domain 5: CRDM Emergency Scram Dynamics..." << std::endl;
    // Rod cluster mass = 120 kg, stroke = 3.8 m, spring force = 600 N, water density = 720 kg/m3
    auto res = Phase15Engine::simulateCrdmScram(120.0, 3.8, 720.0, 0.45, 600.0);
    assert(res.scramTimeSeconds < 2.50);
    assert(res.scramTimeSeconds > 0.50);
    assert(res.fastScramCompliant == true);
    std::cout << "  -> Domain 5 Passed: Scram drop time = " << res.scramTimeSeconds << " s (<2.5s)." << std::endl;
}

void TestDomain6_DnbrMargin() {
    std::cout << "[Test 6/10] Domain 6: Thermal-Hydraulic DNBR Margin..." << std::endl;
    // Coolant mass flux = 3500 kg/m2-s, 15.5 MPa, peak local heat flux = 1200 kW/m2
    auto res = Phase15Engine::evaluateThermalHydraulicDnbr(3500.0, 15.5, 1250.0, 1200.0);
    assert(res.dnbr >= 1.30);
    assert(res.noNucleateBoilingDeparture == true);
    std::cout << "  -> Domain 6 Passed: Minimum DNBR = " << res.dnbr << " (>= 1.30 limit)." << std::endl;
}

void TestDomain7_SeismicSseResponse() {
    std::cout << "[Test 7/10] Domain 7: Safe Shutdown Earthquake (SSE) Response..." << std::endl;
    // Containment + NSSS mass = 45,000 tons, stiffness = 85,000 kN/mm, SSE PGA = 0.35g, damping = 5%
    auto res = Phase15Engine::evaluateSeismicResponse(45000.0, 85000.0, 0.35, 0.05);
    assert(res.naturalFrequencyHz > 4.0 && res.naturalFrequencyHz < 15.0);
    assert(res.maxDisplacementMm < 25.0);
    assert(res.meetsSseMargin == true);
    std::cout << "  -> Domain 7 Passed: Structural frequency = " << res.naturalFrequencyHz << " Hz, sway = " << res.maxDisplacementMm << " mm." << std::endl;
}

void TestDomain8_ShroudFiv() {
    std::cout << "[Test 8/10] Domain 8: Core Shroud Flow Induced Vibration (FIV)..." << std::endl;
    // Cross-flow = 4.2 m/s, shroud outer diameter = 3400 mm, natural frequency = 12.5 Hz, St = 0.20
    auto res = Phase15Engine::evaluateShroudVibration(4.2, 3400.0, 12.5, 0.20);
    assert(res.vortexSheddingFrequencyHz < 1.0);
    assert(res.lockInResonanceAvoided == true);
    std::cout << "  -> Domain 8 Passed: Shedding frequency = " << res.vortexSheddingFrequencyHz << " Hz, lock-in avoided." << std::endl;
}

void TestDomain9_ZircaloyOxidation() {
    std::cout << "[Test 9/10] Domain 9: Zircaloy-4 High-Temp Cladding Oxidation..." << std::endl;
    // Peak clad temp = 1050 C, transient duration = 120 s, initial oxide = 15 um
    auto res = Phase15Engine::evaluateZircaloyOxidation(1050.0, 120.0, 15.0);
    assert(res.compliesWith10Cfr5046 == true);
    assert(res.equivalentCladdingReactedPercent < 17.0);
    assert(res.oxideLayerThicknessMicrons > 15.0 && res.oxideLayerThicknessMicrons < 80.0);
    std::cout << "  -> Domain 9 Passed: Clad oxide thickness = " << res.oxideLayerThicknessMicrons << " um, ECR = " << res.equivalentCladdingReactedPercent << "% (<17%)." << std::endl;
}

void TestDomain10_ContainmentTendonCapacity() {
    std::cout << "[Test 10/10] Domain 10: Containment Vessel Prestressed Tendon Capacity..." << std::endl;
    // Initial prestress = 1100 MPa, creep/shrinkage loss = 120 MPa, relaxation loss = 45 MPa, R = 22 m, t = 1.2 m
    auto res = Phase15Engine::evaluateContainmentTendonLoss(1100.0, 120.0, 45.0, 22.0, 1.2);
    assert(res.prestressLossPercent < 22.0);
    assert(res.ultimateBurstPressureMpa >= 0.90);
    assert(res.maintainsContainmentIntegrity == true);
    std::cout << "  -> Domain 10 Passed: Tendon loss = " << res.prestressLossPercent << "%, burst capacity = " << res.ultimateBurstPressureMpa << " MPa." << std::endl;
}

int main() {
    std::cout << "============================================================" << std::endl;
    std::cout << " Starting AndroCAD Faz 15 Nuclear CAD Suite (M2271-M2470)   " << std::endl;
    std::cout << "============================================================" << std::endl;
    
    TestDomain1_RPVStress();
    TestDomain2_ThermalCreep();
    TestDomain3_RadiationEmbrittlement();
    TestDomain4_ShieldingAttenuation();
    TestDomain5_CrdmScram();
    TestDomain6_DnbrMargin();
    TestDomain7_SeismicSseResponse();
    TestDomain8_ShroudFiv();
    TestDomain9_ZircaloyOxidation();
    TestDomain10_ContainmentTendonCapacity();

    std::cout << "============================================================" << std::endl;
    std::cout << " All 10/10 Faz 15 Nuclear Domains Passed Successfully!       " << std::endl;
    std::cout << "============================================================" << std::endl;
    return 0;
}
