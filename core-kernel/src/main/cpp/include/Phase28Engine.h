#pragma once

#include <string>
#include <vector>
#include <cmath>

namespace androcad {
namespace phase28 {

/**
 * Domain 1: Ultra-thin CFRP Tape Spring Hinge Deployment Kinematics & Snap-through Buckling
 */
struct TapeSpringHingeResult {
    bool passed;
    double snapMomentNm;       // Peak snap-through bending moment
    double steadyMomentNm;     // Steady-state folding/unfolding moment
    double latchingTorqueNm;   // Final deployed latching lock torque
    double strainEnergyJ;      // Stored strain energy during folded state
    std::string message;
};

TapeSpringHingeResult analyzeTapeSpringHinge(
    double thicknessMm,        // Tape thickness (e.g., 0.15 mm CFRP)
    double radiusMm,           // Transverse radius of curvature (e.g., 25.0 mm)
    double subtendedAngleDeg,  // Subtended arc angle (e.g., 60.0 deg)
    double longitudinalLengthMm, // Hinge length (e.g., 180.0 mm)
    double youngsModulusGpa    // Longitudinal Young's modulus (e.g., 135.0 GPa)
);

/**
 * Domain 2: Deployable Solar Array Wing Multi-body Dynamics & Modal Frequencies
 */
struct WingModalResult {
    bool passed;
    double f1BendingHz;         // 1st cantilever bending natural frequency deployed (>= 0.15 Hz)
    double f1TorsionHz;         // 1st torsional natural frequency deployed (>= 0.40 Hz)
    double fStowedHz;           // Stowed natural frequency (>= 35 Hz launch vehicle req)
    double tipDeflectionMaxMm;  // Max tip displacement under slew excitation
    std::string message;
};

WingModalResult calculateWingModalFrequencies(
    double wingLengthM,        // Total deployed wing length (e.g., 12.0 m)
    double panelMassKg,        // Total wing mass (e.g., 85.0 kg)
    double rootStiffnessNmRad, // Root hinge assembly torsional stiffness (e.g., 45000 Nm/rad)
    double mastBendingEiNm2,   // Effective mast flexural rigidity EI (e.g., 2.8e5 N*m^2)
    double stowedHeightM       // Stowed panel stack height (e.g., 1.5 m)
);

/**
 * Domain 3: Viscoelastic Micro-Vibration Isolator Struts (Stewart Platform Hexapod)
 */
struct MicroVibrationResult {
    bool passed;
    double resonanceFrequencyHz;// Strut resonance frequency (e.g., 15 to 25 Hz)
    double peakAmplificationQ; // Quality factor at resonance (Q <= 3.5)
    double highFreqAttenuationDb; // Attenuation at 200 Hz (<= -35 dB)
    double transmissibilityRatio; // T at 100 Hz reaction wheel disturbance
    std::string message;
};

MicroVibrationResult evaluateMicroVibrationIsolator(
    double payloadMassKg,      // Isolated optical payload mass (e.g., 120.0 kg)
    double strutAxialK_Nmm,    // Axial stiffness per strut (e.g., 350.0 N/mm)
    double lossFactorEta,      // Viscoelastic damping loss factor (e.g., 0.35)
    int numberOfStruts         // 6 for Stewart platform
);

/**
 * Domain 4: Tensioned Solar Blanket Membrane Modal Tensioning & Wrinkle Prevention
 */
struct MembraneWrinkleResult {
    bool passed;
    double minTensionNm;       // Minimum membrane tension (>= 15 N/m)
    double majorPrincipalStressKpa; // Sigma_1 (positive, in tension)
    double minorPrincipalStressKpa; // Sigma_2 (> 0, non-wrinkling tension field)
    double safetyFactorWrinkling; // Ratio of sigma_2 to wrinkle threshold
    std::string message;
};

MembraneWrinkleResult checkMembraneWrinkling(
    double cornerPreloadN,     // Corner pretensioning spring load (e.g., 45.0 N)
    double blanketWidthM,      // Blanket width (e.g., 2.2 m)
    double blanketLengthM,     // Blanket segment length (e.g., 3.5 m)
    double membraneThicknessUm,// Kapton/Upilex thickness (e.g., 50.0 um)
    double shearDistortionRad  // Array frame diagonal skew (e.g., 0.0012 rad)
);

/**
 * Domain 5: Low-Shock Hold-Down and Release Mechanism (HDRM) Preload & Shock Spectrum
 */
struct HdrmReleaseResult {
    bool passed;
    double boltPreloadKn;       // Preload clamp force (>= 18 kN)
    double gPeakShock10Khz;     // Peak SRS shock response at 10 kHz (<= 450 g)
    double releaseTimeMs;       // Actuation time for non-pyro SMA split spool (<= 35 ms)
    double gappingSafetyFactor; // Separation safety under launch 40g load
    std::string message;
};

HdrmReleaseResult evaluateHdrmRelease(
    double boltNominalDiameterMm, // Inconel 718 tie bolt diameter (e.g., 8.0 mm)
    double boltTorqueNm,          // Tightening torque (e.g., 32.0 Nm)
    double clampLengthMm,         // Stack clamp grip length (e.g., 45.0 mm)
    double launchGLevel,          // Peak quasi-static launch load (e.g., 40.0 g)
    double supportedMassKg        // Mass per hold-down point (e.g., 25.0 kg)
);

/**
 * Domain 6: Eclipse Entry/Exit Thermal Snap Flutter Transient Bending
 */
struct ThermalSnapResult {
    bool passed;
    double maxTransientDeflectionMm; // Dynamic tip snap displacement (<= 3.0 mm)
    double tempGradientK;            // Front-to-back face delta T (e.g., 22 K)
    double peakThermalBendingMomentNm; // Induced thermal moment
    double settlingTimeSec;          // Time to damp down vibration (<= 45 s)
    std::string message;
};

ThermalSnapResult calculateThermalSnapFlutter(
    double solarFluxWm2,        // Solar irradiance (1361 W/m^2)
    double frontAlphaAbs,       // Front solar absorptivity (e.g., 0.88)
    double backEmissivityEps,   // Rear infrared emissivity (e.g., 0.82)
    double mastDiameterMm,      // Tubular boom diameter (e.g., 60.0 mm)
    double mastWallMm,          // Wall thickness (e.g., 1.2 mm)
    double boomLengthM          // Deployed length (e.g., 8.0 m)
);

/**
 * Domain 7: Synchronized Pantograph Linkage Deployment Velocity & Eddy-Current Damper
 */
struct PantographDamperResult {
    bool passed;
    double peakDeployVelocityMs;// Max extension speed (<= 0.08 m/s)
    double dampingTorqueNmsRad; // Damper rate
    double latchImpactEnergyJ;  // Kinetic energy at end of stroke (<= 0.85 J)
    double synchronizationErrorMm; // Left/right scissor displacement mismatch (<= 1.5 mm)
    std::string message;
};

PantographDamperResult evaluatePantographDeployment(
    double stageMassKg,         // Mass per pantograph stage (e.g., 14.0 kg)
    int numStages,              // Number of scissors bays (e.g., 6)
    double springDeploymentForceN, // Driving gas/clock spring force (e.g., 90.0 N)
    double eddyCurrentDampingCoeff, // Eddy damper rating (e.g., 18.0 N*s/m)
    double totalDeployTravelM   // Total deployed reach (e.g., 5.4 m)
);

/**
 * Domain 8: Multi-Junction GaInP/InGaAs/Ge Solar Cell Laydown & Thermal Cycling Fatigue
 */
struct SolarCellFatigueResult {
    bool passed;
    double thermalStressMpa;    // Interconnect solder thermal cyclic stress (<= 45 MPa)
    double fatigueLifeCycles;   // Coffin-Manson low-cycle fatigue life (>= 2000 cycles)
    double coverglassFactorOfSafety; // Micro-crack margin under -175C to +125C
    double powerDegradationFactor; // Radiation + thermal degradation factor (>= 0.78 after 15 yrs)
    std::string message;
};

SolarCellFatigueResult analyzeCellLaydownFatigue(
    double deltaTCelsius,       // Thermal cycle amplitude (e.g., 300 C, -175C to +125C)
    double substrateCtePpm,     // CFRP face sheet CTE (e.g., 1.5 ppm/K)
    double cellGeCtePpm,        // Germanium wafer CTE (e.g., 5.9 ppm/K)
    double solderJointThicknessUm, // Solder/conductive adhesive thickness (e.g., 40.0 um)
    int requiredCycles          // Required qualification cycles (e.g., 2000)
);

/**
 * Domain 9: Solar Array Drive Assembly (SADA) Harmonic Drive & Slip-Ring Arc Suppression
 */
struct SadaHarmonicResult {
    bool passed;
    double rippleTorqueNm;      // Peak harmonic drive kinematic transmission ripple (<= 0.04 Nm)
    double contactResistanceMOhm; // Gold-on-gold slip-ring contact resistance (<= 15 mOhm)
    double trackingRateErrorArcsec; // Sun tracking angular jitter (<= 25 arcsec)
    double arcSuppressionMargin; // Paschen breakdown voltage margin in LEO/GEO vacuum
    std::string message;
};

SadaHarmonicResult evaluateSadaMechanism(
    double gearRatio,           // Harmonic drive reduction ratio (e.g., 160:1)
    double motorStepAngleDeg,   // Stepper motor step angle (e.g., 1.8 deg)
    double slipRingCurrentA,    // Array bus transfer current (e.g., 35.0 A)
    double operatingSpeedRpm    // Array 1 rev/day tracking speed (~0.000694 RPM)
);

/**
 * Domain 10: Spacecraft Attitude Feedforward Compensation & Docking Latch Jitter
 */
struct SpacecraftFeedforwardResult {
    bool passed;
    double disturbanceTorqueNm; // Peak slew/stepping reaction disturbance torque (<= 0.02 Nm)
    double feedforwardCancellationPct; // Feedforward torque cancellation percentage (>= 98.0%)
    double residualJitterMicrorad; // Pointing jitter residual (<= 0.50 urad)
    double latchAlignmentErrorMm; // Robotic docking pin kinematic seat error (<= 0.20 mm)
    std::string message;
};

SpacecraftFeedforwardResult evaluateAttitudeFeedforward(
    double busInertiaKgm2,      // Spacecraft bus principal inertia (e.g., 1200.0 kg*m^2)
    double wingInertiaKgm2,     // Solar array wing inertia about rotation axis (e.g., 180.0 kg*m^2)
    double steppingAccelRadS2,  // SADA stepping acceleration (e.g., 0.008 rad/s^2)
    double feedforwardGain      // Compensation filter gain (e.g., 0.985)
);

} // namespace phase28
} // namespace androcad
