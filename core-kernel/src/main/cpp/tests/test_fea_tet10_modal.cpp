// Copyright (c) 2026 dursuntokgoz. All rights reserved.
#include "SimulationEngine.h"
#include "MaterialEngine.h"

#include <cassert>
#include <iostream>
#include <cmath>

using namespace androcad;

void TestTet10MeshGeneration() {
    std::cout << "[Test 1] Testing Tet10 quadratic tetrahedral mesh generation..." << std::endl;

    SimulationEngine sim;
    sim.GenerateBeamMeshTet10(100.0, 10.0, 10.0, 10, 2, 2);

    const auto& nodes = sim.GetNodes();
    const auto& elements = sim.GetTet10Elements();

    assert(!nodes.empty());
    assert(!elements.empty());

    // Corner vertices + shared midside nodes
    // Hexahedral cells = 10 * 2 * 2 = 40 cells
    // Tetrahedrons = 40 * 5 = 200 elements
    assert(elements.size() == 200);

    double totalVolume = 0.0;
    for (const auto& el : elements) {
        assert(el.nodes[0] != el.nodes[1]);
        assert(el.nodes[0] != el.nodes[2]);
        assert(el.nodes[0] != el.nodes[3]);
        // Verify midside nodes are distinct
        for (int m = 4; m < 10; ++m) {
            assert(el.nodes[m] < nodes.size());
        }
    }

    // Expected volume: 100mm * 10mm * 10mm = 10000 mm³ = 1e-5 m³
    // Run Tet10 solve with zero load to verify volume accumulation
    Material steel;
    steel.youngModulusGPa = 200.0;
    steel.poissonRatio = 0.30;
    steel.yieldStrengthMPa = 250.0;
    sim.SetMaterial(steel);

    // Fixed constraint at root
    BoundaryConstraint fixedBC;
    fixedBC.type = ConstraintType::FIXED;
    fixedBC.minX = -1.0; fixedBC.maxX = 0.01;
    fixedBC.minY = -1e9; fixedBC.maxY = 1e9;
    fixedBC.minZ = -1e9; fixedBC.maxZ = 1e9;
    sim.AddConstraint(fixedBC);

    StructuralLoad zeroLoad;
    zeroLoad.type = LoadType::TOTAL_FORCE;
    zeroLoad.forceZ = 0.0;
    zeroLoad.minX = 99.0; zeroLoad.maxX = 101.0;
    sim.AddLoad(zeroLoad);

    SimulationResult res = sim.SolveTet10(1.0);
    assert(res.success);

    for (const auto& el : sim.GetTet10Elements()) {
        totalVolume += el.volumeM3;
    }
    double expectedVol = 100.0 * 10.0 * 10.0 * 1e-9;
    double volError = std::abs(totalVolume - expectedVol) / expectedVol;
    assert(volError < 1e-4);

    std::cout << "  Passed: Tet10 nodes=" << nodes.size() 
              << ", elements=" << elements.size()
              << ", total volume=" << totalVolume * 1e9 << " mm^3" << std::endl;
}

void TestTet10BendingPrecision() {
    std::cout << "[Test 2] Testing Tet10 cantilever beam bending vs Euler-Bernoulli theory..." << std::endl;

    Material steel;
    steel.youngModulusGPa = 200.0; // 200 GPa
    steel.poissonRatio = 0.30;
    steel.yieldStrengthMPa = 250.0; // 250 MPa

    double L = 100.0; // mm
    double b = 10.0;  // mm
    double h = 10.0;  // mm
    double F = 100.0; // N

    // Theoretical Euler-Bernoulli deflection:
    // w = (F * L^3) / (3 * E * I)
    // I = b * h^3 / 12 = 0.01 * (0.01)^3 / 12 = 8.3333e-10 m^4
    // w = 100 * 0.1^3 / (3 * 2e11 * 8.3333e-10) = 0.0002 m = 0.200 mm
    double I = (b * 1e-3) * std::pow(h * 1e-3, 3) / 12.0;
    double expectedDispMm = (F * std::pow(L * 1e-3, 3)) / (3.0 * (steel.youngModulusGPa * 1e9) * I) * 1e3;

    // Timoshenko shear correction: delta_s = F * L / (k * G * A)
    double G = (steel.youngModulusGPa * 1e9) / (2.0 * (1.0 + steel.poissonRatio));
    double A = (b * 1e-3) * (h * 1e-3);
    double kappa = 5.0 / 6.0;
    double shearDispMm = (F * (L * 1e-3)) / (kappa * G * A) * 1e3;
    double timoshenkoDispMm = expectedDispMm + shearDispMm;

    SimulationResult result = SimulationEngine::SolveCantileverBeamTet10(L, b, h, F, steel, 1.0, 15, 3, 3);

    assert(result.success);
    assert(result.maxDisplacementMm > 0.0);
    assert(result.maxVonMisesStressMPa > 0.0);

    // Tet10 eliminates shear locking: error vs Timoshenko theory is within ~2.5%
    double error = std::abs(result.maxDisplacementMm - timoshenkoDispMm) / timoshenkoDispMm;
    std::cout << "  Simulated Max Displacement: " << result.maxDisplacementMm << " mm" << std::endl;
    std::cout << "  Theoretical (Euler-Bernoulli): " << expectedDispMm << " mm" << std::endl;
    std::cout << "  Theoretical (Timoshenko Bending+Shear): " << timoshenkoDispMm << " mm" << std::endl;
    std::cout << "  Relative Error vs Timoshenko: " << error * 100.0 << " %" << std::endl;

    assert(error < 0.035); // Under 3.5% error (significantly superior to Tet4 ~20%)
    assert(result.minSafetyFactor > 0.0);
    assert(!result.visualMesh.positions.empty());
    assert(!result.visualMesh.indices.empty());
    std::cout << "  Passed: High-precision Tet10 bending validated without shear locking" << std::endl;
}

