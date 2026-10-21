# AndroCAD (Türkçe Dokümantasyon)

<p align="center">
  <img src="docs/images/androcad_app_icon.png" width="128" height="128" alt="AndroCAD Resmi Uygulama Simgesi" />
</p>

[![CI](https://github.com/dursuntokgoz/AndroCad/actions/workflows/ci.yml/badge.svg)](https://github.com/dursuntokgoz/AndroCad/actions/workflows/ci.yml)
[![UI Tests](https://github.com/dursuntokgoz/AndroCad/actions/workflows/ui-tests.yml/badge.svg)](https://github.com/dursuntokgoz/AndroCad/actions/workflows/ui-tests.yml)
[![OCCT Build](https://github.com/dursuntokgoz/AndroCad/actions/workflows/occt-build.yml/badge.svg)](https://github.com/dursuntokgoz/AndroCad/actions/workflows/occt-build.yml)

Android tablet ve telefonlar için sıfırdan geliştirilmiş, Open CASCADE Technology (OCCT 7.8.0) çekirdeği ve PlaneGCS kısıt çözücüsü ile güçlendirilmiş özgün ve profesyonel parametrik 3D CAD yazılımı.

---

## Özellik Durum Tablosu (Feature Status Matrix)

| Modül / Yetenek | Durum | Uygulama ve Doğrulama Detayları |
| :--- | :---: | :--- |
| **3D Katı Modelleme** (Box, Extrude, Revolve, Fillet, Chamfer, Hole, Shell, Combine) | ✅ Çalışıyor | C++ Open CASCADE B-Rep operasyonları; host CTest ve Robolectric testleri ile kanıtlanmıştır. |
| **2D Parametrik Eskiz (Sketch)** | ✅ Çalışıyor | PlaneGCS Levenberg-Marquardt çözücüsü; boyutsal ve geometrik kısıtlar ile Serbestlik Derecesi (DoF) takibi. |
| **Model Tarayıcısı (Browser)** | ✅ Çalışıyor | Hiyerarşik ağaç (Belge Ayarları, Görünümler, Orijin, Gövdeler, Eskizler, İnşa) ve görünürlük/seçim kontrolleri. |
| **Parametrik Zaman Çizelgesi (Timeline)** | ✅ Çalışıyor | Rollback sarma imleci, matematiksel parametreler (`width * 2`) ve otomatik yeniden hesaplama (DAG rebuild). |
| **Storage Access Framework (SAF) G/Ç** | ✅ Çalışıyor | Geniş depolama izinleri YOKTUR (`MANAGE_EXTERNAL_STORAGE` manifestten kaldırılmıştır); sistem dosya seçicisi ile güvenli STEP/STL/JSON alışverişi. |
| **İlk Açılış Öğreticisi (Onboarding)** | ✅ Çalışıyor | 6 adımlı etkileşimli tur; Jetpack DataStore kalıcılığı; her adımda "Geç" butonu ve geri hareketi desteği; menüden yeniden başlatılabilir. |
| **Düzenlenebilir Örnek Tasarımlar** | ✅ Çalışıyor | 4 özgün CAD modeli (L-Braket, Flanşlı Burç, Telefon Standı, Delikli Plaka); açılışta kopya oluşturarak asset dokunulmazlığını korur. |
| **ViewCube & Görünüm Navigasyonu** | ✅ Çalışıyor | 3D yönelim küpü, dik ve izometrik bakışlar, tek parmak yörünge (orbit), iki parmak kaydırma (pan) ve kıstırma (zoom). |
| **Sac Metal & Açınım (Flat Pattern)** | ✅ Çalışıyor | K-faktörü büküm hesabı ($K = 0.44$), yönlü büküm notları ve 2D DXF lazer kesim açınımı dışa aktarımı. |
| **2D Teknik Resim & Çizim Sayfası** | ✅ Çalışıyor | 4 görünüşlü ortografik izdüşüm (ISO/ANSI), HLR kesikli görünmez kenarlar, ölçülendirme, antet ve SVG dışa aktarım. |
| **Statik FEA & Modal Titreşim Analizi** | ✅ Çalışıyor | 10 düğümlü kuadratik tetraedral (Tet10) çözücü, Von Mises gerilme ısı haritası ve modal titreşim mod şekilleri. |
| **Talaşlı İmalat (CAM) & G-Code** | ✅ Çalışıyor | 2.5D ve 3D uyarlamalı kaba boşaltma, finiş konturlama, kademeli delik delme (G83) ve GRBL/Haas/Fanuc post-processor. |
| **Katmanlı Üretim (3D Baskı Dilimleme)** | ✅ Çalışıyor | Cihaz üzerinde doğrudan G-code dilimleme; bal peteği ve ızgara dolgu desenleri; nozzle ve yatak sıcaklık yönetimi. |
| **Montaj & Kinematik Mafsallar** | 🧪 Deneysel | Sabit, döner, silindirik ve kayar mafsal tanımları; çarpışma kontrolü. |
| **Bulut Proje Senkronizasyonu** | 📋 Planlı | Gelecek ana sürümler için planlanmıştır. |

---

## Gerçek Emülatör Ekran Görüntüleri

Tüm ekran görüntüleri gerçek Android tablet (1280x800) ve telefon (393x852) emülatörlerinden alınmıştır:

| 1. Tablet Ana Çalışma Ekranı (Koyu Tema) | 2. Tablet Ana Çalışma Ekranı (Açık Tema) |
| :---: | :---: |
| ![Tablet Koyu Tema](docs/screenshots/01_tablet_main_workspace_dark.png) | ![Tablet Açık Tema](docs/screenshots/02_tablet_main_workspace_light.png) |

| 3. Telefon Uyarlanabilir Çalışma Ekranı | 4. Öğretici: 1. Adım (Görünüm Navigasyonu) |
| :---: | :---: |
| ![Telefon Ekranı](docs/screenshots/03_phone_main_workspace.png) | ![Öğretici Adım 1](docs/screenshots/04_tutorial_onboarding_step1.png) |

| 5. Öğretici: 3. Adım (2D Eskiz Modelleme) | 6. Örnek Tasarımlar Galerisi |
| :---: | :---: |
| ![Öğretici Adım 3](docs/screenshots/05_tutorial_onboarding_step3.png) | ![Örnekler Galerisi](docs/screenshots/06_samples_gallery_modal.png) |

| 7. Yüklenmiş L-Braket Modeli (DAG Rebuild) | 8. Gizlilik Odaklı Karşılama Ekranı |
| :---: | :---: |
| ![Açılmış L-Braket](docs/screenshots/07_sample_opened_l_bracket.png) | ![Boş Durum Ekranı](docs/screenshots/08_empty_state_screen.png) |

---

## İlk Kullanım Deneyimi & Öğretici

AndroCAD, yeni kullanıcıların uygulamaya hızla adapte olabilmesi için 6 adımlı etkileşimli bir ilk açılış öğreticisi içerir:
1. **Görünüm Kontrolü & ViewCube:** Orbit, pan, zoom dokunma hareketleri ve ViewCube standart açıları.
2. **Model Tarayıcısı (Browser):** Orijin düzlemleri, katı gövdeler ve eskizlerin hiyerarşik yönetimi.
3. **2D Eskiz & Çizim Kısıtları:** Kapalı profil çizimi ve PlaneGCS kısıt çözücüsü.
4. **3D Katılaştırma:** 2D profilleri derinlik vererek katı gövdeye dönüştürme ve ikincil operasyonlar (Radyus, Pah, Delik).
5. **Parametrik Zaman Çizelgesi:** Geçmişe sarma (Rollback), ilk ölçüyü değiştirme ve alt özellikleri otomatik güncelleme.
6. **SAF Dosya Alışverişi:** Geniş izinlere ihtiyaç duymadan STEP ve STL modellerini içe/dışa aktarma.

* **Geçilebilir & Rahatsız Etmeyen:** Her adımda "Geç" düğmesi bulunur; geri hareketi de öğreticiyi hemen kapatır.
* **Kalıcı Tercih:** Tercihler DataStore ile saklanır; tamamlanan veya geçilen öğretici sonraki açılışlarda kullanıcıyı rahatsız etmez.
* **Menüden Başlatma:** Üst çubuktaki `🎓 TUTORIAL` düğmesine basılarak öğretici istenildiği an baştan başlatılabilir.

---

## Özgün Düzenlenebilir Örnek Tasarımlar

Kullanıcıların hazır parametrik modelleri inceleyip değiştirebilmesi için `assets/samples/` altında 4 özgün tasarım sunulur:
* **L-Braket (`l_bracket.androcad`):** 90° yapısal montaj braketi, çift delik ve parametrik et kalınlığı ($\approx 36.515\text{ mm}^3$).
* **Flanşlı Burç (`flanged_bushing.androcad`):** Mil yuvası ve montaj flanşlı yataklama burcu ($\approx 31.476\text{ mm}^3$).
* **Telefon Standı (`phone_stand.androcad`):** Açılı telefon tutucu ve şarj kablosu geçiş yuvası ($\approx 84.672\text{ mm}^3$).
* **Delikli Plaka (`perforated_plate.androcad`):** 2x2 simetrik delik ızgaralı hassas montaj plakası ($\approx 62.391\text{ mm}^3$).

> [!IMPORTANT]
> **Asset Dokunulmazlığı:** Bir örnek açıldığında otomatik olarak `context.filesDir/samples/` altına düzenlenebilir bir **kopya** olarak kaydedilir. APK içindeki orijinal şablon hiçbir zaman bozulmaz.

Ayrıntılı geometrik parametreler, formüller ve hacim değişmezleri için [`docs/samples.md`](docs/samples.md) belgesini inceleyebilirsiniz.

---

## Derleme Talimatları (GitHub Actions CI)

Proje kuralları gereği yerel araç zinciri kullanılmamaktadır:
* **Yerel Bağımlılık Yoktur:** Tüm C++ çekirdek derlemeleri, NDK `arm64-v8a` paketlemesi ve testler GitHub Actions CI üzerinde çalışır.
* **İş Akışları:**
  - [`.github/workflows/ci.yml`](.github/workflows/ci.yml): Host CTest (smoke, TNP, Project I/O), Android derleme, manifest izin denetimi ve APK üretimi.
  - [`.github/workflows/ui-tests.yml`](.github/workflows/ui-tests.yml): Telefon (dikey) ve tablet (yatay) Compose UI test matrisi.

---

## Lisans ve Yasal Bilgilendirme

* **Uygulama Kodu ve Özgün Varlıklar:**  
  Telif Hakkı (c) 2026 dursuntokgoz. **Tüm hakları saklıdır.**  
  Telif hakkı sahibinin önceden yazılı izni olmaksızın bu tescilli yazılımın kullanılması, kopyalanması, değiştirilmesi veya dağıtılması yasaktır.

* **Üçüncü Taraf Açık Kaynak Bileşenler:**  
  Open CASCADE Technology (OCCT 7.8.0), PlaneGCS, Eigen 3 ve Boost gibi üçüncü taraf bileşenler kendi açık kaynak lisanslarına tabidir. OCCT ve PlaneGCS dinamik paylaşımlı kütüphane (`.so`) olarak bağlanmıştır.  
  LGPL 2.1 Madde 6 kapsamındaki dinamik yeniden bağlama talimatları ve bildirimler için [`THIRD_PARTY_NOTICES.md`](./THIRD_PARTY_NOTICES.md) ve [`docs/licenses.md`](./docs/licenses.md) belgelerine bakınız.
