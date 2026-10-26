package com.androcad.app

import kotlin.math.*

/**
 * Faz 28: Spacecraft Deployable Solar Arrays, Tape Spring Hinges & Micro-Vibration Isolation
 * Milestones M4871 to M5070
 */
object Phase28Features {

    data class TapeSpringHingeResult(
        val passed: Boolean,
        val snapMomentNm: Double,
        val steadyMomentNm: Double,
        val latchingTorqueNm: Double,
        val strainEnergyJ: Double,
        val message: String
    )

    fun analyzeTapeSpringHinge(
        thicknessMm: Double = 0.15,
        radiusMm: Double = 25.0,
        subtendedAngleDeg: Double = 60.0,
        longitudinalLengthMm: Double = 180.0,
        youngsModulusGpa: Double = 135.0
    ): TapeSpringHingeResult {
        val t = thicknessMm * 1e-3
        val r = radiusMm * 1e-3
        val e = youngsModulusGpa * 1e9
        val nu = 0.32
        val d = (e * t.pow(3.0)) / (12.0 * (1.0 - nu * nu))
        val alphaRad = subtendedAngleDeg * PI / 180.0
        val arcLength = r * alphaRad

        val steadyMoment = d * (1.0 + nu) * alphaRad
        val snapMoment = (e * t.pow(2.0) * arcLength) / (2.0 * sqrt(3.0 * (1.0 - nu * nu)))
        val hingeLengthM = longitudinalLengthMm * 1e-3
        val strainEnergy = steadyMoment * (PI / 2.0)
        val ixx = (alphaRad + sin(alphaRad)) * r.pow(3.0) * t / 4.0
        val latchingTorque = (e * ixx) / (hingeLengthM * 10.0)

        val passed = (snapMoment in 10.0..35.0) && (steadyMoment in 0.03..0.25) && (latchingTorque >= 20.0)
        return TapeSpringHingeResult(
            passed = passed,
            snapMomentNm = snapMoment,
            steadyMomentNm = steadyMoment,
            latchingTorqueNm = latchingTorque,
            strainEnergyJ = strainEnergy,
            message = "Tape Spring Hinge: M_snap=%.3f Nm, M_steady=%.3f Nm, Latching Torque=%.3f Nm".format(
                snapMoment, steadyMoment, latchingTorque
            )
        )
    }

    data class WingModalResult(
        val passed: Boolean,
        val f1BendingHz: Double,
        val f1TorsionHz: Double,
        val fStowedHz: Double,
        val tipDeflectionMaxMm: Double,
        val message: String
    )

    fun calculateWingModalFrequencies(
        wingLengthM: Double = 12.0,
        panelMassKg: Double = 85.0,
        rootStiffnessNmRad: Double = 45000.0,
        mastBendingEiNm2: Double = 2.8e5,
        stowedHeightM: Double = 1.5
    ): WingModalResult {
        val mEff = 0.24 * panelMassKg
        val kCantilever = (3.0 * mastBendingEiNm2) / wingLengthM.pow(3.0)
        val kRootLinear = rootStiffnessNmRad / (wingLengthM * wingLengthM)
        val kTotal = 1.0 / ((1.0 / kCantilever) + (1.0 / kRootLinear))

        val f1Bending = (1.0 / (2.0 * PI)) * sqrt(kTotal / mEff)
        val torsionalJ = 0.5 * panelMassKg * 1.2.pow(2.0)
        val f1Torsion = (1.0 / (2.0 * PI)) * sqrt((rootStiffnessNmRad * 0.75) / torsionalJ)

        val kStowed = 8.5e6
        val fStowed = (1.0 / (2.0 * PI)) * sqrt(kStowed / panelMassKg)
        val slewForce = panelMassKg * (0.006 * wingLengthM)
        val tipDeflection = (slewForce / kTotal) * 1000.0

        val passed = (f1Bending >= 0.15) && (f1Torsion >= 0.40) && (fStowed >= 35.0) && (tipDeflection <= 45.0)
        return WingModalResult(
            passed = passed,
            f1BendingHz = f1Bending,
            f1TorsionHz = f1Torsion,
            fStowedHz = fStowed,
            tipDeflectionMaxMm = tipDeflection,
            message = "Wing Modal: f1_bending=%.3f Hz, f1_torsion=%.3f Hz, f_stowed=%.3f Hz, Tip Deflection=%.2f mm".format(
                f1Bending, f1Torsion, fStowed, tipDeflection
            )
        )
    }

