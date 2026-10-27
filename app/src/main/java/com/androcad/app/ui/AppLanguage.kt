// Copyright (c) 2026 dursuntokgoz. All rights reserved.
package com.androcad.app.ui

import androidx.compose.runtime.Composable
import androidx.compose.runtime.State
import androidx.compose.runtime.mutableStateOf

enum class AppLanguage(val code: String, val displayName: String, val flag: String) {
    EN("en", "English", "🇬🇧"),
    TR("tr", "Türkçe", "🇹🇷")
}

object LocalizationManager {
    val currentLanguage = mutableStateOf(AppLanguage.TR)

    fun toggleLanguage() {
        currentLanguage.value = if (currentLanguage.value == AppLanguage.TR) AppLanguage.EN else AppLanguage.TR
    }

    fun setLanguage(lang: AppLanguage) {
        currentLanguage.value = lang
    }

    fun getTabLabel(tab: String, lang: AppLanguage = currentLanguage.value): String {
        if (lang == AppLanguage.EN) return tab
        return when (tab) {
            "Solid" -> "Katı Model"
            "Sketch" -> "Eskiz"
            "Surface" -> "Yüzey"
            "Sheet Metal" -> "Sac Metal"
            "Mesh" -> "Ağ (Mesh)"
            "Simulation" -> "Simülasyon"
            "Manufacture (CAM)" -> "İmalat (CAM)"
            "Additive (3D Print)" -> "Katmanlı (3B)"
            "Drawing" -> "Teknik Resim"
            "Assemble" -> "Montaj"
            "Inspect" -> "İncele"
            "File / IO" -> "Dosya / G/Ç"
            else -> tab
        }
    }

    fun getToolLabel(tool: String, lang: AppLanguage = currentLanguage.value): String {
        if (lang == AppLanguage.EN) return tool
        return when {
            tool == "Box" -> "Kutu"
            tool == "Cylinder" -> "Silindir"
            tool == "Sphere" -> "Küre"
            tool == "Torus" -> "Halka"
            tool == "Cone" -> "Koni"
            tool == "Extrude" -> "Ekstrüzyon"
            tool == "Revolve" -> "Döndür"
            tool == "Fillet" -> "Radyus"
            tool == "Chamfer" -> "Pah"
            tool == "Hole" -> "Delik"
            tool == "Shell" -> "Kabuk"
            tool == "Combine" -> "Birleştir"
            tool == "Move/Rotate" -> "Taşı / Döndür"
            tool == "Rect Pattern" -> "Dikdörtgen Dizi"
            tool == "Circular Pattern" -> "Dairesel Dizi"
            tool == "Mirror" -> "Aynalama"
            tool == "Draft Angle" -> "Açısal Eğim"
            tool.startsWith("ISO Hole") -> "ISO Delik (M6 CB)"
            tool.startsWith("Threaded Bolt") -> "Metrik Civata (M8)"
            // Sketch
            tool == "Rectangle" -> "Dikdörtgen"
            tool == "Line" -> "Çizgi"
            tool == "Circle" -> "Daire"
            tool == "Polygon" -> "Çokgen"
            tool == "Slot" -> "Kanal (Slot)"
            tool == "Offset" -> "Öteleme"
            tool == "B-Spline" -> "B-Spline Eğrisi"
            tool == "Project Edge" -> "Kenar İzdüşür"
            tool == "Dimension" -> "Ölçülendir"
            tool == "Constraints" -> "Kısıtlamalar"
            tool.startsWith("Finish Sketch") -> "Eskizi Bitir ✓"
            // File IO
            tool == "Save" -> "Kaydet"
            tool == "Open" -> "Aç"
            tool == "Export STEP" -> "STEP Dışa Aktar"
            tool == "Import STEP" -> "STEP İçe Aktar"
            tool == "Export STL" -> "STL Dışa Aktar"
            tool == "Import STL" -> "STL İçe Aktar"
            // Surface
            tool == "Loft" -> "Loft Yüzey"
            tool == "Sweep Pipe" -> "Süpürme Boru"
            tool == "Split Body" -> "Gövde Böl"
            tool.contains("Press-Pull (+10)") -> "İt/Çek (+10)"
            tool.contains("Press-Pull (-5)") -> "İt/Çek (-5)"
            tool.contains("Thicken") -> "Kalınlaştır"
            // Sheet Metal
            tool == "U-Bracket" -> "U-Braket"
            tool == "Unfold Pattern" -> "Açınım Deseni"
            tool == "Export DXF" -> "DXF Dışa Aktar"
            // Drawing
            tool == "4-View Sheet" -> "4-Görünüş Sayfası"
            tool == "Section Cut" -> "Kesit Görünüşü"
            tool == "BOM & Balloons" -> "Parça Listesi & Balon"
            tool == "GD&T Frame" -> "GD&T Tolerans Çerçevesi"
            tool.startsWith("Detail View") -> "Detay Görünüşü 2:1"
            tool == "Auxiliary View" -> "Yardımcı Görünüş"
            tool == "Multi-Sheet Doc" -> "Çok Sayfalı Belge"
            tool == "Export SVG" -> "SVG Dışa Aktar"
            // Inspect
            tool.startsWith("Mass (Steel)") -> "Kütle (Çelik)"
            tool.startsWith("Mat: Aluminum") -> "Malzeme: Alüminyum"
            tool.startsWith("Mat: Titanium") -> "Malzeme: Titanyum"
            tool.startsWith("Mat: Brass") -> "Malzeme: Pirinç"
            tool == "CoG & Inertia" -> "Ağırlık Merkezi & Eylemsizlik"
            // Mesh
            tool == "Analyze Mesh" -> "Ağ Analizi"
            tool == "Repair Mesh" -> "Ağ Onarımı"
            tool == "Smooth Mesh" -> "Ağ Pürüzsüzleştir"
            tool == "Mesh to Solid" -> "Ağdan Katıya"
            // Simulation
            tool == "Solve Beam FEA" -> "Kiriş FEA Çöz"
            tool == "Tet10 Quadratic FEA" -> "Tet10 Kuadratik FEA"
            tool == "Modal Analysis" -> "Modal Analiz"
            tool == "Mode 1 Vibration" -> "Mod 1 Titreşim"
            tool == "Mode 2 Vibration" -> "Mod 2 Titreşim"
            tool == "Mode 3 Vibration" -> "Mod 3 Titreşim"
            tool == "Solve Part FEA" -> "Parça FEA Çöz"
            tool == "Fixed Support" -> "Sabit Mesnet"
            tool == "Apply Load" -> "Yük Uygula"
            tool == "Safety Factor" -> "Güvenlik Faktörü"
            tool == "Displacement Field" -> "Deplasman Alanı"
            // CAM
            tool.startsWith("Generate G-Code") -> "G-Kodu Üret (GRBL)"
            tool.startsWith("Haas G-Code") -> "Haas G-Kodu"
            tool.startsWith("Fanuc G-Code") -> "Fanuc G-Kodu"
            tool == "3D Adaptive Clearing" -> "3B Uyarlanabilir Boşaltma"
            tool == "3D Scallop Finishing" -> "3B Scallop İnce İşleme"
            tool == "Facing Pass" -> "Yüzey Temizleme"
            tool == "2D Pocketing" -> "2B Havuz Boşaltma"
            tool == "2D Contour" -> "2B Profil Çevre"
            tool == "Peck Drilling" -> "Gagalamalı Delme"
            // Additive
            tool.startsWith("Slice Solid") -> "Dilimle (0.2mm)"
            tool.startsWith("Honeycomb Infill") -> "Petek Dolgu (%30)"
            tool.startsWith("Grid Infill") -> "Izgara Dolgu (%20)"
            // Assemble
            tool == "Robot Arm" -> "Robot Kolu"
            tool.startsWith("Rotate (+15°)") -> "Döndür (+15°)"
            tool == "Reset Joint" -> "Eklemi Sıfırla"
            tool == "Check Collision" -> "Çarpışma Denetimi"
            else -> tool
        }
    }

