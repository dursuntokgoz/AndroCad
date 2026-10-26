package com.androcad.app

import kotlin.math.*

/**
 * Faz 15: Nuclear CAD / CAE Engineering Suite (M2271 - M2470).
 * Implements deterministic nuclear reactor pressure vessel stress analysis,
 * thermal creep relaxation, neutron radiation embrittlement, gamma attenuation,
 * CRDM emergency scram dynamics, DNBR thermal margin, and containment capacity.
 */
object Phase15Features {

    data class RPVStressResult(
        val hoopStressMpa: Double,
        val longitudinalStressMpa: Double,
        val vonMisesStressMpa: Double,
        val allowableStressMpa: Double,
        val safetyFactor: Double,
        val meetsAsmeSectionIII: Boolean
    )

    data class ThermalCreepResult(
        val creepStrainRatePerHour: Double,
        val totalCreepStrainPercent: Double,
        val relaxedStressMpa: Double,
        val ruptureLifeHours: Double,
        val withinCreepLimit: Boolean
    )

    data class RadiationEmbrittlementResult(
        val fluence10e19Ncm2: Double,
        val deltaRtNdtCelsius: Double,
        val adjustedRtNdtCelsius: Double,
        val upperShelfEnergyJoules: Double,
        val acceptableCharpyToughness: Boolean
    )

    data class RadiationShieldingResult(
        val transmissionFraction: Double,
        val buildupFactor: Double,
        val exitDoseRateMilliSvPerHour: Double,
        val attenuationDecades: Double,
        val meetsAlaraStandard: Boolean
    )

    data class ScramDynamicsResult(
        val scramTimeSeconds: Double,
        val impactVelocityMs: Double,
        val peakDecelerationG: Double,
        val rodInsertionDepthPercent: Double,
        val fastScramCompliant: Boolean
    )

    data class DnbrMarginResult(
        val criticalHeatFluxKwM2: Double,
        val actualPeakHeatFluxKwM2: Double,
        val dnbr: Double,
        val minimumAcceptableDnbr: Double,
        val noNucleateBoilingDeparture: Boolean
    )

    data class SeismicResponseResult(
        val naturalFrequencyHz: Double,
        val spectralAccelerationG: Double,
        val maxDisplacementMm: Double,
        val baseShearKn: Double,
        val meetsSseMargin: Boolean
    )

    data class ShroudFivResult(
        val vortexSheddingFrequencyHz: Double,
        val structuralNaturalFrequencyHz: Double,
        val frequencyRatio: Double,
        val crossFlowVelocityMs: Double,
        val lockInResonanceAvoided: Boolean
    )

    data class CladdingOxidationResult(
        val oxideLayerThicknessMicrons: Double,
        val equivalentCladdingReactedPercent: Double,
        val hydrogenUptakePpm: Double,
        val compliesWith10Cfr5046: Boolean
    )

    data class ContainmentCapacityResult(
        val initialPrestressMpa: Double,
        val prestressLossPercent: Double,
        val effectiveTendonTensionKn: Double,
        val ultimateBurstPressureMpa: Double,
        val maintainsContainmentIntegrity: Boolean
    )

    fun calculateRPVStress(internalPressureMpa: Double, innerRadiusMm: Double, wallThicknessMm: Double, allowableStressMpa: Double): RPVStressResult {
        val meanRadius = innerRadiusMm + 0.5 * wallThicknessMm
        val hoop = (internalPressureMpa * meanRadius) / wallThicknessMm
        val longitudinal = (internalPressureMpa * meanRadius) / (2.0 * wallThicknessMm)
        val vonMises = sqrt(hoop * hoop - hoop * longitudinal + longitudinal * longitudinal)
        val sf = allowableStressMpa / vonMises
        return RPVStressResult(hoop, longitudinal, vonMises, allowableStressMpa, sf, sf >= 1.15)
    }

    fun evaluateThermalCreep(initialStressMpa: Double, temperatureKelvin: Double, exposureHours: Double, constantA: Double, stressExponentN: Double): ThermalCreepResult {
        val activationTerm = exp(-250000.0 / (8.314 * temperatureKelvin))
        val rate = constantA * initialStressMpa.pow(stressExponentN) * activationTerm
        val totalStrainPercent = (rate * exposureHours) * 100.0
        val relaxed = max(0.1 * initialStressMpa, initialStressMpa * exp(-rate * 200000.0 * exposureHours / initialStressMpa))
        val rupture = max(exposureHours * 1.5, 10.0.pow((45.0 - initialStressMpa * 0.1) * 1000.0 / temperatureKelvin - 20.0))
        return ThermalCreepResult(rate, totalStrainPercent, relaxed, rupture, totalStrainPercent < 1.0)
    }

    fun evaluateNeutronEmbrittlement(fastNeutronFluenceE19: Double, copperWeightPercent: Double, nickelWeightPercent: Double, initialRtNdtC: Double): RadiationEmbrittlementResult {
        val chemistryFactor = copperWeightPercent * 180.0 + nickelWeightPercent * 70.0 + 35.0
        val fluencePower = 0.28 - 0.10 * log10(max(0.01, fastNeutronFluenceE19))
        val fluenceFactor = fastNeutronFluenceE19.pow(fluencePower)
        val delta = chemistryFactor * fluenceFactor
        val adjusted = initialRtNdtC + delta
        val use = max(68.0, 160.0 - delta * 0.75)
        return RadiationEmbrittlementResult(fastNeutronFluenceE19, delta, adjusted, use, adjusted < 132.0 && use >= 68.0)
    }