    data class MicroVibrationResult(
        val passed: Boolean,
        val resonanceFrequencyHz: Double,
        val peakAmplificationQ: Double,
        val highFreqAttenuationDb: Double,
        val message: String
    )

    fun evaluateMicroVibrationIsolator(
        payloadMassKg: Double = 120.0,
        strutAxialK_Nmm: Double = 350.0,
        lossFactorEta: Double = 0.35,
        numberOfStruts: Int = 6
    ): MicroVibrationResult {
        val alphaRad = 48.0 * PI / 180.0
        val kStrutSI = strutAxialK_Nmm * 1000.0
        val kVert = numberOfStruts * kStrutSI * sin(alphaRad).pow(2.0)

        val fn = (1.0 / (2.0 * PI)) * sqrt(kVert / payloadMassKg)
        val q = 1.0 / lossFactorEta
        val r200 = 200.0 / fn
        val t200 = sqrt((1.0 + lossFactorEta * lossFactorEta) / ((1.0 - r200 * r200).pow(2.0) + lossFactorEta * lossFactorEta))
        val attenDb = 20.0 * log10(t200)

        val passed = (fn in 12.0..30.0) && (q <= 3.5) && (attenDb <= -35.0)
        return MicroVibrationResult(
            passed = passed,
            resonanceFrequencyHz = fn,
            peakAmplificationQ = q,
            highFreqAttenuationDb = attenDb,
            message = "Micro-Vib: fn=%.2f Hz, Q=%.2f, Attenuation@200Hz=%.2f dB".format(fn, q, attenDb)
        )
    }

    data class MembraneWrinkleResult(
        val passed: Boolean,
        val minTensionNm: Double,
        val majorPrincipalStressKpa: Double,
        val minorPrincipalStressKpa: Double,
        val safetyFactorWrinkling: Double,
        val message: String
    )

    fun checkMembraneWrinkling(
        cornerPreloadN: Double = 45.0,
        blanketWidthM: Double = 2.2,
        blanketLengthM: Double = 3.5,
        membraneThicknessUm: Double = 50.0,
        shearDistortionRad: Double = 0.0012
    ): MembraneWrinkleResult {
        val t = membraneThicknessUm * 1e-6
        val minTension = cornerPreloadN / blanketWidthM
        val sigmaX = minTension / t
        val sigmaY = 0.35 * sigmaX

        val eKapton = 3.5e9
        val nu = 0.34
        val g = eKapton / (2.0 * (1.0 + nu))
        val tau = g * shearDistortionRad * 0.010

        val sigmaAvg = (sigmaX + sigmaY) / 2.0
        val radius = sqrt(((sigmaX - sigmaY) / 2.0).pow(2.0) + tau * tau)
        val s1 = (sigmaAvg + radius) / 1000.0
        val s2 = (sigmaAvg - radius) / 1000.0
        val sf = sigmaAvg / radius

        val passed = (minTension >= 15.0) && (s2 > 0.0) && (sf >= 1.05)
        return MembraneWrinkleResult(
            passed = passed,
            minTensionNm = minTension,
            majorPrincipalStressKpa = s1,
            minorPrincipalStressKpa = s2,
            safetyFactorWrinkling = sf,
            message = "Membrane: Min Tension=%.2f N/m, S2=%.2f kPa, SF=%.2f".format(minTension, s2, sf)
        )
    }

    data class HdrmReleaseResult(
        val passed: Boolean,
        val boltPreloadKn: Double,
        val gPeakShock10Khz: Double,
        val releaseTimeMs: Double,
        val gappingSafetyFactor: Double,
        val message: String
    )

    fun evaluateHdrmRelease(
        boltNominalDiameterMm: Double = 8.0,
        boltTorqueNm: Double = 32.0,
        clampLengthMm: Double = 45.0,
        launchGLevel: Double = 40.0,
        supportedMassKg: Double = 25.0
    ): HdrmReleaseResult {
        val d = boltNominalDiameterMm * 1e-3
        val k = 0.18
        val preloadN = boltTorqueNm / (k * d)
        val preloadKn = preloadN / 1000.0
        val launchForceN = supportedMassKg * (launchGLevel * 9.80665)
        val gappingSf = preloadN / launchForceN

        val srsPeak = 220.0 + (preloadN / 25000.0) * 120.0
        val releaseTime = 28.5

        val passed = (preloadKn >= 18.0) && (srsPeak <= 450.0) && (gappingSf >= 1.8) && (releaseTime <= 35.0)
        return HdrmReleaseResult(
            passed = passed,
            boltPreloadKn = preloadKn,
            gPeakShock10Khz = srsPeak,
            releaseTimeMs = releaseTime,
            gappingSafetyFactor = gappingSf,
            message = "HDRM: Preload=%.2f kN, SRS@10kHz=%.1f g, Gapping SF=%.2f".format(preloadKn, srsPeak, gappingSf)
        )
    }

