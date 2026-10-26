#include <iostream>
#include <cassert>
#include <cmath>
#include "Phase21Engine.h"

using namespace AndroCAD::Phase21;

void TestDomain1_YoungLaplace() {
    std::cout << "[Test 1/10] Domain 1: Young-Laplace Capillary Pressure..." << std::endl;
    // Water in PDMS/glass channel: gamma = 0.0728 N/m, theta = 35 deg, w = 100 um, h = 50 um, rho = 1000 kg/m3
    auto res = Phase21Engine::evaluateYoungLaplace(0.0728, 35.0, 100.0, 50.0, 1000.0);
    assert(res.youngLaplacePressurePa > 500.0);
    assert(res.capillaryRiseHeightMm > 100.0);
    assert(res.spontaneousCapillaryFilling == true);
    std::cout << "  -> Domain 1 Passed: Capillary pressure = " << res.youngLaplacePressurePa << " Pa, Rise height = " << res.capillaryRiseHeightMm << " mm." << std::endl;
}

void TestDomain2_DropletGeneration() {
    std::cout << "[Test 2/10] Domain 2: Droplet Generation in Flow-Focusing Geometry..." << std::endl;
    // Continuous = 120 uL/h, dispersed = 20 uL/h, mu = 0.05 Pa*s (mineral oil), gamma = 0.005 N/m, w = 50 um
    auto res = Phase21Engine::evaluateDropletGeneration(120.0, 20.0, 0.05, 0.005, 50.0);
    assert(res.capillaryNumberCa >= 0.005 && res.capillaryNumberCa <= 0.25);
    assert(res.dropletVolumePicoliters > 5.0 && res.dropletVolumePicoliters < 250.0);
    assert(res.monodisperseDropletRegime == true);
    std::cout << "  -> Domain 2 Passed: Ca = " << res.capillaryNumberCa << ", Droplet volume = " << res.dropletVolumePicoliters << " pL, Frequency = " << res.generationFrequencyHz << " Hz." << std::endl;
}

void TestDomain3_DeanFocusing() {
    std::cout << "[Test 3/10] Domain 3: Dean Vortex Inertial Particle Focusing..." << std::endl;
    // Q = 1.5 mL/min, w = 200 um, h = 50 um, R = 15 mm, d_p = 15 um (mammalian cells), mu = 0.001 Pa*s, rho = 1000 kg/m3
    auto res = Phase21Engine::evaluateDeanFocusing(1.5, 200.0, 50.0, 15.0, 15.0, 0.001, 1000.0);
    assert(res.reynoldsNumberRe > 50.0);
    assert(res.deanNumberDe >= 1.0 && res.deanNumberDe <= 60.0);
    assert(res.singleStreamCellFocusingAchieved == true);
    std::cout << "  -> Domain 3 Passed: Dean number = " << res.deanNumberDe << ", Lift force = " << res.inertialLiftForcePicoNewtons << " pN." << std::endl;
}

void TestDomain4_ElectroOsmoticFlow() {
    std::cout << "[Test 4/10] Domain 4: Electro-Osmotic Flow (EOF) Dynamics..." << std::endl;
    // E = 15,000 V/m, zeta = -65 mV, eps_r = 78.5, mu = 0.001 Pa*s, I = 0.01 Molar
    auto res = Phase21Engine::evaluateElectroOsmoticFlow(15000.0, -65.0, 78.5, 0.001, 0.01);
    assert(res.eofVelocityMmPerS > 0.05);
    assert(res.debyeScreeningLengthNanometers < 50.0);
    assert(res.plugFlowProfileMaintained == true);
    std::cout << "  -> Domain 4 Passed: EOF velocity = " << res.eofVelocityMmPerS << " mm/s, Debye length = " << res.debyeScreeningLengthNanometers << " nm." << std::endl;
}

void TestDomain5_MicroDiffusion() {
    std::cout << "[Test 5/10] Domain 5: Diffusion Flux & Peclet Number Mixing..." << std::endl;
    // U = 2.0 mm/s, w = 100 um, D = 1.0e-9 m2/s (small molecule fluorescein)
    auto res = Phase21Engine::evaluateMicroDiffusion(2.0, 100.0, 1.0e-9);
    assert(res.pecletNumberPe > 50.0);
    assert(res.completeMolecularMixing == true);
    std::cout << "  -> Domain 5 Passed: Peclet number = " << res.pecletNumberPe << ", Uniformity = " << res.concentrationUniformityPercent << "%." << std::endl;
}

