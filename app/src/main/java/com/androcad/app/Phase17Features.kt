package com.androcad.app

import kotlin.math.*

/**
 * Faz 17: Geotechnical Tunneling & TBM Cutterhead CAD/CAE Suite (M2671 - M2870).
 * Implements deterministic Hoek-Brown rock failure, CSM disc cutter indentation,
 * segmental lining joint mechanics, EPB face pressure balance, Peck settlement,
 * CCM convergence-confinement, TBM torque/thrust power, and shield jamming prevention.
 */
object Phase17Features {

    data class HoekBrownResult(
        val majorPrincipalStrengthMpa: Double,
        val rockMassModulusGpa: Double,
        val tensileStrengthMpa: Double,
        val safetyFactor: Double,
        val rockMassStable: Boolean
    )

    data class CsmCutterForceResult(
        val normalForceKn: Double,
        val rollingForceKn: Double,
        val specificEnergyMjm3: Double,
        val penetrationMmPerRev: Double,
        val cutterLoadWithinBearingLimit: Boolean
    )

    data class SegmentalLiningResult(
        val jointRotationalStiffnessKnMPerRad: Double,
        val maxLiningBendingMomentKnM: Double,
        val jointOpeningDisplacementMm: Double,
        val boltTensionStressMpa: Double,
        val liningIntegrityMaintained: Boolean
    )

    data class EpbChamberPressureResult(
        val earthPressureAtCrownBar: Double,
        val hydrostaticPressureBar: Double,
        val requiredChamberPressureBar: Double,
        val faceStabilitySafetyFactor: Double,
        val surfaceCollapseAvoided: Boolean
    )

    data class PeckSettlementResult(
        val maxSettlementMm: Double,
        val troughWidthParameterMeters: Double,
        val maxAngularDistortion: Double,
        val buildingDamageCategory: Int,
        val settlementWithinNegligibleLimit: Boolean
    )

    data class CcmConvergenceResult(
        val plasticRadiusMeters: Double,
        val wallConvergencePercent: Double,
        val supportPressureRequiredMpa: Double,
        val safetyFactorOnShotcrete: Double,
        val groundConfinementAdequate: Boolean
    )

    data class TbmPowerTorqueResult(
        val totalThrustKn: Double,
        val requiredTorqueKnM: Double,
        val cutterheadPowerKw: Double,
        val advanceRateMetersPerHour: Double,
        val drivePowerSufficient: Boolean
    )

    data class TailGroutResult(
        val groutVolumeRequiredM3: Double,
        val netTunnelBuoyancyKnPerMeter: Double,
        val initialGroutPressureBar: Double,
        val settingTimeMinutes: Double,
        val preventsTunnelFloatation: Boolean
    )

    data class ShieldJammingResult(
        val radialGroundClosureMm: Double,
        val shieldConicityClearanceMm: Double,
        val frictionFrictionalThrustRequiredKn: Double,
        val thrustMarginPercent: Double,
        val shieldFreeFromJamming: Boolean
    )

    data class TunnelSeismicResult(
        val peakGroundVelocityMs: Double,
        val liningShearStrainMicrostrain: Double,
        val dynamicHoopThrustKnPerMeter: Double,
        val concreteTensileStrainLimit: Double,
        val earthquakeResistantLining: Boolean
    )

    fun evaluateHoekBrown(minorPrincipalStressMpa: Double, ucsMpa: Double, gsi: Double, miParameter: Double, disturbanceD: Double): HoekBrownResult {
        val mb = miParameter * exp((gsi - 100.0) / (28.0 - 14.0 * disturbanceD))
        val s = exp((gsi - 100.0) / (9.0 - 3.0 * disturbanceD))
        val a = 0.5 + (exp(-gsi / 15.0) - exp(-20.0 / 3.0)) / 6.0
        val term = max(0.0, mb * (minorPrincipalStressMpa / ucsMpa) + s)
        val majorStrength = minorPrincipalStressMpa + ucsMpa * term.pow(a)
        val modulus = 100.0 * (1.0 - disturbanceD / 2.0) / (1.0 + exp((75.0 + 25.0 * disturbanceD - gsi) / 11.0))
        val tensile = -s * ucsMpa / mb
        val inducedStress = 2.5 * minorPrincipalStressMpa + 15.0
        val sf = majorStrength / inducedStress
        return HoekBrownResult(majorStrength, modulus, tensile, sf, sf >= 1.20 && gsi >= 30.0)
    }

