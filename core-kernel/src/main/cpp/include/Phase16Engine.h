#ifndef ANDROCAD_PHASE16_ENGINE_H
#define ANDROCAD_PHASE16_ENGINE_H

#include <string>
#include <vector>
#include <cmath>

namespace AndroCAD {
namespace Phase16 {

// Domain 1: GIS Electric Field Stress & Dielectric Breakdown
struct GisFieldStressResult {
    double maxElectricFieldKvm;
    double meanElectricFieldKvm;
    double schwaigerFactor;
    double criticalBreakdownFieldKvm;
    double dielectricSafetyMargin;
    bool withstandsLightningImpulse;
};

// Domain 2: Arc Plasma Magnetohydrodynamics & Energy Balance
struct ArcPlasmaResult {
    double centralArcTemperatureKelvin;
    double arcConductivitySiemensM;
    double arcVoltageVolts;
    double radiativeEnergyLossFraction;
    bool arcQuenchedSuccessfully;
};

// Domain 3: Transient Recovery Voltage (TRV) Interruption
struct TrvInterruptionResult {
    double peakTrvKv;
    double timeToPeakMicroseconds;
    double rrrvKvPerMicrosecond;
    double postArcCurrentAmps;
    bool meetsIec62271_100;
};

// Domain 4: SF6 Alternative Gas Mixture (C4-FN / CO2 / O2)
struct EcoGasMixtureResult {
    double dielectricStrengthRatioToSf6;
    double globalWarmingPotentialGwp;
    double liquefactionTempCelsius;
    double safeOperatingPressureMpa;
    bool compliantWithEuFgasRegulation;
};

// Domain 5: Very Fast Transient Overvoltage (VFTO)
struct VftoDynamicsResult {
    double peakVftoPu;
    double wavefrontRiseTimeNanoseconds;
    double reflectionCoefficient;
    double maxInternalBarrierStressKv;
    bool spacerInsulatorProtected;
};

// Domain 6: Vacuum Interrupter AMF/RMF Arc Control
struct VacuumInterrupterResult {
    double axialMagneticFieldTesla;
    double criticalAmfThresholdTesla;
    double contactErosionRateMmPerKa;
    double diffuseArcStabilityIndex;
    bool avoidsAnodeSpotMelting;
};

// Domain 7: Busbar Electrodynamic Short-Circuit Forces
struct ElectrodynamicForceResult {
    double peakShortCircuitCurrentKa;
    double peakPeakForceKnPerMeter;
    double busbarBendingStressMpa;
    double supportInsulatorCantileverLoadKn;
    bool withstands63KaShortCircuit;
};

// Domain 8: Capacitive Graded Bushing & Partial Discharge
struct BushingGradingResult {
    double maxRadialFieldKvm;
    double maxAxialFieldKvm;
    double partialDischargePicoCoulombs;
    double capacitancePicoFarads;
    bool meetsIec60137;
};

// Domain 9: Operating Mechanism Kinematics & Opening Time
struct SwitchgearKinematicsResult {
    double contactSeparationVelocityMs;
    double totalOpeningTimeMilliseconds;
    double strokeTravelDistanceMm;
    double latchReleaseDelayMs;
    bool highSpeedInterruptionMet;
};

// Domain 10: Arc Flash Blast Pressure & Thermal Energy
struct ArcFlashHazardResult {
    double incidentEnergyCalCm2;
    double flashProtectionBoundaryMeters;
    double peakBlastPressureKpa;
    int ppeCategoryRequired;
    bool internalArcClassifiedIac;
};

class Phase16Engine {
public:
    static GisFieldStressResult evaluateGisElectricField(double appliedVoltageKv, double innerRadiusMm, double outerRadiusMm, double gasPressureMpa);
    static ArcPlasmaResult simulateArcPlasma(double arcCurrentAmps, double arcLengthMm, double blastPressureMpa, double gasSonicVelocityMs);
    static TrvInterruptionResult evaluateTrvInterruption(double shortCircuitCurrentKa, double systemInductanceMh, double parallelCapacitanceNf, double powerFrequencyHz);
    static EcoGasMixtureResult evaluateEcoGasMixture(double c4fnMolePercent, double co2MolePercent, double o2MolePercent, double totalPressureMpa);
    static VftoDynamicsResult calculateVftoWavefront(double strikeVoltageKv, double surgeImpedanceOhms, double busLengthMeters, double sparkResistanceOhms);
    static VacuumInterrupterResult evaluateVacuumArc(double interruptionCurrentKa, double contactDiameterMm, double coilTurns, double contactGapMm);
    static ElectrodynamicForceResult calculateBusbarForces(double rmsCurrentKa, double phaseSpacingMm, double spanLengthMm, double busbarModulusZcm3);
    static BushingGradingResult evaluateBushingStress(double ratedVoltageKv, double condenserLayers, double coreLengthMm, double dielectricPermittivity);
    static SwitchgearKinematicsResult simulateOperatingMechanism(double springPreloadN, double movingMassKg, double travelStrokeMm, double oilDashpotDamping);
    static ArcFlashHazardResult evaluateArcFlashHazard(double faultCurrentKa, double arcDurationSeconds, double workingDistanceMm, double enclosureDimensionMm);
};

} // namespace Phase16
} // namespace AndroCAD

#endif // ANDROCAD_PHASE16_ENGINE_H
