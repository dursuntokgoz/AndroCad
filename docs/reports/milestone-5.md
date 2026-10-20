# Milestone 5 Raporu: Parametrik Zaman Çizelgesi, Parametre İfadeleri ve Kalıcı Topolojik İsimlendirme (TNP)

**Tarih:** 2026-10-20  
**Durum:** Tamamlandı  
**Pull Request:** [#5 - Milestone 5: Parametric Timeline, Parameter Evaluation & Persistent Naming (TNP)](https://github.com/dursuntokgoz/AndroCad/pull/5)  

---

## 1. Yönetici Özeti

Milestone 5 kapsamında **AndroCAD** projesinin kalbi olan **Parametrik Modelleme Zaman Çizelgesi (Timeline)**, **Kullanıcı Parametreleri / İfade Ayrıştırıcı (Expression Engine)** ve CAD dünyasının en kritik problemlerinden biri olan **Kalıcı Topolojik İsimlendirme Çözücüsü (TNP Resolver)** başarıyla geliştirilmiş ve 5 senaryolu kapsamlı regresyon paketi ile doğrulanmıştır.

### 1.1 Aritmetik İfade Ayrıştırıcı (`ExpressionParser.h`, `ExpressionParser.cpp`)
* **Özellikler:** Recursive descent / shunting-yard algoritması ile string tabanlı parametre ifadelerini (`width * 2 + 10`, `sqrt(depth^2 + height^2)`, `sin(angle * PI / 180)`) ayrıştırır ve hesaplar.
* **Desteklenen İşlemler:** Temel operatörler (`+`, `-`, `*`, `/`, `^`, tekil eksi), matematiksel fonksiyonlar (`sqrt`, `abs`, `sin`, `cos`, `tan`, `min`, `max`) ve evrensel sabitler (`PI`, `E`).
* **Döngü Tespiti (Cycle Detection):** Parametreler arasındaki dairesel bağımlılıkları (`a = b + 1`, `b = a * 2`) derinlik öncelikli arama (DFS) ile tespit ederek sonsuz döngüleri engeller.

### 1.2 Kalıcı Topolojik İsimlendirme (TNP) Çözücüsü (`TNPResolver.h`, `TNPResolver.cpp`)
OpenCASCADE modellerinde bir unsur değiştirildiğinde topolojik etiketlerin (örneğin Face 3 -> Face 5) değişmesi veya kaybolması problemine karşı 3 kademeli hiyerarşik eşleme mimarisi kurulmuştur:
1. **Kademe 1 (Anlamsal Alt-Şekil Etiketleme):** Üretici unsurlardan (`Extrude`, `Revolve`, `Fillet`) doğan geometrik alt-elemanlar semantik olarak isimlendirilir (`Pad1:Cap_End`, `Pad1:Side_Face_0`, vb.).
2. **Kademe 2 (`BRepTools_History` Evrim Takibi):** Bir Boole veya modifikasyon operasyonunda bölünen veya değişen yüzey/kenarlar OCCT'nin `BRepTools_History` geçmiş ağacı üzerinden takip edilir.
3. **Kademe 3 (Geometrik Parmak İzi - Invariant Fingerprint):** Şeklin kütle/ağırlık merkezi (Centroid), normal vektörü, yüzey/eğri tipi (`Plane`, `Cylinder`, `Line`, vb.) ve alan/uzunluk metrikleri üzerinden ağırlıklı Öklid mesafesi ile en yakın eşleşme bulunur.

### 1.3 Parametrik Zaman Çizelgesi DAG Motoru (`TimelineDAG.h`, `TimelineDAG.cpp`)
* **Bağımlılık Grafiği (DAG):** Her unsurun girdilerini ve çıktığı geometrik referansları izleyen yönlendirilmiş döngüsüz çizge.
* **Geri Alma İşaretçisi (Rollback Marker):** Çizelge geçmişteki herhangi bir adıma geri sarılabilir (`SetRollbackIndex`); bu noktadan sonraki adımlar bastırılır ve yeni unsurlar araya eklenebilir.
* **Deterministik Yeniden Hesaplama (`Recompute`):** Bir kullanıcı parametresi değiştirildiğinde DAG topolojik sırayla taranarak etkilenen tüm unsurlar yeniden oluşturulur.
* **Zarif Hata Yönetimi (`BrokenReference`):** Yok olan veya çözülemeyen referanslarda çekirdek çökmez; hata izole edilerek kullanıcıya arayüzde kırık referans bilgisi sunulur.

---

## 2. TNP Regresyon Testleri ve Doğrulama (`test_tnp_regression.cpp`)

`docs/architecture/tnp-test-plan.md` mimari dökümanında taahhüt edilen 5 kritik senaryonun tamamı otomatik CTest paketinde uygulanmış ve başarıyla geçmiştir:

```
[AndroCAD Test] Running TNP Regression Test Suite...
  TNP Scenario 1 Passed: Downstream sketch face resolved successfully after upstream dimension change!
  TNP Scenario 2 Passed: Fillet edge resolved successfully after depth extrusion change!
  TNP Scenario 3 Passed: Split face disambiguated successfully via BRepTools_History & centroid!
  TNP Scenario 4 Passed: Parametric Timeline DAG recomputed successfully!
  TNP Scenario 5 Passed: Vanishing topology handled gracefully without crash!
[PASS] All 5 TNP Regression Scenarios passed with 100% success!
100% tests passed, 0 tests failed out of 1
```

### Senaryoların Detayları:
1. **Senaryo 1 (Yukarı Akış Boyut Değişikliği):** 100x50x30 mm prizmanın üst yüzüne çizilip ekstrüde edilen ikinci blok, taban prizma 150x80x60 mm'ye büyütüldüğünde TNP çözücüsü sayesinde üst kapağı bulmuş ve model doğru şekilde yeniden hesaplanmıştır.
2. **Senaryo 2 (Kenar Yuvarlatma Kalıcılığı):** Prizma üzerindeki bir kenara uygulanan yuvarlatma, ekstrüzyon derinliği 20 mm'den 50 mm'ye çıkarıldığında parmak izi eşleme ile doğru kenarı bularak yuvarlatmayı korumuştur.
3. **Senaryo 3 (Boole Kesmesi ile Yüz Bölünmesi ve Centroid Ayrıştırması):** Bir bloğun üst yüzü Boole farkı ile ikiye bölündüğünde, `BRepTools_History` ve ağırlık merkezi ayrıştırması ile hedeflenen yarım yüz doğru şekilde tespit edilmiştir.
4. **Senaryo 4 (Zaman Çizelgesi DAG ve Rollback):** Kullanıcı parametreleri (`width = 120.0`, `scale = 2.0`) DAG boyunca yayılarak modeller güncellenmiş ve rollback pozisyonunda modelin geçmiş anındaki hali başarıyla üretilmiştir.
5. **Senaryo 5 (Yok Olan Topoloji ve Çökme Koruması):** Referans alınan bir geometri tamamen kaybolduğunda motor `BrokenReference` bildirmiş ve güvenli şekilde çalışmaya devam etmiştir.

---

## 3. Kullanıcı Arayüzü ve JNI Entegrasyonu

* **`NativeTimeline` JNI Bağlantısı:** `NativeKernel.kt` içine `createTimeline()`, `addFeature()`, `setParameter()`, `setRollbackIndex()`, `rebuild()` ve `evaluateExpression()` JNI fonksiyonları eklenmiştir.
* **Kullanıcı Parametreleri Paneli:** Android arayüzünde aktif parametreler (`width`, `height`, `depth`, vb.) ve analitik formülleri görüntülenmektedir.
* **Etkileşimli Zaman Çizelgesi (Timeline UI):** Alt panelde modelleme adımları sırayla kartlar halinde listelenir; rollback imleci ile istenen adıma geri sarılabilir ve ileri sarılabilir.
* **Birim Testleri:** `NativeKernelTest.kt` içinde ifade ayrıştırıcı ve zaman çizelgesi veri kontratları test edilmiştir.
