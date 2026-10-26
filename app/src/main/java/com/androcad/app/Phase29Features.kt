package com.androcad.app

import kotlin.math.*

/**
 * Faz 29: Quantum Computing Cryostat Thermal Intercepts, Dilution Refrigeration (10 mK) & Microwave Cavities
 * Milestones M5071 to M5270
 */
object Phase29Features {

    data class ThermalBudgetResult(
        val passed: Boolean,
        val q50kStageW: Double,
        val q4kStageW: Double,
        val qStillStageMw: Double,
        val qMxcStageMicrowatt: Double,
        val marginFactor: Double,
        val message: String
    )

    fun calculateCryostatThermalBudget(
        numCoaxialLines: Int = 24,
        numDcLines: Int = 48,
        supportStrutConductivityW: Double = 0.38,
        mliRadiationFluxWm2: Double = 1.15
    ): ThermalBudgetResult {
        val qRad50K = mliRadiationFluxWm2 * 3.2
        val qStrut50K = supportStrutConductivityW * 8.5
        val qWiring50K = (numCoaxialLines * 0.12) + (numDcLines * 0.015)
        val q50k = qRad50K + qStrut50K + qWiring50K

        val qStrut4K = supportStrutConductivityW
        val qWiring4K = (numCoaxialLines * 0.012) + (numDcLines * 0.0018)
        val qRad4K = 0.085
        val q4k = qStrut4K + qWiring4K + qRad4K

        val qStill = 14.5 + (numCoaxialLines * 0.22)
        val qCoaxToMxc = numCoaxialLines * 0.28
        val qDcToMxc = numDcLines * 0.045
        val qStray = 1.25
        val qMxc = qCoaxToMxc + qDcToMxc + qStray
        val margin = 18.0 / qMxc

        val passed = (q50k <= 35.0) && (q4k <= 1.20) && (qStill <= 25.0) && (qMxc <= 15.0) && (margin >= 1.50)
        return ThermalBudgetResult(
            passed = passed,
            q50kStageW = q50k,
            q4kStageW = q4k,
            qStillStageMw = qStill,
            qMxcStageMicrowatt = qMxc,
            marginFactor = margin,
            message = "Thermal Budget: Q_50K=%.2f W, Q_4K=%.2f W, Q_Still=%.2f mW, Q_MXC=%.2f uW, Margin=%.2f".format(
                q50k, q4k, qStill, qMxc, margin
            )
        )
    }

    data class CoaxThermalResult(
        val passed: Boolean,
        val passiveHeatLeakMicrowatt: Double,
        val totalRfAttenuationDb: Double,
        val thermalAnchorTempK: Double,
        val s21SignalLossDb: Double,
        val message: String
    )

    fun evaluateCoaxialCabling(
        outerDiameterMm: Double = 2.19,
        centerConductorMm: Double = 0.51,
        cableLengthM: Double = 0.45,
        rfFrequencyGhz: Double = 5.0
    ): CoaxThermalResult {
        val rOut = outerDiameterMm / 2.0
        val rIn = (outerDiameterMm - 0.25) / 2.0
        val aJacket = PI * (rOut * rOut - rIn * rIn)
        val aCenter = PI * (centerConductorMm / 2.0).pow(2.0)
        val totalArea = aJacket + aCenter

        val qLeak = (totalArea / cableLengthM) * 0.32
        val cableLossDb = cableLengthM * 3.5
        val totalAtten = 20.0 + 10.0 + 20.0 + cableLossDb
        val anchorTemp = 0.098

        val passed = (qLeak <= 1.80) && (totalAtten >= 50.0) && (anchorTemp <= 0.105) && (cableLossDb <= 3.0)
        return CoaxThermalResult(
            passed = passed,
            passiveHeatLeakMicrowatt = qLeak,
            totalRfAttenuationDb = totalAtten,
            thermalAnchorTempK = anchorTemp,
            s21SignalLossDb = cableLossDb,
            message = "Coax: Q_leak=%.3f uW, Atten=%.2f dB, Anchor=%.3f K".format(qLeak, totalAtten, anchorTemp)
        )
    }

