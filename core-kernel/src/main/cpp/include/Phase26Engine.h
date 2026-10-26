#pragma once

#include <string>
#include <vector>
#include <cmath>

namespace AndroCAD {
namespace Phase26 {

struct TitaniumSphereResult {
    double outerDiameterMm;
    double wallThicknessMm;
    double hydrostaticPressureMpa;
    double membraneStressMpa;
    double bifurcationBucklingPressureMpa;
    double asmePvhoSafetyFactor;
    bool fullOceanDepthCertified;
};

struct AcrylicViewportResult {
    double minorDiameterMm;
    double majorDiameterMm;
    double thicknessMm;
    double seatBendingStressMpa;
    double viscoelasticCreepDeflectionMm;
    double opticalTransmittancePercent;
    bool asmePvho1ViewportApproved;
};

struct SyntacticFoamResult {
    double foamDensityKgM3;
    double compressiveStrengthMpa;
    double hydrostaticCollapsePressureMpa;
    double waterAbsorptionPercent;
    double netPositiveBuoyancyKn;
    bool deepSeaBuoyancyCertified;
};

struct PbofEnclosureResult {
    double interiorPressureMpa;
    double differentialPressureBar;
    double diaphragmVolumeDisplacementLiters;
    double dielectricBreakdownKv;
    bool pressureBalancedSealed;
};

struct SubseaManipulatorResult {
    double maxReachMeters;
    double payloadCapacityKg;
    double hydraulicOperatingPressureBar;
    double jointPositionErrorMm;
    bool sevenDofTeleoperationVerified;
};

struct HighPressurePenetratorResult {
    double workingPressureMpa;
    double heliumLeakRateMbarLPerS;
    double electricalInsulationResistanceGigaOhms;
    double pinCount;
    bool hermeticFeedthroughCertified;
};

struct LifeSupportResult {
    double metabolicOxygenRateLpm;
    double liohAbsorptionEfficiencyPercent;
    double cabinCo2PartialPressureMbar;
    double cabinThermalLossWatts;
    double missionEnduranceHours;
    bool lifeSupportBudgetCompliant;
};

struct AscentBallastResult {
    double jettisonBallastMassKg;
    double electromagneticHoldingForceKn;
    double galvanicReleaseDelayMinutes;
    double terminalAscentSpeedMs;
    bool failsafeEmergencyAscentGuaranteed;
};

struct SubmersibleThrusterResult {
    double bollardThrustNewtons;
    double magneticCouplingBreakawayTorqueNm;
    double thrusterEfficiencyPercent;
    double motorShaftSpeedRpm;
    bool magneticDriveImmersionVerified;
};

struct UsblTelemetryResult {
    double acousticCarrierFrequencyKhz;
    double slantRangeMeters;
    double acousticOneWayLatencySeconds;
    double positioningAccuracyMeters;
    bool fullOceanDepthNavigationLocked;
};

class Phase26Engine {
public:
    static TitaniumSphereResult evaluateTitaniumSphere(
        double outerDiameterM,
        double oceanDepthM,
        double yieldStrengthMpa,
        double outOfRoundnessFactor
    );

    static AcrylicViewportResult evaluateAcrylicViewport(
        double viewDiameterMm,
        double depthMeters,
        double coneIncludedAngleDeg,
        double designLifeHours
    );

    static SyntacticFoamResult evaluateSyntacticFoam(
        double foamVolumeM3,
        double targetDepthM,
        double microsphereVolumeFraction
    );

    static PbofEnclosureResult evaluatePbofEnclosure(
        double oceanPressureMpa,
        double internalFluidVolumeLiters,
        double ambientTempC
    );

    static SubseaManipulatorResult evaluateSubseaManipulator(
        double linkLengthM,
        double hydraulicSupplyPressureBar,
        double maxTargetPayloadKg
    );

    static HighPressurePenetratorResult evaluateHighPressurePenetrator(
        double testPressureMpa,
        int electricalPins,
        double ceramicInsulatorThicknessMm
    );

    static LifeSupportResult evaluateLifeSupport(
        int crewCount,
        double cabinAirVolumeM3,
        double liohBedMassKg,
        double waterTempC
    );

    static AscentBallastResult evaluateAscentBallast(
        double submersibleDryMassKg,
        double displacementVolumeM3,
        double ballastMassKg
    );

    static SubmersibleThrusterResult evaluateSubmersibleThruster(
        double motorElectricalPowerKw,
        double propDiameterMm,
        double magneticGapMm
    );

    static UsblTelemetryResult evaluateUsblTelemetry(
        double oceanDepthM,
        double acousticFrequencyKhz,
        double soundSpeedMs
    );
};

} // namespace Phase26
} // namespace AndroCAD
