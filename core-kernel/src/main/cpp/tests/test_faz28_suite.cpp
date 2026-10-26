#include "../include/Phase28Engine.h"
#include <cassert>
#include <iostream>

using namespace androcad::phase28;

int main() {
    std::cout << "Starting Faz 28 Test Suite (Spacecraft Deployable Solar Arrays & Hinges)...\n";

    // 1. Tape Spring Hinge
    auto res1 = analyzeTapeSpringHinge(0.15, 25.0, 60.0, 180.0, 135.0);
    std::cout << "[Domain 1] " << res1.message << "\n";
    assert(res1.passed);

    // 2. Wing Modal
    auto res2 = calculateWingModalFrequencies(12.0, 85.0, 45000.0, 2.8e5, 1.5);
    std::cout << "[Domain 2] " << res2.message << "\n";
    assert(res2.passed);

    // 3. Micro-Vibration Isolator
    auto res3 = evaluateMicroVibrationIsolator(120.0, 350.0, 0.35, 6);
    std::cout << "[Domain 3] " << res3.message << "\n";
    assert(res3.passed);

    // 4. Membrane Wrinkling
    auto res4 = checkMembraneWrinkling(45.0, 2.2, 3.5, 50.0, 0.0012);
    std::cout << "[Domain 4] " << res4.message << "\n";
    assert(res4.passed);

    // 5. HDRM Release
    auto res5 = evaluateHdrmRelease(8.0, 32.0, 45.0, 40.0, 25.0);
    std::cout << "[Domain 5] " << res5.message << "\n";
    assert(res5.passed);

    // 6. Thermal Snap Flutter
    auto res6 = calculateThermalSnapFlutter(1361.0, 0.88, 0.82, 60.0, 1.2, 8.0);
    std::cout << "[Domain 6] " << res6.message << "\n";
    assert(res6.passed);

    // 7. Pantograph Deployment
    auto res7 = evaluatePantographDeployment(14.0, 6, 90.0, 18.0, 5.4);
    std::cout << "[Domain 7] " << res7.message << "\n";
    assert(res7.passed);

    // 8. Solar Cell Fatigue
    auto res8 = analyzeCellLaydownFatigue(300.0, 1.5, 5.9, 40.0, 2000);
    std::cout << "[Domain 8] " << res8.message << "\n";
    assert(res8.passed);

    // 9. SADA Mechanism
    auto res9 = evaluateSadaMechanism(160.0, 1.8, 35.0, 0.000694);
    std::cout << "[Domain 9] " << res9.message << "\n";
    assert(res9.passed);

    // 10. Attitude Feedforward
    auto res10 = evaluateAttitudeFeedforward(1200.0, 180.0, 0.008, 0.988);
    std::cout << "[Domain 10] " << res10.message << "\n";
    assert(res10.passed);

    std::cout << "All 10 Faz 28 Domains PASSED successfully!\n";
    return 0;
}
