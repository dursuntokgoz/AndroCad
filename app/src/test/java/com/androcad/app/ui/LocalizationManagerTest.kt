// Copyright (c) 2026 dursuntokgoz. All rights reserved.
package com.androcad.app.ui

import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Before
import org.junit.Test

class LocalizationManagerTest {

    @Before
    fun setUp() {
        LocalizationManager.setLanguage(AppLanguage.TR)
    }

    @Test
    fun testLanguageToggle() {
        assertEquals(AppLanguage.TR, LocalizationManager.currentLanguage.value)
        LocalizationManager.toggleLanguage()
        assertEquals(AppLanguage.EN, LocalizationManager.currentLanguage.value)
        LocalizationManager.toggleLanguage()
        assertEquals(AppLanguage.TR, LocalizationManager.currentLanguage.value)
    }

    @Test
    fun testTabTranslations() {
        LocalizationManager.setLanguage(AppLanguage.TR)
        assertEquals("Katı Model", LocalizationManager.getTabLabel("Solid"))
        assertEquals("Eskiz", LocalizationManager.getTabLabel("Sketch"))
        assertEquals("Yüzey", LocalizationManager.getTabLabel("Surface"))
        assertEquals("Sac Metal", LocalizationManager.getTabLabel("Sheet Metal"))
        assertEquals("Simülasyon", LocalizationManager.getTabLabel("Simulation"))
        assertEquals("İmalat (CAM)", LocalizationManager.getTabLabel("Manufacture (CAM)"))
        assertEquals("Teknik Resim", LocalizationManager.getTabLabel("Drawing"))
        assertEquals("Montaj", LocalizationManager.getTabLabel("Assemble"))
        assertEquals("Dosya / G/Ç", LocalizationManager.getTabLabel("File / IO"))

        LocalizationManager.setLanguage(AppLanguage.EN)
        assertEquals("Solid", LocalizationManager.getTabLabel("Solid"))
        assertEquals("Sketch", LocalizationManager.getTabLabel("Sketch"))
        assertEquals("File / IO", LocalizationManager.getTabLabel("File / IO"))
    }

    @Test
    fun testToolTranslations() {
        LocalizationManager.setLanguage(AppLanguage.TR)
        assertEquals("Kutu", LocalizationManager.getToolLabel("Box"))
        assertEquals("Silindir", LocalizationManager.getToolLabel("Cylinder"))
        assertEquals("Küre", LocalizationManager.getToolLabel("Sphere"))
        assertEquals("Radyus", LocalizationManager.getToolLabel("Fillet"))
        assertEquals("Pah", LocalizationManager.getToolLabel("Chamfer"))
        assertEquals("Delik", LocalizationManager.getToolLabel("Hole"))
        assertEquals("Eskizi Bitir ✓", LocalizationManager.getToolLabel("Finish Sketch ✓"))
        assertEquals("Kaydet", LocalizationManager.getToolLabel("Save"))
        assertEquals("Aç", LocalizationManager.getToolLabel("Open"))
        assertEquals("STEP Dışa Aktar", LocalizationManager.getToolLabel("Export STEP"))

        LocalizationManager.setLanguage(AppLanguage.EN)
        assertEquals("Box", LocalizationManager.getToolLabel("Box"))
        assertEquals("Cylinder", LocalizationManager.getToolLabel("Cylinder"))
        assertEquals("Finish Sketch ✓", LocalizationManager.getToolLabel("Finish Sketch ✓"))
    }

    @Test
    fun testViewCubeAndNavTranslations() {
        LocalizationManager.setLanguage(AppLanguage.TR)
        assertEquals("ÜST", LocalizationManager.getViewCubeLabel("TOP"))
        assertEquals("ÖN", LocalizationManager.getViewCubeLabel("FRONT"))
        assertEquals("İZO", LocalizationManager.getViewCubeLabel("ISO"))
        assertEquals("Yörünge", LocalizationManager.getNavLabel("Orbit"))
        assertEquals("Kaydır", LocalizationManager.getNavLabel("Pan"))
        assertEquals("Sığdır", LocalizationManager.getNavLabel("Fit"))

        LocalizationManager.setLanguage(AppLanguage.EN)
        assertEquals("TOP", LocalizationManager.getViewCubeLabel("TOP"))
        assertEquals("Orbit", LocalizationManager.getNavLabel("Orbit"))
    }

    @Test
    fun testTopButtonTranslations() {
        LocalizationManager.setLanguage(AppLanguage.TR)
        assertTrue(LocalizationManager.getTopButtonLabel("tutorial").contains("REHBER"))
        assertTrue(LocalizationManager.getTopButtonLabel("samples").contains("ÖRNEKLER"))
        assertTrue(LocalizationManager.getThemeLabel(true).contains("KOYU"))
        assertTrue(LocalizationManager.getThemeLabel(false).contains("AÇIK"))

        LocalizationManager.setLanguage(AppLanguage.EN)
        assertTrue(LocalizationManager.getTopButtonLabel("tutorial").contains("TUTORIAL"))
        assertTrue(LocalizationManager.getTopButtonLabel("samples").contains("SAMPLES"))
        assertTrue(LocalizationManager.getThemeLabel(true).contains("DARK"))
        assertTrue(LocalizationManager.getThemeLabel(false).contains("LIGHT"))
    }
}
