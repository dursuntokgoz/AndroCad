// Copyright (c) 2026 dursuntokgoz. All rights reserved.
#include "SimulationEngine.h"
#include "MaterialEngine.h"

#include <BRepPrimAPI_MakeBox.hxx>
#include <cassert>
#include <iostream>
#include <cmath>

using namespace androcad;

void TestBeamDiscretization() {
    std::cout << "[Test 1] Testing beam mesh tetrahedral discretization..." << std::endl;

    SimulationEngine sim;
    sim.GenerateBeamMesh(100.0, 10.0, 10.0, 20, 4, 4);

    const auto& nodes = sim.GetNodes();
    const auto& elements = sim.GetElements();

    assert(!nodes.empty());
    assert(!elements.empty());
    assert(nodes.size() == (21 * 5 * 5));
    assert(elements.size() == (20 * 4 * 4 * 5));

    double totalVolume = 0.0;
    for (const auto& el : elements) {
        const auto& n0 = nodes[el.nodes[0]];
        const auto& n1 = nodes[el.nodes[1]];
        const auto& n2 = nodes[el.nodes[2]];
        const auto& n3 = nodes[el.nodes[3]];

        double v1x = n1.x - n0.x, v1y = n1.y - n0.y, v1z = n1.z - n0.z;
        double v2x = n2.x - n0.x, v2y = n2.y - n0.y, v2z = n2.z - n0.z;
        double v3x = n3.x - n0.x, v3y = n3.y - n0.y, v3z = n3.z - n0.z;

        double det = v1x * (v2y * v3z - v2z * v3y) -
                     v1y * (v2x * v3z - v2z * v3x) +
                     v1z * (v2x * v3y - v2y * v3x);
        double vol = std::abs(det) / 6.0;
        assert(vol > 1e-12);
        totalVolume += vol;
    }

    // Expected volume: 100mm * 10mm * 10mm = 10000 mm³ = 1e-5 m³
    double expectedVol = 100.0 * 10.0 * 10.0 * 1e-9;
    assert(std::abs(totalVolume - expectedVol) / expectedVol < 1e-4);

    std::cout << "  Passed: nodes=" << nodes.size() << ", elements=" << elements.size()
              << ", total volume=" << totalVolume * 1e9 << " mm^3" << std::endl;
}

void TestCantileverBeamDeflection() {
    std::cout << "[Test 2] Testing cantilever beam tip deflection vs Euler-Bernoulli theory..." << std::endl;

    Material steel;
    steel.youngModulusGPa = 200.0;
    steel.poissonRatio = 0.30;
    steel.yieldStrengthMPa = 250.0;

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

    SimulationResult result = SimulationEngine::SolveCantileverBeam(L, b, h, F, steel, 1.0, 25, 5, 5);

    assert(result.success);
    assert(result.maxDisplacementMm > 0.0);

    // Constant Strain Tetrahedra are slightly stiffer due to shear locking, expect within ~20%
    double error = std::abs(result.maxDisplacementMm - expectedDispMm) / expectedDispMm;
    std::cout << "  Simulated Max Displacement: " << result.maxDisplacementMm << " mm" << std::endl;
    std::cout << "  Theoretical EB Displacement: " << expectedDispMm << " mm (rel diff: " << (error * 100.0) << "%)" << std::endl;
    assert(error < 0.25);

    std::cout << "  Passed!" << std::endl;
}

void TestStressDistributionAndSafetyFactor() {
    std::cout << "[Test 3] Testing stress distribution and safety factor..." << std::endl;

    Material steel;
    steel.youngModulusGPa = 200.0;
    steel.poissonRatio = 0.30;
    steel.yieldStrengthMPa = 250.0;

    SimulationResult result = SimulationEngine::SolveCantileverBeam(100.0, 10.0, 10.0, 100.0, steel, 1.0, 20, 4, 4);

    assert(result.success);
    assert(result.maxVonMisesStressMPa > 0.0);
    assert(result.minVonMisesStressMPa >= 0.0);
    assert(result.maxVonMisesStressMPa > result.minVonMisesStressMPa);

    // Bending moment at root M = F * L = 100 N * 0.1 m = 10 N*m
    // sigma_max = M * c / I = 10 * 0.005 / (8.3333e-10) = 60.0 MPa
    double expectedSigmaMPa = 60.0;
    std::cout << "  Simulated Max Stress: " << result.maxVonMisesStressMPa << " MPa" << std::endl;
    std::cout << "  Theoretical Max Bending Stress: " << expectedSigmaMPa << " MPa" << std::endl;
    assert(result.maxVonMisesStressMPa > 30.0 && result.maxVonMisesStressMPa < 90.0);

    // Safety factor = 250 / maxStress
    assert(result.minSafetyFactor > 0.0);
    double expectedSF = 250.0 / result.maxVonMisesStressMPa;
    assert(std::abs(result.minSafetyFactor - expectedSF) < 0.01);
    std::cout << "  Safety Factor: " << result.minSafetyFactor << " (yield=" << steel.yieldStrengthMPa << " MPa)" << std::endl;

    std::cout << "  Passed!" << std::endl;
}