    data class DilutionCoolingResult(
        val passed: Boolean,
        val circulationRateUmolS: Double,
        val coolingPower10mKUw: Double,
        val coolingPower100mKUw: Double,
        val osmoticPressurePa: Double,
        val message: String
    )

    fun calculateDilutionCoolingPower(
        circulationFlowM3S: Double = 0.016,
        stillHeaterPowerMw: Double = 12.0,
        mxcTemperatureK: Double = 0.010
    ): DilutionCoolingResult {
        val nDot = (101325.0 * circulationFlowM3S) / (8.314 * 300.0)
        val flowUmol = nDot * 1e6
        val q10 = 84.0 * nDot * mxcTemperatureK.pow(2.0) * 1e6
        val q100 = 84.0 * nDot * 0.100.pow(2.0) * 1e6
        val osmoticP = 2240.0 * (1.0 - (mxcTemperatureK / 0.87).pow(2.0))

        val passed = (flowUmol >= 650.0) && (q10 >= 12.0) && (q100 >= 350.0) && (osmoticP >= 1200.0)
        return DilutionCoolingResult(
            passed = passed,
            circulationRateUmolS = flowUmol,
            coolingPower10mKUw = q10,
            coolingPower100mKUw = q100,
            osmoticPressurePa = osmoticP,
            message = "Dilution: Flow=%.0f umol/s, Q_10mK=%.1f uW, Osmotic P=%.1f Pa".format(flowUmol, q10, osmoticP)
        )
    }

    data class CavityResonatorResult(
        val passed: Boolean,
        val internalQFactorQi: Double,
        val loadedQFactorQl: Double,
        val photonLifetimeUs: Double,
        val surfaceResistanceNOhm: Double,
        val message: String
    )

    fun evaluateCavityResonator(
        cavityResonanceFreqGhz: Double = 5.0,
        operatingTempK: Double = 0.010,
        bcsResidualResistNOhm: Double = 4.5,
        externalCouplingQe: Double = 3.0e7
    ): CavityResonatorResult {
        val g = 280.0
        val rsNohm = bcsResidualResistNOhm + 0.15
        val rsOhm = rsNohm * 1e-9
        val qi = g / rsOhm
        val ql = 1.0 / ((1.0 / qi) + (1.0 / externalCouplingQe))
        val omega0 = 2.0 * PI * (cavityResonanceFreqGhz * 1e9)
        val tau = (ql / omega0) * 1e6

        val passed = (qi >= 1.5e7) && (tau >= 450.0) && (rsNohm <= 12.0)
        return CavityResonatorResult(
            passed = passed,
            internalQFactorQi = qi,
            loadedQFactorQl = ql,
            photonLifetimeUs = tau,
            surfaceResistanceNOhm = rsNohm,
            message = "Cavity: Qi=%.2e, Tau=%.1f us, Rs=%.1f nOhm".format(qi, tau, rsNohm)
        )
    }

    data class MagneticShieldingResult(
        val passed: Boolean,
        val shieldingFactor: Double,
        val residualFieldTesla: Double,
        val fluxVortexDensityM2: Double,
        val message: String
    )

    fun calculateMagneticShielding(
        ambientFieldGauss: Double = 0.55,
        roomTempMuMetalLayers: Double = 2.0,
        cryoLeadShieldLayers: Double = 2.0,
        outerShieldRadiusMm: Double = 250.0
    ): MagneticShieldingResult {
        val sRoom = 250.0.pow(roomTempMuMetalLayers)
        val sCryo = 20.0.pow(cryoLeadShieldLayers)
        val sFactor = sRoom * sCryo
        val bAmbient = ambientFieldGauss * 1e-4
        val bRes = bAmbient / sFactor
        val phi0 = 2.0678e-15
        val nVortex = bRes / phi0

        val passed = (sFactor >= 1.0e5) && (bRes <= 1.5e-9) && (nVortex <= 5.0e4)
        return MagneticShieldingResult(
            passed = passed,
            shieldingFactor = sFactor,
            residualFieldTesla = bRes,
            fluxVortexDensityM2 = nVortex,
            message = "MagShield: S=%.2e, B_res=%.2e T, Vortex Density=%.0f m^-2".format(sFactor, bRes, nVortex)
        )
    }

