# AndroCAD — Kararlılık ve İlk Kullanım Deneyimi Raporu (Stabilization Report)

**Tarih:** 22 Ekim 2026  
**Durum:** Başarıyla Tamamlandı (Tüm Kabul Kriterleri Karşılandı)  
**Hedef Dal:** `main`  
**İlgili İş Akışları:**  
- CI (Derleme, CTest, Manifest İzin Denetimi): `https://github.com/dursuntokgoz/AndroCad/actions/workflows/ci.yml`  
- UI Testleri (Telefon ve Tablet Compose Matrisi): `https://github.com/dursuntokgoz/AndroCad/actions/workflows/ui-tests.yml`

---

## 1. Bulunan Hatalar ve Uygulanan Çözümler

Tüm hatalar düzeltilmeden önce otomatik testlerle yeniden üretilmiş, ardından tescilli mimarimize uygun olarak giderilmiştir:

### Hata 1: Sol Model Tarayıcısı (Browser Paneli) Çalışmıyordu
* **Belirti:** Sol taraftaki ağaç yapısı (`Document Settings`, `Named Views`, `Origin`, `Bodies`, `Sketches`, `Construction`) statik kalıyordu; katı gövdelerin görünürlüğü gizlenemiyor, seçim yapılamıyor, yeniden adlandırma veya silme yapılamıyordu.
* **Kök Neden:** Arayüz bileşeninin reaktif bir durum modeli (`BrowserState`) ile GL viewport seçim hattına bağlı olmaması.
* **Düzeltme:** 
  - `com.androcad.app.model.BrowserState`, `CadBodyItem` ve `CadSketchItem` veri modelleri geliştirildi.
  - `com.androcad.app.ui.CadBrowserView` bileşeni inşa edildi: genişletilebilir başlıklar, göz simgesi ile anında görünürlük açıp kapama, tıklanan elemanı viewport'ta vurgulama, yeniden adlandırma ve silme yetenekleri eklendi.
  - Tablet düzeninde sol sabit panele, telefon düzeninde ise alt modal çekmeceye entegre edildi.
* **Doğrulayan Testler:** `app/src/test/java/com/androcad/app/model/BrowserStateTest.kt`, `app/src/test/java/com/androcad/app/ui/CadBrowserViewTest.kt`.

### Hata 2: Bazı Butonlar Yanlış Davranıyor ve İkonları Karışıyordu
* **Belirti:** Araç çubuğundaki farklı CAD operasyonları (Sac Metal, CAM, Simülasyon, Delik Sihirbazı) varsayılan `ic_box` kutu ikonuna düşüyor ve görsel geri bildirim bozuluyordu.
* **Kök Neden:** `getToolIconRes(tool)` fonksiyonunda çok sayıda aracın eşleşme eksikliği nedeniyle `else -> R.drawable.ic_box` dalına yönlenmesi.
* **Düzeltme:** 54 farklı CAD operasyonunun tümü için özgün vektör çizim kaynakları haritalandı; hiçbir aracın yanlış ikona düşmemesi sağlandı.
* **Doğrulayan Testler:** `app/src/test/java/com/androcad/app/ButtonBehaviorTest.kt`.

### Hata 3: Import ve Export İşlemleri Çalışmıyordu
* **Belirti:** Kullanıcı STEP veya STL dosyasını içeri aktarmak ya da dışa aktarmak istediğinde işlem sessizce başarısız oluyor ya da çöküyordu.
* **Kök Neden:** Native Open CASCADE C++ (`STEPControl_Reader`, `StlAPI_Writer`) kodları doğrudan POSIX dosya yolları (`/path/to/file`) ile çalışır. Modern Android'in sağladığı `content://` URI'lerini doğrudan C++ std::ifstream/ofstream açamaz.
* **Düzeltme:** `com.androcad.app.io.CadStorageManager` köprüsü inşa edildi:
  - İçe aktarmada (Import): `content://` akışı güvenli geçici önbellek dosyasına (`context.cacheDir`) kopyalanır, native C++ çağrısı yapılır ve geçici dosya `finally` bloğunda silinir.
  - Dışa aktarmada (Export): Native C++ önce geçici bir dosyaya yazar, ardından `FileOutputStream` ile SAF hedef URI akışına yazılır.
* **Doğrulayan Testler:** `app/src/test/java/com/androcad/app/io/CadStorageManagerTest.kt`, `core-kernel/src/main/cpp/tests/test_project_io.cpp`.

### Hata 4: Depolama İzinleri Beklentisi ve Güvenlik Riski
* **Belirti:** Açılışta geniş depolama izni isteneceği beklentisi modern Android mimarisine ve Google Play politikalarına aykırıydı.
* **Mimari Karar:** `docs/roadmap.md` içerisine Karar 6 (Storage Access Framework vs. Geniş Depolama İzinleri) eklendi. `READ_EXTERNAL_STORAGE`, `WRITE_EXTERNAL_STORAGE` ve `MANAGE_EXTERNAL_STORAGE` izinleri kesinlikle manifestte yer almayacak şekilde yapılandırıldı. CI sürecine otomatik manifest denetim adımı eklendi.
* **Boş Durum Karşılaması:** Uygulama açılışında kullanıcıyı karşılayan, izin istemeyen, öğreticiye, örneklere veya SAF ile dosya seçmeye yönlendiren `EmptyStateView` eklendi.
* **Doğrulayan Testler:** `app/src/test/java/com/androcad/app/ui/EmptyStateViewTest.kt`, CI Manifest Audit adımı.

---

## 2. Yeni Geliştirilen Özellikler

