# AndroCAD — Faz 3 Mühendislik Notları, Mimari Gerekçeler ve Sürüm Kayıt Defteri (NOTLAR.md)

**Son Güncelleme:** 2026-10-23  
**Aktif Dal:** `otonom-calisma`  
**Başlangıç Sürümü:** `v1.0.11` (Milestone 1–20 ve Release `v1.0.1`–`v1.0.11` Tamamlandı)  
**Hedef Kapsam:** Faz 3: Post-1.0 Hardening & Expansion (`Milestone 21` – `Milestone 70`, Toplam 50 Milestone)

---

## 1. Mimari ve Operasyonel Gerekçeler

1. **Roadmap Senkronizasyonu (`ROADMAP.md` ve `docs/roadmap.md`):**
   - Depoda tarihsel olarak `docs/roadmap.md` bulunmaktaydı. Hem kök dizindeki `ROADMAP.md` hem de `docs/roadmap.md` birebir aynı içerikle tutulmakta ve her milestone geçişinde senkronize olarak güncellenmektedir.
   - Son milestone numarası `Milestone 20` olduğu için Faz 3 kapsamındaki 50 yeni milestone **`Milestone 21` – `Milestone 70`** olarak numaralandırılmıştır.
2. **İlk 10 Milestone (M21 – M30) — Stabilizasyon ve Cihaz Hata Sınıflarının Önlenmesi:**
   - `v1.0.1` (Xiaomi 15T Pro & Xiaomi Pad 6): Eksik `.so` bağımlılık yükleme sırası (`UnsatisfiedLinkError`), UI thread'inden doğrudan OpenGL ES çağrısı (`GL_INVALID_OPERATION` siyah ekran) ve dar ekranlarda ribbon taşması.
   - `v1.0.2`: Kotlin `internal external fun` kaynaklı JVM bytecode isim bozulması (`$core_kernel_debug` JNI name mangling) ve STEP/STL sınır durumları.
   - `v1.0.3`: SAF `content://` URI köprüsü, manifest geniş depolama izinlerinin yasaklanması ve ertelenmiş hiyerarşik STEP montaj ayrıştırması (`stabilization.md` Bölüm 5.1).
   - M21–M30 bu hata sınıflarının tamamı için kalıcı C++ / Kotlin denetçileri ve regresyon testleri kurar.
3. **Kalan 40 Milestone (M31 – M70) — Endüstriyel CAD / CAM / CAE / Platform Genişlemesi:**
   - Değişken yarıçaplı fillet (M31), asimetrik chamfer (M32), konfigürasyon tabloları (M33), denklem eğrileri (M34), kalıp tasarımı draft/undercut ve core/cavity ayrımı (M35–M36), 3B baskı ağaç destek ve TPMS Gyroid dilimleme (M37–M38), trokoidal adaptif CAM, CNC torna ve 3+2 eksen indeksli işleme (M39–M41), lineer burkulma, kararlı hal termal ve termomekanik FEA (M42–M44), SIMP topoloji optimizasyonu (M45), Gauss/Ortalama eğrilik, Zebra şeritleri ve et kalınlığı analizi (M46–M48), IGES 5.3, Wavefront OBJ/MTL ve 3MF veri değişimi (M49–M51), Stylus/S-Pen basınç ve avuç içi reddi (M52), WCAG AAA erişilebilirlik (M53), 5 dilde yerelleştirme + RTL (M54), AndroScript makro motoru (M55), Vulkan 1.1 backend soyutlaması (M56), çelik profil kaynaklı konstrüksiyon (M57), evolvent dişli üretici (M58), 3B borulama (M59), sac metal hem/louver (M60), plastik snap-fit/boss/rib (M61), 3B emboss/wrap (M62), patlatılmış montaj görünümü (M63), 3B hassas kumpas (M64), çok düzlemli kesit kutusu (M65), ECAD/PCB köprüsü (M66), Git-uyumlu JSON-CAD diff/merge (M67), çevrimdışı ISO 286 tolerans el kitabı (M68), LGPL v2.1 dinamik relink doğrulama kiti (M69) ve Google Play Store 16KB sayfa hizalama / üretim hazırlığı denetimi (M70).
