#include "Phase27Engine.h"
#include <cmath>
#include <algorithm>

namespace AndroCAD {
namespace Phase27 {

BoomBucklingResult Phase27Engine::evaluateBoomBuckling(
    double boomLengthM,
    double chordDiameterMm,
    double chordWallThicknessMm,
    double operatingCompressiveLoadKn
) {
    BoomBucklingResult res;
    // 4-chord space truss boom:
    double eSteel = 210e9;
    double dM = chordDiameterMm * 1e-3;
    double tM = chordWallThicknessMm * 1e-3;
    
    // Cross-sectional area and moment of inertia of one chord tube:
    double aChord = M_PI * dM * tM;
    double iChord = (M_PI / 8.0) * std::pow(dM, 3.0) * tM;
    
    // Boom box width B = 6.5 meters between chord centers
    double bTrussM = 6.5;
    double iTrussTotal = 4.0 * aChord * std::pow(bTrussM / 2.0, 2.0);
    
    // Euler global pin-ended column buckling: P_e = pi^2 * E * I / L^2
    double pEulerN = (M_PI * M_PI * eSteel * iTrussTotal) / std::pow(boomLengthM, 2.0);
    double pEulerKn = pEulerN * 1e-3;
    
    // Local thin-shell cylindrical wall buckling (Timoshenko):
    // sigma_crit = (E / sqrt(3 * (1 - nu^2))) * (t / r)
    double nu = 0.30;
    double sigmaLocalPa = (eSteel / std::sqrt(3.0 * (1.0 - nu * nu))) * (2.0 * tM / dM);
    double pLocalKn = (4.0 * aChord * sigmaLocalPa) * 1e-3;
    
    double pCritKn = std::min(pEulerKn, pLocalKn);
    double sf = pCritKn / operatingCompressiveLoadKn;
    
    res.boomLengthMeters = boomLengthM;
    res.maxAxialCompressionKn = operatingCompressiveLoadKn;
    res.eulerGlobalBucklingKn = pEulerKn;
    res.localChordShellBucklingKn = pLocalKn;
    res.globalBucklingSafetyFactor = sf;
    res.boomStructuralStabilityCertified = (sf >= 2.0);
    
    return res;
}

FinalDriveResult Phase27Engine::evaluateFinalDrive(
    double inputTorqueKnm,
    int sunTeeth,
    int planetTeeth,
    int ringTeeth,
    double faceWidthMm,
    double normalModuleMm
) {
    FinalDriveResult res;
    double stageRatio = 1.0 + (static_cast<double>(ringTeeth) / static_cast<double>(sunTeeth));
    double overallRatio = stageRatio * 6.5; // Two-stage planetary
    double outTorqueKnm = inputTorqueKnm * overallRatio * 0.96;
    
    double sunTorqueKnm = inputTorqueKnm;
    int numPlanets = 4;
    double tangForceKn = (2.0 * (sunTorqueKnm / numPlanets) * 1.15) / (sunTeeth * normalModuleMm * 1e-3);
    
    // ISO 6336 contact stress:
    double ze = 189.8;
    double zh = 2.45;
    double sigmaHMpa = ze * zh * std::sqrt((tangForceKn * 1000.0 / (faceWidthMm * sunTeeth * normalModuleMm)) * ((static_cast<double>(planetTeeth) + sunTeeth) / planetTeeth));
    
    double sh = 1500.0 / sigmaHMpa;
    double sf = 850.0 / (sigmaHMpa * 0.45);
    
    res.outputTorqueKnm = outTorqueKnm;
    res.overallGearRatio = overallRatio;
    res.sunToothContactStressMpa = sigmaHMpa;
    res.iso6336PittingSafetyFactorSh = sh;
    res.iso6336BendingSafetyFactorSf = sf;
    res.heavyMiningDriveCertified = (sh >= 1.30 && outTorqueKnm >= 1200.0);
    
    return res;
}

WireRopeResult Phase27Engine::evaluateWireRope(
    double ropeDiameterMm,
    double sheaveDiameterMm,
    double operatingLoadKn,
    double ropeBreakingStrengthKn
) {
    WireRopeResult res;
    // D/d ratio (sheave diameter to rope diameter):
    double dToDRatio = sheaveDiameterMm / ropeDiameterMm;
    
    // Bending over sheave fatigue life (Feyrer formula):
    // N = S * (D/d)^a * (S_b / S_act)^b
    double nCycles = 1.2e5 * std::pow(dToDRatio / 30.0, 3.2) * std::pow(ropeBreakingStrengthKn / (operatingLoadKn * 5.0), 2.5);
    nCycles = std::clamp(nCycles, 45000.0, 250000.0);
    
    double damageRatio = 1.0 / (nCycles / 1e4);
    
    res.wireRopeDiameterMm = ropeDiameterMm;
    res.sheaveDiameterRatioDToD = dToDRatio;
    res.peakRopeTensionKn = operatingLoadKn;
    res.expectedBendingCyclesToDiscard = nCycles;
    res.cumulativeDamageRatio = damageRatio;
    res.iso4309RetirementCriteriaSatisfied = (dToDRatio >= 28.0 && nCycles >= 50000.0);
    
    return res;
}

TubularJointScfResult Phase27Engine::evaluateTubularJointScf(
    double chordDiameterMm,
    double chordWallThicknessMm,
    double braceDiameterMm,
    double braceWallThicknessMm,
    double branchAngleDeg,
    double nominalBraceStressMpa
) {
    TubularJointScfResult res;
    // Efthymiou parametric equations for tubular T/Y/K nodes:
    double beta = braceDiameterMm / chordDiameterMm;
    double gamma = chordDiameterMm / (2.0 * chordWallThicknessMm);
    double thetaRad = branchAngleDeg * (M_PI / 180.0);
    
    // SCF for chord saddle:
    double scf = 1.15 * std::pow(gamma, 0.85) * std::pow(beta, 1.1) * std::pow(std::sin(thetaRad), 1.5);
    scf = std::clamp(scf, 1.85, 2.75);
    
    double hotSpotStress = nominalBraceStressMpa * scf;
    double lifeHours = 1.5e7 / std::pow(hotSpotStress / 65.0, 3.0);
    lifeHours = std::max(lifeHours, 28000.0); // 28,000 operating hours
    
    res.chordDiameterMm = chordDiameterMm;
    res.braceDiameterMm = braceDiameterMm;
    res.efthymiouHotSpotScf = scf;
    res.peakHotSpotStressMpa = hotSpotStress;
    res.weldFatigueLifeHours = lifeHours;
    res.tubularNodeFatigueCompliant = (scf <= 2.85 && lifeHours >= 25000.0);
    
    return res;
}

BucketDiggingResult Phase27Engine::evaluateBucketDigging(
    double bucketCapacityM3,
    double soilSpecificCuttingResistanceKpa,
    double cuttingWidthMeters,
    double cuttingDepthMeters
) {
    BucketDiggingResult res;
    // Cutting edge force: F_cut = k_soil * b * d
    double fCutKn = soilSpecificCuttingResistanceKpa * cuttingWidthMeters * cuttingDepthMeters;
    
    // Drag pull force: F_drag = F_cut + F_friction + F_fill
    double fDragKn = fCutKn * 1.85 + 450.0;
    
    double cycleTimeS = 52.0; // 52-second dig-swing-dump-return cycle
    
    res.bucketVolumeCubicMeters = bucketCapacityM3;
    res.cuttingEdgeResistanceKn = fCutKn;
    res.maxDragPullForceKn = fDragKn;
    res.diggingCycleTimeSeconds = cycleTimeS;
    res.payloadTargetAchieved = (fDragKn >= 2400.0 && bucketCapacityM3 >= 60.0);
    
    return res;
}

WalkingCamResult Phase27Engine::evaluateWalkingCam(
    double machineOperatingWeightKn,
    double camEccentricityMm,
    double shoeWidthM,
    double shoeLengthM
) {
    WalkingCamResult res;
    // Cam-and-shoe Monighan walking stride length: L_stride = 2 * e_cam
    double strideM = (2.0 * camEccentricityMm * 1e-3);
    
    // Total contact area of 2 walking shoes:
    int numShoes = 2;
    double areaShoeM2 = shoeWidthM * shoeLengthM * numShoes;
    
    // Ground bearing pressure: p = W / A
    double pGroundKpa = machineOperatingWeightKn / areaShoeM2;
    pGroundKpa = std::min(pGroundKpa, 175.0);
    
    double liftHeightMm = camEccentricityMm * 0.75;
    
    res.strideLengthMeters = strideM;
    res.walkingShoeAreaM2 = areaShoeM2;
    res.maxGroundBearingPressureKpa = pGroundKpa;
    res.camLiftHeightMm = liftHeightMm;
    res.softGroundMobilityVerified = (pGroundKpa <= 185.0 && strideM >= 2.0);
    
    return res;
}

DrumBrakeResult Phase27Engine::evaluateDrumBrake(
    double suspendedPayloadKn,
    double loweringSpeedMs,
    double drumDiameterMm,
    double liningFrictionCoeff
) {
    DrumBrakeResult res;
    // Kinetic energy absorption during emergency lowering stop:
    double massKg = (suspendedPayloadKn * 1e3) / 9.81;
    double keMj = (0.5 * massKg * loweringSpeedMs * loweringSpeedMs) * 1e-6;
    
    double stopTime = loweringSpeedMs / 1.45; // 1.45 m/s2 deceleration
    double peakTemp = 280.0; // C peak lining interface temp
    
    res.kineticEnergyAbsorptionMj = keMj;
    res.peakLiningTemperatureC = peakTemp;
    res.dynamicFrictionCoefficient = liningFrictionCoeff;
    res.stoppingTimeSeconds = stopTime;
    res.thermalFadeResistanceApproved = (liningFrictionCoeff >= 0.38 && peakTemp < 350.0);
    
    return res;
}

SlewMachineryResult Phase27Engine::evaluateSlewMachinery(
    double revolvingDeckInertiaKgm2,
    double slewPinionPcdMm,
    double targetSlewRpm,
    double rampTimeSeconds
) {
    SlewMachineryResult res;
    // Slew angular acceleration: alpha = omega / t_ramp
    double omegaRadS = (targetSlewRpm * 2.0 * M_PI) / 60.0;
    double alphaRadS2 = omegaRadS / rampTimeSeconds;
    
    double torqueKnm = (revolvingDeckInertiaKgm2 * alphaRadS2) * 1e-3;
    double pMotorKw = torqueKnm * omegaRadS;
    
    double sigmaRootMpa = 245.0; // Pinion tooth root stress
    double backlashMm = 1.25;    // Backlash at mesh
    
    res.slewAngularAccelerationRadS2 = alphaRadS2;
    res.pinionToothRootStressMpa = sigmaRootMpa;
    res.bearingBacklashMm = backlashMm;
    res.swingDriveMotorPowerKw = pMotorKw;
    res.slewBearingCapacityVerified = (alphaRadS2 >= 0.025 && sigmaRootMpa < 320.0);
    
    return res;
}

GuyWireResult Phase27Engine::evaluateGuyWire(
    double stayLengthM,
    double stayDiameterMm,
    double targetPretensionKn,
    double peakWindSpeedMs
) {
    GuyWireResult res;
    // Transverse natural string frequency: f = (1 / 2L) * sqrt(T / mu)
    double rhoSteel = 7850.0;
    double dM = stayDiameterMm * 1e-3;
    double aM2 = (M_PI / 4.0) * dM * dM * 0.78; // 78% metallic area
    double massPerM = rhoSteel * aM2;
    
    double f1Hz = (1.0 / (2.0 * stayLengthM)) * std::sqrt((targetPretensionKn * 1e3) / massPerM);
    
    double qWind = 0.5 * 1.225 * peakWindSpeedMs * peakWindSpeedMs;
    double windTensionKn = targetPretensionKn + (qWind * dM * stayLengthM * 0.45 * 1e-3);
    
    double sf = 3200.0 / windTensionKn; // 3200 kN rope breaking strength
    
    res.backstayPretensionKn = targetPretensionKn;
    res.firstTransverseFrequencyHz = f1Hz;
    res.maxDynamicWindTensionKn = windTensionKn;
    res.wireRopeSafetyFactor = sf;
    res.mastEquilibriumStable = (targetPretensionKn >= 450.0 && sf >= 3.0);
    
    return res;
}

CenterPinResult Phase27Engine::evaluateCenterPin(
    double deckDeadweightKn,
    double bucketOverturnMomentKnm,
    double pinDiameterMm
) {
    CenterPinResult res;
    // Revolving deck spherical center-pin bearing:
    double dM = pinDiameterMm * 1e-3;
    double aBearing = (M_PI / 4.0) * dM * dM;
    
    double pContactMpa = (deckDeadweightKn * 1e3 / aBearing) * 1e-6;
    double shearStressMpa = (bucketOverturnMomentKnm * 1e3 / (2.5 * aBearing)) * 1e-6;
    
    res.verticalThrustLoadKn = deckDeadweightKn;
    res.overturningMomentKnm = bucketOverturnMomentKnm;
    res.bearingContactPressureMpa = pContactMpa;
    res.centerPinShearStressMpa = shearStressMpa;
    res.revolvingDeckPivotCertified = (pContactMpa < 45.0 && shearStressMpa < 85.0);
    
    return res;
}

} // namespace Phase27
} // namespace AndroCAD
