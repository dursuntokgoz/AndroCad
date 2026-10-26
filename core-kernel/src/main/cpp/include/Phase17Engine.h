#ifndef ANDROCAD_PHASE17_ENGINE_H
#define ANDROCAD_PHASE17_ENGINE_H

#include <string>
#include <vector>
#include <cmath>

namespace AndroCAD {
namespace Phase17 {

// Domain 1: Hoek-Brown Rock Mass Failure
struct HoekBrownResult {
    double majorPrincipalStrengthMpa;
    double rockMassModulusGpa;
    double tensileStrengthMpa;
    double safetyFactor;
    bool rockMassStable;
};

// Domain 2: TBM Disc Cutter Rolling Indentation (CSM Model)
struct CsmCutterForceResult {
    double normalForceKn;
    double rollingForceKn;
    double specificEnergyMjm3;
    double penetrationMmPerRev;
    bool cutterLoadWithinBearingLimit;
};

// Domain 3: Precast Segmental Lining Ring Joint Mechanics
struct SegmentalLiningResult {
    double jointRotationalStiffnessKnMPerRad;
    double maxLiningBendingMomentKnM;
    double jointOpeningDisplacementMm;
    double boltTensionStressMpa;
    bool liningIntegrityMaintained;
};

// Domain 4: EPB Chamber Pressure Balance
struct EpbChamberPressureResult {
    double earthPressureAtCrownBar;
    double hydrostaticPressureBar;
    double requiredChamberPressureBar;
    double faceStabilitySafetyFactor;
    bool surfaceCollapseAvoided;
};

// Domain 5: Peck Greenfield Settlement Trough
struct PeckSettlementResult {
    double maxSettlementMm;
    double troughWidthParameterMeters;
    double maxAngularDistortion;
    int buildingDamageCategory;
    bool settlementWithinNegligibleLimit;
};

// Domain 6: Convergence-Confinement Method (CCM)
struct CcmConvergenceResult {
    double plasticRadiusMeters;
    double wallConvergencePercent;
    double supportPressureRequiredMpa;
    double safetyFactorOnShotcrete;
    bool groundConfinementAdequate;
};

// Domain 7: TBM Cutterhead Power & Torque Kinematics
struct TbmPowerTorqueResult {
    double totalThrustKn;
    double requiredTorqueKnM;
    double cutterheadPowerKw;
    double advanceRateMetersPerHour;
    bool drivePowerSufficient;
};

// Domain 8: Annular Gap Tail Grout Solidification
struct TailGroutResult {
    double groutVolumeRequiredM3;
    double netTunnelBuoyancyKnPerMeter;
    double initialGroutPressureBar;
    double settingTimeMinutes;
    bool preventsTunnelFloatation;
};

// Domain 9: Squeezing Ground Shield Jamming Risk
struct ShieldJammingResult {
    double radialGroundClosureMm;
    double shieldConicityClearanceMm;
    double frictionFrictionalThrustRequiredKn;
    double thrustMarginPercent;
    bool shieldFreeFromJamming;
};

// Domain 10: Tunnel Seismic Dynamic Shear Strain
struct TunnelSeismicResult {
    double peakGroundVelocityMs;
    double liningShearStrainMicrostrain;
    double dynamicHoopThrustKnPerMeter;
    double concreteTensileStrainLimit;
    bool earthquakeResistantLining;
};

class Phase17Engine {
public:
    static HoekBrownResult evaluateHoekBrown(double minorPrincipalStressMpa, double ucsMpa, double gsi, double miParameter, double disturbanceD);
    static CsmCutterForceResult calculateCsmCutterForces(double cutterDiameterInch, double tipWidthMm, double spacingMm, double penetrationMm, double ucsMpa, double shearStrengthMpa);
    static SegmentalLiningResult evaluateSegmentalLining(double tunnelRadiusM, double overburdenDepthM, double groundStiffnessMpa, double segmentThicknessM, double jointBoltPreloadKn);
    static EpbChamberPressureResult evaluateEpbFacePressure(double depthToCrownM, double waterTableDepthM, double soilDensityKnM3, double soilCohesionKpa, double internalFrictionAngleDeg);
    static PeckSettlementResult calculatePeckSettlement(double tunnelDepthM, double tunnelDiameterM, double volumeLossPercent, double offsetDistanceM);
    static CcmConvergenceResult evaluateCcmConvergence(double tunnelRadiusM, double inSituHydrostaticStressMpa, double cohesionMpa, double frictionAngleDeg, double youngsModulusGpa);
    static TbmPowerTorqueResult calculateTbmPowerTorque(double tbmDiameterM, int cutterCount, double penetrationMmPerRev, double rpm, double rockUcsMpa);
    static TailGroutResult evaluateTailGrouting(double tunnelOuterRadiusM, double excavatedRadiusM, double segmentLiningWeightKnPerM, double waterDensityKnM3, double soilPorePressureBar);
    static ShieldJammingResult evaluateShieldJammingRisk(double tunnelRadiusM, double overburdenStressMpa, double rockCompetenceFactor, double shieldLengthM, double shieldTaperClearanceMm, double maxAvailableThrustKn);
    static TunnelSeismicResult evaluateTunnelSeismicResponse(double peakGroundAccelG, double shearWaveVelocityMs, double liningRadiusM, double concreteElasticModulusGpa);
};

} // namespace Phase17
} // namespace AndroCAD

#endif // ANDROCAD_PHASE17_ENGINE_H
