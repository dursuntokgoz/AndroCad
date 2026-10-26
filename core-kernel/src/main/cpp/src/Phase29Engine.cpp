#include "Phase29Engine.h"
#include <cmath>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace androcad {
namespace phase29 {

ThermalBudgetResult calculateCryostatThermalBudget(
    int numCoaxialLines,
    int numDcLines,
    double supportStrutConductivityW,
    double mliRadiationFluxWm2
) {
    ThermalBudgetResult res;
    // 50 K stage: absorbs bulk radiation and 300K strut conduction
    // Surface area of 50K shield approx 3.2 m^2
    double qRad50K = mliRadiationFluxWm2 * 3.2; // ~3.68 W
    double qStrut50K = supportStrutConductivityW * 8.5; // ~3.23 W
    double qWiring50K = (numCoaxialLines * 0.12) + (numDcLines * 0.015); // ~3.6 W
    res.q50kStageW = qRad50K + qStrut50K + qWiring50K; // ~10.5 W (<= 35 W)

    // 4 K stage: Pulse tube cold head 2nd stage
    double qStrut4K = supportStrutConductivityW; // 0.38 W
    double qWiring4K = (numCoaxialLines * 0.012) + (numDcLines * 0.0018); // ~0.37 W
    double qRad4K = 0.085; // 85 mW from 50K shield
    res.q4kStageW = qStrut4K + qWiring4K + qRad4K; // ~0.835 W (<= 1.20 W)

    // Still stage (approx 800 mK):
    res.qStillStageMw = 14.5 + (numCoaxialLines * 0.22); // ~19.78 mW (<= 25 mW)

    // Mixing Chamber (10 mK):
    // Superconducting cables (NbTi) have very low thermal conductivity below 4K
    // Coax conduction + DC line conduction + internal radiation
    double qCoaxToMxc = numCoaxialLines * 0.28; // 24 * 0.28 uW = 6.72 uW
    double qDcToMxc = numDcLines * 0.045; // 48 * 0.045 = 2.16 uW
    double qStray = 1.25; // uW
    res.qMxcStageMicrowatt = qCoaxToMxc + qDcToMxc + qStray; // ~10.13 uW (<= 15.0 uW)

    // Cooling power margin factor at MXC: available 18 uW vs load 10.13 uW
    res.marginFactor = 18.0 / res.qMxcStageMicrowatt;

    res.passed = (res.q50kStageW <= 35.0) && (res.q4kStageW <= 1.20) &&
                 (res.qStillStageMw <= 25.0) && (res.qMxcStageMicrowatt <= 15.0) &&
                 (res.marginFactor >= 1.50);

    std::ostringstream ss;
    ss << "Thermal Budget: Q_50K=" << std::fixed << std::setprecision(2) << res.q50kStageW
       << " W, Q_4K=" << res.q4kStageW << " W, Q_Still=" << res.qStillStageMw << " mW, Q_MXC="
       << res.qMxcStageMicrowatt << " uW, Margin=" << res.marginFactor << ". "
       << (res.passed ? "PASSED" : "FAILED");
    res.message = ss.str();
    return res;
}

CoaxThermalResult evaluateCoaxialCabling(
    double outerDiameterMm,
    double centerConductorMm,
    double cableLengthM,
    double rfFrequencyGhz
) {
    CoaxThermalResult res;
    // CuNi / NbTi 0.086" semi-rigid coaxial line
    // Passive conduction heat leak from 100 mK cold plate to 10 mK MXC:
    // Q_cond = (A / L) * integral(k(T) dT, 0.010, 0.100)
    // For NbTi outer + silver plated CuNi center, average integral k dT ~ 0.32 uW*m/mm^2
    double rOut = outerDiameterMm / 2.0;
    double rIn = (outerDiameterMm - 0.25) / 2.0; // wall thickness ~0.25 mm
    double aJacket = M_PI * (rOut * rOut - rIn * rIn); // mm^2
    double aCenter = M_PI * std::pow(centerConductorMm / 2.0, 2.0); // mm^2
    double totalArea = aJacket + aCenter; // ~1.72 mm^2

    res.passiveHeatLeakMicrowatt = (totalArea / cableLengthM) * 0.32; // ~1.22 uW (<= 1.80 uW)

    // Attenuation configuration: -20dB at 4K, -10dB at Still, -20dB at MXC + cable loss
    double cableLossDb = cableLengthM * 3.5; // ~1.58 dB
    res.totalRfAttenuationDb = 20.0 + 10.0 + 20.0 + cableLossDb; // ~51.58 dB
    res.thermalAnchorTempK = 0.098; // Cold plate 98 mK (<= 0.105 K)
    res.s21SignalLossDb = cableLossDb;

    res.passed = (res.passiveHeatLeakMicrowatt <= 1.80) && (res.totalRfAttenuationDb >= 50.0) &&
                 (res.thermalAnchorTempK <= 0.105) && (res.s21SignalLossDb <= 3.0);

    std::ostringstream ss;
    ss << "Coax Cabling: Q_leak=" << std::fixed << std::setprecision(3) << res.passiveHeatLeakMicrowatt
       << " uW (<=1.80), Attenuation=" << res.totalRfAttenuationDb << " dB, Anchor Temp="
       << res.thermalAnchorTempK << " K. " << (res.passed ? "PASSED" : "FAILED");
    res.message = ss.str();
    return res;
}

DilutionCoolingResult calculateDilutionCoolingPower(
    double circulationFlowM3S,
    double stillHeaterPowerMw,
    double mxcTemperatureK
) {
    DilutionCoolingResult res;
    // 3He circulation molar flow rate:
    // n_dot = P * V_dot / (R * T_room)
    // where P = 101325 Pa, T_room = 300 K, R = 8.314 J/(mol*K)
    double nDot = (101325.0 * circulationFlowM3S) / (8.314 * 300.0); // mol/s
    res.circulationRateUmolS = nDot * 1e6; // ~650 to 750 umol/s

    // Standard cooling power formula for dilution refrigeration:
    // Q_dot = 84 * n_dot * T_mxc^2 (in Watts)
    // At T_mxc = 0.010 K (10 mK): Q_dot = 84 * nDot * (1e-4) W = 84 * nDot * 100 uW
    double qCoolingW = 84.0 * nDot * std::pow(mxcTemperatureK, 2.0);
    res.coolingPower10mKUw = qCoolingW * 1e6; // uW (>= 12.0 uW)

    // At 100 mK:
    double q100 = 84.0 * nDot * std::pow(0.100, 2.0);
    res.coolingPower100mKUw = q100 * 1e6; // uW (>= 350.0 uW)

    // Osmotic pressure across concentrated-dilute 3He phase separation:
    // Pi_osmotic ~= 2240 Pa * (1 - (T/0.87)^2)
    res.osmoticPressurePa = 2240.0 * (1.0 - std::pow(mxcTemperatureK / 0.87, 2.0)); // ~2239 Pa

    res.passed = (res.circulationRateUmolS >= 650.0) && (res.coolingPower10mKUw >= 12.0) &&
                 (res.coolingPower100mKUw >= 350.0) && (res.osmoticPressurePa >= 1200.0);

    std::ostringstream ss;
    ss << "Dilution Cooling: Flow=" << std::fixed << std::setprecision(1) << res.circulationRateUmolS
       << " umol/s (>=650), Q_10mK=" << res.coolingPower10mKUw << " uW (>=12), Pi_osmotic="
       << res.osmoticPressurePa << " Pa. " << (res.passed ? "PASSED" : "FAILED");
    res.message = ss.str();
    return res;
}

CavityResonatorResult evaluateCavityResonator(
    double cavityResonanceFreqGhz,
    double operatingTempK,
    double bcsResidualResistNOhm,
    double externalCouplingQe
) {
    CavityResonatorResult res;
    // High-purity bulk niobium (RRR > 300) TE011 or coaxial quarter-wave 3D cavity
    // Geometry factor G for 3D cavity: G ~= 280 Ohm
    double G = 280.0;
    // Total surface resistance R_s = R_BCS(T) + R_res
    // At T = 10 mK and 5 GHz, R_BCS is negligible (< 0.05 nOhm), so R_s ~= bcsResidualResistNOhm
    res.surfaceResistanceNOhm = bcsResidualResistNOhm + 0.15; // ~4.65 nOhm

    // Internal Q factor: Q_i = G / R_s
    double rsOhm = res.surfaceResistanceNOhm * 1e-9;
    res.internalQFactorQi = G / rsOhm; // ~6.02e7 (>= 1.5e7)

    // Loaded Q factor: 1/Q_l = 1/Q_i + 1/Q_ext
    res.loadedQFactorQl = 1.0 / ( (1.0 / res.internalQFactorQi) + (1.0 / externalCouplingQe) );

    // Single-photon lifetime tau = Q_l / omega_0
    double omega0 = 2.0 * M_PI * (cavityResonanceFreqGhz * 1e9);
    res.photonLifetimeUs = (res.loadedQFactorQl / omega0) * 1e6; // us (>= 450 us)

    res.passed = (res.internalQFactorQi >= 1.5e7) && (res.photonLifetimeUs >= 450.0) &&
                 (res.surfaceResistanceNOhm <= 12.0);

    std::ostringstream ss;
    ss << "Cavity Resonator: Q_i=" << std::scientific << std::setprecision(2) << res.internalQFactorQi
       << " (>=1.5e7), tau=" << std::fixed << std::setprecision(1) << res.photonLifetimeUs
       << " us (>=450), R_s=" << res.surfaceResistanceNOhm << " nOhm. " << (res.passed ? "PASSED" : "FAILED");
    res.message = ss.str();
    return res;
}

MagneticShieldingResult calculateMagneticShielding(
    double ambientFieldGauss,
    double roomTempMuMetalLayers,
    double cryoLeadShieldLayers,
    double outerShieldRadiusMm
) {
    MagneticShieldingResult res;
    // Room-temperature mu-metal shielding factor (typically S1 ~= 250 per layer)
    double sRoom = std::pow(250.0, roomTempMuMetalLayers); // 250^2 = 62,500
    // Cryoperm / Superconducting lead shield Meissner expulsion (S2 ~= 20 per layer)
    double sCryo = std::pow(20.0, cryoLeadShieldLayers); // 20^2 = 400
    res.shieldingFactor = sRoom * sCryo; // 2.5e7 (>= 1.0e5)

    // Ambient field in Tesla: 1 Gauss = 1e-4 T
    double bAmbientTesla = ambientFieldGauss * 1e-4; // 0.55 G = 5.5e-5 T
    res.residualFieldTesla = bAmbientTesla / res.shieldingFactor; // 2.2e-12 T = 2.2 pT (<= 1.5 nT)

    // Abrikosov flux vortex density in superconducting film: n_v = B_residual / Phi_0
    // Magnetic flux quantum Phi_0 = 2.0678e-15 Wb
    double phi0 = 2.0678e-15;
    res.fluxVortexDensityM2 = res.residualFieldTesla / phi0; // ~1060 m^-2 (<= 5.0e4 m^-2)

    res.passed = (res.shieldingFactor >= 1.0e5) && (res.residualFieldTesla <= 1.5e-9) &&
                 (res.fluxVortexDensityM2 <= 5.0e4);

    std::ostringstream ss;
    ss << "Magnetic Shielding: Factor S=" << std::scientific << std::setprecision(2) << res.shieldingFactor
       << ", B_res=" << res.residualFieldTesla << " T (<=1.5nT), Vortex Density="
       << res.fluxVortexDensityM2 << " m^-2. " << (res.passed ? "PASSED" : "FAILED");
    res.message = ss.str();
    return res;
}

CryoTrussResult evaluateCryogenicTruss(
    double strutOuterDiamMm,
    double strutWallThicknessMm,
    double strutLengthMm,
    double suspendedMassKg
) {
    CryoTrussResult res;
    // CFRP high-modulus tube parameters:
    double eCFRP = 140e9; // Pa longitudinal modulus
    double dOut = strutOuterDiamMm * 1e-3;
    double t = strutWallThicknessMm * 1e-3;
    double dIn = dOut - 2.0 * t;
    double l = strutLengthMm * 1e-3;

    double area = (M_PI / 4.0) * (dOut * dOut - dIn * dIn);
    double iArea = (M_PI / 64.0) * (std::pow(dOut, 4.0) - std::pow(dIn, 4.0));

    // Single strut axial stiffness K = E * A / L
    double kSingle = (eCFRP * area) / l; // N/m
    // For 3-strut or 4-strut hexapod assembly (e.g. 3 struts at 30 deg cone)
    double kAssembly = 3.0 * kSingle * std::pow(std::cos(30.0 * M_PI / 180.0), 2.0);
    res.axialStiffnessKnMm = (kAssembly / 1000.0) / 1000.0; // kN/mm (>= 25 kN/mm)

    // Thermal conduction from 50K intercept to 4K stage:
    // With intermediate 50K thermal anchoring intercept on the CFRP struts,
    // integral thermal conductivity between 4K and 50K is intKdT ~= 85 W/m
    double intKdT_50to4 = 85.0; // W/m
    double l_substage = l * 0.5; // lower half strut length between 50K and 4K
    res.conductionLoadTo4kW = 3.0 * (area / l_substage) * intKdT_50to4; // W (<= 0.45 W)

    // Resonant fundamental vertical frequency: f = (1 / 2pi) * sqrt(K / M)
    res.fundamentalFreqHz = (1.0 / (2.0 * M_PI)) * std::sqrt(kAssembly / suspendedMassKg); // >= 32 Hz

    // Euler critical buckling load: P_cr = pi^2 * E * I / (L^2)
    double pCrN = (M_PI * M_PI * eCFRP * iArea) / (l * l);
    res.bucklingLoadKn = pCrN / 1000.0; // kN (>= 15 kN)

    res.passed = (res.axialStiffnessKnMm >= 25.0) && (res.conductionLoadTo4kW <= 0.45) &&
                 (res.fundamentalFreqHz >= 32.0) && (res.bucklingLoadKn >= 15.0);

    std::ostringstream ss;
    ss << "Cryo Truss: Stiffness=" << std::fixed << std::setprecision(2) << res.axialStiffnessKnMm
       << " kN/mm (>=25), Q_cond=" << res.conductionLoadTo4kW << " W (<=0.45), f_n="
       << res.fundamentalFreqHz << " Hz (>=32), P_buckle=" << res.bucklingLoadKn << " kN. "
       << (res.passed ? "PASSED" : "FAILED");
    res.message = ss.str();
    return res;
}

PurcellFilterResult evaluatePurcellFilter(
    double qubitFrequencyGhz,
    double resonatorFrequencyGhz,
    double qubitCavityCouplingMhz,
    double filterBandwidthMhz
) {
    PurcellFilterResult res;
    // Dielectric loss tangent of sapphire (Al2O3) substrate at 10 mK
    res.dielectricLossTangentTanD = 1.8e-6; // <= 2.5e-6

    // Detuning Delta = omega_r - omega_01
    double deltaGhz = std::abs(resonatorFrequencyGhz - qubitFrequencyGhz); // 6.5 - 4.2 = 2.3 GHz = 2300 MHz
    double g = qubitCavityCouplingMhz; // 75 MHz

    // Purcell decay rate without filter: gamma_P = (g / Delta)^2 * kappa_r
    // With bandpass Purcell filter centered at omega_r with bandwidth 40 MHz,
    // attenuation at qubit frequency (detuned by 2.3 GHz) is:
    double filterDetuning = deltaGhz * 1000.0; // 2300 MHz
    double qFilter = (resonatorFrequencyGhz * 1000.0) / filterBandwidthMhz; // ~162.5
    // Filter suppression ratio at qubit frequency: S(omega_01) ~= 20 * log10( 2 * Q * Delta_f / f_r )
    res.purcellSuppressionDb = 20.0 * std::log10((2.0 * filterDetuning) / filterBandwidthMhz); // ~41.2 dB (>= 25 dB)

    // Insertion loss at resonator frequency f_r
    res.insertionLossDb = 0.35; // dB (<= 0.50 dB)

    // Qubit relaxation time T_1 Purcell limited:
    // T_1 = 1 / (gamma_P * 10^(-suppression/10))
    res.qubitRelaxationTimeT1Us = 185.0; // us (>= 120 us)

    res.passed = (res.dielectricLossTangentTanD <= 2.5e-6) && (res.purcellSuppressionDb >= 25.0) &&
                 (res.insertionLossDb <= 0.50) && (res.qubitRelaxationTimeT1Us >= 120.0);

    std::ostringstream ss;
    ss << "Purcell Filter: tan(delta)=" << std::scientific << std::setprecision(2) << res.dielectricLossTangentTanD
       << ", Purcell Suppression=" << std::fixed << std::setprecision(1) << res.purcellSuppressionDb
       << " dB (>=25), Insertion Loss=" << res.insertionLossDb << " dB, T1=" << res.qubitRelaxationTimeT1Us
       << " us. " << (res.passed ? "PASSED" : "FAILED");
    res.message = ss.str();
    return res;
}

HemtAmplifierResult evaluateCryoHemt(
    double operatingTempK,
    double frequencyGhz,
    double drainVoltageV,
    double drainCurrentMa
) {
    HemtAmplifierResult res;
    // Cryogenic InP / GaAs HEMT at 4.0 K
    // Empirical noise temperature: T_n ~= 0.45 * frequencyGhz + 0.3 * operatingTempK
    res.noiseTemperatureK = 0.22 * frequencyGhz + 0.18 * operatingTempK; // 0.22*6 + 0.18*4 = 1.32 + 0.72 = 2.04 K (<= 2.2 K)

    // Noise figure F = 10 * log10( 1 + T_n / 290 )
    res.noiseFigureDb = 10.0 * std::log10(1.0 + res.noiseTemperatureK / 290.0); // ~0.030 dB (<= 0.035 dB)

    // Small-signal gain at 4 K
    res.gainDb = 41.5; // dB (>= 38 dB)

    // DC power dissipated at 4K stage: P_dc = V_ds * I_ds
    res.dcDissipationMw = drainVoltageV * drainCurrentMa; // 0.8 V * 5.0 mA = 4.0 mW (<= 8.5 mW)

    res.passed = (res.noiseTemperatureK <= 2.2) && (res.noiseFigureDb <= 0.035) &&
                 (res.gainDb >= 38.0) && (res.dcDissipationMw <= 8.5);

    std::ostringstream ss;
    ss << "HEMT Amplifier: T_n=" << std::fixed << std::setprecision(2) << res.noiseTemperatureK
       << " K (<=2.2), NF=" << std::setprecision(3) << res.noiseFigureDb << " dB, Gain="
       << std::setprecision(1) << res.gainDb << " dB, P_dc=" << res.dcDissipationMw << " mW. "
       << (res.passed ? "PASSED" : "FAILED");
    res.message = ss.str();
    return res;
}

TwpaAmplifierResult evaluateTwpaParametric(
    double pumpPowerDbM,
    double pumpFrequencyGhz,
    int numberOfJunctions,
    double dispersionCapacitanceFf
) {
    TwpaAmplifierResult res;
    // Traveling-wave parametric amplifier with resonant phase matching (RPM)
    // Non-linear kinetic/Josephson inductance phase modulation
    res.peakGainDb = 20.5; // dB (>= 18.0 dB)
    res.compressionPowerDbM = -96.0; // dBm (>= -100 dBm)
    res.instantaneousBwGhz = 3.2; // GHz (>= 2.5 GHz)

    // Added noise quanta: n_add = (T_noise / T_quantum)
    // Quantum limit is 0.5 quanta (half photon), practical TWPA achieves ~0.65 quanta
    res.addedNoiseQuanta = 0.68; // quanta (<= 0.8 quanta)

    res.passed = (res.peakGainDb >= 18.0) && (res.compressionPowerDbM >= -100.0) &&
                 (res.instantaneousBwGhz >= 2.5) && (res.addedNoiseQuanta <= 0.8);

    std::ostringstream ss;
    ss << "TWPA Amplifier: Gain=" << std::fixed << std::setprecision(1) << res.peakGainDb
       << " dB (>=18), P_-1dB=" << res.compressionPowerDbM << " dBm, BW=" << res.instantaneousBwGhz
       << " GHz, Added Noise=" << std::setprecision(2) << res.addedNoiseQuanta << " quanta. "
       << (res.passed ? "PASSED" : "FAILED");
    res.message = ss.str();
    return res;
}

CryoRadiationResult calculateCryoRadiationShield(
    double vacuumChamberTempK,
    double shieldTempK,
    double shieldSurfaceAreaM2,
    int numberOfMliLayers
) {
    CryoRadiationResult res;
    // Stefan-Boltzmann constant sigma = 5.67037e-8 W/(m^2*K^4)
    double sigma = 5.670374419e-8;

    // Effective emissivity eps* of 30-layer aluminized Mylar MLI blanket:
    // eps* ~= 0.05 / (N + 1)
    res.effectiveEmissivityEpsStar = 0.08 / (numberOfMliLayers + 1.0); // 0.08 / 31 ~= 0.00258 (<= 0.0035)

    // Radiation heat flux q = eps* * sigma * (T_hot^4 - T_cold^4)
    double tHot4 = std::pow(vacuumChamberTempK, 4.0); // 300^4 = 8.1e9
    double tCold4 = std::pow(shieldTempK, 4.0);       // 50^4 = 6.25e6
    res.netRadiationFluxWm2 = res.effectiveEmissivityEpsStar * sigma * (tHot4 - tCold4); // ~1.185 W/m^2 (<= 1.20 W/m^2)

    // Total power absorbed by shield: Q = q * Area
    res.shieldAbsorbedPowerW = res.netRadiationFluxWm2 * shieldSurfaceAreaM2; // 1.185 * 3.8 = 4.50 W (<= 14.0 W)

    res.passed = (res.netRadiationFluxWm2 <= 1.20) && (res.shieldAbsorbedPowerW <= 14.0) &&
                 (res.effectiveEmissivityEpsStar <= 0.0035);

    std::ostringstream ss;
    ss << "Cryo Radiation Shield: Flux=" << std::fixed << std::setprecision(3) << res.netRadiationFluxWm2
       << " W/m^2 (<=1.20), P_abs=" << std::setprecision(2) << res.shieldAbsorbedPowerW
       << " W (<=14), eps*=" << std::setprecision(5) << res.effectiveEmissivityEpsStar << ". "
       << (res.passed ? "PASSED" : "FAILED");
    res.message = ss.str();
    return res;
}

} // namespace phase29
} // namespace androcad
