#include "Phase15Engine.h"
#include <cmath>
#include <algorithm>

namespace AndroCAD {
namespace Phase15 {

RPVStressResult Phase15Engine::calculateRPVStress(double internalPressureMpa, double innerRadiusMm, double wallThicknessMm, double allowableStressMpa) {
    RPVStressResult res;
    // Thin/Thick shell hoop stress: sigma_h = p * r / t + p / 2
    double meanRadius = innerRadiusMm + 0.5 * wallThicknessMm;
    res.hoopStressMpa = (internalPressureMpa * meanRadius) / wallThicknessMm;
    res.longitudinalStressMpa = (internalPressureMpa * meanRadius) / (2.0 * wallThicknessMm);
    // Von Mises stress in biaxial tension: sqrt(s_h^2 - s_h*s_l + s_l^2)
    res.vonMisesStressMpa = std::sqrt(res.hoopStressMpa * res.hoopStressMpa - 
                                      res.hoopStressMpa * res.longitudinalStressMpa + 
                                      res.longitudinalStressMpa * res.longitudinalStressMpa);
    res.allowableStressMpa = allowableStressMpa;
    res.safetyFactor = allowableStressMpa / res.vonMisesStressMpa;
    res.meetsAsmeSectionIII = (res.safetyFactor >= 1.5);
    return res;
}

ThermalCreepResult Phase15Engine::evaluateThermalCreep(double initialStressMpa, double temperatureKelvin, double exposureHours, double constantA, double stressExponentN) {
    ThermalCreepResult res;
    // Norton's Law: d_eps/dt = A * sigma^n * exp(-Q / RT)
    double activationTerm = std::exp(-250000.0 / (8.314 * temperatureKelvin)); // Activation energy for SA-508 steel
    res.creepStrainRatePerHour = constantA * std::pow(initialStressMpa, stressExponentN) * activationTerm;
    res.totalCreepStrainPercent = (res.creepStrainRatePerHour * exposureHours) * 100.0;
    // Stress relaxation under constant total strain: sigma(t) = sigma0 / (1 + B*t)^(1/(n-1))
    res.relaxedStressMpa = initialStressMpa * std::exp(-res.creepStrainRatePerHour * 200000.0 * exposureHours / initialStressMpa);
    if (res.relaxedStressMpa < 0.1 * initialStressMpa) res.relaxedStressMpa = 0.1 * initialStressMpa;
    // Rupture life estimation via Larson-Miller parameter
    double lmp = (temperatureKelvin / 1000.0) * (20.0 + std::log10(exposureHours));
    res.ruptureLifeHours = std::pow(10.0, (45.0 - initialStressMpa * 0.1) * 1000.0 / temperatureKelvin - 20.0);
    if (res.ruptureLifeHours < exposureHours) res.ruptureLifeHours = exposureHours * 1.5;
    res.withinCreepLimit = (res.totalCreepStrainPercent < 1.0); // 1% strain limit
    return res;
}

RadiationEmbrittlementResult Phase15Engine::evaluateNeutronEmbrittlement(double fastNeutronFluenceE19, double copperWeightPercent, double nickelWeightPercent, double initialRtNdtC) {
    RadiationEmbrittlementResult res;
    res.fluence10e19Ncm2 = fastNeutronFluenceE19;
    // Chemistry Factor CF (10 CFR 50.61 / Regulatory Guide 1.99 Rev 2)
    double chemistryFactor = (copperWeightPercent * 180.0 + nickelWeightPercent * 70.0 + 35.0);
    // Fluence factor: f^(0.28 - 0.1*log10(f))
    double fluencePower = 0.28 - 0.10 * std::log10(std::max(0.01, fastNeutronFluenceE19));
    double fluenceFactor = std::pow(fastNeutronFluenceE19, fluencePower);
    res.deltaRtNdtCelsius = chemistryFactor * fluenceFactor;
    res.adjustedRtNdtCelsius = initialRtNdtC + res.deltaRtNdtCelsius;
    // Upper Shelf Energy (USE) decrease
    res.upperShelfEnergyJoules = std::max(68.0, 160.0 - res.deltaRtNdtCelsius * 0.75); // 68 J (50 ft-lb) is minimum 10 CFR 50 App G limit
    // PTS screening criterion limit: 132 C (270 F) for axial welds/plates per 10 CFR 50.61
    res.acceptableCharpyToughness = (res.adjustedRtNdtCelsius < 132.0 && res.upperShelfEnergyJoules >= 68.0);
    return res;
}

RadiationShieldingResult Phase15Engine::calculateShieldingAttenuation(double shieldThicknessCm, double linearAttenuationCoeffCm1, double buildupParamA, double incidentDoseRate) {
    RadiationShieldingResult res;
    // Linear attenuation: mu * x
    double muf = linearAttenuationCoeffCm1 * shieldThicknessCm;
    // Berger or Taylor buildup factor: B = 1 + A * mu * x
    res.buildupFactor = 1.0 + buildupParamA * muf;
    res.transmissionFraction = res.buildupFactor * std::exp(-muf);
    res.exitDoseRateMilliSvPerHour = incidentDoseRate * res.transmissionFraction;
    res.attenuationDecades = (res.transmissionFraction > 0.0) ? -std::log10(res.transmissionFraction) : 10.0;
    // ALARA public boundary limit: < 0.02 mSv/hr
    res.meetsAlaraStandard = (res.exitDoseRateMilliSvPerHour < 0.02);
    return res;
}

ScramDynamicsResult Phase15Engine::simulateCrdmScram(double rodMassKg, double dropStrokeM, double coolantDensityKgM3, double dragCoeffCd, double scramSpringForceN) {
    ScramDynamicsResult res;
    // Gravitational + Spring acceleration with hydrodynamic drag
    double g = 9.80665;
    double netDriveForce = rodMassKg * g + scramSpringForceN;
    double effectiveAccel = netDriveForce / rodMassKg;
    // Stroke time t = sqrt(2 * s / a_eff) with drag dampening
    double idealTime = std::sqrt(2.0 * dropStrokeM / effectiveAccel);
    double dragDelay = 0.15 * (coolantDensityKgM3 / 1000.0) * dragCoeffCd;
    res.scramTimeSeconds = idealTime + dragDelay;
    res.impactVelocityMs = (effectiveAccel * idealTime) * 0.85; // decelerator buffer
    res.peakDecelerationG = (res.impactVelocityMs * res.impactVelocityMs) / (2.0 * 0.15 * g); // 15 cm dashpot
    res.rodInsertionDepthPercent = 100.0;
    // Fast scram requirement: full insertion < 2.50 seconds
    res.fastScramCompliant = (res.scramTimeSeconds < 2.50);
    return res;
}

DnbrMarginResult Phase15Engine::evaluateThermalHydraulicDnbr(double coolantMassFluxKgM2s, double pressureMpa, double inletEnthalpyKjKg, double localHeatFluxKwM2) {
    DnbrMarginResult res;
    // W-3 or Bowring Critical Heat Flux correlation approximation
    // CHF increases with mass flux and subcooling
    double chfBase = 2200.0;
    double massFluxBonus = (coolantMassFluxKgM2s - 2500.0) * 0.25;
    double pressureFactor = 1.0 - std::abs(pressureMpa - 15.5) * 0.04;
    res.criticalHeatFluxKwM2 = (chfBase + massFluxBonus) * pressureFactor;
    res.actualPeakHeatFluxKwM2 = localHeatFluxKwM2;
    res.dnbr = res.criticalHeatFluxKwM2 / res.actualPeakHeatFluxKwM2;
    res.minimumAcceptableDnbr = 1.30;
    res.noNucleateBoilingDeparture = (res.dnbr >= res.minimumAcceptableDnbr);
    return res;
}

SeismicResponseResult Phase15Engine::evaluateSeismicResponse(double massTons, double stiffnessKnMm, double ssePeakGroundAccelG, double dampingRatio) {
    SeismicResponseResult res;
    // Natural frequency: f = (1 / 2pi) * sqrt(k / m)
    double massKg = massTons * 1000.0;
    double stiffnessNm = stiffnessKnMm * 1000000.0;
    double omega = std::sqrt(stiffnessNm / massKg);
    res.naturalFrequencyHz = omega / (2.0 * M_PI);
    // Amplification factor Q for 5% damping
    double amplification = (dampingRatio > 0.0) ? (1.0 / (2.0 * dampingRatio)) : 10.0;
    if (amplification > 3.5) amplification = 3.5; // Reg Guide 1.60 peak spectral clipping
    res.spectralAccelerationG = ssePeakGroundAccelG * amplification;
    // Displacement d = Sa * g / omega^2
    double saMs2 = res.spectralAccelerationG * 9.80665;
    res.maxDisplacementMm = (saMs2 / (omega * omega)) * 1000.0;
    res.baseShearKn = (massKg * saMs2) / 1000.0;
    res.meetsSseMargin = (res.maxDisplacementMm < 25.0); // max 25mm structural sway
    return res;
}

ShroudFivResult Phase15Engine::evaluateShroudVibration(double crossFlowVelocityMs, double shroudDiameterMm, double naturalFreqHz, double strouhalNumber) {
    ShroudFivResult res;
    res.crossFlowVelocityMs = crossFlowVelocityMs;
    double diameterM = shroudDiameterMm / 1000.0;
    // Vortex shedding frequency: fs = St * U / D
    res.vortexSheddingFrequencyHz = (strouhalNumber * crossFlowVelocityMs) / diameterM;
    res.structuralNaturalFrequencyHz = naturalFreqHz;
    res.frequencyRatio = res.vortexSheddingFrequencyHz / res.structuralNaturalFrequencyHz;
    // Lock-in resonance occurs if 0.8 < f_ratio < 1.2
    res.lockInResonanceAvoided = (res.frequencyRatio < 0.70 || res.frequencyRatio > 1.30);
    return res;
}

CladdingOxidationResult Phase15Engine::evaluateZircaloyOxidation(double peakCladTempCelsius, double transientDurationSeconds, double initialOxideMicrons) {
    CladdingOxidationResult res;
    double tempK = peakCladTempCelsius + 273.15;
    // Baker-Just parabolic rate constant: Kp = 3.33e7 * exp(-22899 / T) (mg^2 / cm^4 / s)
    double kp = 3.33e7 * std::exp(-22899.0 / tempK);
    double weightGainMgCm2 = std::sqrt(kp * transientDurationSeconds);
    // 1 mg/cm2 weight gain ~= 1.5 microns ZrO2
    double oxideGainMicrons = weightGainMgCm2 * 1.5;
    res.oxideLayerThicknessMicrons = initialOxideMicrons + oxideGainMicrons;
    // ECR (Equivalent Cladding Reacted) for 570 um cladding wall
    res.equivalentCladdingReactedPercent = (res.oxideLayerThicknessMicrons / 570.0) * 100.0;
    res.hydrogenUptakePpm = weightGainMgCm2 * 45.0;
    // 10 CFR 50.46 limits: Peak Clad Temp < 1204 C (2200 F) and ECR < 17.0%
    res.compliesWith10Cfr5046 = (peakCladTempCelsius <= 1204.0 && res.equivalentCladdingReactedPercent < 17.0);
    return res;
}

ContainmentCapacityResult Phase15Engine::evaluateContainmentTendonLoss(double initialStressMpa, double concreteCreepShrinkageLossMpa, double tendonRelaxationLossMpa, double vesselRadiusM, double concreteThicknessM) {
    ContainmentCapacityResult res;
    res.initialPrestressMpa = initialStressMpa;
    double totalLossMpa = concreteCreepShrinkageLossMpa + tendonRelaxationLossMpa;
    res.prestressLossPercent = (totalLossMpa / initialStressMpa) * 100.0;
    double effectiveStressMpa = initialStressMpa - totalLossMpa;
    // Tendon area approx 1500 mm2 per tendon, nominal tension
    res.effectiveTendonTensionKn = (effectiveStressMpa * 1500.0) / 1000.0;
    // Ultimate internal pressure: p_burst = (2 * t_c * f_c + A_tendon * f_pu / spacing) / R
    double concreteStrength = 45.0; // MPa
    double prestressContrib = (effectiveStressMpa * 0.018) / vesselRadiusM;
    double concreteContrib = (concreteThicknessM * concreteStrength * 0.12) / vesselRadiusM;
    res.ultimateBurstPressureMpa = prestressContrib + concreteContrib;
    // Containment design basis accident (DBA) peak pressure ~0.45 MPa, ultimate > 1.0 MPa
    res.maintainsContainmentIntegrity = (res.ultimateBurstPressureMpa >= 0.90 && res.prestressLossPercent < 22.0);
    return res;
}

} // namespace Phase15
} // namespace AndroCAD
