#pragma once

#include <string>
#include <vector>
#include <cmath>

namespace AndroCAD {
namespace Phase24 {

struct DfnElectrochemicalResult {
    double cellVoltageVolts;
    double stateOfChargePercent;
    double seiFilmThicknessNanometers;
    double overpotentialMilliVolts;
    double internalResistanceMilliOhms;
    bool electrochemicalKineticsStable;
};

struct ThermalRunawayResult {
    double peakTemperatureCelsius;
    double maxHeatGenerationRateWatts;
    double timeToThermalRunawaySeconds;
    double gasGenerationVolumeLiters;
    bool thermalRunawayContained;
};

struct PackCrashResult {
    double energyAbsorptionKiloJoules;
    double peakCrushForceKiloNewtons;
    double maxCellIntrusionDistanceMm;
    double plasticDeformationMarginMm;
    bool unEceR100CrashApproved;
};

struct VentingGasResult {
    double burstDiscRupturePressureBar;
    double ventingGasVelocityMs;
    double ventThroatMachNumber;
    double plenumExhaustPressureKpa;
    bool safeOverpressureReliefAchieved;
};

struct ThermalBarrierResult {
    double aerogelThermalConductivityWMK;
    double maxAdjacentCellTemperatureC;
    double heatFluxAttenuationPercent;
    bool cascadingPropagationPrevented;
};

struct ColdPlateCoolingResult {
    double coolantPressureDropKpa;
    double maxCellTemperatureC;
    double moduleTempNonUniformityDeltaC;
    double heatTransferCoeffWPerM2K;
    bool optimalThermalUniformityAchieved;
};

struct BusbarJointResult {
    double jointContactResistanceMicroOhms;
    double jouleHeatingDissipationWatts;
    double maxJointTemperatureRiseK;
    double shearTensileStrengthKn;
    bool highConductivityJointCertified;
};

struct PyroFuseResult {
    double clearingTimeMilliseconds;
    double peakShortCircuitCurrentKa;
    double arcVoltageVolts;
    double energyDissipationKiloJoules;
    bool highVoltageIsolationGuaranteed;
};

struct SwellingBreathingResult {
    double cellSwellingPressureMpa;
    double tieRodTensileStressMpa;
    double endPlateDeflectionMm;
    double remainingCompressionMarginMpa;
    bool mechanicalIntegrityPreserved;
};

struct EisDiagnosticResult {
    double chargeTransferResistanceMilliOhms;
    double solidElectrolyteInterphaseResMilliOhms;
    double ultrasonicTimeOfFlightNs;
    double stateOfHealthPercent;
    bool un38_3ComplianceCertified;
};

class Phase24Engine {
public:
    static DfnElectrochemicalResult evaluateDfnElectrochemistry(
        double appliedCurrentAmps,
        double nominalCapacityAh,
        double ambientTempK,
        double cycleCount
    );

    static ThermalRunawayResult evaluateThermalRunaway(
        double initialCellTempC,
        double stateOfChargeFraction,
        double triggerEnergyJoules,
        double cellMassKg
    );

    static PackCrashResult evaluatePackCrash(
        double impactVelocityKmh,
        double vehicleMassKg,
        double crushLengthMm,
        double enclosureYieldStressMpa
    );

    static VentingGasResult evaluateVentingGas(
        double peakInternalPressureBar,
        double gasTempK,
        double ventOrificeDiameterMm,
        double molecularWeightKgMol
    );

    static ThermalBarrierResult evaluateThermalBarrier(
        double thermalRunawayTempC,
        double barrierThicknessMm,
        double aerogelConductivityWMK,
        double adjacentCellHeatSinkConductance
    );

    static ColdPlateCoolingResult evaluateColdPlateCooling(
        double coolantFlowRateLpm,
        double totalHeatLoadWatts,
        double channelWidthMm,
        double channelHeightMm,
        double coolantViscosityPaS
    );

    static BusbarJointResult evaluateBusbarJoint(
        double continuousCurrentAmps,
        double weldAreaMm2,
        double electricalConductivityIacsPercent,
        double ambientTempC
    );

    static PyroFuseResult evaluatePyroFuse(
        double shortCircuitCurrentKa,
        double circuitInductanceMicroHenries,
        double batteryPackVoltageVolts,
        double pyrotechnicIgnitionDelayMs
    );

    static SwellingBreathingResult evaluateSwellingBreathing(
        double stateOfChargeFraction,
        double cellCycleAge,
        double tieRodCrossSectionMm2,
        double endPlateThicknessMm
    );

    static EisDiagnosticResult evaluateEisDiagnostic(
        double acExcitationFrequencyHz,
        double measuredRealImpedanceMilliOhms,
        double measuredImagImpedanceMilliOhms,
        double acousticBaselineTimeNs
    );
};

} // namespace Phase24
} // namespace AndroCAD
