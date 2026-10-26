#include "../include/Phase29Engine.h"
#include <cassert>
#include <iostream>

using namespace androcad::phase29;

int main() {
    std::cout << "Starting Faz 29 Test Suite (Quantum Computing Dilution Cryostat)...\n";

    // 1. Thermal Budget
    auto res1 = calculateCryostatThermalBudget(24, 48, 0.38, 1.15);
    std::cout << "[Domain 1] " << res1.message << "\n";
    assert(res1.passed);

    // 2. Coax Cabling
    auto res2 = evaluateCoaxialCabling(2.19, 0.51, 0.45, 5.0);
    std::cout << "[Domain 2] " << res2.message << "\n";
    assert(res2.passed);

    // 3. Dilution Cooling
    auto res3 = calculateDilutionCoolingPower(0.016, 12.0, 0.010);
    std::cout << "[Domain 3] " << res3.message << "\n";
    assert(res3.passed);

    // 4. Cavity Resonator
    auto res4 = evaluateCavityResonator(5.0, 0.010, 4.5, 3.0e7);
    std::cout << "[Domain 4] " << res4.message << "\n";
    assert(res4.passed);

    // 5. Magnetic Shielding
    auto res5 = calculateMagneticShielding(0.55, 2, 2, 250.0);
    std::cout << "[Domain 5] " << res5.message << "\n";
    assert(res5.passed);

    // 6. Cryo Truss
    auto res6 = evaluateCryogenicTruss(28.0, 2.0, 280.0, 45.0);
    std::cout << "[Domain 6] " << res6.message << "\n";
    assert(res6.passed);

    // 7. Purcell Filter
    auto res7 = evaluatePurcellFilter(4.2, 6.5, 75.0, 40.0);
    std::cout << "[Domain 7] " << res7.message << "\n";
    assert(res7.passed);

    // 8. HEMT Amplifier
    auto res8 = evaluateCryoHemt(4.0, 6.0, 0.8, 5.0);
    std::cout << "[Domain 8] " << res8.message << "\n";
    assert(res8.passed);

    // 9. TWPA Amplifier
    auto res9 = evaluateTwpaParametric(-52.0, 7.8, 2000, 35.0);
    std::cout << "[Domain 9] " << res9.message << "\n";
    assert(res9.passed);

    // 10. Radiation Shield
    auto res10 = calculateCryoRadiationShield(300.0, 50.0, 3.8, 30);
    std::cout << "[Domain 10] " << res10.message << "\n";
    assert(res10.passed);

    std::cout << "All 10 Faz 29 Domains PASSED successfully!\n";
    return 0;
}
