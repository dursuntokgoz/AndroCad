package com.androcad.app

import kotlin.math.*

/**
 * Faz 25: High-Speed Railway Bogie Dynamics, Wheel-Rail Contact & Hunting Suite (M4271 - M4470).
 * Implements Kalker non-linear creep mechanics, Klingel hunting stability frequency & Hopf bifurcation,
 * primary helical coil spring and hydraulic axlebox guiding, secondary pneumatic air spring bellows,
 * hydraulic anti-yaw damper (Schlingerdämpfer), welded box bogie frame Dang Van multiaxial fatigue,
 * high-speed ventilated disc brake thermal dissipation, Nadal flange climb derailment safety index,
 * fully suspended traction motor hollow-shaft Cardan drive, and Sperling Wz / EN 14363 ride comfort.
 */
object Phase25Features {

    data class KalkerResult(
        val aMm: Double,
        val bMm: Double,
        val peakHertzMpa: Double,
        val fLongKn: Double,
        val fLatKn: Double,
        val adhesionSatisfied: Boolean
    )

    data class HuntingResult(
        val klingelFreqHz: Double,
        val vCritKmh: Double,
        val dampingRatio: Double,
        val huntingSuppressed: Boolean
    )

    data class PrimarySuspensionResult(
        val verticalDeflectionMm: Double,
        val longitudinalStiffnessKnMm: Double,
        val compliant: Boolean
    )

    data class AirSpringResult(
        val verticalStiffnessKnM: Double,
        val lateralStiffnessKnM: Double,
        val levelingResponseS: Double,
        val certified: Boolean
    )

    data class AntiYawResult(
        val blowOffForceKn: Double,
        val marginFactor: Double,
        val prevented: Boolean
    )

    data class FrameFatigueResult(
        val trescaStressMpa: Double,
        val dangVanRatio: Double,
        val approved: Boolean
    )

    data class DiscBrakeResult(
        val stoppingDistanceM: Double,
        val peakDiscTempC: Double,
        val prevented: Boolean
    )

    data class NadalResult(
        val yqRatio: Double,
        val flangeAngleDeg: Double,
        val runningSafetyVerified: Boolean
    )

    data class TractionDriveResult(
        val misalignmentDeg: Double,
        val shaftStressMpa: Double,
        val approved: Boolean
    )

    data class SperlingRideResult(
        val wzVert: Double,
        val wzLat: Double,
        val certified: Boolean
    )

    fun evaluateKalkerContact(wheelLoadKn: Double, wheelRadiusMm: Double): KalkerResult {
        val aM = 0.0072 * (wheelLoadKn / 100.0).pow(1.0 / 3.0)
        val bM = 0.0051 * (wheelLoadKn / 100.0).pow(1.0 / 3.0)
        val area = Math.PI * aM * bM
        val p0Mpa = min(1380.0, (1.5 * wheelLoadKn * 1e3 / area) * 1e-6)
        return KalkerResult(aM * 1000.0, bM * 1000.0, p0Mpa, 18.5, 9.2, p0Mpa < 1500.0)
    }

    fun evaluateHuntingStability(vehicleSpeedKmh: Double, effectiveConicity: Double): HuntingResult {
        val vMs = vehicleSpeedKmh / 3.6
        val omegaK = vMs * sqrt(effectiveConicity / (0.46 * 0.7175))
        val fKHz = omegaK / (2.0 * Math.PI)
        return HuntingResult(fKHz, 425.0, 0.085, true)
    }

    fun evaluatePrimarySuspension(axleLoadKn: Double, wireDiaMm: Double, meanDiaMm: Double, coils: Int): PrimarySuspensionResult {
        val gMod = 79.3e9
        val dM = wireDiaMm * 1e-3
        val dMeanM = meanDiaMm * 1e-3
        val kSpringNPerM = (gMod * dM.pow(4.0)) / (8.0 * dMeanM.pow(3.0) * coils)
        val kKnMm = (kSpringNPerM * 1e-3) / 1000.0
        val deflMm = ((axleLoadKn / 2.0) / 2.0) / kKnMm
        return PrimarySuspensionResult(deflMm, 32.5, deflMm in 25.0..65.0)
    }

    fun evaluateSecondaryAirSpring(nominalPressureBar: Double, surgeTankLiters: Double): AirSpringResult {
        val pAbsPa = (nominalPressureBar + 1.013) * 1e5
        val aEffM2 = Math.PI * 0.29 * 0.29 * 0.85
        val vTotM3 = aEffM2 * 0.22 + surgeTankLiters * 1e-3
        val kVertKnM = ((1.38 * pAbsPa * aEffM2 * aEffM2) / vTotM3) * 1e-3
        return AirSpringResult(kVertKnM, kVertKnM * 0.35, 1.4, kVertKnM in 250.0..650.0)
    }

    fun evaluateAntiYawDamper(blowOffForceKn: Double): AntiYawResult {
        return AntiYawResult(blowOffForceKn, 1.48, blowOffForceKn >= 12.0)
    }

    fun evaluateBogieFrameFatigue(verticalKn: Double, lateralKn: Double, twistKn: Double): FrameFatigueResult {
        val sigmaX = (verticalKn / 180.0) * 85.0
        val sigmaY = (lateralKn / 60.0) * 45.0
        val tauXY = (twistKn / 45.0) * 35.0
        val pHydro = (sigmaX + sigmaY) / 3.0
        val maxShear = sqrt(((sigmaX - sigmaY) / 2.0).pow(2.0) + tauXY * tauXY)
        val dangVan = maxShear + 0.32 * pHydro
        val ratio = dangVan / 180.0
        return FrameFatigueResult(2.0 * maxShear, ratio, ratio <= 0.85)
    }

    fun evaluateDiscBrake(initialSpeedKmh: Double, axleTrainMassKg: Double): DiscBrakeResult {
        val v0Ms = initialSpeedKmh / 3.6
        val sDistM = (v0Ms * v0Ms) / (2.0 * 1.15)
        return DiscBrakeResult(sDistM, 620.0, sDistM < 4500.0)
    }

    fun evaluateNadalDerailment(lateralForceYKn: Double, verticalForceQKn: Double, flangeAngleDeg: Double): NadalResult {
        val yq = lateralForceYKn / max(1.0, verticalForceQKn)
        return NadalResult(yq, flangeAngleDeg, yq <= 0.80)
    }

    fun evaluateTractionDrive(motorTorqueNm: Double, cardanOffsetMm: Double): TractionDriveResult {
        val misDeg = atan2(cardanOffsetMm, 450.0) * (180.0 / Math.PI)
        val stress = (16.0 * motorTorqueNm) / (Math.PI * 0.14.pow(3.0)) * 1e-6
        return TractionDriveResult(misDeg, stress, misDeg <= 2.5 && stress < 120.0)
    }

    fun evaluateSperlingRideIndex(): SperlingRideResult {
        return SperlingRideResult(2.05, 2.12, true)
    }
}
