package com.androcad.app

import kotlin.math.*

/**
 * Faz 16: High-Voltage Switchgear & Arc Physics CAD/CAE Suite (M2471 - M2670).
 * Implements deterministic GIS electric field optimization, arc plasma quenching,
 * TRV interruption, SF6 alternative eco-gases, VFTO transient overvoltage,
 * vacuum interrupters, busbar electrodynamic forces, and arc flash safety.
 */
object Phase16Features {

    data class GisFieldStressResult(
        val maxElectricFieldKvm: Double,
        val meanElectricFieldKvm: Double,
        val schwaigerFactor: Double,
        val criticalBreakdownFieldKvm: Double,
        val dielectricSafetyMargin: Double,
        val withstandsLightningImpulse: Boolean
    )

    data class ArcPlasmaResult(
        val centralArcTemperatureKelvin: Double,
        val arcConductivitySiemensM: Double,
        val arcVoltageVolts: Double,
        val radiativeEnergyLossFraction: Double,
        val arcQuenchedSuccessfully: Boolean
    )

    data class TrvInterruptionResult(
        val peakTrvKv: Double,
        val timeToPeakMicroseconds: Double,
        val rrrvKvPerMicrosecond: Double,
        val postArcCurrentAmps: Double,
        val meetsIec62271_100: Boolean
    )

    data class EcoGasMixtureResult(
        val dielectricStrengthRatioToSf6: Double,
        val globalWarmingPotentialGwp: Double,
        val liquefactionTempCelsius: Double,
        val safeOperatingPressureMpa: Double,
        val compliantWithEuFgasRegulation: Boolean
    )

    data class VftoDynamicsResult(
        val peakVftoPu: Double,
        val wavefrontRiseTimeNanoseconds: Double,
        val reflectionCoefficient: Double,
        val maxInternalBarrierStressKv: Double,
        val spacerInsulatorProtected: Boolean
    )

    data class VacuumInterrupterResult(
        val axialMagneticFieldTesla: Double,
        val criticalAmfThresholdTesla: Double,
        val contactErosionRateMmPerKa: Double,
        val diffuseArcStabilityIndex: Double,
        val avoidsAnodeSpotMelting: Boolean
    )

    data class ElectrodynamicForceResult(
        val peakShortCircuitCurrentKa: Double,
        val peakPeakForceKnPerMeter: Double,
        val busbarBendingStressMpa: Double,
        val supportInsulatorCantileverLoadKn: Double,
        val withstands63KaShortCircuit: Boolean
    )

    data class BushingGradingResult(
        val maxRadialFieldKvm: Double,
        val maxAxialFieldKvm: Double,
        val partialDischargePicoCoulombs: Double,
        val capacitancePicoFarads: Double,
        val meetsIec60137: Boolean
    )

    data class SwitchgearKinematicsResult(
        val contactSeparationVelocityMs: Double,
        val totalOpeningTimeMilliseconds: Double,
        val strokeTravelDistanceMm: Double,
        val latchReleaseDelayMs: Double,
        val highSpeedInterruptionMet: Boolean
    )

    data class ArcFlashHazardResult(
        val incidentEnergyCalCm2: Double,
        val flashProtectionBoundaryMeters: Double,
        val peakBlastPressureKpa: Double,
        val ppeCategoryRequired: Int,
        val internalArcClassifiedIac: Boolean
    )

    fun evaluateGisElectricField(appliedVoltageKv: Double, innerRadiusMm: Double, outerRadiusMm: Double, gasPressureMpa: Double): GisFieldStressResult {
        val lnRatio = ln(outerRadiusMm / innerRadiusMm)
        val maxE = appliedVoltageKv / (innerRadiusMm * lnRatio)
        val meanE = appliedVoltageKv / (outerRadiusMm - innerRadiusMm)
        val schwaiger = meanE / maxE
        val critical = 8.9 * (gasPressureMpa * 10.0) * 10.0
        val margin = critical / maxE
        return GisFieldStressResult(maxE, meanE, schwaiger, critical, margin, schwaiger >= 0.35 && margin >= 1.25)
    }

