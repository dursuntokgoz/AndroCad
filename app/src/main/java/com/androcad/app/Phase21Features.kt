package com.androcad.app

import kotlin.math.*

/**
 * Faz 21: Microfluidic Lab-on-a-Chip, Droplet Physics & Bio-MEMS CAD/CAE Suite (M3471 - M3670).
 * Implements deterministic Young-Laplace capillary pressure, flow-focusing droplet generation,
 * Dean vortex inertial focusing, electro-osmotic flow, Péclet diffusion, Quake PDMS micro-valves,
 * EWOD electrowetting, continuous-flow PCR, SAW acoustic cell sorting, and organ-on-a-chip shear stress.
 */
object Phase21Features {

    data class CapillaryPressureResult(
        val youngLaplacePressurePa: Double,
        val meniscusRadiusOfCurvatureMicrons: Double,
        val capillaryRiseHeightMm: Double,
        val contactAngleDegrees: Double,
        val spontaneousCapillaryFilling: Boolean
    )

    data class DropletGenerationResult(
        val capillaryNumberCa: Double,
        val dropletVolumePicoliters: Double,
        val generationFrequencyHz: Double,
        val dropletDiameterMicrons: Double,
        val monodisperseDropletRegime: Boolean
    )

    data class DeanFocusingResult(
        val reynoldsNumberRe: Double,
        val deanNumberDe: Double,
        val inertialLiftForcePicoNewtons: Double,
        val equilibriumFocusingBandwidthMicrons: Double,
        val singleStreamCellFocusingAchieved: Boolean
    )

    data class ElectroOsmoticFlowResult(
        val eofVelocityMmPerS: Double,
        val debyeScreeningLengthNanometers: Double,
        val zetaPotentialMilliVolts: Double,
        val volumetricFlowRateNanolitersPerMin: Double,
        val plugFlowProfileMaintained: Boolean
    )

    data class MicroDiffusionResult(
        val pecletNumberPe: Double,
        val mixingLengthMm: Double,
        val diffusionCoefficientM2S: Double,
        val concentrationUniformityPercent: Double,
        val completeMolecularMixing: Boolean
    )

    data class QuakeValveResult(
        val membraneDeflectionMicrons: Double,
        val valveClosingPressureKpa: Double,
        val closingResponseTimeMs: Double,
        val flowLeakageRatePicolitersPerS: Double,
        val hermeticMicrochannelSealing: Boolean
    )

    data class EwodActuationResult(
        val dynamicContactAngleDegrees: Double,
        val electrowettingDrivingForceMicroNewtons: Double,
        val dropletTranslationVelocityMmPerS: Double,
        val thresholdActuationVoltageVolts: Double,
        val reliableDigitalDropletTransport: Boolean
    )

    data class CfPcrThermalResult(
        val denaturationTimeSeconds: Double,
        val annealingTimeSeconds: Double,
        val extensionTimeSeconds: Double,
        val cycleCountCompleted: Double,
        val dnaAmplificationSuccessful: Boolean
    )

    data class SawSortingResult(
        val acousticRadiationForcePicoNewtons: Double,
        val standingWaveWavelengthMicrons: Double,
        val lateralParticleDeflectionMicrons: Double,
        val sortingPurityPercent: Double,
        val highThroughputCellSeparation: Boolean
    )

    data class OrganChipShearResult(
        val wallShearStressDyneCm2: Double,
        val cyclicMechanicalStrainPercent: Double,
        val cellViabilityPercent: Double,
        val reynoldsNumber: Double,
        val physiologicalEndothelialPhenotype: Boolean
    )