    fun getNavLabel(tool: String, lang: AppLanguage = currentLanguage.value): String {
        if (lang == AppLanguage.EN) return tool
        return when (tool) {
            "Orbit" -> "Yörünge"
            "Pan" -> "Kaydır"
            "Fit" -> "Sığdır"
            "Shaded+Edges" -> "Gölgeli+Kenar"
            "Shaded" -> "Gölgeli"
            "Wireframe" -> "Tel Kafes"
            else -> tool
        }
    }

    fun getViewCubeLabel(label: String, lang: AppLanguage = currentLanguage.value): String {
        if (lang == AppLanguage.EN) return label
        return when (label) {
            "TOP" -> "ÜST"
            "FRONT" -> "ÖN"
            "BOTTOM" -> "ALT"
            "ISO" -> "İZO"
            "L" -> "SOL"
            "R" -> "SAĞ"
            else -> label
        }
    }

    fun getTopButtonLabel(id: String, lang: AppLanguage = currentLanguage.value): String {
        return when (id) {
            "tutorial" -> if (lang == AppLanguage.TR) "🎓 REHBER" else "🎓 TUTORIAL"
            "samples" -> if (lang == AppLanguage.TR) "📁 ÖRNEKLER" else "📁 SAMPLES"
            "undo" -> if (lang == AppLanguage.TR) "↶ GERİ AL" else "↶ UNDO"
            "redo" -> if (lang == AppLanguage.TR) "↷ YİNELE" else "↷ REDO"
            "new_design" -> if (lang == AppLanguage.TR) "✨ YENİ TASARIM" else "✨ NEW DESIGN"
            else -> id
        }
    }

    fun getThemeLabel(isDark: Boolean, lang: AppLanguage = currentLanguage.value): String {
        return if (lang == AppLanguage.TR) {
            if (isDark) "🌙 KOYU" else "☀️ AÇIK"
        } else {
            if (isDark) "🌙 DARK" else "☀️ LIGHT"
        }
    }
}
