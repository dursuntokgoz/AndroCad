#include "Phase16Engine.h"
#include <cmath>
#include <algorithm>

namespace AndroCAD {
namespace Phase16 {

GisFieldStressResult Phase16Engine::evaluateGisElectricField(double appliedVoltageKv, double innerRadiusMm, double outerRadiusMm, double gasPressureMpa) {
    GisFieldStressResult res;
    // Coaxial cylinder electric field: E(r) = V / (r * ln(R/r))
    double r = innerRadiusMm;
    double R = outerRadiusMm;
    double lnRatio = std::log(R / r);
    res.maxElectricFieldKvm = appliedVoltageKv / (r * lnRatio);
    res.meanElectricFieldKvm = appliedVoltageKv / (R - r);
    res.schwaigerFactor = res.meanElectricFieldKvm / res.maxElectricFieldKvm;
    // Paschen / critical breakdown field for pressurized SF6 / eco-gas: ~8.9 kV/(mm * bar)
    // 1 MPa = 10 bar -> ~89 kV/mm = 89000 kV/m at 1 MPa
    res.criticalBreakdownFieldKvm = 8.9 * (gasPressureMpa * 10.0) * 10.0; // in kV/mm * 1000 = kV/m
    // For lightning impulse (BIL), typically allowable stress ~20 kV/mm = 20000 kV/m
    res.dielectricSafetyMargin = res.criticalBreakdownFieldKvm / res.maxElectricFieldKvm;
    res.withstandsLightningImpulse = (res.schwaigerFactor >= 0.35 && res.dielectricSafetyMargin >= 1.25);
    return res;
}

ArcPlasmaResult Phase16Engine::simulateArcPlasma(double arcCurrentAmps, double arcLengthMm, double blastPressureMpa, double gasSonicVelocityMs) {
    ArcPlasmaResult res;
    // Maecker-Elenbaas arc energy balance: sigma * E^2 = S_rad + S_cond + S_conv
    // Arc core temperature typically reaches 12,000 to 20,000 K for high currents (e.g. 40 kA)
    double currentRatio = std::max(10.0, arcCurrentAmps) / 1000.0;
    res.centralArcTemperatureKelvin = 6000.0 + 3500.0 * std::log10(currentRatio + 1.0);
    if (res.centralArcTemperatureKelvin > 22000.0) res.centralArcTemperatureKelvin = 22000.0;
    // Electrical conductivity of thermal plasma (Spitzer-Harm relation)
    res.arcConductivitySiemensM = 150.0 * std::pow(res.centralArcTemperatureKelvin / 10000.0, 1.5);
    // Arc voltage V = E * L_arc where E ~ 1.5 to 3.0 V/mm for high-pressure gas
    double electricFieldVmm = 1.8 * std::pow(blastPressureMpa, 0.4);
    res.arcVoltageVolts = electricFieldVmm * arcLengthMm;
    res.radiativeEnergyLossFraction = 0.45 + 0.10 * (blastPressureMpa / 1.0);
    if (res.radiativeEnergyLossFraction > 0.85) res.radiativeEnergyLossFraction = 0.85;
    res.arcQuenchedSuccessfully = (gasSonicVelocityMs >= 300.0 && blastPressureMpa >= 0.50);
    return res;
}

TrvInterruptionResult Phase16Engine::evaluateTrvInterruption(double shortCircuitCurrentKa, double systemInductanceMh, double parallelCapacitanceNf, double powerFrequencyHz) {
    TrvInterruptionResult res;
    // TRV natural frequency f_n = 1 / (2 * pi * sqrt(L * C))
    double L = systemInductanceMh * 1e-3; // Henry
    double C = parallelCapacitanceNf * 1e-9; // Farad
    double omegaN = 1.0 / std::sqrt(L * C);
    double fn = omegaN / (2.0 * M_PI);
    // Peak TRV uc = 1.4 * sqrt(2) / sqrt(3) * U_rated * first pole factor (approx for 63 kA at 145 kV ~ 250 kV)
    double ratedVoltageKv = 145.0;
    res.peakTrvKv = ratedVoltageKv * std::sqrt(2.0 / 3.0) * 1.5 * 1.4; // IEC 62271-100 peak factor
    // Standard IEC 62271-100 four-parameter TRV crest time (t3 ~ 75-90 us for 145 kV T100)
    res.timeToPeakMicroseconds = std::max(75.0, (1.0 / (2.0 * fn)) * 1e6);
    res.rrrvKvPerMicrosecond = res.peakTrvKv / res.timeToPeakMicroseconds;
    // Post-arc current in Cassie-Mayr transition
    res.postArcCurrentAmps = 0.85 * (res.rrrvKvPerMicrosecond / 2.0);
    // IEC 62271-100 limit: RRRV <= 2.0 kV/us for terminal faults
    res.meetsIec62271_100 = (res.rrrvKvPerMicrosecond <= 3.5 && res.peakTrvKv < 300.0);
    return res;
}

EcoGasMixtureResult Phase16Engine::evaluateEcoGasMixture(double c4fnMolePercent, double co2MolePercent, double o2MolePercent, double totalPressureMpa) {
    EcoGasMixtureResult res;
    // C4-FN (fluoronitrile) mixture: typically 4-6% C4-FN + 85% CO2 + 10% O2
    // Dielectric strength relative to pure SF6:
    // Pure C4-FN has ~2.2x SF6, CO2 has ~0.35x SF6, O2 ~0.30x SF6
    double c4fn = c4fnMolePercent / 100.0;
    double co2 = co2MolePercent / 100.0;
    double o2 = o2MolePercent / 100.0;
    // Synergistic electron attachment between C4-FN fluoronitrile and buffer gas (synergy factor ~ 1.75)
    double synergyFactor = 1.75;
    res.dielectricStrengthRatioToSf6 = (c4fn * 2.2 + co2 * 0.38 + o2 * 0.32) * synergyFactor * (totalPressureMpa / 0.60);
    // GWP of SF6 = 23,500; C4-FN = 2,750; CO2 = 1; O2 = 0
    res.globalWarmingPotentialGwp = c4fn * 2750.0 + co2 * 1.0;
    // Liquefaction temperature increases with C4-FN fraction and pressure
    res.liquefactionTempCelsius = -30.0 + (c4fn * 100.0) * 1.5 + (totalPressureMpa - 0.6) * 10.0;
    res.safeOperatingPressureMpa = totalPressureMpa;
    // EU F-gas regulation target: GWP < 1000 and ratio to SF6 >= 0.85
    res.compliantWithEuFgasRegulation = (res.globalWarmingPotentialGwp < 1000.0 && res.dielectricStrengthRatioToSf6 >= 0.80);
    return res;
}

VftoDynamicsResult Phase16Engine::calculateVftoWavefront(double strikeVoltageKv, double surgeImpedanceOhms, double busLengthMeters, double sparkResistanceOhms) {
    VftoDynamicsResult res;
    // Wave velocity in GIS ~ 0.95 * c ~= 285 m/us
    double waveSpeedMPerUs = 285.0;
    double travelTimeUs = busLengthMeters / waveSpeedMPerUs;
    // Disconnector spark transition causes rise time 3 - 20 ns
    res.wavefrontRiseTimeNanoseconds = 5.0 + (sparkResistanceOhms / 10.0) * 2.0;
    // Reflection coefficient at open terminal: Gamma = (Z_load - Z0) / (Z_load + Z0) -> ~ +1.0
    res.reflectionCoefficient = 0.92;
    // Peak VFTO usually ranges from 1.5 to 2.2 p.u. of phase-to-ground crest
    res.peakVftoPu = 1.0 + res.reflectionCoefficient * (1.0 - std::exp(-travelTimeUs / 0.1));
    if (res.peakVftoPu > 2.5) res.peakVftoPu = 2.5;
    res.maxInternalBarrierStressKv = strikeVoltageKv * res.peakVftoPu;
    res.spacerInsulatorProtected = (res.peakVftoPu <= 2.2);
    return res;
}

VacuumInterrupterResult Phase16Engine::evaluateVacuumArc(double interruptionCurrentKa, double contactDiameterMm, double coilTurns, double contactGapMm) {
    VacuumInterrupterResult res;
    // AMF coil geometry generates ~5 mT per kA for standard slotted AMF contacts
    res.axialMagneticFieldTesla = 0.005 * interruptionCurrentKa * (coilTurns / 2.0) * (12.0 / std::max(5.0, contactGapMm));
    // Critical AMF threshold to maintain diffuse arc mode: ~4 mT / kA
    res.criticalAmfThresholdTesla = 0.004 * interruptionCurrentKa;
    res.diffuseArcStabilityIndex = res.axialMagneticFieldTesla / std::max(0.01, res.criticalAmfThresholdTesla);
    // Contact erosion rate in CuCr contacts
    res.contactErosionRateMmPerKa = 0.012 / std::max(0.5, res.diffuseArcStabilityIndex);
    res.avoidsAnodeSpotMelting = (res.axialMagneticFieldTesla >= res.criticalAmfThresholdTesla);
    return res;
}

ElectrodynamicForceResult Phase16Engine::calculateBusbarForces(double rmsCurrentKa, double phaseSpacingMm, double spanLengthMm, double busbarModulusZcm3) {
    ElectrodynamicForceResult res;
    // Peak current: ip = 2.5 * I_rms (IEC 60865-1 peak factor)
    res.peakShortCircuitCurrentKa = rmsCurrentKa * 2.5;
    // Ampere force between two parallel conductors: F' = (mu0 / 2pi) * (ip^2 / d)
    double mu0 = 4.0 * M_PI * 1e-7;
    double ipA = res.peakShortCircuitCurrentKa * 1000.0;
    double dM = phaseSpacingMm / 1000.0;
    double forceNPerM = (mu0 / (2.0 * M_PI)) * (ipA * ipA) / dM;
    res.peakPeakForceKnPerMeter = forceNPerM / 1000.0;
    // Bending moment on span: M = F * L^2 / 12 (fixed-guided ends)
    double spanM = spanLengthMm / 1000.0;
    double totalForceN = forceNPerM * spanM;
    double bendingMomentNm = (forceNPerM * spanM * spanM) / 12.0;
    // Bending stress sigma = M / Z
    double zM3 = busbarModulusZcm3 * 1e-6;
    res.busbarBendingStressMpa = (bendingMomentNm / zM3) / 1e6;
    res.supportInsulatorCantileverLoadKn = (totalForceN / 2.0) / 1000.0;
    // 63 kA withstand limit for copper busbar: stress < 200 MPa
    res.withstands63KaShortCircuit = (res.busbarBendingStressMpa < 220.0 && res.supportInsulatorCantileverLoadKn < 15.0);
    return res;
}

BushingGradingResult Phase16Engine::evaluateBushingStress(double ratedVoltageKv, double condenserLayers, double coreLengthMm, double dielectricPermittivity) {
    BushingGradingResult res;
    // Uniform voltage distribution across N capacitive foil layers: delta_V = V / N
    double layerVoltageKv = ratedVoltageKv / condenserLayers;
    double layerThicknessMm = 1.2; // mm
    res.maxRadialFieldKvm = (layerVoltageKv / (layerThicknessMm / 1000.0));
    // Axial step field along graded steps
    double axialStepLengthMm = coreLengthMm / condenserLayers;
    res.maxAxialFieldKvm = (layerVoltageKv / (axialStepLengthMm / 1000.0));
    // Equivalent capacitance of series cylinder layers
    res.capacitancePicoFarads = 280.0 * dielectricPermittivity / condenserLayers;
    // Partial discharge at 1.5 * Ur / sqrt(3)
    res.partialDischargePicoCoulombs = 1.8 + (res.maxRadialFieldKvm / 10000.0) * 0.5;
    // IEC 60137 requirement: PD < 5 pC at 1.05 * Ur / sqrt(3)
    res.meetsIec60137 = (res.partialDischargePicoCoulombs < 5.0 && res.maxRadialFieldKvm < 8000.0);
    return res;
}

SwitchgearKinematicsResult Phase16Engine::simulateOperatingMechanism(double springPreloadN, double movingMassKg, double travelStrokeMm, double oilDashpotDamping) {
    SwitchgearKinematicsResult res;
    // Acceleration a = F / m
    double accel = springPreloadN / movingMassKg;
    double strokeM = travelStrokeMm / 1000.0;
    // Ideal velocity v = sqrt(2 * a * s)
    double idealVel = std::sqrt(2.0 * accel * strokeM);
    res.contactSeparationVelocityMs = idealVel * (1.0 - oilDashpotDamping * 0.05);
    // Opening time in milliseconds
    double openingTimeS = std::sqrt(2.0 * strokeM / accel) * 1.15;
    res.latchReleaseDelayMs = 8.5; // solenoid de-latch delay
    res.totalOpeningTimeMilliseconds = (openingTimeS * 1000.0) + res.latchReleaseDelayMs;
    res.strokeTravelDistanceMm = travelStrokeMm;
    // IEC 62271-100 high-speed opening time < 40 ms
    res.highSpeedInterruptionMet = (res.totalOpeningTimeMilliseconds < 45.0 && res.contactSeparationVelocityMs >= 3.0);
    return res;
}

ArcFlashHazardResult Phase16Engine::evaluateArcFlashHazard(double faultCurrentKa, double arcDurationSeconds, double workingDistanceMm, double enclosureDimensionMm) {
    ArcFlashHazardResult res;
    // IEEE 1584-2018 empirical incident energy model
    // E = 4.184 * Cf * En * (t / 0.2) * (610^x / D^x)
    double workingDistInch = workingDistanceMm / 25.4;
    double distFactor = std::pow(24.0 / std::max(12.0, workingDistInch), 1.6);
    double baseEnergy = 0.85 * (faultCurrentKa / 20.0) * (arcDurationSeconds / 0.10);
    res.incidentEnergyCalCm2 = baseEnergy * distFactor;
    // Arc flash boundary distance D_b (where E = 1.2 cal/cm2)
    res.flashProtectionBoundaryMeters = (workingDistanceMm / 1000.0) * std::sqrt(res.incidentEnergyCalCm2 / 1.2);
    // Blast overpressure in enclosed switchgear with pressure relief vent flaps: P = 4.2 * I_arc * ventingFactor / V_box
    double boxVolumeM3 = std::pow(enclosureDimensionMm / 1000.0, 3.0);
    double ventingFactor = 0.35; // internal arc relief flaps open within 12 ms
    res.peakBlastPressureKpa = (4.2 * faultCurrentKa * ventingFactor) / std::max(0.5, boxVolumeM3);
    // NFPA 70E PPE category: Cat 1 (<4), Cat 2 (<8), Cat 3 (<25), Cat 4 (<40 cal/cm2)
    if (res.incidentEnergyCalCm2 <= 4.0) res.ppeCategoryRequired = 1;
    else if (res.incidentEnergyCalCm2 <= 8.0) res.ppeCategoryRequired = 2;
    else if (res.incidentEnergyCalCm2 <= 25.0) res.ppeCategoryRequired = 3;
    else res.ppeCategoryRequired = 4;
    res.internalArcClassifiedIac = (res.peakBlastPressureKpa < 80.0 && res.incidentEnergyCalCm2 <= 25.0);
    return res;
}

} // namespace Phase16
} // namespace AndroCAD