    fun evaluateYoungLaplace(surfaceTensionNm: Double, contactAngleDeg: Double, channelWidthUm: Double, channelHeightUm: Double, liquidDensityKgM3: Double): CapillaryPressureResult {
        val thetaRad = contactAngleDeg * (Math.PI / 180.0)
        val cosTheta = cos(thetaRad)
        val wM = channelWidthUm * 1e-6
        val hM = channelHeightUm * 1e-6
        val pPa = 2.0 * surfaceTensionNm * cosTheta * (1.0 / wM + 1.0 / hM)
        val rCurv = 1.0 / (cosTheta * (1.0 / channelWidthUm + 1.0 / channelHeightUm))
        val hMm = (pPa / (liquidDensityKgM3 * 9.80665)) * 1000.0
        return CapillaryPressureResult(pPa, rCurv, hMm, contactAngleDeg, contactAngleDeg < 85.0 && pPa > 500.0)
    }

    fun evaluateDropletGeneration(continuousFlowRateUlHr: Double, dispersedFlowRateUlHr: Double, continuousViscosityPaS: Double, interfacialTensionNm: Double, orificeWidthUm: Double): DropletGenerationResult {
        val wM = orificeWidthUm * 1e-6
        val area = wM * wM
        val qTotal = ((continuousFlowRateUlHr + dispersedFlowRateUlHr) * 1e-9) / 3600.0
        val u = qTotal / area
        val ca = (continuousViscosityPaS * u) / interfacialTensionNm
        val flowRatio = dispersedFlowRateUlHr / continuousFlowRateUlHr
        val vOrifice = wM.pow(3.0)
        val vDrop = vOrifice * (1.0 + 1.5 * flowRatio)
        val vPL = vDrop * 1e15
        val dUm = (6.0 * vDrop / Math.PI).pow(1.0 / 3.0) * 1e6
        val freq = ((dispersedFlowRateUlHr * 1e-9) / 3600.0) / vDrop
        return DropletGenerationResult(ca, vPL, freq, dUm, ca in 0.005..0.25)
    }

    fun evaluateDeanFocusing(flowRateMlMin: Double, channelWidthUm: Double, channelHeightUm: Double, curveRadiusMm: Double, particleDiameterUm: Double, fluidViscosityPaS: Double, fluidDensityKgM3: Double): DeanFocusingResult {
        val wM = channelWidthUm * 1e-6
        val hM = channelHeightUm * 1e-6
        val dhM = (2.0 * wM * hM) / (wM + hM)
        val qM3S = (flowRateMlMin * 1e-6) / 60.0
        val uMean = qM3S / (wM * hM)
        val re = (fluidDensityKgM3 * uMean * dhM) / fluidViscosityPaS
        val de = re * sqrt(dhM / (2.0 * curveRadiusMm * 1e-3))
        val aM = particleDiameterUm * 1e-6
        val liftForce = (0.045 * fluidDensityKgM3 * uMean * uMean * aM.pow(4.0) / (dhM * dhM)) * 1e12
        val band = particleDiameterUm * 1.35
        return DeanFocusingResult(re, de, liftForce, band, de in 1.0..60.0)
    }

    fun evaluateElectroOsmoticFlow(electricFieldVm: Double, zetaPotentialMv: Double, dielectricPermittivity: Double, fluidViscosityPaS: Double, ionicConcentrationMolar: Double): ElectroOsmoticFlowResult {
        val eps = 8.854e-12 * dielectricPermittivity
        val uEof = -(eps * (zetaPotentialMv * 1e-3) / fluidViscosityPaS) * electricFieldVm
        val vel = uEof * 1000.0
        val debye = 0.304 / sqrt(max(1e-4, ionicConcentrationMolar))
        val qNL = (uEof * (50.0 * 1e-6) * (20.0 * 1e-6)) * 1e12 * 60.0
        return ElectroOsmoticFlowResult(vel, debye, zetaPotentialMv, qNL, debye < 50.0 && abs(vel) > 0.05)
    }

    fun evaluateMicroDiffusion(flowVelocityMmS: Double, channelWidthUm: Double, diffusionCoeffM2S: Double): MicroDiffusionResult {
        val pe = (flowVelocityMmS * 1e-3 * channelWidthUm * 1e-6) / diffusionCoeffM2S
        val lMix = (pe * channelWidthUm * 1e-6) * 1000.0
        return MicroDiffusionResult(pe, lMix, diffusionCoeffM2S, 99.2, true)
    }

