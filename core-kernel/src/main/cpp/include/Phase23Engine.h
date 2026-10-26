#pragma once

#include <string>
#include <vector>
#include <cmath>

namespace AndroCAD {
namespace Phase23 {

struct WhiffletreeResult {
    double rmsSurfaceDeformationNm;
    double peakToValleyDeflectionNm;
    int supportPointsCount;
    double gravitySagNanometers;
    bool diffractionLimitedSurfaceFigure;
};

struct ActiveOpticsResult {
    double strokeResolutionNanometers;
    double maxCorrectionForceN;
    double residualWavefrontRmsNm;
    double dynamicBandwidthHz;
    bool activeFigureCorrectionCertified;
};

struct ZernikeWavefrontResult {
    double defocusCoeffMicrons;
    double astigmatismCoeffMicrons;
    double comaCoeffMicrons;
    double sphericalCoeffMicrons;
    double totalRmsWavefrontErrorNm;
    double strehlRatio;
    bool marechalCriterionSatisfied;
};

struct HexapodKinematicsResult {
    double actuatorLengthMm[6];
    double maxAxialPositionErrorMicrons;
    double maxAngularPositionErrorMicrorad;
    double jacobianConditionNumber;
    bool sixDofKinematicsVerified;
};

struct ZerodurThermalResult {
    double ctePpmPerK;
    double maxThermalGradientK;
    double thermalDeformationPeakNm;
    double thermalSoakTimeHours;
    bool thermalFigureDistortionNegligible;
};

struct SpiderDiffractionResult {
    double firstEigenFrequencyHz;
    double windBuffetingDeflectionMicrons;
    double relativeDiffractionIntensityPercent;
    double airySpikeSuppressionDb;
    bool spiderStiffnessCompliant;
};

struct CryoDewarResult {
    double conductedHeatLoadMilliWatts;
    double radiatedHeatLoadMilliWatts;
    double totalHeatLoadMilliWatts;
    double focalPlaneArrayTempK;
    bool cryogenicCoolingBudgetSatisfied;
};

struct FastSteeringMirrorResult {
    double closedLoopBandwidthHz;
    double strokeAngularRangeArcsec;
    double angularResolutionMilliArcsec;
    double voiceCoilDissipationWatts;
    bool tipTiltJitterSuppressed;
};

struct HydrostaticBearingResult {
    double oilFilmThicknessMicrons;
    double frictionCoefficient;
    double recessPressureBar;
    double oilFlowRateLitersPerMin;
    bool smoothSubArcsecondTracking;
};

struct DomeSeeingResult {
    double friedParameterR0Cm;
    double seeingAngleArcsec;
    double domeAirExchangeRatePerHour;
    double thermalSeeingContributionArcsec;
    bool subArcsecondSeeingAchieved;
};

class Phase23Engine {
public:
    static WhiffletreeResult evaluateWhiffletree(
        double mirrorDiameterM,
        double mirrorThicknessM,
        double mirrorMassKg,
        int supportPoints,
        double altitudeAngleDeg
    );

    static ActiveOpticsResult evaluateActiveOptics(
        double appliedPiezoVoltageV,
        double actuatorStiffnessNPerM,
        double mirrorFlexibilityNmPerN,
        double opticalSamplingFreqHz
    );

    static ZernikeWavefrontResult evaluateZernikeAberrations(
        double wavelengthNm,
        double cDefocus,
        double cAstig,
        double cComa,
        double cSpherical
    );

    static HexapodKinematicsResult evaluateHexapodKinematics(
        double baseRadiusMm,
        double platformRadiusMm,
        double deltaXMicrons,
        double deltaYMicrons,
        double deltaZMicrons,
        double tipArcsec,
        double tiltArcsec,
        double clockingArcsec
    );

    static ZerodurThermalResult evaluateZerodurThermals(
        double mirrorDiameterM,
        double mirrorThicknessM,
        double ambientTempChangeKPerHr,
        double ctePpmK
    );

    static SpiderDiffractionResult evaluateSpiderDiffraction(
        double spiderVaneThicknessMm,
        double spiderVaneLengthM,
        double spiderDepthMm,
        double windVelocityMs
    );

    static CryoDewarResult evaluateCryoDewar(
        double dewarInnerTempK,
        double dewarOuterTempK,
        int strutCount,
        double strutLengthMm,
        double strutAreaMm2,
        double mliLayersCount
    );

    static FastSteeringMirrorResult evaluateFastSteeringMirror(
        double mirrorInertiaKgm2,
        double flexurePivotStiffnessNmPerRad,
        double voiceCoilForceConstantNA,
        double peakCurrentAmps
    );

    static HydrostaticBearingResult evaluateHydrostaticBearing(
        double padAreaM2,
        double telescopeWeightKn,
        double oilViscosityPaS,
        double recessDepthMm,
        double supplyPressureBar
    );

    static DomeSeeingResult evaluateDomeSeeing(
        double domeTemperatureDeltaK,
        double flushingWindSpeedMs,
        double domeApertureAreaM2,
        double zenithAngleDeg
    );
};

} // namespace Phase23
} // namespace AndroCAD