    fun calculateShieldingAttenuation(shieldThicknessCm: Double, linearAttenuationCoeffCm1: Double, buildupParamA: Double, incidentDoseRate: Double): RadiationShieldingResult {
        val muf = linearAttenuationCoeffCm1 * shieldThicknessCm
        val b = 1.0 + buildupParamA * muf
        val trans = b * exp(-muf)
        val exit = incidentDoseRate * trans
        val decades = if (trans > 0.0) -log10(trans) else 10.0
        return RadiationShieldingResult(trans, b, exit, decades, exit < 0.02)
    }

    fun simulateCrdmScram(rodMassKg: Double, dropStrokeM: Double, coolantDensityKgM3: Double, dragCoeffCd: Double, scramSpringForceN: Double): ScramDynamicsResult {
        val g = 9.80665
        val netForce = rodMassKg * g + scramSpringForceN
        val accel = netForce / rodMassKg
        val idealTime = sqrt(2.0 * dropStrokeM / accel)
        val dragDelay = 0.15 * (coolantDensityKgM3 / 1000.0) * dragCoeffCd
        val scramTime = idealTime + dragDelay
        val impactVel = (accel * idealTime) * 0.85
        val peakDecel = (impactVel * impactVel) / (2.0 * 0.15 * g)
        return ScramDynamicsResult(scramTime, impactVel, peakDecel, 100.0, scramTime < 2.50)
    }

    fun evaluateThermalHydraulicDnbr(coolantMassFluxKgM2s: Double, pressureMpa: Double, inletEnthalpyKjKg: Double, localHeatFluxKwM2: Double): DnbrMarginResult {
        val chfBase = 2200.0
        val massFluxBonus = (coolantMassFluxKgM2s - 2500.0) * 0.25
        val pressureFactor = 1.0 - abs(pressureMpa - 15.5) * 0.04
        val chf = (chfBase + massFluxBonus) * pressureFactor
        val dnbr = chf / localHeatFluxKwM2
        return DnbrMarginResult(chf, localHeatFluxKwM2, dnbr, 1.30, dnbr >= 1.30)
    }

    fun evaluateSeismicResponse(massTons: Double, stiffnessKnMm: Double, ssePeakGroundAccelG: Double, dampingRatio: Double): SeismicResponseResult {
        val massKg = massTons * 1000.0
        val stiffnessNm = stiffnessKnMm * 1000000.0
        val omega = sqrt(stiffnessNm / massKg)
        val freq = omega / (2.0 * Math.PI)
        val amp = min(3.5, if (dampingRatio > 0.0) 1.0 / (2.0 * dampingRatio) else 10.0)
        val saG = ssePeakGroundAccelG * amp
        val saMs2 = saG * 9.80665
        val disp = (saMs2 / (omega * omega)) * 1000.0
        val baseShear = (massKg * saMs2) / 1000.0
        return SeismicResponseResult(freq, saG, disp, baseShear, disp < 25.0)
    }

    fun evaluateShroudVibration(crossFlowVelocityMs: Double, shroudDiameterMm: Double, naturalFreqHz: Double, strouhalNumber: Double): ShroudFivResult {
        val diameterM = shroudDiameterMm / 1000.0
        val sheddingFreq = (strouhalNumber * crossFlowVelocityMs) / diameterM
        val ratio = sheddingFreq / naturalFreqHz
        val avoided = ratio < 0.70 || ratio > 1.30
        return ShroudFivResult(sheddingFreq, naturalFreqHz, ratio, crossFlowVelocityMs, avoided)
    }

    fun evaluateZircaloyOxidation(peakCladTempCelsius: Double, transientDurationSeconds: Double, initialOxideMicrons: Double): CladdingOxidationResult {
        val tempK = peakCladTempCelsius + 273.15
        val kp = 3.33e7 * exp(-22899.0 / tempK)
        val weightGain = sqrt(kp * transientDurationSeconds)
        val oxideGain = weightGain * 1.5
        val totalOxide = initialOxideMicrons + oxideGain
        val ecr = (totalOxide / 570.0) * 100.0
        val h2 = weightGain * 45.0
        return CladdingOxidationResult(totalOxide, ecr, h2, peakCladTempCelsius <= 1204.0 && ecr < 17.0)
    }

    fun evaluateContainmentTendonLoss(initialStressMpa: Double, concreteCreepShrinkageLossMpa: Double, tendonRelaxationLossMpa: Double, vesselRadiusM: Double, concreteThicknessM: Double): ContainmentCapacityResult {
        val totalLoss = concreteCreepShrinkageLossMpa + tendonRelaxationLossMpa
        val lossPercent = (totalLoss / initialStressMpa) * 100.0
        val effective = initialStressMpa - totalLoss
        val tension = (effective * 1500.0) / 1000.0
        val concreteStrength = 45.0
        val prestressContrib = (effective * 0.018) / vesselRadiusM
        val concreteContrib = (concreteThicknessM * concreteStrength * 0.12) / vesselRadiusM
        val burst = prestressContrib + concreteContrib
        return ContainmentCapacityResult(initialStressMpa, lossPercent, tension, burst, burst >= 0.90 && lossPercent < 22.0)
    }
}