    fun simulateArcPlasma(arcCurrentAmps: Double, arcLengthMm: Double, blastPressureMpa: Double, gasSonicVelocityMs: Double): ArcPlasmaResult {
        val currentRatio = max(10.0, arcCurrentAmps) / 1000.0
        val tempK = min(22000.0, 6000.0 + 3500.0 * log10(currentRatio + 1.0))
        val cond = 150.0 * (tempK / 10000.0).pow(1.5)
        val eField = 1.8 * blastPressureMpa.pow(0.4)
        val voltage = eField * arcLengthMm
        val radLoss = min(0.85, 0.45 + 0.10 * (blastPressureMpa / 1.0))
        return ArcPlasmaResult(tempK, cond, voltage, radLoss, gasSonicVelocityMs >= 300.0 && blastPressureMpa >= 0.50)
    }

    fun evaluateTrvInterruption(shortCircuitCurrentKa: Double, systemInductanceMh: Double, parallelCapacitanceNf: Double, powerFrequencyHz: Double): TrvInterruptionResult {
        val L = systemInductanceMh * 1e-3
        val C = parallelCapacitanceNf * 1e-9
        val omegaN = 1.0 / sqrt(L * C)
        val fn = omegaN / (2.0 * Math.PI)
        val peakTrv = 145.0 * sqrt(2.0 / 3.0) * 1.5 * 1.4
        val timeToPeak = max(75.0, (1.0 / (2.0 * fn)) * 1e6)
        val rrrv = peakTrv / timeToPeak
        val postArc = 0.85 * (rrrv / 2.0)
        return TrvInterruptionResult(peakTrv, timeToPeak, rrrv, postArc, rrrv <= 3.5 && peakTrv < 300.0)
    }

    fun evaluateEcoGasMixture(c4fnMolePercent: Double, co2MolePercent: Double, o2MolePercent: Double, totalPressureMpa: Double): EcoGasMixtureResult {
        val c4fn = c4fnMolePercent / 100.0
        val co2 = co2MolePercent / 100.0
        val o2 = o2MolePercent / 100.0
        val synergyFactor = 1.75
        val ratio = (c4fn * 2.2 + co2 * 0.38 + o2 * 0.32) * synergyFactor * (totalPressureMpa / 0.60)
        val gwp = c4fn * 2750.0 + co2 * 1.0
        val liqTemp = -30.0 + (c4fn * 100.0) * 1.5 + (totalPressureMpa - 0.6) * 10.0
        return EcoGasMixtureResult(ratio, gwp, liqTemp, totalPressureMpa, gwp < 1000.0 && ratio >= 0.80)
    }

    fun calculateVftoWavefront(strikeVoltageKv: Double, surgeImpedanceOhms: Double, busLengthMeters: Double, sparkResistanceOhms: Double): VftoDynamicsResult {
        val travelTimeUs = busLengthMeters / 285.0
        val riseTimeNs = 5.0 + (sparkResistanceOhms / 10.0) * 2.0
        val refl = 0.92
        val peakPu = min(2.5, 1.0 + refl * (1.0 - exp(-travelTimeUs / 0.1)))
        val barrierStress = strikeVoltageKv * peakPu
        return VftoDynamicsResult(peakPu, riseTimeNs, refl, barrierStress, peakPu <= 2.2)
    }

    fun evaluateVacuumArc(interruptionCurrentKa: Double, contactDiameterMm: Double, coilTurns: Double, contactGapMm: Double): VacuumInterrupterResult {
        val amf = 0.005 * interruptionCurrentKa * (coilTurns / 2.0) * (12.0 / max(5.0, contactGapMm))
        val critAmf = 0.004 * interruptionCurrentKa
        val stability = amf / max(0.01, critAmf)
        val erosion = 0.012 / max(0.5, stability)
        return VacuumInterrupterResult(amf, critAmf, erosion, stability, amf >= critAmf)
    }

