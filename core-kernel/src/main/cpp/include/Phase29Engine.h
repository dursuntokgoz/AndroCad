#pragma once

#include <string>
#include <vector>
#include <cmath>

namespace androcad {
namespace phase29 {

/**
 * Domain 1: Multi-stage Thermal Intercept Heat Load Budgeting
 */
struct ThermalBudgetResult {
    bool passed;
    double q50kStageW;         // 50 K first stage heat load (<= 35.0 W)
    double q4kStageW;          // 4 K pulse tube second stage heat load (<= 1.20 W)
    double qStillStageMw;      // 800 mK Still stage heat load (<= 25.0 mW)
    double qMxcStageMicrowatt; // 10 mK Mixing Chamber heat load (<= 15.0 uW)
    double marginFactor;       // Cooling power excess margin (>= 1.50)
    std::string message;
};

ThermalBudgetResult calculateCryostatThermalBudget(
    int numCoaxialLines,       // Number of semi-rigid coax cables (e.g., 24)
    int numDcLines,            // Number of twisted pair DC lines (e.g., 48)
    double supportStrutConductivityW, // Structural conduction load to 4K (e.g., 0.38 W)
    double mliRadiationFluxWm2 // Radiation flux through 30 layers MLI (e.g., 1.15 W/m^2)
);

/**
 * Domain 2: Coaxial Cryogenic RF Cabling Attenuation & Thermal Anchoring
 */
struct CoaxThermalResult {
    bool passed;
    double passiveHeatLeakMicrowatt; // Conduction heat leak to MXC per line (<= 1.80 uW)
    double totalRfAttenuationDb;    // Total cryogenic RF attenuation at 5 GHz (e.g., 60 dB)
    double thermalAnchorTempK;      // Cold plate anchor temperature (<= 0.105 K)
    double s21SignalLossDb;         // Return signal transmission loss through low-loss coax
    std::string message;
};

CoaxThermalResult evaluateCoaxialCabling(
    double outerDiameterMm,    // 0.086 inch CuNi/NbTi outer diameter (~2.19 mm)
    double centerConductorMm,  // Silver-plated CuNi center conductor (~0.51 mm)
    double cableLengthM,       // Length between 4K and MXC (e.g., 0.45 m)
    double rfFrequencyGhz      // 5.0 GHz qubit readout band
);

/**
 * Domain 3: Dilution Refrigerator 3He/4He Circulation & Phase Separation Osmotic Pressure
 */
struct DilutionCoolingResult {
    bool passed;
    double circulationRateUmolS; // 3He circulation molar flow rate (>= 650 umol/s)
    double coolingPower10mKUw;   // Net cooling power at 10 mK (>= 12.0 uW)
    double coolingPower100mKUw;  // Net cooling power at 100 mK (>= 350.0 uW)
    double osmoticPressurePa;    // Osmotic pressure across phase boundary (>= 1200 Pa)
    std::string message;
};

DilutionCoolingResult calculateDilutionCoolingPower(
    double circulationFlowM3S, // Volume flow rate of 3He gas at 300K/1bar (e.g., 0.016 m^3/s)
    double stillHeaterPowerMw, // Still electric heater power (e.g., 12.0 mW)
    double mxcTemperatureK     // Mixing chamber operating temperature (e.g., 0.010 K = 10 mK)
);

/**
 * Domain 4: Superconducting 3D Microwave Cavity Resonator Internal Quality Factor
 */
struct CavityResonatorResult {
    bool passed;
    double internalQFactorQi;  // Unloaded internal Q factor (>= 1.5e7)
    double loadedQFactorQl;    // Loaded Q factor with critical coupling
    double photonLifetimeUs;   // Single-photon cavity storage lifetime (>= 450 us)
    double surfaceResistanceNOhm; // BCS + residual surface resistance (<= 12.0 nOhm)
    std::string message;
};

CavityResonatorResult evaluateCavityResonator(
    double cavityResonanceFreqGhz, // Cavity resonance frequency (e.g., 5.0 GHz)
    double operatingTempK,         // 0.010 K
    double bcsResidualResistNOhm,  // Niobium high-purity RRR>300 residual resistance (e.g., 4.5 nOhm)
    double externalCouplingQe      // External waveguide coupling Q_ext (e.g., 3.0e7)
);

/**
 * Domain 5: Multi-layer Cryoperm / Mu-Metal Magnetic Shielding Attenuation
 */
struct MagneticShieldingResult {
    bool passed;
    double shieldingFactor;        // Total attenuation factor S (>= 1.0e5)
    double residualFieldTesla;     // Residual DC magnetic field at qubit plane (<= 1.5e-9 T = 1.5 nT)
    double fluxVortexDensityM2;    // Trapped Abrikosov flux vortex density (<= 5.0e4 m^-2)
    std::string message;
};

MagneticShieldingResult calculateMagneticShielding(
    double ambientFieldGauss,      // Earth + lab magnetic field (e.g., 0.55 Gauss = 55 uT)
    double roomTempMuMetalLayers,  // Outer shields at 300K (e.g., 2 layers Cryoperm)
    double cryoLeadShieldLayers,   // Inner superconducting lead/aluminum can at 4K/10mK (e.g., 2)
    double outerShieldRadiusMm     // Outer shield diameter (e.g., 250.0 mm)
);

/**
 * Domain 6: Cryogenic Carbon-Fiber Truss Support Struts Stiffness & Heat Bypass
 */
struct CryoTrussResult {
    bool passed;
    double axialStiffnessKnMm;     // Axial structural stiffness (>= 25.0 kN/mm)
    double conductionLoadTo4kW;    // Conduction heat load from 300K to 4K (<= 0.45 W)
    double fundamentalFreqHz;      // Cryostat suspended stage resonant frequency (>= 32.0 Hz)
    double bucklingLoadKn;         // Euler critical buckling capacity (>= 15.0 kN)
    std::string message;
};

CryoTrussResult evaluateCryogenicTruss(
    double strutOuterDiamMm,       // CFRP tube OD (e.g., 28.0 mm)
    double strutWallThicknessMm,   // Wall thickness (e.g., 2.0 mm)
    double strutLengthMm,          // Length between 300K and 4K (e.g., 280.0 mm)
    double suspendedMassKg         // Cold mass below 4K (e.g., 45.0 kg)
);

/**
 * Domain 7: Superconducting Qubit Chip Sapphire Substrate Loss Tangent & Purcell Filter
 */
struct PurcellFilterResult {
    bool passed;
    double dielectricLossTangentTanD; // Sapphire substrate loss tangent tan(delta) (<= 2.5e-6)
    double purcellSuppressionDb;      // Qubit decay suppression at f_01 (>= 25.0 dB)
    double insertionLossDb;           // Readout passband insertion loss at f_r (<= 0.50 dB)
    double qubitRelaxationTimeT1Us;   // Purcell-limited T_1 relaxation time (>= 120.0 us)
    std::string message;
};

PurcellFilterResult evaluatePurcellFilter(
    double qubitFrequencyGhz,         // Qubit transition f_01 (e.g., 4.2 GHz)
    double resonatorFrequencyGhz,     // Readout resonator f_r (e.g., 6.5 GHz)
    double qubitCavityCouplingMhz,    // Jaynes-Cummings coupling g/2pi (e.g., 75.0 MHz)
    double filterBandwidthMhz         // Purcell filter bandwidth (e.g., 40.0 MHz)
);

/**
 * Domain 8: Cryogenic HEMT Low-Noise Amplifier Noise Temperature
 */
struct HemtAmplifierResult {
    bool passed;
    double noiseTemperatureK;         // HEMT noise temperature T_n (<= 2.2 K)
    double noiseFigureDb;             // Noise figure F (<= 0.035 dB)
    double gainDb;                    // Amplifier gain (>= 38.0 dB)
    double dcDissipationMw;           // DC power dissipated at 4K stage (<= 8.5 mW)
    std::string message;
};

HemtAmplifierResult evaluateCryoHemt(
    double operatingTempK,            // Anchored stage (4.0 K)
    double frequencyGhz,              // Readout band (4.0 to 8.0 GHz)
    double drainVoltageV,             // Biased V_ds (e.g., 0.8 V)
    double drainCurrentMa             // Biased I_ds (e.g., 5.0 mA)
);

/**
 * Domain 9: Traveling Wave Parametric Amplifier (TWPA) Non-linear Dispersion
 */
struct TwpaAmplifierResult {
    bool passed;
    double peakGainDb;                // Signal param gain (>= 18.0 dB)
    double compressionPowerDbM;       // 1-dB gain compression point (>= -100.0 dBm)
    double instantaneousBwGhz;        // 3-dB instantaneous bandwidth (>= 2.5 GHz)
    double addedNoiseQuanta;          // Added noise photons (<= 0.8 quanta, near quantum limit)
    std::string message;
};

TwpaAmplifierResult evaluateTwpaParametric(
    double pumpPowerDbM,              // RF pump drive power (e.g., -52.0 dBm)
    double pumpFrequencyGhz,          // Pump frequency (e.g., 7.8 GHz)
    int numberOfJunctions,            // SNAIL / Josephson junction cells (e.g., 2000)
    double dispersionCapacitanceFf    // Resonant phase matching loading (e.g., 35.0 fF)
);

/**
 * Domain 10: Cryogenic Vacuum Chamber Thermal Radiation Baffle & MLI Heat Flux
 */
struct CryoRadiationResult {
    bool passed;
    double netRadiationFluxWm2;       // Radiation flux arriving at 50K shield (<= 1.20 W/m^2)
    double shieldAbsorbedPowerW;      // Total radiative heat load absorbed by 50K shield (<= 14.0 W)
    double effectiveEmissivityEpsStar;// Effective emissivity eps* across 30 MLI layers (<= 0.0035)
    std::string message;
};

CryoRadiationResult calculateCryoRadiationShield(
    double vacuumChamberTempK,        // Outer 300 K vacuum envelope
    double shieldTempK,               // 50 K pulse tube shield
    double shieldSurfaceAreaM2,       // Radiative surface area (e.g., 3.8 m^2)
    int numberOfMliLayers             // Aluminized Mylar layers (e.g., 30)
);

} // namespace phase29
} // namespace androcad
