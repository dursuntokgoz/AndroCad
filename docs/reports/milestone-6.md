# Milestone 6 Raporu: Versiyonlu Proje I/O ve CAD Veri Değişimi (STEP, STL, .androcad)

**Tarih:** 2026-10-20  
**Durum:** Tamamlandı  
**Pull Request:** [#6 - Milestone 6: Versioned Project I/O & Data Exchange (STEP, STL, .androcad)](https://github.com/dursuntokgoz/AndroCad/pull/6)  

---

## 1. Yönetici Özeti

Milestone 6 kapsamında **AndroCAD** için endüstri standardı CAD veri değişimi ve yerel versiyonlu proje dosyası serileştirme mimarisi (`ProjectIO.h`, `ProjectIO.cpp`) geliştirilmiş, OpenCASCADE Technology (`TKDESTEP`, `TKDESTL`, `TKXSBase`) ve yerel C++ katmanı üzerinde tam döngü (roundtrip) testleri ile doğrulanmıştır:

### 1.1 Yerel Proje Formatı (`.androcad` JSON)
* **Versiyonlu Başlık (Versioning Header):** Format versiyonu (`format_version = 1`), uygulama adı (`appName = "AndroCAD"`), sürüm (`appVersion = "0.1.0"`), birim sistemi (`units = "mm"`) ve şema tanımı (`androcad.parametric.project.v1`).
* **Kullanıcı Parametreleri Tablosu:** Tüm adlandırılmış kullanıcı parametreleri ve analitik matematiksel ifadeleri saklanır (`width: 120.0`, `height: width / 3`, vb.).
* **Zaman Çizelgesi DAG ve TNP Referansları:** Tüm modelleme unsurları, semantik etiketleri, `BRepTools_History` geçmişi ve 3. kademe geometrik parmak izleri (kütle merkezi, normal vektör, alan/uzunluk metriği) eksiksiz olarak JSON yapısına yazılır ve okunur.
* **Geri Alma İşaretçisi (Rollback Index):** Çizelgenin kaydedildiği andaki geri sarma konumu saklanır ve dosya açıldığında model aynı deterministik geçmiş noktasına restore edilir.

### 1.2 STEP AP214 / AP242 Veri Değişimi
* **Dışa Aktarma (`ExportSTEP`):** `STEPControl_Writer` kullanılarak model topolojik B-Rep katı geometrisi bozulmadan AP214 / AP242 standardında milimetrik hassasiyetle STEP dosyasına dönüştürülür.
* **İçe Aktarma (`ImportSTEP`):** `STEPControl_Reader` ile üçüncü parti CAD yazılımlarından (Fusion 360, FreeCAD, SolidWorks, CATIA) gelen STEP dosyaları okunup `TopoDS_Shape` katı gövdeye dönüştürülür ve GPU üçgen ağı oluşturularak anında 3B viewport üzerinde görüntülenir.
* **Hacim Korunumu:** Dışa aktarılan ve geri içe aktarılan model hacimleri karşılaştırıldığında $|\Delta V| < 10^{-2}\text{ mm}^3$ hassasiyet sağlandığı kanıtlanmıştır.

### 1.3 İkili ve ASCII STL Veri Değişimi
* **Dışa Aktarma (`ExportSTL`):** `BRepMesh_IncrementalMesh` ile dinamik açısal ve çizgisel sapma (deflection) toleransı ayarlanarak katı gövde üçgen ağına dönüştürülür ve `StlAPI_Writer` ile kompakt ikili (binary) STL formatında diske yazılır.
* **İçe Aktarma (`ImportSTL`):** `StlAPI_Reader` ile dış kaynaklı STL üçgen ağları okunup topolojik kabuk / yüzey yapısına aktarılır.

---

## 2. Test ve Doğrulama Sonuçları (`test_project_io.cpp`)

Milestone 6 için oluşturulan otomatik CTest doğrulama paketi 4 kapsamlı test senaryosunu içermektedir:

```
[AndroCAD Test] Running STEP AP214 Export & Import Roundtrip Test...
  Original Model Volume: 140575 mm³ (Expected: ~140575 mm³)
  Export STEP result: SUCCESS
  Import STEP result: SUCCESS
  Imported Model Volume: 140575 mm³
  Volume Conservation Delta: 0 mm³
  Imported Face Count: 7 (Expected: 7)
  [PASS] STEP AP214 roundtrip test verified successfully!

[AndroCAD Test] Running STL Binary Export & Import Roundtrip Test...
  Export STL result: SUCCESS
  Import STL result: SUCCESS
  STL Bounding Box: [60 x 40 x 20]
  [PASS] STL roundtrip test verified successfully!

[AndroCAD Test] Running Native Project (.androcad JSON) Roundtrip Test...
  Original Project Volume: 287900 mm³
  SaveProject result: SUCCESS, nodes=2
  LoadProject result: SUCCESS, nodes=2
  Loaded Project Recomputed Volume: 287900 mm³
  [PASS] Native .androcad project serialization verified successfully!

[AndroCAD Test] Running Data Exchange Robustness & Safety Checks...
  [PASS] All robustness and error guards verified without crashes!

[PASS] All Milestone 6 Data Exchange & Project I/O Tests Passed Successfully!
100% tests passed, 0 tests failed out of 1
```

---

## 3. Kullanıcı Arayüzü ve Android Entegrasyonu

* **"File / IO" Çalışma Alanı Sekmesi:** Üst araç çubuğuna eklenen File / IO sekmesi ile kullanıcılar tek dokunuşla projeyi kaydedebilir (`Save Project`), var olan projeyi açabilir (`Open Project`), STEP ve STL formatlarında model aktarımı gerçekleştirebilir (`Export/Import STEP`, `Export/Import STL`).
* **Android Depolama Alanı:** Dosyalar güvenli bir şekilde uygulamanın dahili dizininde (`context.filesDir`) yönetilir; bellek sızıntıları önlenmiş ve tüm yerel C++ nesneleri güvenli JNI `Closeable` sarmalayıcıları ile korunmuştur.
* **JVM Birim Testleri:** `NativeKernelTest.kt` içerisine `testDataExchangeContract` testi eklenmiş ve yerel ikili dosyalar yüklenmediğinde bile JNI sarmalayıcılarının güvenle hata döndürdüğü doğrulanmıştır.