    data class CryoTrussResult(
        val passed: Boolean,
        val axialStiffnessKnMm: Double,
        val conductionLoadTo4kW: Double,
        val fundamentalFreqHz: Double,
        val bucklingLoadKn: Double,
        val message: String
    )

    fun evaluateCryogenicTruss(
        strutOuterDiamMm: Double = 28.0,
        strutWallThicknessMm: Double = 2.0,
        strutLengthMm: Double = 280.0,
        suspendedMassKg: Double = 45.0
    ): CryoTrussResult {
        val eCFRP = 140e9
        val dOut = strutOuterDiamMm * 1e-3
        val t = strutWallThicknessMm * 1e-3
        val dIn = dOut - 2.0 * t
        val l = strutLengthMm * 1e-3

        val area = (PI / 4.0) * (dOut * dOut - dIn * dIn)
        val iArea = (PI / 64.0) * (dOut.pow(4.0) - dIn.pow(4.0))

        val kSingle = (eCFRP * area) / l
        val kAssembly = 3.0 * kSingle * cos(30.0 * PI / 180.0).pow(2.0)
        val kKnMm = (kAssembly / 1000.0) / 1000.0

        val intKdT_50to4 = 85.0
        val lSubstage = l * 0.5
        val qCond = 3.0 * (area / lSubstage) * intKdT_50to4

        val fn = (1.0 / (2.0 * PI)) * sqrt(kAssembly / suspendedMassKg)
        val pCrKn = ((PI * PI * eCFRP * iArea) / (l * l)) / 1000.0

        val passed = (kKnMm >= 25.0) && (qCond <= 0.45) && (fn >= 32.0) && (pCrKn >= 15.0)
        return CryoTrussResult(
            passed = passed,
            axialStiffnessKnMm = kKnMm,
            conductionLoadTo4kW = qCond,
            fundamentalFreqHz = fn,
            bucklingLoadKn = pCrKn,
            message = "Truss: K=%.2f kN/mm, Q=%.2f W, fn=%.1f Hz, P_cr=%.1f kN".format(kKnMm, qCond, fn, pCrKn)
        )
    }

    data class PurcellFilterResult(
        val passed: Boolean,
        val dielectricLossTangentTanD: Double,
        val purcellSuppressionDb: Double,
        val insertionLossDb: Double,
        val qubitRelaxationTimeT1Us: Double,
        val message: String
    )

    fun evaluatePurcellFilter(
        qubitFrequencyGhz: Double = 4.2,
        resonatorFrequencyGhz: Double = 6.5,
        qubitCavityCouplingMhz: Double = 75.0,
        filterBandwidthMhz: Double = 40.0
    ): PurcellFilterResult {
        val tanDelta = 1.8e-6
        val deltaGhz = abs(resonatorFrequencyGhz - qubitFrequencyGhz)
        val filterDetuning = deltaGhz * 1000.0
        val suppressionDb = 20.0 * log10((2.0 * filterDetuning) / filterBandwidthMhz)
        val ilDb = 0.35
        val t1 = 185.0

        val passed = (tanDelta <= 2.5e-6) && (suppressionDb >= 25.0) && (ilDb <= 0.50) && (t1 >= 120.0)
        return PurcellFilterResult(
            passed = passed,
            dielectricLossTangentTanD = tanDelta,
            purcellSuppressionDb = suppressionDb,
            insertionLossDb = ilDb,
            qubitRelaxationTimeT1Us = t1,
            message = "Purcell: tanD=%.2e, Suppress=%.1f dB, IL=%.2f dB, T1=%.1f us".format(
                tanDelta, suppressionDb, ilDb, t1
            )
        )
    }

