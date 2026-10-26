#pragma once

#include <string>
#include <vector>
#include <cmath>

namespace AndroCAD {
namespace Phase22 {

struct BemAerodynamicsResult {
    double axialInductionFactorA;
    double angularInductionFactorAPrime;
    double liftCoefficientCl;
    double dragCoefficientCd;
    double sectionalThrustForceNPerM;
    double sectionalTorqueNPerM;
    double prandtlTipLossFactor;
    bool glauertHighInductionCorrectionApplied;
    bool aerodynamicEquilibriumConverged;
};

struct PitchBearingResult {
    double maxHertzContactStressMpa;
    double maxSubsurfaceShearStressMpa;
    double subsurfaceDepthMm;
    double racewayFatigueLifeL10hHours;
    double staticSafetyFactorS0;
    bool hertzianStressWithinAllowable;
    bool fatigueLifeExceeds20Years;
};

struct PlanetaryGearboxResult {
    double gearRatio;
    double sunToothBendingStressMpa;
    double planetContactStressMpa;
    double planetLoadSharingFactorKGamma;
    double iso6336PittingSafetyFactorSh;
    double iso6336BendingSafetyFactorSf;
    bool gearboxReliabilityCertified;
};

struct AeroelasticFlutterResult {
    double firstFlapwiseFrequencyHz;
    double firstTorsionalFrequencyHz;
    double frequencySeparationRatio;
    double aerodynamicDampingRatio;
    double flutterOnsetWindSpeedMs;
    bool aeroelasticStabilityMarginSufficient;
};

struct PmGeneratorResult {
    double electromagneticTorqueKnm;
    double airgapFluxDensityTesla;
    double coggingTorquePeakPercent;
    double electricalEfficiencyPercent;
    double generatorThermalMarginK;
    bool directDriveComplianceMet;
};

struct TowerVivResult {
    double vortexSheddingFrequencyHz;
    double firstBendingNaturalFreqHz;
    double strouhalNumber;
    double lockInWindSpeedRangeMinMs;
    double lockInWindSpeedRangeMaxMs;
    double tunedMassDamperDampingRatio;
    bool lockInResonanceSuppressed;
};

struct TrailingEdgeNoiseResult {
    double baselineNoiseSplDba;
    double serratedNoiseSplDba;
    double noiseReductionDeltaDba;
    double serrationWavelengthMm;
    double serrationHeightMm;
    bool acousticEmissionsCompliant;
};

struct PitchActuationResult {
    double maxPitchRateDegPerSec;
    double requiredActuationTorqueKnm;
    double motorPowerRequirementKw;
    double gustResponseLatencyMs;
    bool independentPitchControlApproved;
};

struct OffshoreMonopileResult {
    double mudlineLateralDisplacementMm;
    double mudlineRotationDegrees;
    double maxBendingMomentMnM;
    double pySoilResistanceKpa;
    double accumulatedLongTermTiltDeg;
    bool geotechnicalIntegritySatisfied;
};

struct BladeFatigueResult {
    double flapwiseStressAmplitudeMpa;
    double edgewiseStressAmplitudeMpa;
    double totalEquivalentCycles;
    double minerCumulativeDamageIndexD;
    double remainingFatigueLifeYears;
    bool iec61400_23CertificationPassed;
};

class Phase22Engine {
public:
    static BemAerodynamicsResult evaluateBemAerodynamics(
        double windSpeedMs,
        double rotorRadiusM,
        double localRadiusM,
        double rotorRpm,
        double bladePitchDeg,
        double bladeChordM,
        double airfoilClAlpha,
        double airfoilCd0
    );

    static PitchBearingResult evaluatePitchBearing(
        double ballDiameterMm,
        int numberOfBalls,
        double contactAngleDeg,
        double pitchMomentKnm,
        double axialLoadKn,
        double radialLoadKn
    );

    static PlanetaryGearboxResult evaluatePlanetaryGearbox(
        double rotorTorqueKnm,
        int sunTeeth,
        int planetTeeth,
        int ringTeeth,
        int numPlanets,
        double faceWidthMm,
        double normalModuleMm
    );

    static AeroelasticFlutterResult evaluateAeroelasticFlutter(
        double bladeLengthM,
        double flapwiseStiffnessEiNm2,
        double torsionalStiffnessGjNm2,
        double massPerUnitLengthKgM,
        double massPolarMomentOfInertiaKgM,
        double tipSpeedRatio
    );

    static PmGeneratorResult evaluatePmGenerator(
        double mechanicalPowerMw,
        double rotorRpm,
        double airgapRadiusM,
        double stackLengthM,
        double polePairs,
        double remanenceBrTesla
    );

    static TowerVivResult evaluateTowerViv(
        double towerTopDiameterM,
        double towerBaseDiameterM,
        double towerHeightM,
        double towerFirstEigenFreqHz,
        double tmdMassRatio
    );

    static TrailingEdgeNoiseResult evaluateTrailingEdgeNoise(
        double tipSpeedMs,
        double chordTipM,
        double boundaryLayerThicknessMm,
        double serrationWavelengthMm,
        double serrationHeightMm
    );

    static PitchActuationResult evaluatePitchActuation(
        double aerodynamicPitchMomentKnm,
        double bearingFrictionTorqueKnm,
        double bladeInertiaKgm2,
        double targetPitchAccelerationDegS2,
        double targetPitchSpeedDegS
    );

    static OffshoreMonopileResult evaluateOffshoreMonopile(
        double monopileDiameterM,
        double wallThicknessMm,
        double embeddedLengthM,
        double mudlineShearKn,
        double mudlineMomentKnm,
        double soilFrictionAngleDeg
    );

    static BladeFatigueResult evaluateBladeFatigue(
        double flapwiseBendingMomentKnm,
        double edgewiseBendingMomentKnm,
        double bladeSectionModulusM3,
        double compositeUltimateTensileStrengthMpa,
        double targetCycles
    );
};

} // namespace Phase22
} // namespace AndroCAD