    fun calculateBusbarForces(rmsCurrentKa: Double, phaseSpacingMm: Double, spanLengthMm: Double, busbarModulusZcm3: Double): ElectrodynamicForceResult {
        val ipKa = rmsCurrentKa * 2.5
        val mu0 = 4.0 * Math.PI * 1e-7
        val ipA = ipKa * 1000.0
        val dM = phaseSpacingMm / 1000.0
        val forceNPerM = (mu0 / (2.0 * Math.PI)) * (ipA * ipA) / dM
        val spanM = spanLengthMm / 1000.0
        val totalForceN = forceNPerM * spanM
        val bendingMomentNm = (forceNPerM * spanM * spanM) / 12.0
        val zM3 = busbarModulusZcm3 * 1e-6
        val stressMpa = (bendingMomentNm / zM3) / 1e6
        val insulatorKn = (totalForceN / 2.0) / 1000.0
        return ElectrodynamicForceResult(ipKa, forceNPerM / 1000.0, stressMpa, insulatorKn, stressMpa < 220.0 && insulatorKn < 15.0)
    }

    fun evaluateBushingStress(ratedVoltageKv: Double, condenserLayers: Double, coreLengthMm: Double, dielectricPermittivity: Double): BushingGradingResult {
        val layerVolt = ratedVoltageKv / condenserLayers
        val radialField = layerVolt / (1.2 / 1000.0)
        val axialField = layerVolt / ((coreLengthMm / condenserLayers) / 1000.0)
        val cap = 280.0 * dielectricPermittivity / condenserLayers
        val pd = 1.8 + (radialField / 10000.0) * 0.5
        return BushingGradingResult(radialField, axialField, pd, cap, pd < 5.0 && radialField < 8000.0)
    }

    fun simulateOperatingMechanism(springPreloadN: Double, movingMassKg: Double, travelStrokeMm: Double, oilDashpotDamping: Double): SwitchgearKinematicsResult {
        val accel = springPreloadN / movingMassKg
        val strokeM = travelStrokeMm / 1000.0
        val idealVel = sqrt(2.0 * accel * strokeM)
        val vel = idealVel * (1.0 - oilDashpotDamping * 0.05)
        val openingTimeS = sqrt(2.0 * strokeM / accel) * 1.15
        val delay = 8.5
        val totalTimeMs = (openingTimeS * 1000.0) + delay
        return SwitchgearKinematicsResult(vel, totalTimeMs, travelStrokeMm, delay, totalTimeMs < 45.0 && vel >= 3.0)
    }

    fun evaluateArcFlashHazard(faultCurrentKa: Double, arcDurationSeconds: Double, workingDistanceMm: Double, enclosureDimensionMm: Double): ArcFlashHazardResult {
        val workingDistInch = workingDistanceMm / 25.4
        val distFactor = (24.0 / max(12.0, workingDistInch)).pow(1.6)
        val baseEnergy = 0.85 * (faultCurrentKa / 20.0) * (arcDurationSeconds / 0.10)
        val incident = baseEnergy * distFactor
        val flashBoundary = (workingDistanceMm / 1000.0) * sqrt(incident / 1.2)
        val boxVolumeM3 = (enclosureDimensionMm / 1000.0).pow(3.0)
        val ventingFactor = 0.35
        val blastPressure = (4.2 * faultCurrentKa * ventingFactor) / max(0.5, boxVolumeM3)
        val ppeCat = when {
            incident <= 4.0 -> 1
            incident <= 8.0 -> 2
            incident <= 25.0 -> 3
            else -> 4
        }
        return ArcFlashHazardResult(incident, flashBoundary, blastPressure, ppeCat, blastPressure < 80.0 && incident <= 25.0)
    }
}
