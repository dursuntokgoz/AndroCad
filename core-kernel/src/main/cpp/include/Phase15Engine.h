#ifndef ANDROCAD_PHASE15_ENGINE_H
#define ANDROCAD_PHASE15_ENGINE_H

#include <string>
#include <vector>
#include <cmath>

namespace AndroCAD {
namespace Phase15 {

// Domain 1: RPV ASME Section III Stress Analysis
struct RPVStressResult {
    double hoopStressMpa;
    double longitudinalStressMpa;
    double vonMisesStressMpa;
    double allowableStressMpa;
    double safetyFactor;
    bool meetsAsmeSectionIII;
};

// Domain 2: High-Temperature Thermal Creep Relaxation
struct ThermalCreepResult {
    double creepStrainRatePerHour;
    double totalCreepStrainPercent;
    double relaxedStressMpa;
    double ruptureLifeHours;
    bool withinCreepLimit;
};

// Domain 3: Fast Neutron Radiation Embrittlement
struct RadiationEmbrittlementResult {
    double fluence10e19Ncm2;
    double deltaRtNdtCelsius;
    double adjustedRtNdtCelsius;
    double upperShelfEnergyJoules;
    bool acceptableCharpyToughness;
};

// Domain 4: Gamma-Ray Shielding Attenuation
struct RadiationShieldingResult {
    double transmissionFraction;
    double buildupFactor;
    double exitDoseRateMilliSvPerHour;
    double attenuationDecades;
    bool meetsAlaraStandard;
};

// Domain 5: CRDM Emergency Scram Dynamics
struct ScramDynamicsResult {
    double scramTimeSeconds;
    double impactVelocityMs;
    double peakDecelerationG;
    double rodInsertionDepthPercent;
    bool fastScramCompliant;
};

// Domain 6: Thermal-Hydraulic DNBR Margin
struct DnbrMarginResult {
    double criticalHeatFluxKwM2;
    double actualPeakHeatFluxKwM2;
    double dnbr;
    double minimumAcceptableDnbr;
    bool noNucleateBoilingDeparture;
};

// Domain 7: Safe Shutdown Earthquake (SSE) Seismic Analysis
struct SeismicResponseResult {
    double naturalFrequencyHz;
    double spectralAccelerationG;
    double maxDisplacementMm;
    double baseShearKn;
    bool meetsSseMargin;
};

// Domain 8: Core Shroud Flow Induced Vibration (FIV)
struct ShroudFivResult {
    double vortexSheddingFrequencyHz;
    double structuralNaturalFrequencyHz;
    double frequencyRatio;
    double crossFlowVelocityMs;
    bool lockInResonanceAvoided;
};

// Domain 9: Zircaloy-4 High-Temp Cladding Oxidation
struct CladdingOxidationResult {
    double oxideLayerThicknessMicrons;
    double equivalentCladdingReactedPercent;
    double hydrogenUptakePpm;
    bool compliesWith10Cfr5046;
};

// Domain 10: Containment Vessel Prestressed Tendon Capacity
struct ContainmentCapacityResult {
    double initialPrestressMpa;
    double prestressLossPercent;
    double effectiveTendonTensionKn;
    double ultimateBurstPressureMpa;
    bool maintainsContainmentIntegrity;
};

class Phase15Engine {
public:
    static RPVStressResult calculateRPVStress(double internalPressureMpa, double innerRadiusMm, double wallThicknessMm, double allowableStressMpa);
    static ThermalCreepResult evaluateThermalCreep(double initialStressMpa, double temperatureKelvin, double exposureHours, double constantA, double stressExponentN);
    static RadiationEmbrittlementResult evaluateNeutronEmbrittlement(double fastNeutronFluenceE19, double copperWeightPercent, double nickelWeightPercent, double initialRtNdtC);
    static RadiationShieldingResult calculateShieldingAttenuation(double shieldThicknessCm, double linearAttenuationCoeffCm1, double buildupParamA, double incidentDoseRate);
    static ScramDynamicsResult simulateCrdmScram(double rodMassKg, double dropStrokeM, double coolantDensityKgM3, double dragCoeffCd, double scramSpringForceN);
    static DnbrMarginResult evaluateThermalHydraulicDnbr(double coolantMassFluxKgM2s, double pressureMpa, double inletEnthalpyKjKg, double localHeatFluxKwM2);
    static SeismicResponseResult evaluateSeismicResponse(double massTons, double stiffnessKnMm, double ssePeakGroundAccelG, double dampingRatio);
    static ShroudFivResult evaluateShroudVibration(double crossFlowVelocityMs, double shroudDiameterMm, double naturalFreqHz, double strouhalNumber);
    static CladdingOxidationResult evaluateZircaloyOxidation(double peakCladTempCelsius, double transientDurationSeconds, double initialOxideMicrons);
    static ContainmentCapacityResult evaluateContainmentTendonLoss(double initialStressMpa, double concreteCreepShrinkageLossMpa, double tendonRelaxationLossMpa, double vesselRadiusM, double concreteThicknessM);
};

} // namespace Phase15
} // namespace AndroCAD

#endif // ANDROCAD_PHASE15_ENGINE_H
