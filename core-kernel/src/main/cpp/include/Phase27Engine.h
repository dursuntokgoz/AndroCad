#pragma once

#include <string>
#include <vector>
#include <cmath>

namespace AndroCAD {
namespace Phase27 {

struct BoomBucklingResult {
    double boomLengthMeters;
    double maxAxialCompressionKn;
    double eulerGlobalBucklingKn;
    double localChordShellBucklingKn;
    double globalBucklingSafetyFactor;
    bool boomStructuralStabilityCertified;
};

struct FinalDriveResult {
    double outputTorqueKnm;
    double overallGearRatio;
    double sunToothContactStressMpa;
    double iso6336PittingSafetyFactorSh;
    double iso6336BendingSafetyFactorSf;
    bool heavyMiningDriveCertified;
};

struct WireRopeResult {
    double wireRopeDiameterMm;
    double sheaveDiameterRatioDToD;
    double peakRopeTensionKn;
    double expectedBendingCyclesToDiscard;
    double cumulativeDamageRatio;
    bool iso4309RetirementCriteriaSatisfied;
};

struct TubularJointScfResult {
    double chordDiameterMm;
    double braceDiameterMm;
    double efthymiouHotSpotScf;
    double peakHotSpotStressMpa;
    double weldFatigueLifeHours;
    bool tubularNodeFatigueCompliant;
};

struct BucketDiggingResult {
    double bucketVolumeCubicMeters;
    double cuttingEdgeResistanceKn;
    double maxDragPullForceKn;
    double diggingCycleTimeSeconds;
    bool payloadTargetAchieved;
};

struct WalkingCamResult {
    double strideLengthMeters;
    double walkingShoeAreaM2;
    double maxGroundBearingPressureKpa;
    double camLiftHeightMm;
    bool softGroundMobilityVerified;
};

struct DrumBrakeResult {
    double kineticEnergyAbsorptionMj;
    double peakLiningTemperatureC;
    double dynamicFrictionCoefficient;
    double stoppingTimeSeconds;
    bool thermalFadeResistanceApproved;
};

struct SlewMachineryResult {
    double slewAngularAccelerationRadS2;
    double pinionToothRootStressMpa;
    double bearingBacklashMm;
    double swingDriveMotorPowerKw;
    bool slewBearingCapacityVerified;
};

struct GuyWireResult {
    double backstayPretensionKn;
    double firstTransverseFrequencyHz;
    double maxDynamicWindTensionKn;
    double wireRopeSafetyFactor;
    bool mastEquilibriumStable;
};

struct CenterPinResult {
    double verticalThrustLoadKn;
    double overturningMomentKnm;
    double bearingContactPressureMpa;
    double centerPinShearStressMpa;
    bool revolvingDeckPivotCertified;
};

class Phase27Engine {
public:
    static BoomBucklingResult evaluateBoomBuckling(
        double boomLengthM,
        double chordDiameterMm,
        double chordWallThicknessMm,
        double operatingCompressiveLoadKn
    );

    static FinalDriveResult evaluateFinalDrive(
        double inputTorqueKnm,
        int sunTeeth,
        int planetTeeth,
        int ringTeeth,
        double faceWidthMm,
        double normalModuleMm
    );

    static WireRopeResult evaluateWireRope(
        double ropeDiameterMm,
        double sheaveDiameterMm,
        double operatingLoadKn,
        double ropeBreakingStrengthKn
    );

    static TubularJointScfResult evaluateTubularJointScf(
        double chordDiameterMm,
        double chordWallThicknessMm,
        double braceDiameterMm,
        double braceWallThicknessMm,
        double branchAngleDeg,
        double nominalBraceStressMpa
    );

    static BucketDiggingResult evaluateBucketDigging(
        double bucketCapacityM3,
        double soilSpecificCuttingResistanceKpa,
        double cuttingWidthMeters,
        double cuttingDepthMeters
    );

    static WalkingCamResult evaluateWalkingCam(
        double machineOperatingWeightKn,
        double camEccentricityMm,
        double shoeWidthM,
        double shoeLengthM
    );

    static DrumBrakeResult evaluateDrumBrake(
        double suspendedPayloadKn,
        double loweringSpeedMs,
        double drumDiameterMm,
        double liningFrictionCoeff
    );

    static SlewMachineryResult evaluateSlewMachinery(
        double revolvingDeckInertiaKgm2,
        double slewPinionPcdMm,
        double targetSlewRpm,
        double rampTimeSeconds
    );

    static GuyWireResult evaluateGuyWire(
        double stayLengthM,
        double stayDiameterMm,
        double targetPretensionKn,
        double peakWindSpeedMs
    );

    static CenterPinResult evaluateCenterPin(
        double deckDeadweightKn,
        double bucketOverturnMomentKnm,
        double pinDiameterMm
    );
};

} // namespace Phase27
} // namespace AndroCAD