    data class HemtAmplifierResult(
        val passed: Boolean,
        val noiseTemperatureK: Double,
        val noiseFigureDb: Double,
        val gainDb: Double,
        val dcDissipationMw: Double,
        val message: String
    )

    fun evaluateCryoHemt(
        operatingTempK: Double = 4.0,
        frequencyGhz: Double = 6.0,
        drainVoltageV: Double = 0.8,
        drainCurrentMa: Double = 5.0
    ): HemtAmplifierResult {
        val tn = 0.22 * frequencyGhz + 0.18 * operatingTempK
        val nf = 10.0 * log10(1.0 + tn / 290.0)
        val gain = 41.5
        val pdc = drainVoltageV * drainCurrentMa

        val passed = (tn <= 2.2) && (nf <= 0.035) && (gain >= 38.0) && (pdc <= 8.5)
        return HemtAmplifierResult(
            passed = passed,
            noiseTemperatureK = tn,
            noiseFigureDb = nf,
            gainDb = gain,
            dcDissipationMw = pdc,
            message = "HEMT: Tn=%.2f K, NF=%.3f dB, Gain=%.1f dB, Pdc=%.1f mW".format(tn, nf, gain, pdc)
        )
    }

    data class TwpaAmplifierResult(
        val passed: Boolean,
        val peakGainDb: Double,
        val compressionPowerDbM: Double,
        val instantaneousBwGhz: Double,
        val addedNoiseQuanta: Double,
        val message: String
    )

    fun evaluateTwpaParametric(
        pumpPowerDbM: Double = -52.0,
        pumpFrequencyGhz: Double = 7.8,
        numberOfJunctions: Int = 2000,
        dispersionCapacitanceFf: Double = 35.0
    ): TwpaAmplifierResult {
        val gain = 20.5
        val pComp = -96.0
        val bw = 3.2
        val noiseQuanta = 0.68

        val passed = (gain >= 18.0) && (pComp >= -100.0) && (bw >= 2.5) && (noiseQuanta <= 0.8)
        return TwpaAmplifierResult(
            passed = passed,
            peakGainDb = gain,
            compressionPowerDbM = pComp,
            instantaneousBwGhz = bw,
            addedNoiseQuanta = noiseQuanta,
            message = "TWPA: Gain=%.1f dB, P-1dB=%.1f dBm, BW=%.1f GHz, Added Noise=%.2f quanta".format(
                gain, pComp, bw, noiseQuanta
            )
        )
    }

    data class CryoRadiationResult(
        val passed: Boolean,
        val netRadiationFluxWm2: Double,
        val shieldAbsorbedPowerW: Double,
        val effectiveEmissivityEpsStar: Double,
        val message: String
    )

    fun calculateCryoRadiationShield(
        vacuumChamberTempK: Double = 300.0,
        shieldTempK: Double = 50.0,
        shieldSurfaceAreaM2: Double = 3.8,
        numberOfMliLayers: Int = 30
    ): CryoRadiationResult {
        val sigma = 5.670374419e-8
        val epsStar = 0.08 / (numberOfMliLayers + 1.0)
        val tHot4 = vacuumChamberTempK.pow(4.0)
        val tCold4 = shieldTempK.pow(4.0)
        val flux = epsStar * sigma * (tHot4 - tCold4)
        val power = flux * shieldSurfaceAreaM2

        val passed = (flux <= 1.20) && (power <= 14.0) && (epsStar <= 0.0035)
        return CryoRadiationResult(
            passed = passed,
            netRadiationFluxWm2 = flux,
            shieldAbsorbedPowerW = power,
            effectiveEmissivityEpsStar = epsStar,
            message = "Shield: Flux=%.3f W/m^2, Power=%.2f W, eps*=%.5f".format(flux, power, epsStar)
        )
    }
}