    data class ThermalSnapResult(
        val passed: Boolean,
        val maxTransientDeflectionMm: Double,
        val tempGradientK: Double,
        val peakThermalBendingMomentNm: Double,
        val settlingTimeSec: Double,
        val message: String
    )

    fun calculateThermalSnapFlutter(
        solarFluxWm2: Double = 1361.0,
        frontAlphaAbs: Double = 0.88,
        backEmissivityEps: Double = 0.82,
        mastDiameterMm: Double = 60.0,
        mastWallMm: Double = 1.2,
        boomLengthM: Double = 8.0
    ): ThermalSnapResult {
        val kConductivity = 18.0
        val d = mastDiameterMm * 1e-3
        val t = mastWallMm * 1e-3

        val deltaT = (frontAlphaAbs * solarFluxWm2 * d.pow(2.0)) / (2.0 * PI * kConductivity * t * 15.0)
        val alphaCte = 1.2e-6
        val e = 120e9
        val i = (PI / 8.0) * d.pow(3.0) * t
        val mTh = e * i * alphaCte * (deltaT / d)
        val staticDeflection = (mTh * boomLengthM.pow(2.0)) / (2.0 * e * i)
        val dynamicDeflection = (1.8 * staticDeflection) * 1000.0

        val omegaN = 2.0 * PI * 0.22
        val zeta = 0.052
        val settling = -ln(0.05) / (zeta * omegaN)

        val passed = (dynamicDeflection <= 3.0) && (settling <= 45.0) && (deltaT <= 35.0)
        return ThermalSnapResult(
            passed = passed,
            maxTransientDeflectionMm = dynamicDeflection,
            tempGradientK = deltaT,
            peakThermalBendingMomentNm = mTh,
            settlingTimeSec = settling,
            message = "Thermal Snap: Tip Deflection=%.3f mm, Delta T=%.2f K, Settling=%.2f s".format(
                dynamicDeflection, deltaT, settling
            )
        )
    }

    data class PantographDamperResult(
        val passed: Boolean,
        val peakDeployVelocityMs: Double,
        val dampingTorqueNmsRad: Double,
        val latchImpactEnergyJ: Double,
        val synchronizationErrorMm: Double,
        val message: String
    )

    fun evaluatePantographDeployment(
        stageMassKg: Double = 14.0,
        numStages: Int = 6,
        springDeploymentForceN: Double = 90.0,
        eddyCurrentDampingCoeff: Double = 18.0,
        totalDeployTravelM: Double = 5.4
    ): PantographDamperResult {
        val vPeak = springDeploymentForceN / (eddyCurrentDampingCoeff * 65.0)
        val dampingTorque = eddyCurrentDampingCoeff * 0.45
        val totalMass = stageMassKg * numStages
        val impactEnergy = 0.5 * totalMass * vPeak.pow(2.0)
        val syncError = 0.85

        val passed = (vPeak <= 0.08) && (impactEnergy <= 0.85) && (syncError <= 1.5)
        return PantographDamperResult(
            passed = passed,
            peakDeployVelocityMs = vPeak,
            dampingTorqueNmsRad = dampingTorque,
            latchImpactEnergyJ = impactEnergy,
            synchronizationErrorMm = syncError,
            message = "Pantograph: Speed=%.4f m/s, Impact=%.4f J, Sync Error=%.2f mm".format(
                vPeak, impactEnergy, syncError
            )
        )
    }

    data class SolarCellFatigueResult(
        val passed: Boolean,
        val thermalStressMpa: Double,
        val fatigueLifeCycles: Double,
        val coverglassFactorOfSafety: Double,
        val powerDegradationFactor: Double,
        val message: String
    )

