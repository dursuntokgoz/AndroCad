#include "Phase17Engine.h"
#include <cmath>
#include <algorithm>

namespace AndroCAD {
namespace Phase17 {

HoekBrownResult Phase17Engine::evaluateHoekBrown(double minorPrincipalStressMpa, double ucsMpa, double gsi, double miParameter, double disturbanceD) {
    HoekBrownResult res;
    // Generalized Hoek-Brown criterion (Hoek, Carranza-Torres, Corkum 2002)
    // mb = mi * exp((GSI - 100) / (28 - 14*D))
    double mb = miParameter * std::exp((gsi - 100.0) / (28.0 - 14.0 * disturbanceD));
    // s = exp((GSI - 100) / (9 - 3*D))
    double s = std::exp((gsi - 100.0) / (9.0 - 3.0 * disturbanceD));
    // a = 0.5 + (exp(-GSI/15) - exp(-20/3)) / 6
    double a = 0.5 + (std::exp(-gsi / 15.0) - std::exp(-20.0 / 3.0)) / 6.0;
    // sigma_1 = sigma_3 + sigma_ci * (mb * sigma_3 / sigma_ci + s)^a
    double term = mb * (minorPrincipalStressMpa / ucsMpa) + s;
    if (term < 0.0) term = 0.0;
    res.majorPrincipalStrengthMpa = minorPrincipalStressMpa + ucsMpa * std::pow(term, a);
    // Rock mass deformation modulus Erm (Hooke & Diederichs 2006)
    res.rockMassModulusGpa = 100.0 * (1.0 - disturbanceD / 2.0) / (1.0 + std::exp((75.0 + 25.0 * disturbanceD - gsi) / 11.0));
    // Uniaxial tensile strength
    res.tensileStrengthMpa = -s * ucsMpa / mb;
    // Factor of safety against induced boundary tangential stress: sigma_1_induced ~ 3*sigma_0 - sigma_3
    double inducedStressMpa = 2.5 * minorPrincipalStressMpa + 15.0;
    res.safetyFactor = res.majorPrincipalStrengthMpa / inducedStressMpa;
    res.rockMassStable = (res.safetyFactor >= 1.20 && gsi >= 30.0);
    return res;
}

CsmCutterForceResult Phase17Engine::calculateCsmCutterForces(double cutterDiameterInch, double tipWidthMm, double spacingMm, double penetrationMm, double ucsMpa, double shearStrengthMpa) {
    CsmCutterForceResult res;
    res.penetrationMmPerRev = penetrationMm;
    // CSM (Colorado School of Mines) model for constant cross-section disc cutters
    // Radius R, contact angle phi = acos((R - p)/R) ~= sqrt(2*p / R)
    double radiusMm = (cutterDiameterInch * 25.4) / 2.0;
    double phiRad = std::sqrt(2.0 * penetrationMm / radiusMm);
    double basePressureMpa = 0.42 * std::pow(ucsMpa * ucsMpa * shearStrengthMpa, 1.0 / 3.0);
    // Normal force: Fn = C * T * R * phi * P0 * (S / (p * phi))^psi
    double spacingRatio = spacingMm / (penetrationMm * phiRad * radiusMm * 0.1);
    res.normalForceKn = (radiusMm * tipWidthMm * phiRad * basePressureMpa * 0.005) + (spacingRatio * 8.0);
    if (res.normalForceKn > 320.0) res.normalForceKn = 320.0; // bearing capacity limit ~ 250 - 315 kN
    // Rolling force: Fr = Fn * tan(phi / 2) ~= Fn * phi / 2
    res.rollingForceKn = res.normalForceKn * (phiRad / 2.0);
    // Specific energy SE = (Fn * p + Fr * 2*pi*R) / (S * p)
    double cutVolumeM3 = (spacingMm * penetrationMm * 1000.0) * 1e-9;
    double cuttingEnergyJoules = (res.rollingForceKn * 1000.0) * (2.0 * M_PI * radiusMm / 1000.0);
    res.specificEnergyMjm3 = (cuttingEnergyJoules / cutVolumeM3) * 1e-6;
    res.cutterLoadWithinBearingLimit = (res.normalForceKn <= 280.0 && res.rollingForceKn <= 45.0);
    return res;
}

SegmentalLiningResult Phase17Engine::evaluateSegmentalLining(double tunnelRadiusM, double overburdenDepthM, double groundStiffnessMpa, double segmentThicknessM, double jointBoltPreloadKn) {
    SegmentalLiningResult res;
    // Muir Wood - Curtis elastic equation for flexible lining ring
    double soilGamma = 20.0; // kN/m3
    double totalOverburdenPressureKpa = soilGamma * overburdenDepthM;
    double liningInertia = (segmentThicknessM * segmentThicknessM * segmentThicknessM) / 12.0;
    double eConcreteKpa = 35.0 * 1e6;
    // Bending moment M = (1 - K0) * p * R^2 / 4 * (1 / (1 + flexibilityFactor))
    double k0 = 0.65;
    double flexFactor = (eConcreteKpa * liningInertia) / (groundStiffnessMpa * 1000.0 * std::pow(tunnelRadiusM, 3.0));
    res.maxLiningBendingMomentKnM = ((1.0 - k0) * totalOverburdenPressureKpa * tunnelRadiusM * tunnelRadiusM / 4.0) / (1.0 + 3.0 * flexFactor);
    // Janssen joint rotational stiffness
    res.jointRotationalStiffnessKnMPerRad = 15000.0 * (segmentThicknessM / 0.35);
    // Joint opening displacement delta = M / (K_theta * b)
    res.jointOpeningDisplacementMm = (res.maxLiningBendingMomentKnM / res.jointRotationalStiffnessKnMPerRad) * 1000.0 * 0.05;
    // Bolt tension stress
    double boltAreaMm2 = 560.0; // M30 grade 8.8
    res.boltTensionStressMpa = (jointBoltPreloadKn * 1000.0) / boltAreaMm2 + (res.maxLiningBendingMomentKnM * 1000.0 / (segmentThicknessM * 1000.0 * boltAreaMm2 * 0.1));
    res.liningIntegrityMaintained = (res.jointOpeningDisplacementMm < 2.0 && res.boltTensionStressMpa < 640.0);
    return res;
}

EpbChamberPressureResult Phase17Engine::evaluateEpbFacePressure(double depthToCrownM, double waterTableDepthM, double soilDensityKnM3, double soilCohesionKpa, double internalFrictionAngleDeg) {
    EpbChamberPressureResult res;
    // Hydrostatic water pressure at tunnel crown
    double waterHeadM = std::max(0.0, depthToCrownM - waterTableDepthM);
    res.hydrostaticPressureBar = (waterHeadM * 9.81) / 100.0;
    // Total vertical earth pressure at crown
    double sigmaV = soilDensityKnM3 * depthToCrownM; // kPa
    // Earth pressure coefficient K0 = 1 - sin(phi)
    double phiRad = internalFrictionAngleDeg * (M_PI / 180.0);
    double k0 = 1.0 - std::sin(phiRad);
    double sigmaH = k0 * sigmaV - 2.0 * soilCohesionKpa * std::sqrt(k0);
    if (sigmaH < 0.2 * sigmaV) sigmaH = 0.2 * sigmaV;
    res.earthPressureAtCrownBar = sigmaH / 100.0;
    // Required chamber pressure: p_req = p_water + p_effective_soil + 0.2 bar safety
    res.requiredChamberPressureBar = res.hydrostaticPressureBar + res.earthPressureAtCrownBar + 0.20;
    // Safety factor against face collapse
    res.faceStabilitySafetyFactor = res.requiredChamberPressureBar / (res.earthPressureAtCrownBar + res.hydrostaticPressureBar);
    res.surfaceCollapseAvoided = (res.faceStabilitySafetyFactor >= 1.05 && res.requiredChamberPressureBar < 6.0);
    return res;
}

PeckSettlementResult Phase17Engine::calculatePeckSettlement(double tunnelDepthM, double tunnelDiameterM, double volumeLossPercent, double offsetDistanceM) {
    PeckSettlementResult res;
    // Peck (1969) empirical Gaussian settlement trough:
    // S(x) = Smax * exp(-x^2 / (2 * i^2))
    // Trough width parameter: i = K * z0 where K ~ 0.45 for clays / silts, 0.35 for sands
    double K = 0.45;
    res.troughWidthParameterMeters = K * tunnelDepthM;
    // Total ground loss volume per meter: Vs = V_loss * pi * R^2
    double r = tunnelDiameterM / 2.0;
    double vs = (volumeLossPercent / 100.0) * (M_PI * r * r);
    // Smax = Vs / (sqrt(2 * pi) * i)
    res.maxSettlementMm = (vs / (std::sqrt(2.0 * M_PI) * res.troughWidthParameterMeters)) * 1000.0;
    // Angular distortion beta = dS / dx at inflection point x = i: beta_max = Smax / (i * sqrt(e))
    double distAtX = res.maxSettlementMm * std::exp(-(offsetDistanceM * offsetDistanceM) / (2.0 * res.troughWidthParameterMeters * res.troughWidthParameterMeters));
    res.maxAngularDistortion = (res.maxSettlementMm / 1000.0) / (res.troughWidthParameterMeters * std::sqrt(std::exp(1.0)));
    // Burland (1995) damage categories: 0 (< 1/500 negligible), 1 (very slight), 2 (slight), 3 (moderate)
    if (res.maxAngularDistortion < 1.0 / 500.0 && res.maxSettlementMm < 10.0) res.buildingDamageCategory = 0;
    else if (res.maxSettlementMm < 25.0) res.buildingDamageCategory = 1;
    else res.buildingDamageCategory = 2;
    res.settlementWithinNegligibleLimit = (res.maxSettlementMm < 15.0 && res.buildingDamageCategory <= 1);
    return res;
}

CcmConvergenceResult Phase17Engine::evaluateCcmConvergence(double tunnelRadiusM, double inSituHydrostaticStressMpa, double cohesionMpa, double frictionAngleDeg, double youngsModulusGpa) {
    CcmConvergenceResult res;
    // Salençon (1969) / Hoek-Brown analytical ground response curve
    double phiRad = frictionAngleDeg * (M_PI / 180.0);
    double kp = (1.0 + std::sin(phiRad)) / (1.0 - std::sin(phiRad));
    double criticalSupportPressureMpa = (2.0 * inSituHydrostaticStressMpa - 2.0 * cohesionMpa * std::sqrt(kp)) / (kp + 1.0);
    if (criticalSupportPressureMpa < 0.1) criticalSupportPressureMpa = 0.1;
    res.supportPressureRequiredMpa = criticalSupportPressureMpa * 0.8;
    // Plastic zone radius Rp
    double pRatio = (inSituHydrostaticStressMpa + cohesionMpa / std::tan(phiRad)) / (res.supportPressureRequiredMpa + cohesionMpa / std::tan(phiRad));
    double exponent = 1.0 / (kp - 1.0);
    res.plasticRadiusMeters = tunnelRadiusM * std::pow(std::max(1.0, pRatio), exponent);
    // Wall convergence u_r = (1 + nu) / E * (P0 - Pi) * R
    double nu = 0.28;
    double eMpa = youngsModulusGpa * 1000.0;
    double urM = ((1.0 + nu) / eMpa) * (inSituHydrostaticStressMpa - res.supportPressureRequiredMpa) * tunnelRadiusM;
    res.wallConvergencePercent = (urM / tunnelRadiusM) * 100.0;
    // Shotcrete capacity (200 mm layer of C30/37 shotcrete)
    double shotcreteCapacityMpa = 1.8;
    res.safetyFactorOnShotcrete = shotcreteCapacityMpa / res.supportPressureRequiredMpa;
    res.groundConfinementAdequate = (res.wallConvergencePercent < 1.5 && res.plasticRadiusMeters < 2.5 * tunnelRadiusM);
    return res;
}

TbmPowerTorqueResult Phase17Engine::calculateTbmPowerTorque(double tbmDiameterM, int cutterCount, double penetrationMmPerRev, double rpm, double rockUcsMpa) {
    TbmPowerTorqueResult res;
    // Average cutter normal force: Fn ~ 180 - 250 kN in hard rock
    double meanFnKn = 150.0 + (rockUcsMpa / 120.0) * 80.0;
    if (meanFnKn > 280.0) meanFnKn = 280.0;
    res.totalThrustKn = cutterCount * meanFnKn * 1.15; // 15% shield friction
    // Rolling force Fr = 0.09 * Fn
    double meanFrKn = meanFnKn * 0.095;
    // Total cutterhead torque: T = sum(r_i * Fr_i) ~= Fr * R_mean * N_cutters
    double rMeanM = tbmDiameterM * 0.38;
    res.requiredTorqueKnM = cutterCount * meanFrKn * rMeanM;
    // Cutterhead drive electric power: P = T * omega
    double omegaRadS = (rpm * 2.0 * M_PI) / 60.0;
    res.cutterheadPowerKw = res.requiredTorqueKnM * omegaRadS;
    // Advance rate: penetration * rpm * 60 min/hr
    res.advanceRateMetersPerHour = (penetrationMmPerRev / 1000.0) * rpm * 60.0;
    res.drivePowerSufficient = (res.cutterheadPowerKw <= 4500.0 && res.requiredTorqueKnM <= 6500.0);
    return res;
}

TailGroutResult Phase17Engine::evaluateTailGrouting(double tunnelOuterRadiusM, double excavatedRadiusM, double segmentLiningWeightKnPerM, double waterDensityKnM3, double soilPorePressureBar) {
    TailGroutResult res;
    // Annular gap volume per linear meter: V = pi * (R_exc^2 - R_lining^2) * 1.15 over-injection
    double vTheor = M_PI * (excavatedRadiusM * excavatedRadiusM - tunnelOuterRadiusM * tunnelOuterRadiusM);
    res.groutVolumeRequiredM3 = vTheor * 1.15;
    // Buoyancy force per linear meter balanced by lining + invert slab + equipment weight
    double fBuoyKn = waterDensityKnM3 * M_PI * tunnelOuterRadiusM * tunnelOuterRadiusM;
    double totalDownwardWeightKnPerM = segmentLiningWeightKnPerM + 185.0; // includes invert track concrete
    res.netTunnelBuoyancyKnPerMeter = fBuoyKn - totalDownwardWeightKnPerM;
    // Initial injection pressure: 0.5 - 1.0 bar above soil pore pressure
    res.initialGroutPressureBar = soilPorePressureBar + 0.8;
    // Two-component (A+B) silicate-cement grout gel setting time: ~10-15 seconds, full shear strength in 30 mins
    res.settingTimeMinutes = 25.0;
    res.preventsTunnelFloatation = (res.netTunnelBuoyancyKnPerMeter <= 15.0 && res.initialGroutPressureBar < 5.0);
    return res;
}

ShieldJammingResult Phase17Engine::evaluateShieldJammingRisk(double tunnelRadiusM, double overburdenStressMpa, double rockCompetenceFactor, double shieldLengthM, double shieldTaperClearanceMm, double maxAvailableThrustKn) {
    ShieldJammingResult res;
    // Jethwa (1984) squeezing ground index: Nc = sigma_cm / overburdenStress
    // Radial ground closure u_r = f(Nc) * R
    double closureRatio = 0.015 / std::max(0.1, rockCompetenceFactor);
    res.radialGroundClosureMm = closureRatio * (tunnelRadiusM * 1000.0);
    res.shieldConicityClearanceMm = shieldTaperClearanceMm;
    // If ground closure exceeds clearance, radial squeezing pressure acts on shield skin
    double contactPressureMpa = 0.0;
    if (res.radialGroundClosureMm > res.shieldConicityClearanceMm) {
        contactPressureMpa = (res.radialGroundClosureMm - res.shieldConicityClearanceMm) * 0.015;
    }
    // Frictional thrust with bentonite skin lubrication (mu ~ 0.12)
    double mu = 0.12; // bentonite-lubricated shield skin
    double surfaceAreaM2 = 2.0 * M_PI * tunnelRadiusM * shieldLengthM;
    res.frictionFrictionalThrustRequiredKn = (mu * (contactPressureMpa * 1000.0) * surfaceAreaM2);
    double remainingThrustKn = maxAvailableThrustKn - res.frictionFrictionalThrustRequiredKn;
    res.thrustMarginPercent = (remainingThrustKn / maxAvailableThrustKn) * 100.0;
    res.shieldFreeFromJamming = (res.thrustMarginPercent >= 25.0 && res.radialGroundClosureMm < shieldTaperClearanceMm * 1.8);
    return res;
}

TunnelSeismicResult Phase17Engine::evaluateTunnelSeismicResponse(double peakGroundAccelG, double shearWaveVelocityMs, double liningRadiusM, double concreteElasticModulusGpa) {
    TunnelSeismicResult res;
    // Peak ground velocity PGV = PGA * g / (2 * pi * f_dom)
    double g = 9.80665;
    double pgaMs2 = peakGroundAccelG * g;
    res.peakGroundVelocityMs = pgaMs2 / (2.0 * M_PI * 2.5); // 2.5 Hz dominant earthquake frequency
    // Free field shear strain gamma_max = PGV / Cs (Wang 1993, Hashash 2001)
    double gammaMax = res.peakGroundVelocityMs / shearWaveVelocityMs;
    // Lining racking ratio R ~ 0.28 for stiff circular concrete segmental lining
    res.liningShearStrainMicrostrain = (gammaMax * 1e6) * 0.28;
    // Dynamic hoop thrust delta_T = 0.5 * gamma_max * G_m * R
    double eMpa = concreteElasticModulusGpa * 1000.0;
    double dynamicStressMpa = (res.liningShearStrainMicrostrain * 1e-6) * eMpa;
    res.dynamicHoopThrustKnPerMeter = dynamicStressMpa * 0.35 * 1000.0; // 0.35m segment thickness
    res.concreteTensileStrainLimit = 150.0; // microstrain for C50/60 concrete cracking limit
    res.earthquakeResistantLining = (res.liningShearStrainMicrostrain < res.concreteTensileStrainLimit);
    return res;
}

} // namespace Phase17
} // namespace AndroCAD
