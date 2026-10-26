#ifndef ANDROCAD_PHASE21_ENGINE_H
#define ANDROCAD_PHASE21_ENGINE_H

#include <string>
#include <vector>
#include <cmath>

namespace AndroCAD {
namespace Phase21 {

// Domain 1: Young-Laplace Capillary Pressure
struct CapillaryPressureResult {
    double youngLaplacePressurePa;
    double meniscusRadiusOfCurvatureMicrons;
    double capillaryRiseHeightMm;
    double contactAngleDegrees;
    bool spontaneousCapillaryFilling;
};

// Domain 2: Droplet Generation in Flow-Focusing Geometry
struct DropletGenerationResult {
    double capillaryNumberCa;
    double dropletVolumePicoliters;
    double generationFrequencyHz;
    double dropletDiameterMicrons;
    bool monodisperseDropletRegime;
};

// Domain 3: Dean Vortex Inertial Particle Focusing
struct DeanFocusingResult {
    double reynoldsNumberRe;
    double deanNumberDe;
    double inertialLiftForcePicoNewtons;
    double equilibriumFocusingBandwidthMicrons;
    bool singleStreamCellFocusingAchieved;
};

// Domain 4: Electro-Osmotic Flow (EOF) Dynamics
struct ElectroOsmoticFlowResult {
    double eofVelocityMmPerS;
    double debyeScreeningLengthNanometers;
    double zetaPotentialMilliVolts;
    double volumetricFlowRateNanolitersPerMin;
    bool plugFlowProfileMaintained;
};

// Domain 5: Diffusion Flux & Peclet Number Mixing
struct MicroDiffusionResult {
    double pecletNumberPe;
    double mixingLengthMm;
    double diffusionCoefficientM2S;
    double concentrationUniformityPercent;
    bool completeMolecularMixing;
};

// Domain 6: Pneumatic Quake PDMS Micro-Valve
struct QuakeValveResult {
    double membraneDeflectionMicrons;
    double valveClosingPressureKpa;
    double closingResponseTimeMs;
    double flowLeakageRatePicolitersPerS;
    bool hermeticMicrochannelSealing;
};

// Domain 7: Electrowetting-on-Dielectric (EWOD) Actuation
struct EwodActuationResult {
    double dynamicContactAngleDegrees;
    double electrowettingDrivingForceMicroNewtons;
    double dropletTranslationVelocityMmPerS;
    double thresholdActuationVoltageVolts;
    bool reliableDigitalDropletTransport;
};

// Domain 8: Continuous-Flow PCR Thermal Cycling
struct CfPcrThermalResult {
    double denaturationTimeSeconds;
    double annealingTimeSeconds;
    double extensionTimeSeconds;
    double cycleCountCompleted;
    bool dnaAmplificationSuccessful;
};

// Domain 9: Acoustic Radiation Force (SAW) Particle Sorting
struct SawSortingResult {
    double acousticRadiationForcePicoNewtons;
    double standingWaveWavelengthMicrons;
    double lateralParticleDeflectionMicrons;
    double sortingPurityPercent;
    bool highThroughputCellSeparation;
};

// Domain 10: Organ-on-a-Chip Endothelial Shear Stress
struct OrganChipShearResult {
    double wallShearStressDyneCm2;
    double cyclicMechanicalStrainPercent;
    double cellViabilityPercent;
    double reynoldsNumber;
    bool physiologicalEndothelialPhenotype;
};

class Phase21Engine {
public:
    static CapillaryPressureResult evaluateYoungLaplace(double surfaceTensionNm, double contactAngleDeg, double channelWidthUm, double channelHeightUm, double liquidDensityKgM3);
    static DropletGenerationResult evaluateDropletGeneration(double continuousFlowRateUlHr, double dispersedFlowRateUlHr, double continuousViscosityPaS, double interfacialTensionNm, double orificeWidthUm);
    static DeanFocusingResult evaluateDeanFocusing(double flowRateMlMin, double channelWidthUm, double channelHeightUm, double curveRadiusMm, double particleDiameterUm, double fluidViscosityPaS, double fluidDensityKgM3);
    static ElectroOsmoticFlowResult evaluateElectroOsmoticFlow(double electricFieldVm, double zetaPotentialMv, double dielectricPermittivity, double fluidViscosityPaS, double ionicConcentrationMolar);
    static MicroDiffusionResult evaluateMicroDiffusion(double flowVelocityMmS, double channelWidthUm, double diffusionCoeffM2S);
    static QuakeValveResult evaluateQuakeValve(double actuationPressureKpa, double membraneThicknessUm, double membraneWidthUm, double pdmsYoungsModulusMpa);
    static EwodActuationResult evaluateEwodActuation(double appliedVoltageVolts, double initialContactAngleDeg, double dielectricThicknessUm, double dielectricConstantK, double interfacialTensionNm);
    static CfPcrThermalResult evaluateCfPcrThermal(double flowRateUlMin, double denaturationLengthMm, double annealingLengthMm, double extensionLengthMm, double totalCycles);
    static SawSortingResult evaluateSawSorting(double acousticFrequencyMhz, double acousticPowerWatts, double particleRadiusUm, double particleDensityKgM3, double fluidDensityKgM3, double soundSpeedFluidMs);
    static OrganChipShearResult evaluateOrganChipShear(double volumetricFlowRateUlMin, double channelWidthUm, double channelHeightUm, double fluidViscosityPaS, double cyclicStretchPercent);
};

} // namespace Phase21
} // namespace AndroCAD

#endif // ANDROCAD_PHASE21_ENGINE_H