void TestDomain6_QuakeValve() {
    std::cout << "[Test 6/10] Domain 6: Pneumatic Quake PDMS Micro-Valve..." << std::endl;
    // Actuation = 150 kPa, membrane t = 25 um, width = 200 um, E = 2.0 MPa
    auto res = Phase21Engine::evaluateQuakeValve(150.0, 25.0, 200.0, 2.0);
    assert(res.membraneDeflectionMicrons >= 25.0);
    assert(res.hermeticMicrochannelSealing == true);
    std::cout << "  -> Domain 6 Passed: Deflection = " << res.membraneDeflectionMicrons << " um, Closing pressure = " << res.valveClosingPressureKpa << " kPa." << std::endl;
}

void TestDomain7_EwodActuation() {
    std::cout << "[Test 7/10] Domain 7: Electrowetting-on-Dielectric (EWOD) Actuation..." << std::endl;
    // V = 75 V, theta0 = 115 deg, t_d = 2.0 um, k = 3.2, gamma = 0.0728 N/m
    auto res = Phase21Engine::evaluateEwodActuation(75.0, 115.0, 2.0, 3.2, 0.0728);
    assert(res.dynamicContactAngleDegrees < 115.0);
    assert(res.electrowettingDrivingForceMicroNewtons >= 5.0);
    assert(res.reliableDigitalDropletTransport == true);
    std::cout << "  -> Domain 7 Passed: Dynamic contact angle = " << res.dynamicContactAngleDegrees << " deg, Driving force = " << res.electrowettingDrivingForceMicroNewtons << " uN." << std::endl;
}

void TestDomain8_CfPcrThermal() {
    std::cout << "[Test 8/10] Domain 8: Continuous-Flow PCR Thermal Cycling..." << std::endl;
    // Flow = 1.2 uL/min, denat = 15 mm, anneal = 25 mm, ext = 40 mm, cycles = 35
    auto res = Phase21Engine::evaluateCfPcrThermal(1.2, 15.0, 25.0, 40.0, 35.0);
    assert(res.denaturationTimeSeconds >= 3.0);
    assert(res.annealingTimeSeconds >= 5.0);
    assert(res.dnaAmplificationSuccessful == true);
    std::cout << "  -> Domain 8 Passed: Denaturation = " << res.denaturationTimeSeconds << " s, Annealing = " << res.annealingTimeSeconds << " s (35 cycles)." << std::endl;
}

void TestDomain9_SawSorting() {
    std::cout << "[Test 9/10] Domain 9: Acoustic Radiation Force (SAW) Particle Sorting..." << std::endl;
    // f = 19.5 MHz, power = 2.0 W, r_p = 5.0 um, rho_p = 1050 kg/m3, rho_f = 1000 kg/m3, c = 1495 m/s
    auto res = Phase21Engine::evaluateSawSorting(19.5, 2.0, 5.0, 1050.0, 1000.0, 1495.0);
    assert(res.acousticRadiationForcePicoNewtons >= 10.0);
    assert(res.sortingPurityPercent >= 95.0);
    assert(res.highThroughputCellSeparation == true);
    std::cout << "  -> Domain 9 Passed: Radiation force = " << res.acousticRadiationForcePicoNewtons << " pN, Sorting purity = " << res.sortingPurityPercent << "%." << std::endl;
}

void TestDomain10_OrganChipShear() {
    std::cout << "[Test 10/10] Domain 10: Organ-on-a-Chip Endothelial Shear Stress..." << std::endl;
    // Q = 60 uL/min, w = 1000 um, h = 120 um, mu = 0.0035 Pa*s (blood analog), stretch = 10%
    auto res = Phase21Engine::evaluateOrganChipShear(60.0, 1000.0, 120.0, 0.0035, 10.0);
    assert(res.wallShearStressDyneCm2 >= 10.0 && res.wallShearStressDyneCm2 <= 25.0);
    assert(res.physiologicalEndothelialPhenotype == true);
    std::cout << "  -> Domain 10 Passed: Wall shear stress = " << res.wallShearStressDyneCm2 << " dyne/cm2 (physiological), Viability = " << res.cellViabilityPercent << "%." << std::endl;
}

int main() {
    std::cout << "============================================================" << std::endl;
    std::cout << " Starting AndroCAD Faz 21 Microfluidics Suite (M3471-M3670) " << std::endl;
    std::cout << "============================================================" << std::endl;

    TestDomain1_YoungLaplace();
    TestDomain2_DropletGeneration();
    TestDomain3_DeanFocusing();
    TestDomain4_ElectroOsmoticFlow();
    TestDomain5_MicroDiffusion();
    TestDomain6_QuakeValve();
    TestDomain7_EwodActuation();
    TestDomain8_CfPcrThermal();
    TestDomain9_SawSorting();
    TestDomain10_OrganChipShear();

    std::cout << "============================================================" << std::endl;
    std::cout << " All 10/10 Faz 21 Microfluidics Domains Passed Successfully!" << std::endl;
    std::cout << "============================================================" << std::endl;
    return 0;
}