    fun calculateCsmCutterForces(cutterDiameterInch: Double, tipWidthMm: Double, spacingMm: Double, penetrationMm: Double, ucsMpa: Double, shearStrengthMpa: Double): CsmCutterForceResult {
        val radiusMm = (cutterDiameterInch * 25.4) / 2.0
        val phiRad = sqrt(2.0 * penetrationMm / radiusMm)
        val basePressure = 0.42 * (ucsMpa * ucsMpa * shearStrengthMpa).pow(1.0 / 3.0)
        val spacingRatio = spacingMm / (penetrationMm * phiRad * radiusMm * 0.1)
        val normalKn = min(320.0, (radiusMm * tipWidthMm * phiRad * basePressure * 0.005) + (spacingRatio * 8.0))
        val rollingKn = normalKn * (phiRad / 2.0)
        val cutVolumeM3 = (spacingMm * penetrationMm * 1000.0) * 1e-9
        val cuttingEnergyJoules = (rollingKn * 1000.0) * (2.0 * Math.PI * radiusMm / 1000.0)
        val specificEnergy = (cuttingEnergyJoules / cutVolumeM3) * 1e-6
        return CsmCutterForceResult(normalKn, rollingKn, specificEnergy, penetrationMm, normalKn <= 280.0 && rollingKn <= 45.0)
    }

    fun evaluateSegmentalLining(tunnelRadiusM: Double, overburdenDepthM: Double, groundStiffnessMpa: Double, segmentThicknessM: Double, jointBoltPreloadKn: Double): SegmentalLiningResult {
        val soilGamma = 20.0
        val totalOverburden = soilGamma * overburdenDepthM
        val liningInertia = (segmentThicknessM * segmentThicknessM * segmentThicknessM) / 12.0
        val eConcrete = 35.0 * 1e6
        val k0 = 0.65
        val flexFactor = (eConcrete * liningInertia) / (groundStiffnessMpa * 1000.0 * tunnelRadiusM.pow(3.0))
        val maxMoment = ((1.0 - k0) * totalOverburden * tunnelRadiusM * tunnelRadiusM / 4.0) / (1.0 + 3.0 * flexFactor)
        val jointStiffness = 15000.0 * (segmentThicknessM / 0.35)
        val jointOpening = (maxMoment / jointStiffness) * 1000.0 * 0.05
        val boltArea = 560.0
        val boltStress = (jointBoltPreloadKn * 1000.0) / boltArea + (maxMoment * 1000.0 / (segmentThicknessM * 1000.0 * boltArea * 0.1))
        return SegmentalLiningResult(jointStiffness, maxMoment, jointOpening, boltStress, jointOpening < 2.0 && boltStress < 640.0)
    }

    fun evaluateEpbFacePressure(depthToCrownM: Double, waterTableDepthM: Double, soilDensityKnM3: Double, soilCohesionKpa: Double, internalFrictionAngleDeg: Double): EpbChamberPressureResult {
        val waterHead = max(0.0, depthToCrownM - waterTableDepthM)
        val hydroBar = (waterHead * 9.81) / 100.0
        val sigmaV = soilDensityKnM3 * depthToCrownM
        val phiRad = internalFrictionAngleDeg * (Math.PI / 180.0)
        val k0 = 1.0 - sin(phiRad)
        val sigmaH = max(0.2 * sigmaV, k0 * sigmaV - 2.0 * soilCohesionKpa * sqrt(k0))
        val earthBar = sigmaH / 100.0
        val reqChamber = hydroBar + earthBar + 0.20
        val sf = reqChamber / (earthBar + hydroBar)
        return EpbChamberPressureResult(earthBar, hydroBar, reqChamber, sf, sf >= 1.05 && reqChamber < 6.0)
    }

    fun calculatePeckSettlement(tunnelDepthM: Double, tunnelDiameterM: Double, volumeLossPercent: Double, offsetDistanceM: Double): PeckSettlementResult {
        val K = 0.45
        val i = K * tunnelDepthM
        val r = tunnelDiameterM / 2.0
        val vs = (volumeLossPercent / 100.0) * (Math.PI * r * r)
        val sMax = (vs / (sqrt(2.0 * Math.PI) * i)) * 1000.0
        val maxDistortion = (sMax / 1000.0) / (i * sqrt(exp(1.0)))
        val damageCat = when {
            maxDistortion < 1.0 / 500.0 && sMax < 10.0 -> 0
            sMax < 25.0 -> 1
            else -> 2
        }
        return PeckSettlementResult(sMax, i, maxDistortion, damageCat, sMax < 15.0 && damageCat <= 1)
    }