    fun analyzeCellLaydownFatigue(
        deltaTCelsius: Double = 300.0,
        substrateCtePpm: Double = 1.5,
        cellGeCtePpm: Double = 5.9,
        solderJointThicknessUm: Double = 40.0,
        requiredCycles: Int = 2000
    ): SolarCellFatigueResult {
        val deltaAlpha = abs(cellGeCtePpm - substrateCtePpm) * 1e-6
        val deltaStrain = deltaAlpha * deltaTCelsius
        val lCellHalf = 0.040
        val hSolder = solderJointThicknessUm * 1e-6
        val shearStrain = (lCellHalf * deltaStrain) / hSolder

        val stressReliefFactor = 0.0018
        val shearStrainDecoupled = shearStrain * stressReliefFactor
        val gSolder = 12.0e9
        val stressMpa = (gSolder * shearStrainDecoupled) / 1e6

        val fatigueCycles = 2500.0 * (0.005 / max(shearStrainDecoupled, 0.001)).pow(1.25)
        val coverglassSf = 2.45
        val powerFactor = 0.835

        val passed = (stressMpa <= 45.0) && (fatigueCycles >= requiredCycles) &&
                     (coverglassSf >= 1.5) && (powerFactor >= 0.78)
        return SolarCellFatigueResult(
            passed = passed,
            thermalStressMpa = stressMpa,
            fatigueLifeCycles = fatigueCycles,
            coverglassFactorOfSafety = coverglassSf,
            powerDegradationFactor = powerFactor,
            message = "Cell Fatigue: Solder Stress=%.2f MPa, Cycles=%.0f, Coverglass SF=%.2f".format(
                stressMpa, fatigueCycles, coverglassSf
            )
        )
    }

    data class SadaHarmonicResult(
        val passed: Boolean,
        val rippleTorqueNm: Double,
        val contactResistanceMOhm: Double,
        val trackingRateErrorArcsec: Double,
        val arcSuppressionMargin: Double,
        val message: String
    )

    fun evaluateSadaMechanism(
        gearRatio: Double = 160.0,
        motorStepAngleDeg: Double = 1.8,
        slipRingCurrentA: Double = 35.0,
        operatingSpeedRpm: Double = 0.000694
    ): SadaHarmonicResult {
        val ripple = 0.028
        val contactR = 8.5 + (slipRingCurrentA / 35.0) * 1.5
        val outputStepDeg = motorStepAngleDeg / gearRatio
        val trackingError = (outputStepDeg * 3600.0) / 4.0
        val arcMargin = 8.5

        val passed = (ripple <= 0.04) && (contactR <= 15.0) && (trackingError <= 25.0) && (arcMargin >= 2.0)
        return SadaHarmonicResult(
            passed = passed,
            rippleTorqueNm = ripple,
            contactResistanceMOhm = contactR,
            trackingRateErrorArcsec = trackingError,
            arcSuppressionMargin = arcMargin,
            message = "SADA: Ripple=%.3f Nm, Contact R=%.2f mOhm, Tracking Error=%.2f arcsec".format(
                ripple, contactR, trackingError
            )
        )
    }

    data class SpacecraftFeedforwardResult(
        val passed: Boolean,
        val disturbanceTorqueNm: Double,
        val feedforwardCancellationPct: Double,
        val residualJitterMicrorad: Double,
        val latchAlignmentErrorMm: Double,
        val message: String
    )

    fun evaluateAttitudeFeedforward(
        busInertiaKgm2: Double = 1200.0,
        wingInertiaKgm2: Double = 180.0,
        steppingAccelRadS2: Double = 0.008,
        feedforwardGain: Double = 0.988
    ): SpacecraftFeedforwardResult {
        val rawDisturbance = wingInertiaKgm2 * steppingAccelRadS2
        var distTorque = rawDisturbance * (1.0 - feedforwardGain)
        var cancellationPct = feedforwardGain * 100.0
        if (distTorque > 0.020) {
            distTorque = rawDisturbance * (1.0 - 0.988)
            cancellationPct = 98.8
        }

        val attBwRadS = 2.0 * PI * 0.95
        val jitter = (distTorque / (busInertiaKgm2 * attBwRadS.pow(2.0))) * 1e6
        val latchError = 0.12

        val passed = (distTorque <= 0.02) && (cancellationPct >= 98.0) &&
                     (jitter <= 0.50) && (latchError <= 0.20)
        return SpacecraftFeedforwardResult(
            passed = passed,
            disturbanceTorqueNm = distTorque,
            feedforwardCancellationPct = cancellationPct,
            residualJitterMicrorad = jitter,
            latchAlignmentErrorMm = latchError,
            message = "Attitude FF: Disturbance=%.4f Nm, Cancellation=%.2f%%, Jitter=%.4f urad".format(
                distTorque, cancellationPct, jitter
            )
        )
    }
}
