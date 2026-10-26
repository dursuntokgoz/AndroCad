#pragma once

#include <string>
#include <vector>
#include <cmath>

namespace AndroCAD {
namespace Phase25 {

struct KalkerContactResult {
    double contactEllipseSemiAxisAMm;
    double contactEllipseSemiAxisBMm;
    double longitudinalCreepForceKn;
    double lateralCreepForceKn;
    double spinCreepMomentNm;
    double peakHertzContactStressMpa;
    bool adhesionLimitSatisfied;
};

struct HuntingStabilityResult {
    double klingelFrequencyHz;
    double criticalHuntingSpeedKmh;
    double hopfBifurcationDampingRatio;
    double lateralWheelsetAccelerationG;
    bool highSpeedHuntingSuppressed;
};

struct PrimarySuspensionResult {
    double verticalDeflectionMm;
    double longitudinalStiffnessKnMm;
    double lateralStiffnessKnMm;
    double axleboxBearingVibrationG;
    bool primaryIsolationCompliant;
};

struct SecondaryAirSpringResult {
    double airBellowsPressureBar;
    double verticalStiffnessKnM;
    double lateralStiffnessKnM;
    double carbodyLevelingResponseTimeS;
    double auxiliaryRubberBumperMarginMm;
    bool secondarySuspensionCertified;
};

struct AntiYawDamperResult {
    double blowOffDampingForceKn;
    double blowOffVelocityMs;
    double dynamicYawStiffnessKnM;
    double yawStabilityMarginFactor;
    bool bogieBogieHuntingPrevented;
};

struct BogieFrameFatigueResult {
    double maxTrescaStressMpa;
    double hydrostaticStressMpa;
    double dangVanEquivalentStressMpa;
    double dangVanFatigueLimitRatio;
    bool en13749FrameStructuralApproved;
};

struct DiscBrakeResult {
    double emergencyBrakingDistanceM;
    double peakDiscTemperatureC;
    double meanDecelerationMs2;
    double frictionalDissipationMw;
    bool thermalCrackingPrevented;
};

struct NadalDerailmentResult {
    double lateralWheelRailForceYKn;
    double verticalWheelRailForceQKn;
    double nadalDerailmentRatioYQ;
    double flangeContactAngleDegrees;
    bool en14363RunningSafetyVerified;
};

struct TractionDriveResult {
    double cardanShaftMisalignmentDeg;
    double hollowShaftTorsionalStressMpa;
    double centrifugalGreaseRetentionBar;
    double gearMeshTransmissionEfficiency;
    bool fullySuspendedDriveApproved;
};

struct SperlingRideIndexResult {
    double verticalSperlingIndexWz;
    double lateralSperlingIndexWz;
    double rmsCarbodyAccelerationMs2;
    double en14363RideComfortRating;
    bool superiorRideQualityCertified;
};

class Phase25Engine {
public:
    static KalkerContactResult evaluateKalkerContact(
        double wheelLoadKn,
        double wheelRadiusMm,
        double railProfileRadiusMm,
        double longitudinalCreep,
        double lateralCreep,
        double spinCreepRadM
    );

    static HuntingStabilityResult evaluateHuntingStability(
        double vehicleSpeedKmh,
        double effectiveConicity,
        double wheelsetMassKg,
        double primaryYawStiffnessKnRad,
        double trackGaugeMm
    );

    static PrimarySuspensionResult evaluatePrimarySuspension(
        double axleLoadKn,
        double coilSpringWireDiaMm,
        double coilMeanDiaMm,
        int activeCoils,
        double axleboxDamperRatingNsM
    );

    static SecondaryAirSpringResult evaluateSecondaryAirSpring(
        double carbodyMassKg,
        double airBellowsDiameterMm,
        double surgeTankVolumeLiters,
        double nominalPressureBar
    );

    static AntiYawDamperResult evaluateAntiYawDamper(
        double yawVelocityMs,
        double blowOffForceKn,
        double blowOffVelocityThresholdMs
    );

    static BogieFrameFatigueResult evaluateBogieFrameFatigue(
        double verticalLoadKn,
        double lateralLoadKn,
        double twistLoadKn,
        double frameWeldYieldMpa
    );

    static DiscBrakeResult evaluateDiscBrake(
        double initialSpeedKmh,
        double axleTrainMassKg,
        double brakeDiscDiameterMm,
        double frictionCoeff
    );

    static NadalDerailmentResult evaluateNadalDerailment(
        double lateralForceYKn,
        double verticalForceQKn,
        double flangeAngleDeg,
        double wheelRailFrictionMu
    );

    static TractionDriveResult evaluateTractionDrive(
        double motorTorqueNm,
        double maxRpm,
        double cardanOffsetMm,
        double hollowShaftDiaMm
    );

    static SperlingRideIndexResult evaluateSperlingRideIndex(
        double vehicleSpeedKmh,
        double trackQualityClass,
        double carbodyMassKg
    );
};

} // namespace Phase25
} // namespace AndroCAD
