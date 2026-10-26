#include "../include/Phase30Engine.h"
#include <cassert>
#include <iostream>

using namespace androcad::phase30;

int main() {
    std::cout << "Starting Faz 30 Test Suite (Grand Unified CAD/CAE/CAM/PLM Core & 5470 Milestone Grand Certification)...\n";

    // 1. Deterministic Picking
    auto res1 = evaluateDeterministicPicking(0.0, 0.0, 10.0, 0.0, 0.0, -1.0, -1.0, -1.0, 0.0, 1.0, -1.0, 0.0, 0.0, 1.0, 0.0);
    std::cout << "[Domain 1] " << res1.message << "\n";
    assert(res1.passed);

    // 2. Timeline DAG
    auto res2 = evaluateTimelineDag(5470, 2.5);
    std::cout << "[Domain 2] " << res2.message << "\n";
    assert(res2.passed);

    // 3. CAE Solver
    auto res3 = evaluateFiniteElementSolver(120000, 500.0, 210.0, 350.0);
    std::cout << "[Domain 3] " << res3.message << "\n";
    assert(res3.passed);

    // 4. OCCT Modernization
    auto res4 = evaluateOcctModernization("arm64-v8a", 16384);
    std::cout << "[Domain 4] " << res4.message << "\n";
    assert(res4.passed);

    // 5. CAM Toolpath
    auto res5 = evaluateCamToolpath(6.0, 50.0, 0.25, 0.002);
    std::cout << "[Domain 5] " << res5.message << "\n";
    assert(res5.passed);

    // 6. Tolerance Stack
    std::vector<double> nominals = {25.0, 40.0, 35.0};
    std::vector<double> tols = {0.02, 0.03, 0.025};
    auto res6 = evaluateToleranceStack(nominals, tols, 0.20);
    std::cout << "[Domain 6] " << res6.message << "\n";
    assert(res6.passed);

    // 7. SAF Compliance
    std::vector<std::string> perms = {"android.permission.INTERNET"};
    auto res7 = verifySafCompliance(perms);
    std::cout << "[Domain 7] " << res7.message << "\n";
    assert(res7.passed);

    // 8. Viewport Render
    auto res8 = evaluateViewportRendering(500000, 450000);
    std::cout << "[Domain 8] " << res8.message << "\n";
    assert(res8.passed);

    // 9. Digital Twin
    auto res9 = evaluateDigitalTwinSync(0.02, 10.0, 1000);
    std::cout << "[Domain 9] " << res9.message << "\n";
    assert(res9.passed);

    // 10. Grand Certification
    auto res10 = evaluateGrandCertification(5470, 30, 10940);
    std::cout << "[Domain 10] " << res10.message << "\n";
    assert(res10.passed);

    std::cout << "All 10 Faz 30 Domains PASSED successfully! 5470 Milestone Grand Architecture Certified!\n";
    return 0;
}