void TestMaterialStiffnessVariation() {
    std::cout << "[Test 4] Testing material variation (Steel vs Aluminum)..." << std::endl;

    Material steel;
    steel.youngModulusGPa = 200.0;
    steel.poissonRatio = 0.30;
    steel.yieldStrengthMPa = 250.0;

    Material aluminum;
    aluminum.youngModulusGPa = 68.9;
    aluminum.poissonRatio = 0.33;
    aluminum.yieldStrengthMPa = 276.0;

    SimulationResult resSteel = SimulationEngine::SolveCantileverBeam(80.0, 8.0, 8.0, 50.0, steel, 1.0, 16, 4, 4);
    SimulationResult resAl = SimulationEngine::SolveCantileverBeam(80.0, 8.0, 8.0, 50.0, aluminum, 1.0, 16, 4, 4);

    assert(resSteel.success);
    assert(resAl.success);

    // Deflection is inversely proportional to Young's modulus
    double ratioDisp = resAl.maxDisplacementMm / resSteel.maxDisplacementMm;
    double ratioModulus = steel.youngModulusGPa / aluminum.youngModulusGPa; // ~ 2.90

    std::cout << "  Steel Max Disp: " << resSteel.maxDisplacementMm << " mm" << std::endl;
    std::cout << "  Al Max Disp:    " << resAl.maxDisplacementMm << " mm" << std::endl;
    std::cout << "  Ratio of Displacements: " << ratioDisp << " vs Modulus Ratio: " << ratioModulus << std::endl;

    assert(std::abs(ratioDisp - ratioModulus) / ratioModulus < 0.08);

    std::cout << "  Passed!" << std::endl;
}

void TestBRepSolidDiscretizationAndVisualMesh() {
    std::cout << "[Test 5] Testing OCCT B-Rep solid discretization and visual surface mesh..." << std::endl;

    TopoDS_Shape box = BRepPrimAPI_MakeBox(40.0, 20.0, 20.0).Shape();
    assert(!box.IsNull());

    SimulationEngine sim;
    sim.SetMaterialProperties(200.0, 0.30, 250.0);
    sim.DiscretizeShape(box, 10.0);

    assert(!sim.GetNodes().empty());
    assert(!sim.GetElements().empty());

    // Fixed constraint on X=0 face
    BoundaryConstraint bc;
    bc.type = ConstraintType::FIXED;
    bc.minX = -1.0;
    bc.maxX = 1.0;
    sim.AddConstraint(bc);

    // Tensile load on X=40 face
    StructuralLoad load;
    load.type = LoadType::TOTAL_FORCE;
    load.forceX = 500.0; // 500 N tension
    load.minX = 39.0;
    load.maxX = 41.0;
    sim.AddLoad(load);

    SimulationResult res = sim.Solve(5.0);
    assert(res.success);
    assert(res.maxDisplacementMm > 0.0);

    const auto& vmesh = res.visualMesh;
    assert(!vmesh.positions.empty());
    assert(!vmesh.colors.empty());
    assert(!vmesh.indices.empty());
    assert(vmesh.positions.size() == vmesh.colors.size());
    assert(vmesh.indices.size() % 3 == 0);

    // Verify RGB colors in [0, 1]
    for (size_t i = 0; i < vmesh.colors.size(); ++i) {
        assert(vmesh.colors[i] >= 0.0f && vmesh.colors[i] <= 1.0f);
    }

    std::cout << "  Visual Mesh: vertices=" << vmesh.positions.size() / 3
              << ", triangles=" << vmesh.indices.size() / 3
              << ", Max Stress=" << res.maxVonMisesStressMPa << " MPa" << std::endl;

    std::cout << "  Passed!" << std::endl;
}

int main() {
    std::cout << "=== AndroCAD Simulation Engine (FEA) Test Suite ===" << std::endl;

    try {
        TestBeamDiscretization();
        TestCantileverBeamDeflection();
        TestStressDistributionAndSafetyFactor();
        TestMaterialStiffnessVariation();
        TestBRepSolidDiscretizationAndVisualMesh();

        std::cout << "\nAll 5 Simulation Engine tests PASSED successfully!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Simulation Test FAILED with exception: " << e.what() << std::endl;
        return 1;
    }
}
