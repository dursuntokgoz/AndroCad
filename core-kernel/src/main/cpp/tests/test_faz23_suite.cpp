#include <iostream>
#include <cassert>
#include <cmath>
#include "Phase23Engine.h"

using namespace AndroCAD::Phase23;

int main() {
    std::cout << "============================================================" << std::endl;
    std::cout << " Starting AndroCAD Faz 23 Telescope Active Optics Suite     " << std::endl;
    std::cout << "============================================================" << std::endl;

    // Test 1: Whiffletree Primary Mount
    std::cout << "[Test 1/10] Domain 1: Primary Mirror Whiffletree Mount..." << std::endl;
    auto whiffle = Phase23Engine::evaluateWhiffletree(4.0, 0.25, 6200.0, 27, 60.0);
    assert(whiffle.diffractionLimitedSurfaceFigure);
    assert(whiffle.rmsSurfaceDeformationNm <= 15.0);
    assert(whiffle.supportPointsCount == 27);
    std::cout << "  -> Domain 1 Passed: RMS = " << whiffle.rmsSurfaceDeformationNm << " nm, PV = " << whiffle.peakToValleyDeflectionNm << " nm." << std::endl;

    // Test 2: Active Optics Piezo Correction
    std::cout << "[Test 2/10] Domain 2: Active Optics Piezo Figure Correction..." << std::endl;
    auto active = Phase23Engine::evaluateActiveOptics(85.0, 2.5e7, 0.08, 100.0);
    assert(active.activeFigureCorrectionCertified);
    assert(active.residualWavefrontRmsNm <= 10.0);
    assert(active.strokeResolutionNanometers < 2.0);
    std::cout << "  -> Domain 2 Passed: Residual RMS = " << active.residualWavefrontRmsNm << " nm, Resolution = " << active.strokeResolutionNanometers << " nm." << std::endl;

    // Test 3: Zernike Wavefront Aberrations
    std::cout << "[Test 3/10] Domain 3: Zernike Wavefront Polynomials & Strehl..." << std::endl;
    auto zernike = Phase23Engine::evaluateZernikeAberrations(632.8, 0.025, 0.018, 0.012, 0.008);
    assert(zernike.marechalCriterionSatisfied);
    assert(zernike.strehlRatio >= 0.80);
    assert(zernike.totalRmsWavefrontErrorNm < 45.0);
    std::cout << "  -> Domain 3 Passed: Wavefront RMS = " << zernike.totalRmsWavefrontErrorNm << " nm, Strehl = " << zernike.strehlRatio << std::endl;

    // Test 4: 6-DOF Stewart Hexapod Kinematics
    std::cout << "[Test 4/10] Domain 4: Secondary Mirror 6-DOF Stewart Hexapod..." << std::endl;
    auto hexapod = Phase23Engine::evaluateHexapodKinematics(450.0, 320.0, 15.0, -10.0, 45.0, 2.5, -1.8, 0.5);
    assert(hexapod.sixDofKinematicsVerified);
    assert(hexapod.maxAxialPositionErrorMicrons < 0.5);
    assert(hexapod.maxAngularPositionErrorMicrorad < 1.0);
    std::cout << "  -> Domain 4 Passed: Condition No = " << hexapod.jacobianConditionNumber << ", Pos Err = " << hexapod.maxAxialPositionErrorMicrons << " um." << std::endl;

    // Test 5: Zerodur Glass-Ceramic Thermals
    std::cout << "[Test 5/10] Domain 5: Zerodur Ultra-Low Expansion Thermal Distortion..." << std::endl;
    auto zerodur = Phase23Engine::evaluateZerodurThermals(4.0, 0.25, 0.5, 0.01);
    assert(zerodur.thermalFigureDistortionNegligible);
    assert(zerodur.thermalDeformationPeakNm < 5.0);
    std::cout << "  -> Domain 5 Passed: Peak deformation = " << zerodur.thermalDeformationPeakNm << " nm, Soak time = " << zerodur.thermalSoakTimeHours << " hrs." << std::endl;

    // Test 6: Optical Spider Vane Wind Buffeting
    std::cout << "[Test 6/10] Domain 6: Spider Vane Dynamic Stiffness & Diffraction..." << std::endl;
    auto spider = Phase23Engine::evaluateSpiderDiffraction(4.0, 1.8, 80.0, 12.0);
    assert(spider.spiderStiffnessCompliant);
    assert(spider.firstEigenFrequencyHz >= 18.0);
    std::cout << "  -> Domain 6 Passed: 1st eigenfrequency = " << spider.firstEigenFrequencyHz << " Hz, Wind defl = " << spider.windBuffetingDeflectionMicrons << " um." << std::endl;

    // Test 7: Cryogenic Sensor Dewar
    std::cout << "[Test 7/10] Domain 7: Cryogenic Infrared FPA Dewar Mount..." << std::endl;
    auto dewar = Phase23Engine::evaluateCryoDewar(77.0, 295.0, 6, 85.0, 45.0, 30);
    assert(dewar.cryogenicCoolingBudgetSatisfied);
    assert(dewar.totalHeatLoadMilliWatts < 1200.0);
    std::cout << "  -> Domain 7 Passed: Heat load = " << dewar.totalHeatLoadMilliWatts << " mW (Cond: " << dewar.conductedHeatLoadMilliWatts << " mW, Rad: " << dewar.radiatedHeatLoadMilliWatts << " mW)." << std::endl;

    // Test 8: Fast Steering Mirror (FSM)
    std::cout << "[Test 8/10] Domain 8: Fast Steering Mirror (FSM) Tip-Tilt..." << std::endl;
    auto fsm = Phase23Engine::evaluateFastSteeringMirror(0.00035, 420.0, 18.5, 1.4);
    assert(fsm.tipTiltJitterSuppressed);
    assert(fsm.closedLoopBandwidthHz >= 200.0);
    assert(fsm.strokeAngularRangeArcsec >= 120.0);
    std::cout << "  -> Domain 8 Passed: Bandwidth = " << fsm.closedLoopBandwidthHz << " Hz, Stroke = " << fsm.strokeAngularRangeArcsec << " arcsec." << std::endl;

    // Test 9: Alt-Az Hydrostatic Bearing
    std::cout << "[Test 9/10] Domain 9: Telescope Alt-Az Hydrostatic Oil Bearing..." << std::endl;
    auto bearing = Phase23Engine::evaluateHydrostaticBearing(0.25, 450.0, 0.068, 8.0, 45.0);
    assert(bearing.smoothSubArcsecondTracking);
    assert(bearing.oilFilmThicknessMicrons >= 35.0);
    assert(bearing.frictionCoefficient < 0.0005);
    std::cout << "  -> Domain 9 Passed: Film = " << bearing.oilFilmThicknessMicrons << " um, Friction mu = " << bearing.frictionCoefficient << std::endl;

    // Test 10: Dome Seeing & Flushing
    std::cout << "[Test 10/10] Domain 10: Enclosure Aerodynamic Flushing & Thermal Seeing..." << std::endl;
    auto seeing = Phase23Engine::evaluateDomeSeeing(0.4, 3.5, 45.0, 25.0);
    assert(seeing.subArcsecondSeeingAchieved);
    assert(seeing.seeingAngleArcsec < 0.85);
    assert(seeing.friedParameterR0Cm >= 12.0);
    std::cout << "  -> Domain 10 Passed: Seeing = " << seeing.seeingAngleArcsec << " arcsec, Fried r0 = " << seeing.friedParameterR0Cm << " cm." << std::endl;

    std::cout << "============================================================" << std::endl;
    std::cout << " All 10/10 Faz 23 Telescope Active Optics Domains Passed!   " << std::endl;
    std::cout << "============================================================" << std::endl;
    return 0;
}