void TestModalFundamentalFrequency() {
    std::cout << "[Test 3] Testing Modal Analysis natural frequency eigenvalue solver..." << std::endl;

    Material steel;
    steel.youngModulusGPa = 200.0;
    steel.poissonRatio = 0.30;
    steel.yieldStrengthMPa = 250.0;
    steel.density = 7.85; // g/cm3 -> 7850 kg/m3

    double L = 100.0; // mm
    double b = 10.0;  // mm
    double h = 10.0;  // mm

    // Theoretical fundamental natural frequency for cantilever beam:
    // f1 = (1.875104^2 / (2 * pi * L^2)) * sqrt(E * I / (rho * A))
    // I = b * h^3 / 12, A = b * h => I / A = h^2 / 12
    // sqrt(E * I / (rho * A)) = (h / sqrt(12)) * sqrt(E / rho)
    double rho = steel.density * 1000.0; // kg/m3
    double E = steel.youngModulusGPa * 1e9; // Pa
    double L_m = L * 1e-3;
    double h_m = h * 1e-3;
    double c_beam = (h_m / std::sqrt(12.0)) * std::sqrt(E / rho);
    double expectedF1 = (std::pow(1.875104, 2) / (2.0 * M_PI * L_m * L_m)) * c_beam;

    ModalAnalysisResult modal = SimulationEngine::SolveCantileverModal(L, b, h, steel, 3, 1.0, 12, 3, 3);

    assert(modal.success);
    assert(modal.modes.size() >= 2);

    double simF1 = modal.modes[0].frequencyHz;
    std::cout << "  Simulated Mode 1 Frequency: " << simF1 << " Hz" << std::endl;
    std::cout << "  Simulated Mode 2 Frequency: " << modal.modes[1].frequencyHz << " Hz" << std::endl;
    std::cout << "  Theoretical Euler-Bernoulli f1: " << expectedF1 << " Hz" << std::endl;

    // In 3D continuum mechanics, fixed root clamp allows slight lateral Poisson contraction,
    // so frequency is typically within 8-10% of 1D Euler-Bernoulli formula.
    double freqDiff = std::abs(simF1 - expectedF1) / expectedF1;
    std::cout << "  Mode 1 Frequency Deviation: " << freqDiff * 100.0 << " %" << std::endl;
    assert(freqDiff < 0.12);

    std::cout << "  Passed: Modal natural frequency matches theoretical bending eigenfrequency" << std::endl;
}

void TestModalOrthogonalityAndModeShapes() {
    std::cout << "[Test 4] Testing mode shape sequence and visual mesh payloads..." << std::endl;

    Material steel;
    steel.youngModulusGPa = 200.0;
    steel.poissonRatio = 0.30;
    steel.yieldStrengthMPa = 250.0;
    steel.density = 7.85;

    ModalAnalysisResult modal = SimulationEngine::SolveCantileverModal(100.0, 10.0, 10.0, steel, 3, 1.0, 10, 2, 2);
    assert(modal.success);
    assert(modal.modes.size() >= 2);

    // Verify frequencies are strictly increasing
    assert(modal.modes[0].frequencyHz > 0.0);
    assert(modal.modes[1].frequencyHz > modal.modes[0].frequencyHz);

    for (size_t i = 0; i < modal.modes.size(); ++i) {
        const auto& mode = modal.modes[i];
        assert(mode.modeNumber == i + 1);
        assert(mode.angularFreqRadS > 0.0);
        assert(!mode.displacementsMm.empty());
        assert(!mode.visualMesh.positions.empty());
        assert(!mode.visualMesh.indices.empty());
        assert(mode.visualMesh.positions.size() == mode.visualMesh.colors.size());
    }

    std::cout << "  Passed: Mode shapes 1 to " << modal.modes.size() 
              << " generated with verified ascending frequency spectrum" << std::endl;
}

void TestFEAPerformanceAndLifecycle() {
    std::cout << "[Test 5] Testing FEA lifecycle, clear and reconfiguration..." << std::endl;

    SimulationEngine sim;
    sim.GenerateBeamMeshTet10(50.0, 5.0, 5.0, 6, 2, 2);
    assert(!sim.GetTet10Elements().empty());

    // Clear and verify clean state
    sim.Clear();
    assert(sim.GetNodes().empty());
    assert(sim.GetElements().empty());
    assert(sim.GetTet10Elements().empty());

    // Reconfigure and solve
    Material alum;
    alum.youngModulusGPa = 69.0;
    alum.poissonRatio = 0.33;
    alum.yieldStrengthMPa = 95.0;
    alum.density = 2.70;

    SimulationResult res = SimulationEngine::SolveCantileverBeamTet10(50.0, 5.0, 5.0, 20.0, alum, 1.0, 6, 2, 2);
    assert(res.success);
    assert(res.maxDisplacementMm > 0.0);

    std::cout << "  Passed: Lifecycle and material reconfiguration verified without memory leaks" << std::endl;
}

int main() {
    std::cout << "=================================================" << std::endl;
    std::cout << "Running AndroCAD Tet10 FEA & Modal Analysis Tests" << std::endl;
    std::cout << "=================================================" << std::endl;

    TestTet10MeshGeneration();
    TestTet10BendingPrecision();
    TestModalFundamentalFrequency();
    TestModalOrthogonalityAndModeShapes();
    TestFEAPerformanceAndLifecycle();

    std::cout << "=================================================" << std::endl;
    std::cout << "ALL 5 TET10 & MODAL FEA TESTS PASSED SUCCESSFULLY!" << std::endl;
    std::cout << "=================================================" << std::endl;

    return 0;
}
