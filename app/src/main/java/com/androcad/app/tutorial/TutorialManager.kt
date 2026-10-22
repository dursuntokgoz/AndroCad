// Copyright (c) 2026 dursuntokgoz. All rights reserved.
package com.androcad.app.tutorial

import android.content.Context
import androidx.datastore.core.DataStore
import androidx.datastore.preferences.core.Preferences
import androidx.datastore.preferences.core.booleanPreferencesKey
import androidx.datastore.preferences.core.edit
import androidx.datastore.preferences.preferencesDataStore
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.first
import kotlinx.coroutines.flow.map

val Context.tutorialDataStore: DataStore<Preferences> by preferencesDataStore(name = "androcad_tutorial_prefs")

data class TutorialStep(
    val stepIndex: Int,
    val titleTr: String,
    val titleEn: String,
    val descriptionTr: String,
    val descriptionEn: String,
    val targetTag: String,
    val targetTab: String? = null
)

object TutorialManager {
    private val KEY_TUTORIAL_COMPLETED = booleanPreferencesKey("tutorial_completed")

    /**
     * Testing flag to prevent the tutorial overlay from intercepting clicks in standard UI tests.
     */
    @Volatile
    var isTutorialDisabledForTesting: Boolean = false

    @Volatile
    private var cachedCompleted: Boolean? = null

    fun isTutorialCompletedInitial(): Boolean {
        if (isTutorialDisabledForTesting) return true
        return cachedCompleted ?: false
    }

    fun disableForTesting(disabled: Boolean = true) {
        isTutorialDisabledForTesting = disabled
    }

    val STEPS: List<TutorialStep> = listOf(
        TutorialStep(
            stepIndex = 1,
            titleTr = "Görünüm Kontrolü & ViewCube",
            titleEn = "View Navigation & ViewCube",
            descriptionTr = "Parçayı 3 boyutlu uzayda incelemek için tek parmakla döndürün (Orbit), iki parmakla kaydırın (Pan) ve sıkıştırarak yakınlaştırın (Zoom). ViewCube köşelerine dokunarak standart izometrik ve dik görünümlere anında geçebilirsiniz.",
            descriptionEn = "Rotate with one finger (Orbit), pan with two fingers, and pinch to zoom in 3D space. Tap ViewCube corners for isometric and orthogonal views.",
            targetTag = "viewcube_ISO"
        ),
        TutorialStep(
            stepIndex = 2,
            titleTr = "Model Tarayıcısı (Browser)",
            titleEn = "Model Browser & Feature Tree",
            descriptionTr = "Sol paneldeki Browser ağacı; orijin düzlemleri, katı gövdeler (Bodies) ve eskizleri hiyerarşik olarak listeler. Göz simgeleriyle elemanları gizleyebilir veya seçebilirsiniz.",
            descriptionEn = "The left Browser tree organizes origin planes, bodies, and sketches hierarchically. Toggle visibility or select elements directly.",
            targetTag = "browser_header"
        ),
        TutorialStep(
            stepIndex = 3,
            titleTr = "2D Eskiz & Çizim Kısıtları",
            titleEn = "2D Sketch & Constraints",
            descriptionTr = "Düzlem seçerek 2D eskiz başlatın. Dikdörtgen, çember, çizgi çizip ölçü ve kısıtlar (teğetlik, paralellik, diklik) vererek tam tanımlı eskiz profilleri oluşturun.",
            descriptionEn = "Start 2D sketching on any plane. Draw rectangles, circles, or lines and add constraints and dimensions for fully-defined profiles.",
            targetTag = "tab_Sketch",
            targetTab = "Sketch"
        ),
        TutorialStep(
            stepIndex = 4,
            titleTr = "3D Katılaştırma (Extrude)",
            titleEn = "3D Solid Modeling (Extrude)",
            descriptionTr = "Çizdiğiniz kapalı 2D profili Z ekseninde derinlik vererek katı gövdeye dönüştürün (Extrude). Ardından Revolve, Fillet ve Chamfer ile modelinizi zenginleştirin.",
            descriptionEn = "Extrude closed 2D profiles into 3D solid bodies. Enhance designs with Revolve, Fillet, Chamfer, and Boolean operations.",
            targetTag = "tab_Solid",
            targetTab = "Solid"
        ),
        TutorialStep(
            stepIndex = 5,
            titleTr = "Parametrik Zaman Çizelgesi (Timeline)",
            titleEn = "Parametric Timeline & History",
            descriptionTr = "AndroCAD parametrik bir CAD yazılımıdır. Alt zaman çizelgesindeki geçmiş adımlara tıklayarak geçmişe dönebilir (Rollback), ilk ölçüyü değiştirip tüm tasarımı otomatik güncelleyebilirsiniz.",
            descriptionEn = "AndroCAD is parametric. Tap past steps on the bottom timeline to roll back, modify initial dimensions, and recalculate all downstream features.",
            targetTag = "timeline_step_0"
        ),
        TutorialStep(
            stepIndex = 6,
            titleTr = "SAF ile STEP & STL İçe/Dışa Aktarma",
            titleEn = "SAF STEP & STL Import / Export",
            descriptionTr = "Storage Access Framework sayesinde geniş depolama izinlerine gerek kalmadan tasarımlarınızı endüstri standardı STEP ve STL formatlarında kaydedebilir ya da dışarıdan modelleri içeri aktarabilirsiniz.",
            descriptionEn = "Using Storage Access Framework without broad storage permissions, export and import industry-standard STEP and STL CAD models seamlessly.",
            targetTag = "tab_FileIO",
            targetTab = "File / IO"
        )
    )

    fun isCompletedFlow(context: Context): Flow<Boolean> {
        return context.tutorialDataStore.data.map { preferences ->
            preferences[KEY_TUTORIAL_COMPLETED] ?: false
        }
    }

    suspend fun isCompleted(context: Context): Boolean {
        if (isTutorialDisabledForTesting) return true
        val prefs = context.tutorialDataStore.data.first()
        val completed = prefs[KEY_TUTORIAL_COMPLETED] ?: false
        cachedCompleted = completed
        return completed
    }

    suspend fun setCompleted(context: Context, completed: Boolean = true) {
        cachedCompleted = completed
        context.tutorialDataStore.edit { preferences ->
            preferences[KEY_TUTORIAL_COMPLETED] = completed
        }
    }

    suspend fun resetTutorial(context: Context) {
        cachedCompleted = false
        context.tutorialDataStore.edit { preferences ->
            preferences[KEY_TUTORIAL_COMPLETED] = false
        }
    }
}