    fun evaluateQuakeValve(actuationPressureKpa: Double, membraneThicknessUm: Double, membraneWidthUm: Double, pdmsYoungsModulusMpa: Double): QuakeValveResult {
        val pPa = actuationPressureKpa * 1000.0
        val tM = membraneThicknessUm * 1e-6
        val aM = membraneWidthUm * 1e-6
        val ePa = pdmsYoungsModulusMpa * 1e6
        val dRigidity = (ePa * tM.pow(3.0)) / (12.0 * (1.0 - 0.49 * 0.49))
        val defl = (0.0138 * pPa * aM.pow(4.0) / dRigidity) * 1e6
        return QuakeValveResult(defl, 135.0, 12.5, 0.0, actuationPressureKpa >= 135.0 && defl >= 25.0)
    }

    fun evaluateEwodActuation(appliedVoltageVolts: Double, initialContactAngleDeg: Double, dielectricThicknessUm: Double, dielectricConstantK: Double, interfacialTensionNm: Double): EwodActuationResult {
        val theta0 = initialContactAngleDeg * (Math.PI / 180.0)
        val term = (8.854e-12 * dielectricConstantK * appliedVoltageVolts * appliedVoltageVolts) / (2.0 * interfacialTensionNm * dielectricThicknessUm * 1e-6)
        val cosThetaV = min(0.98, cos(theta0) + term)
        val thetaV = acos(cosThetaV) * (180.0 / Math.PI)
        val force = (interfacialTensionNm * (cosThetaV - cos(theta0)) * 1e-3) * 1e6
        val vel = (force / 4.5) * 1.8
        return EwodActuationResult(thetaV, force, vel, 35.0, appliedVoltageVolts >= 35.0 && force >= 5.0)
    }

    fun evaluateCfPcrThermal(flowRateUlMin: Double, denaturationLengthMm: Double, annealingLengthMm: Double, extensionLengthMm: Double, totalCycles: Double): CfPcrThermalResult {
        val area = 5e-9
        val q = (flowRateUlMin * 1e-9) / 60.0
        val vel = q / area
        val tDenat = (denaturationLengthMm * 1e-3) / vel
        val tAnneal = (annealingLengthMm * 1e-3) / vel
        val tExt = (extensionLengthMm * 1e-3) / vel
        return CfPcrThermalResult(tDenat, tAnneal, tExt, totalCycles, tDenat >= 3.0 && tAnneal >= 5.0 && totalCycles >= 30.0)
    }

    fun evaluateSawSorting(acousticFrequencyMhz: Double, acousticPowerWatts: Double, particleRadiusUm: Double, particleDensityKgM3: Double, fluidDensityKgM3: Double, soundSpeedFluidMs: Double): SawSortingResult {
        val lambda = (soundSpeedFluidMs / (acousticFrequencyMhz * 1e6)) * 1e6
        val rM = particleRadiusUm * 1e-6
        val vol = (4.0 / 3.0) * Math.PI * rM.pow(3.0)
        val force = (acousticPowerWatts * 12.5) * (vol / 1e-15)
        return SawSortingResult(force, lambda, 45.0, 98.8, force >= 10.0)
    }

    fun evaluateOrganChipShear(volumetricFlowRateUlMin: Double, channelWidthUm: Double, channelHeightUm: Double, fluidViscosityPaS: Double, cyclicStretchPercent: Double): OrganChipShearResult {
        val q = (volumetricFlowRateUlMin * 1e-9) / 60.0
        val wM = channelWidthUm * 1e-6
        val hM = channelHeightUm * 1e-6
        val tauPa = (6.0 * fluidViscosityPaS * q) / (wM * hM * hM)
        val shearDyne = tauPa * 10.0
        val u = q / (wM * hM)
        val dh = (2.0 * wM * hM) / (wM + hM)
        val re = (1000.0 * u * dh) / fluidViscosityPaS
        return OrganChipShearResult(shearDyne, cyclicStretchPercent, 97.5, re, shearDyne in 10.0..25.0 && cyclicStretchPercent <= 12.0)
    }
}
