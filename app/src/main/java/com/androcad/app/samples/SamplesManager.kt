// Copyright (c) 2026 dursuntokgoz. All rights reserved.
package com.androcad.app.samples

import android.content.Context
import com.androcad.kernel.BoxMeshResult
import com.androcad.kernel.NativeKernel
import com.androcad.kernel.NativeTimeline
import java.io.File
import java.io.FileOutputStream

data class SampleDesign(
    val id: String,
    val nameTr: String,
    val nameEn: String,
    val descriptionTr: String,
    val descriptionEn: String,
    val assetFileName: String,
    val previewAssetFileName: String,
    val testTag: String,
    val nominalVolumeMm3: Double
)

object SamplesManager {

    val SAMPLES: List<SampleDesign> = listOf(
        SampleDesign(
            id = "l_bracket",
            nameTr = "L-Braket",
            nameEn = "L-Bracket",
            descriptionTr = "90° yapısal montaj braketi, çift delikli ve parametrik et kalınlıklı.",
            descriptionEn = "90° structural mount bracket with dual fastener holes and parametric thickness.",
            assetFileName = "samples/l_bracket.androcad",
            previewAssetFileName = "samples/l_bracket_preview.png",
            testTag = "sample_open_l_bracket",
            nominalVolumeMm3 = 36515.04
        ),
        SampleDesign(
            id = "flanged_bushing",
            nameTr = "Flanşlı Burç",
            nameEn = "Flanged Bushing",
            descriptionTr = "Mil geçişi ve yüzey montajı için flanşlı yataklama burcu.",
            descriptionEn = "Flanged sleeve bushing designed for shaft guide and surface fixing.",
            assetFileName = "samples/flanged_bushing.androcad",
            previewAssetFileName = "samples/flanged_bushing_preview.png",
            testTag = "sample_open_flanged_bushing",
            nominalVolumeMm3 = 31476.11
        ),
        SampleDesign(
            id = "phone_stand",
            nameTr = "Telefon Standı",
            nameEn = "Phone Stand",
            descriptionTr = "Masaüstü telefon tutucu ve alt şarj kablosu yuvası.",
            descriptionEn = "Desktop phone resting stand with charging cable channel recess.",
            assetFileName = "samples/phone_stand.androcad",
            previewAssetFileName = "samples/phone_stand_preview.png",
            testTag = "sample_open_phone_stand",
            nominalVolumeMm3 = 84672.57
        ),
        SampleDesign(
            id = "perforated_plate",
            nameTr = "Delikli Plaka",
            nameEn = "Perforated Plate",
            descriptionTr = "2x2 simetrik delik dizisine sahip hassas montaj ve havalandırma plakası.",
            descriptionEn = "Precision mounting plate with a 2x2 symmetrical hole pattern.",
            assetFileName = "samples/perforated_plate.androcad",
            previewAssetFileName = "samples/perforated_plate_preview.png",
            testTag = "sample_open_perforated_plate",
            nominalVolumeMm3 = 62391.50
        )
    )

    /**
     * Copies the read-only asset sample to a unique writable file in app storage,
     * ensuring the asset template is never mutated.
     */
    fun copySampleToWorkingFile(context: Context, sample: SampleDesign): File {
        val samplesDir = File(context.filesDir, "samples")
        if (!samplesDir.exists()) {
            samplesDir.mkdirs()
        }
        val targetFile = File(samplesDir, "editable_${sample.id}_${System.currentTimeMillis()}.androcad")
        context.assets.open(sample.assetFileName).use { input ->
            FileOutputStream(targetFile).use { output ->
                input.copyTo(output)
            }
        }
        return targetFile
    }

    /**
     * Loads a sample design into a NativeTimeline session and rebuilds the geometry.
     */
    fun loadSample(context: Context, sample: SampleDesign, timeline: NativeTimeline): BoxMeshResult {
        val workingFile = copySampleToWorkingFile(context, sample)
        val loadResult = timeline.loadProject(workingFile.absolutePath)
        if (!loadResult.success) {
            return BoxMeshResult(
                success = false,
                volume = 0.0,
                faceCount = 0,
                edgeCount = 0,
                triangleCount = 0,
                vertexBuffer = null,
                indexBuffer = null,
                edgeVertexBuffer = null,
                boundingBox = com.androcad.kernel.BoundingBox(),
                errorMessage = "Failed to load sample ${sample.id}: ${loadResult.errorMessage}"
            )
        }
        return timeline.rebuild()
    }
}
