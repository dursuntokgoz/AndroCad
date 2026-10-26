#include <iostream>
#include <cassert>
#include <cmath>
#include "Phase27Engine.h"

using namespace AndroCAD::Phase27;

int main() {
    std::cout << "============================================================" << std::endl;
    std::cout << " Starting AndroCAD Faz 27 Mining Dragline Suite (M4671-M4870)" << std::endl;
    std::cout << "============================================================" << std::endl;

    // Test 1: Tubular Boom Buckling
    std::cout << "[Test 1/10] Domain 1: Tubular Space-Truss Boom Global Buckling..." << std::endl;
    auto boom = Phase27Engine::evaluateBoomBuckling(105.0, 610.0, 25.0, 14500.0);
    assert(boom.boomStructuralStabilityCertified);
    assert(boom.globalBucklingSafetyFactor >= 2.0);
    assert(boom.eulerGlobalBucklingKn > 29000.0);
    std::cout << "  -> Domain 1 Passed: Euler P_crit = " << boom.eulerGlobalBucklingKn << " kN, SF = " << boom.globalBucklingSafetyFactor << std::endl;

    // Test 2: Planetary Final Drive
    std::cout << "[Test 2/10] Domain 2: Multi-Stage Planetary Hoist Final Drive..." << std::endl;
    auto drive = Phase27Engine::evaluateFinalDrive(45.0, 21, 39, 99, 450.0, 22.0);
    assert(drive.heavyMiningDriveCertified);
    assert(drive.iso6336PittingSafetyFactorSh >= 1.30);
    assert(drive.outputTorqueKnm >= 1200.0);
    std::cout << "  -> Domain 2 Passed: Output torque = " << drive.outputTorqueKnm << " kNm, Sh = " << drive.iso6336PittingSafetyFactorSh << std::endl;

    // Test 3: Wire Rope Bending Fatigue
    std::cout << "[Test 3/10] Domain 3: Wire Rope Bending Fatigue over Sheaves..." << std::endl;
    auto rope = Phase27Engine::evaluateWireRope(85.0, 2800.0, 650.0, 4800.0);
    assert(rope.iso4309RetirementCriteriaSatisfied);
    assert(rope.sheaveDiameterRatioDToD >= 28.0);
    assert(rope.expectedBendingCyclesToDiscard >= 50000.0);
    std::cout << "  -> Domain 3 Passed: D/d = " << rope.sheaveDiameterRatioDToD << ", Bending cycles = " << rope.expectedBendingCyclesToDiscard << std::endl;

    // Test 4: Tubular Node Stress Concentration
    std::cout << "[Test 4/10] Domain 4: Tubular K-T-Y Joint Efthymiou Hot-Spot SCF..." << std::endl;
    auto joint = Phase27Engine::evaluateTubularJointScf(610.0, 25.0, 350.0, 16.0, 45.0, 85.0);
    assert(joint.tubularNodeFatigueCompliant);
    assert(joint.efthymiouHotSpotScf <= 2.85);
    assert(joint.weldFatigueLifeHours >= 25000.0);
    std::cout << "  -> Domain 4 Passed: Efthymiou SCF = " << joint.efthymiouHotSpotScf << ", Fatigue life = " << joint.weldFatigueLifeHours << " hrs." << std::endl;

    // Test 5: Bucket Digging Resistance
    std::cout << "[Test 5/10] Domain 5: Dragline Bucket Digging & Soil Resistance..." << std::endl;
    auto bucket = Phase27Engine::evaluateBucketDigging(65.0, 320.0, 4.2, 0.95);
    assert(bucket.payloadTargetAchieved);
    assert(bucket.maxDragPullForceKn >= 2400.0);
    assert(bucket.bucketVolumeCubicMeters >= 60.0);
    std::cout << "  -> Domain 5 Passed: Pull force = " << bucket.maxDragPullForceKn << " kN, Bucket vol = " << bucket.bucketVolumeCubicMeters << " m3." << std::endl;

    // Test 6: Walking Cam & Shoe Mobility
    std::cout << "[Test 6/10] Domain 6: Monighan Walking Cam & Soft Ground Mobility..." << std::endl;
    auto cam = Phase27Engine::evaluateWalkingCam(38000.0, 1200.0, 3.8, 18.0);
    assert(cam.softGroundMobilityVerified);
    assert(cam.maxGroundBearingPressureKpa <= 185.0);
    assert(cam.strideLengthMeters >= 2.0);
    std::cout << "  -> Domain 6 Passed: Stride = " << cam.strideLengthMeters << " m, Ground P = " << cam.maxGroundBearingPressureKpa << " kPa." << std::endl;

    // Test 7: Hoist Drum Brakes
    std::cout << "[Test 7/10] Domain 7: Hoist/Drag Double-Drum Friction Brakes..." << std::endl;
    auto brake = Phase27Engine::evaluateDrumBrake(1200.0, 4.5, 3200.0, 0.42);
    assert(brake.thermalFadeResistanceApproved);
    assert(brake.dynamicFrictionCoefficient >= 0.38);
    assert(brake.peakLiningTemperatureC < 350.0);
    std::cout << "  -> Domain 7 Passed: Stopping time = " << brake.stoppingTimeSeconds << " s, Peak T = " << brake.peakLiningTemperatureC << " C." << std::endl;

    // Test 8: Slew Machinery & Gearbox
    std::cout << "[Test 8/10] Domain 8: Slew Machinery Planetary Pinion & Backlash..." << std::endl;
    auto slew = Phase27Engine::evaluateSlewMachinery(8.5e7, 650.0, 1.25, 4.5);
    assert(slew.slewBearingCapacityVerified);
    assert(slew.slewAngularAccelerationRadS2 >= 0.025);
    assert(slew.pinionToothRootStressMpa < 320.0);
    std::cout << "  -> Domain 8 Passed: Slew alpha = " << slew.slewAngularAccelerationRadS2 << " rad/s2, Motor P = " << slew.swingDriveMotorPowerKw << " kW." << std::endl;

    // Test 9: Mast Guy Wire Pre-Tension
    std::cout << "[Test 9/10] Domain 9: Mast Backstay Suspension Guy Wire Dynamics..." << std::endl;
    auto wire = Phase27Engine::evaluateGuyWire(85.0, 68.0, 520.0, 35.0);
    assert(wire.mastEquilibriumStable);
    assert(wire.backstayPretensionKn >= 450.0);
    assert(wire.wireRopeSafetyFactor >= 3.0);
    std::cout << "  -> Domain 9 Passed: Pretension = " << wire.backstayPretensionKn << " kN, f1 = " << wire.firstTransverseFrequencyHz << " Hz, SF = " << wire.wireRopeSafetyFactor << std::endl;

    // Test 10: Revolving Deck Center-Pin
    std::cout << "[Test 10/10] Domain 10: Revolving Deck Center-Pin Hydrostatic Pivot..." << std::endl;
    auto pin = Phase27Engine::evaluateCenterPin(28000.0, 145000.0, 1200.0);
    assert(pin.revolvingDeckPivotCertified);
    assert(pin.bearingContactPressureMpa < 45.0);
    assert(pin.centerPinShearStressMpa < 85.0);
    std::cout << "  -> Domain 10 Passed: Contact P = " << pin.bearingContactPressureMpa << " MPa, Shear stress = " << pin.centerPinShearStressMpa << " MPa." << std::endl;

    std::cout << "============================================================" << std::endl;
    std::cout << " All 10/10 Faz 27 Mining Dragline Domains Passed Cleanly!   " << std::endl;
    std::cout << "============================================================" << std::endl;
    return 0;
}