### Özellik A: İlk Açılış Öğreticisi (Onboarding Tutorial)
* 6 adımlı rehberli tur:
  1. Görünüm Kontrolü & ViewCube (`viewcube_ISO`)
  2. Model Tarayıcısı (Browser) (`browser_header`)
  3. 2D Eskiz & Çizim Kısıtları (`tab_Sketch`)
  4. 3D Katılaştırma (Extrude) (`tab_Solid`)
  5. Parametrik Zaman Çizelgesi (`timeline_step_0`)
  6. SAF STEP & STL İçe/Dışa Aktarma (`tab_FileIO`)
* **Kullanıcı Kolaylığı:** Her adımda "Geç" düğmesi mevcuttur; Android geri hareketi (`BackHandler`) de öğreticiyi derhal kapatır.
* **Kalıcılık:** Tamamlanma veya geçilme durumu Jetpack DataStore (`androcad_tutorial_prefs`) ile kalıcı saklanır; öğretici bir daha otomatik açılmaz.
* **Yeniden Başlatma:** Üst çubuktaki `🎓 TUTORIAL` düğmesi ile istenildiği an sıfırdan açılabilir.
* **Test İzolasyonu:** Diğer UI testlerinin etkilenmemesi için `TutorialManager.disableForTesting()` bayrağı eklendi.
* **Doğrulayan Testler:** `app/src/test/java/com/androcad/app/tutorial/TutorialTest.kt`.

### Özellik B: Özgün Düzenlenebilir Örnek Tasarımlar
* 4 özgün tasarım sıfırdan modellendi (`assets/samples/`):
  1. **L-Braket (`l_bracket.androcad`):** 90° yapısal montaj braketi, çift delik ve parametrik et kalınlığı ($\approx 36.515\text{ mm}^3$).
  2. **Flanşlı Burç (`flanged_bushing.androcad`):** Mil yuvası ve montaj flanşı ($\approx 31.476\text{ mm}^3$).
  3. **Telefon Standı (`phone_stand.androcad`):** Açılı tutucu ve alt kablo yuvası ($\approx 84.672\text{ mm}^3$).
  4. **Delikli Plaka (`perforated_plate.androcad`):** 2x2 simetrik delik ızgaralı montaj sacı ($\approx 62.391\text{ mm}^3$).
* **Asset Dokunulmazlığı (Immutability):** `SamplesManager.copySampleToWorkingFile` ile model açıldığında `context.filesDir/samples/` altına bağımsız bir kopya alınır. APK içindeki orijinal şablon hiçbir zaman değiştirilmez.
* **Parametrik Yeniden Hesaplama:** Parametreler değiştirildiğinde alt DAG düğümleri ve B-Rep hacimleri anında güncellenir.
* **Galeri Arayüzü:** `SamplesGalleryDialog` (`samples_gallery_dialog`, `samples_btn_close`, `sample_open_*`) ile listeleme ve açma.
* **Doğrulayan Testler:** `app/src/test/java/com/androcad/app/samples/SamplesManagerTest.kt`, `app/src/test/java/com/androcad/app/ui/SamplesGalleryDialogTest.kt`, CTest `TestSampleDesignsHostRebuild`.

---

## 3. Ekran Görüntüleri ve UI Envanteri

* `docs/ui-inventory.md` belgesinde yer alan tüm kontroller (%100) arayüzde gerçek `testTag` değerleri ile mevcuttur.
* `docs/screenshots/` altında 8 adet gerçek emülatör ekran görüntüsü üretilmiştir (her biri 10–68 KB, 500 KB sınırının çok altında):
  1. `01_tablet_main_workspace_dark.png` — Tablet ana çalışma alanı (Koyu tema)
  2. `02_tablet_main_workspace_light.png` — Tablet ana çalışma alanı (Açık tema)
  3. `03_phone_main_workspace.png` — Telefon uyarlanabilir arayüzü
  4. `04_tutorial_onboarding_step1.png` — Öğretici 1. adım (Görünüm navigasyonu)
  5. `05_tutorial_onboarding_step3.png` — Öğretici 3. adım (2D Eskiz kısıtları)
  6. `06_samples_gallery_modal.png` — Örnek tasarımlar galerisi penceresi
  7. `07_sample_opened_l_bracket.png` — Sahneye yüklenmiş L-Braket modeli
  8. `08_empty_state_screen.png` — Güvenli boş durum karşılama ekranı

---

## 4. Kararsız Testler ve Çözümleri (Flaky Test Analysis)

1. **DataStore Asenkron Okuma Kararsızlığı:**  
   *Sorun:* Robolectric test ortamında DataStore Flow toplayıcısı bazen ana thread asenkron döngüsünden önce yanıt veremiyordu.  
   *Çözüm:* `runBlocking` blokları ile senkronize edildi ve `TutorialManager.disableForTesting()` bayrağı ile UI testleri arasında katı izolasyon sağlandı.
2. **Geçici Dosya Çakışmaları:**  
   *Sorun:* Hızlı art arda gelen içe/dışa aktarma işlemlerinde geçici dosya adlarının çakışma ihtimali vardı.  
   *Çözüm:* Zaman damgalı (`System.currentTimeMillis()`) ve UUID tabanlı benzersiz dosya adlandırması kullanıldı.

---

## 5. Kalan Bilinen Sorunlar ve Yol Haritası

1. **Montaj Kinematiği:** Çok parçalı montaj mafsal çözücüsü şu anda bellek içi doğrulamaya sahiptir; STEP montaj dosyalarının hiyerarşik B-Rep ayrıştırması sonraki dönüm noktasına bırakılmıştır.
2. **Bulut Senkronizasyonu:** Çok kullanıcılı eşzamanlı düzenleme yol haritasında planlanmıştır.
