# Milestone 2 Raporu: Yerel Görünüm Alanı (Viewport) ve Seçim (Picking) Motoru

**Tarih:** 2026-10-20  
**Durum:** Tamamlandı  
**Pull Request:** [#2 - Milestone 2: Native Viewport & Picking Engine](https://github.com/dursuntokgoz/AndroCad/pull/2)  
**CI Actions Koşuları:**
* Push CI Koşusu: [Run #39323679358](https://github.com/dursuntokgoz/AndroCad/actions/runs/39323679358)
* Pull Request CI Koşusu: [Run #39323683206](https://github.com/dursuntokgoz/AndroCad/actions/runs/39323683206)

---

## 1. Yönetici Özeti

Milestone 2 kapsamında **AndroCAD** için etkileşimli yerel 3B görünüm alanı, ViewCube gezinme bileşeni, çoklu görüntüleme modları ve donanım hızlandırmalı $O(1)$ GPU seçim motoru ile yerel C++ ray-cast fallback altyapısı başarıyla tamamlandı:

1. **Topolojik Kenar Çıkarma (Boundary Edge Extraction):** OCCT `TopExp::MapShapes(..., TopAbs_EDGE)` kullanılarak katı gövdenin tekil kenarları ayrıştırıldı; `BRepAdaptor_Curve` ve `GCPnts_QuasiUniformDeflection` ile 3B çizgi parçalarına dönüştürüldü ve GPU VBO'suna aktarıldı (12 kenar, 24 nokta).
2. **Çift Modlu Seçim (Picking) Mimarisi:**
   * **GPU Renk Kodlu $O(1)$ Seçim:** Ekran dışı Framebuffer Object (FBO) ile yüzey ve kenar ID'leri RGB kanallarına kodlandı. `glReadPixels` ile anlık piksel okuması yapılarak dokunulan yüzey veya kenar tespit edilir.
   * **Yerel C++ Ray-Caster:** GPU okuma gecikmesi (pipeline stall) riskine karşı Möller–Trumbore üçgen ve doğru parçası mesafe algoritmalarıyla doğrudan model koordinatlarında ışın atımı sağlandı.
3. **Etkileşimli Kamera ve ViewCube:**
   * Küresel koordinatlı kamera (Azimuth, Elevation, Distance) ile tek parmak orbit (gimbal lock önleyici sınırlandırma), iki parmak kaydırma (pan) ve iki parmak sıkıştırma ile yakınlaştırma (pinch zoom).
   * Model sınırlayıcı kutusuna (Bounding Box) göre otomatik kadrajlama (Zoom to Fit).
   * 3B ViewCube arayüzü ile standart ortogonal (TOP, FRONT, RIGHT, BACK, LEFT, BOTTOM) ve izometrik (ISO/HOME) görünümlere anında geçiş.
4. **Çoklu Görüntüleme Modları (Display Modes):**
   * `Shaded`: Blinn-Phong ışıklandırmalı düzgün gölgelendirme.
   * `Shaded + Edges`: Poligon ofset (`glPolygonOffset`) ile Z-fighting derinlik çekişmesi engellenmiş belirgin CAD kenar çizgileri.
   * `Wireframe`: Yalnızca topolojik sınır çizgilerinden oluşan tel kafes görünümü.
5. **Seçim Vurgulama:** Seçilen yüzeyler parlak camgöbeği (Cyan `#00A4E4`), seçilen kenarlar ise belirgin turuncu (`#FF9800`) ile anında görselleştirilir ve Özellikler panelinde detayları görüntülenir.

---

## 2. Test Sonuçları

### Host CTest Test Paketi (`androcad_host_smoke_test`)
```
[AndroCAD Test] Running Kernel Smoke Test (Box Creation, Edges & Ray Picking)...
  Calculated Volume: 100000 mm³ (Expected: 100000 mm³)
  Boundary Face Count: 6 (Expected: 6)
  Triangles Generated: 12
  Vertices Generated: 24
  Indices Generated: 36
  Boundary Edge Count: 12 (Expected: 12)
  Discretized Edge Vertices: 24 (Expected >= 24)
  Bounding Box: [-1e-07, -1e-07, -1e-07] to [100, 50, 20]
  Ray Pick Face Result: hit=1 entityId=6 isEdge=0 dist=80
  Ray Pick Edge Result: hit=1 edgeId=12 dist=80
  Near-Edge Pick Result: hit=1 edgeId=12
[PASS] AndroCAD Milestone 2 Kernel & Picking Tests passed successfully!
100% tests passed, 0 tests failed out of 1
```

### JVM Birim Testleri (`NativeKernelTest`)
```
com.androcad.kernel.NativeKernelTest > testBoxMeshResultDataContract PASSED
com.androcad.kernel.NativeKernelTest > testPickResultContract PASSED
```

---

## 3. Ölçülen Boyutlar ve Performans Metrikleri

* **Debug APK Boyutu (Sıkıştırılmamış):** **70 MB**
* **Debug APK ZIP:** **~31 MB**
* **Kenar Çıkarma Süresi:** < 1 ms (100x50x20 kutu için)
* **Ray-Cast Seçim Süresi:** < 0.01 ms (CPU tarafı)
* **GPU Picking Gecikmesi:** 1 kare içinde (anlık $O(1)$)

---

## 4. Bir Sonraki Adım: Milestone 3 (PlaneGCS ile 2B Eskiz Modülü)

Görünüm alanı ve seçim altyapısı hazır olduğundan, bir sonraki aşamada **Milestone 3** başlatılacaktır:
1. FreeCAD'in `PlaneGCS` kütüphanesinin projeye dahil edilmesi (LGPL-2.1-or-later).
2. Temel 2B eskiz geometrileri: Doğru parçası (Line), Yay (Arc), Çember (Circle), Dikdörtgen (Rectangle), Spline.
3. Geometrik kısıtlayıcılar (Constraints): Çakışık (Coincident), Yatay/Dikey (Horizontal/Vertical), Paralel, Dik (Perpendicular), Teğet (Tangent), Eşitlik (Equal), Ölçü/Boyut (Dimension).
4. Kapalı profil döngüsü algılama (Profile Loop Detection) ve katı modelleme yüzeylerinde eskiz düzlemi oluşturma.