4. **Yerel Doğrulama, Ekran Görüntüsü Denetimi ve Bulut CI/CD (`docs/agent-rules.md` Kural 1 Uyumu):**
   - Bu çalışma ortamında yerel `g++` (GCC 12.2.0) derleyicisi mevcuttur; her milestone commit'inden önce tüm Faz 3 C++ birim testleri yerel olarak derlenip çalıştırılır (`ctest` eşdeğeri + CMake kaydı), ayrıca `AndroidManifest.xml` geniş depolama izni denetimi (`READ_EXTERNAL_STORAGE`, `WRITE_EXTERNAL_STORAGE`, `MANAGE_EXTERNAL_STORAGE` yasağı) ve JNI sembol eşleşme denetimi otomatik çalıştırılır.
   - Her milestone için `docs/screenshots/m<N>_tablet.png` (1280x800) ve `docs/screenshots/m<N>_phone.png` (393x852) ekran görüntüleri üretilir ve otomatik Siyah Ekran / Boş Viewport Denetçisi (`ViewportRenderDiagnostic`) tarafından piksel parlaklık ortalaması, standart sapması ve 3B model silüet doluluğu açısından doğrulanır.
   - Depo boyutunun GB'larca şişmesini önlemek için `releases/` dizininde her semver sürümün doğrulanmış sürüm manifestosu ve APK doğrulama kaydı tutulur; GitHub Actions `ci.yml`, `ui-tests.yml` ve `release.yml` iş akışları tam APK paketlerini üretir.

---

## 2. Faz 3 Milestone ve Semver Sürüm Günlüğü

| Milestone | Sürüm | Durum | Özet ve Doğrulama Notu |
| :--- | :--- | :---: | :--- |
| **Plan** | `v1.0.11` | ✅ | 50 yeni milestone (`M21`–`M70`) `ROADMAP.md` ve `docs/roadmap.md` dosyalarına eklendi. |
| **M21** | `v1.0.12` | ✅ Done | JNI Export & Signature Parity Audit (Name-Mangling Regression Guard) — CTest (`[PASS] M21 JNI Export Audit (Kotlin externals=95, C++ exports=95, internalMangled=0, unmatched=0)`) & Screenshots (`t_lum=36.1`, `p_lum=36.5`) doğrulandı. |
| **M22** | `v1.0.13` | ✅ Done | EGL/GL Thread Safety & Render Context Guard — CTest (`[PASS] M22 EGL/GL Thread Safety Guard`) & Screenshots (`t_lum=36.2`, `p_lum=36.6`) doğrulandı. |
| **M23** | `v1.0.14` | ✅ Done | Multi-ABI Native Library Loader & Missing .so Dependency Verifier — CTest (`[PASS] M23 Multi-ABI Native Library Load Order (3 ABIs, 49 libs)`) & Screenshots (`t_lum=36.1`, `p_lum=36.5`) doğrulandı. |
| **M24** | `v1.0.15` | ✅ Done | Multi-Form-Factor Responsive UI Matrix (Small Phone, Foldable, Tablet) — CTest (`[PASS] M24 Multi-Form-Factor Responsive UI Matrix`) & Screenshots (`t_lum=36.2`, `p_lum=36.9`) doğrulandı. |
| **M25** | `v1.0.16` | ✅ Done | Automated Viewport Black-Screen & Blank-Render Diagnostics — CTest (`[PASS] M25 Viewport Black-Screen & Blank-Render Diagnostics`) & Screenshots (`t_lum=34.9`, `p_lum=33.9`) doğrulandı. |
| **M26** | `v1.0.17` | ✅ Done | Memory Budget & High-Poly Performance Profiler (Low-RAM Guard) — CTest (`[PASS] M26 High-Poly Memory Budget Profiler`) & Screenshots (`t_lum=36.0`, `p_lum=36.7`) doğrulandı. |
| **M27** | `v1.0.18` | ✅ Done | Privacy-First Opt-In Crash & OCCT Diagnostic Reporter — CTest (`[PASS] M27 Privacy-First Opt-In Crash & Diagnostic Vault`) & Screenshots (`t_lum=36.1`, `p_lum=36.7`) doğrulandı. |
| **M28** | `v1.0.19` | ✅ Done | Retry & Resolution of Deferred Capabilities: Hierarchical STEP Assembly & Multi-Solid Booleans — CTest (`[PASS] M28 Hierarchical STEP Assembly Tree Parser`) & Screenshots (`t_lum=36.2`, `p_lum=36.7`) doğrulandı. |
| **M29** | `v1.0.20` | ✅ Done | SAF Content-URI Torture & Corrupt CAD File Resilience — CTest (`[PASS] M29 Corrupt CAD I/O Fuzz & SAF Torture Suite`) & Screenshots (`t_lum=36.2`, `p_lum=36.7`) doğrulandı. |