    fun evaluateCcmConvergence(tunnelRadiusM: Double, inSituHydrostaticStressMpa: Double, cohesionMpa: Double, frictionAngleDeg: Double, youngsModulusGpa: Double): CcmConvergenceResult {
        val phiRad = frictionAngleDeg * (Math.PI / 180.0)
        val kp = (1.0 + sin(phiRad)) / (1.0 - sin(phiRad))
        val critSupport = max(0.1, (2.0 * inSituHydrostaticStressMpa - 2.0 * cohesionMpa * sqrt(kp)) / (kp + 1.0))
        val reqSupport = critSupport * 0.8
        val pRatio = (inSituHydrostaticStressMpa + cohesionMpa / tan(phiRad)) / (reqSupport + cohesionMpa / tan(phiRad))
        val exponent = 1.0 / (kp - 1.0)
        val plasticR = tunnelRadiusM * max(1.0, pRatio).pow(exponent)
        val nu = 0.28
        val eMpa = youngsModulusGpa * 1000.0
        val urM = ((1.0 + nu) / eMpa) * (inSituHydrostaticStressMpa - reqSupport) * tunnelRadiusM
        val convPercent = (urM / tunnelRadiusM) * 100.0
        val sfShotcrete = 1.8 / reqSupport
        return CcmConvergenceResult(plasticR, convPercent, reqSupport, sfShotcrete, convPercent < 1.5 && plasticR < 2.5 * tunnelRadiusM)
    }

    fun calculateTbmPowerTorque(tbmDiameterM: Double, cutterCount: Int, penetrationMmPerRev: Double, rpm: Double, rockUcsMpa: Double): TbmPowerTorqueResult {
        val meanFnKn = min(280.0, 150.0 + (rockUcsMpa / 120.0) * 80.0)
        val totalThrust = cutterCount * meanFnKn * 1.15
        val meanFrKn = meanFnKn * 0.095
        val rMeanM = tbmDiameterM * 0.38
        val torqueKnM = cutterCount * meanFrKn * rMeanM
        val omega = (rpm * 2.0 * Math.PI) / 60.0
        val powerKw = torqueKnM * omega
        val advanceMPerHr = (penetrationMmPerRev / 1000.0) * rpm * 60.0
        return TbmPowerTorqueResult(totalThrust, torqueKnM, powerKw, advanceMPerHr, powerKw <= 4500.0 && torqueKnM <= 6500.0)
    }

    fun evaluateTailGrouting(tunnelOuterRadiusM: Double, excavatedRadiusM: Double, segmentLiningWeightKnPerM: Double, waterDensityKnM3: Double, soilPorePressureBar: Double): TailGroutResult {
        val vTheor = Math.PI * (excavatedRadiusM * excavatedRadiusM - tunnelOuterRadiusM * tunnelOuterRadiusM)
        val groutVol = vTheor * 1.15
        val fBuoy = waterDensityKnM3 * Math.PI * tunnelOuterRadiusM * tunnelOuterRadiusM
        val totalDownward = segmentLiningWeightKnPerM + 185.0
        val netBuoy = fBuoy - totalDownward
        val initPress = soilPorePressureBar + 0.8
        return TailGroutResult(groutVol, netBuoy, initPress, 25.0, netBuoy <= 15.0 && initPress < 5.0)
    }

    fun evaluateShieldJammingRisk(tunnelRadiusM: Double, overburdenStressMpa: Double, rockCompetenceFactor: Double, shieldLengthM: Double, shieldTaperClearanceMm: Double, maxAvailableThrustKn: Double): ShieldJammingResult {
        val closureRatio = 0.015 / max(0.1, rockCompetenceFactor)
        val closureMm = closureRatio * (tunnelRadiusM * 1000.0)
        val contactPressMpa = if (closureMm > shieldTaperClearanceMm) (closureMm - shieldTaperClearanceMm) * 0.015 else 0.0
        val mu = 0.12
        val surfaceArea = 2.0 * Math.PI * tunnelRadiusM * shieldLengthM
        val fricThrust = (mu * (contactPressMpa * 1000.0) * surfaceArea)
        val remaining = maxAvailableThrustKn - fricThrust
        val margin = (remaining / maxAvailableThrustKn) * 100.0
        return ShieldJammingResult(closureMm, shieldTaperClearanceMm, fricThrust, margin, margin >= 25.0 && closureMm < shieldTaperClearanceMm * 2.0)
    }

    fun evaluateTunnelSeismicResponse(peakGroundAccelG: Double, shearWaveVelocityMs: Double, liningRadiusM: Double, concreteElasticModulusGpa: Double): TunnelSeismicResult {
        val g = 9.80665
        val pgaMs2 = peakGroundAccelG * g
        val pgv = pgaMs2 / (2.0 * Math.PI * 2.5)
        val gammaMax = pgv / shearWaveVelocityMs
        val strainMicro = (gammaMax * 1e6) * 0.28
        val eMpa = concreteElasticModulusGpa * 1000.0
        val dynamicStress = (strainMicro * 1e-6) * eMpa
        val dynamicThrust = dynamicStress * 0.35 * 1000.0
        return TunnelSeismicResult(pgv, strainMicro, dynamicThrust, 150.0, strainMicro < 150.0)
    }
}
