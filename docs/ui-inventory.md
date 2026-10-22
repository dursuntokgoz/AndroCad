# AndroCAD — UI Kontrol Envanteri (UI Inventory)

Bu belge, AndroCAD mobil ve tablet arayüzündeki tüm tıklanabilir kontrolleri, atanan `testTag` değerlerini, beklenen davranışlarını ve bu kontrolleri doğrulayan otomatik testleri listeler. Kontroller kod içerisindeki gerçek arayüz bileşenleriyle %100 birebir eşleşir.

---

## 1. Üst Çubuk & Çalışma Alanı Sekmeleri (Top Bar & Workspace Tabs)

| Ekran / Alan | Kontrol Adı | `testTag` | Beklenen Davranış | Doğrulayan Test |
| :--- | :--- | :--- | :--- | :--- |
| Ana Ekran (Üst) | Logo / Marka | `app_logo` | Uygulama simgesini ve marka kimliğini gösterir. | `testTopBarControls` |
| Ana Ekran (Üst) | Tema Değiştirici | `btn_theme_toggle` | Koyu (Dark) ve Açık (Light) temalar arasında geçiş yapar. | `testThemeToggle` |
| Ana Ekran (Üst) | Öğreticiyi Başlat | `btn_restart_tutorial` | İlk kullanım öğreticisini (Onboarding Tutorial) yeniden açar. | `testRestartTutorialFromMenu` |
| Ana Ekran (Üst) | Örnekler Galerisi | `btn_open_samples` | Düzenlenebilir CAD örnek tasarımlar galerisini açar. | `testOpenSamplesGallery` |
| Ana Ekran (Sekme) | Katı Modelleme | `tab_Solid` | Katı modelleme (Extrude, Revolve, Fillet vb.) araç çubuğunu açar. | `testWorkspaceTabSwitching` |
| Ana Ekran (Sekme) | 2D Eskiz | `tab_Sketch` | 2D kısıtlı eskiz araçlarını açar ve kamerayı ÜST (Top) bakışa hizalar. | `testWorkspaceTabSwitching` |
| Ana Ekran (Sekme) | Yüzey Modelleme | `tab_Surface` | Loft, Press-Pull, Thicken araç çubuğunu açar. | `testWorkspaceTabSwitching` |
| Ana Ekran (Sekme) | Sac Metal | `tab_SheetMetal` | Sac metal katlama, K-faktörü açınımı ve DXF araçlarını açar. | `testWorkspaceTabSwitching` |
| Ana Ekran (Sekme) | Poligon Ağ | `tab_Mesh` | Mesh analiz, tamir, pürüzsüzleştirme ve B-Rep dönüşümünü açar. | `testWorkspaceTabSwitching` |
| Ana Ekran (Sekme) | Simülasyon | `tab_Simulation` | Doğrusal statik FEA, Tet10 ve modal titreşim analizi araçlarını açar. | `testWorkspaceTabSwitching` |
| Ana Ekran (Sekme) | İmalat (CAM) | `tab_CAM` | 2.5D ve 3D CNC takım yolları ve G-Code post-processor araçlarını açar. | `testWorkspaceTabSwitching` |
| Ana Ekran (Sekme) | Katmanlı Üretim | `tab_Additive` | 3D baskı FDM dilimleme, dolgu deseni ve G-Code araçlarını açar. | `testWorkspaceTabSwitching` |
| Ana Ekran (Sekme) | 2D Teknik Resim | `tab_Drawing` | 4 görünüşlü teknik resim, kesit, GD&T ve BOM araçlarını açar. | `testWorkspaceTabSwitching` |
| Ana Ekran (Sekme) | Montaj & Mafsal | `tab_Assemble` | Çoklu parça montajı, kinematik mafsal ve çakışma araçlarını açar. | `testWorkspaceTabSwitching` |
| Ana Ekran (Sekme) | Ölçüm & Malzeme | `tab_Inspect` | Kütle, malzeme atama (Çelik, Alüminyum, Titanyum) ve ağırlık merkezini açar. | `testWorkspaceTabSwitching` |
| Ana Ekran (Sekme) | Dosya & G/Ç | `tab_FileIO` | Proje Kaydet/Aç, STEP ve STL SAF import/export araçlarını açar. | `testWorkspaceTabSwitching` |

---

## 2. Alt Araç Çubukları (Sub-Toolbars)

### Katı Modelleme (Solid Tab)
| Kontrol Adı | `testTag` | Beklenen Davranış | Doğrulayan Test |
| :--- | :--- | :--- | :--- |
| Kutu Oluştur | `tool_Box` | 100x50x20 mm parametrik analitik katı prizma oluşturur. | `testSolidTools` |
| Ekstrüzyon | `tool_Extrude` | 2D eskiz profilini Z ekseninde katılaştırır. | `testSolidTools` |
| Döndürerek Katıla | `tool_Revolve` | Profili eksen etrafında döndürerek simetrik gövde üretir. | `testSolidTools` |
| Radyus | `tool_Fillet` | Seçili kenara R:5mm yuvarlatma uygular. | `testSolidTools` |
| Pah | `tool_Chamfer` | Seçili kenara D:3mm pah kırar. | `testSolidTools` |
| Delik | `tool_Hole` | Katı yüzeyine Ø20x30mm silindirik delik açar. | `testSolidTools` |
| Kabuk Boşaltma | `tool_Shell` | Katı içi boşaltılarak T:2mm et kalınlığı bırakır. | `testSolidTools` |
| Birleştir (Join) | `tool_Combine` | İki katı gövdeyi Boolean Join ile birleştirir. | `testSolidTools` |
| Dikdörtgen Dizi | `tool_RectPattern` | Katıyı 3x2 matris ızgara şeklinde çoğaltır. | `testSolidTools` |
| Dairesel Dizi | `tool_CircularPattern` | Katıyı eksen etrafında 6 adet 360° çoğaltır. | `testSolidTools` |
| Aynalama | `tool_Mirror` | Düzlem boyunca simetrik kopya oluşturur. | `testSolidTools` |
| Kalıp Açısı | `tool_DraftAngle` | Seçili yüze 3° kalıptan çıkarma eğimi verir. | `testSolidTools` |
| ISO Civata Deliği | `tool_ISOHoleM6` | ISO 273 standart havşalı M6 cıvata deliği açar. | `testSolidTools` |
| Helisel Vida Mili | `tool_ThreadedBoltM8` | ISO 262 standart M8x30 helisel dişli cıvata üretir. | `testSolidTools` |

### 2D Eskiz (Sketch Tab)
| Kontrol Adı | `testTag` | Beklenen Davranış | Doğrulayan Test |
| :--- | :--- | :--- | :--- |
| Dikdörtgen | `tool_Rectangle` | Eskize 4 kenarlı parametrik dikdörtgen ekler. | `testSketchTools` |
| Çizgi | `tool_Line` | 2D doğru parçası çizer. | `testSketchTools` |
| Çember | `tool_Circle` | 2D dairesel geometri çizer. | `testSketchTools` |
| B-Spline Eğrisi | `tool_BSpline` | 5 kontrol noktalı kübik düzgün eğri üretir. | `testSketchTools` |
| Kenar İzdüşümü | `tool_ProjectEdge` | 3D katı kenarlarını aktif eskiz düzlemine aktarır. | `testSketchTools` |
| Ölçülendirme | `tool_Dimension` | Geometriler arasına boyutsal mesafe kısıtı atar. | `testSketchTools` |
| Geometrik Kısıtlar | `tool_Constraints` | Çakışıklık, paralellik, diklik, teğetlik kısıtlar. | `testSketchTools` |
| Eskizi Bitir | `btn_finish_sketch` | PlaneGCS ile çözüp kapalı profili katılaştırmaya hazır hale getirir. | `testSketchTools` |
| Çizim Tuvali | `sketch_canvas_overlay` | 1 parmak dokunmatik çizim, kauçuk bant ve saydam profil dolgusu sunar. | `testSketchOverlayDisplayedInSketchTab` |
| Durum Rozeti | `sketch_status_badge` | Aktif çizim ve kapalı profil (alan, kenar sayısı) durumunu bildirir. | `testSketchOverlayDisplayedInSketchTab` |
| Izgara Yakalama | `btn_sketch_snap` | 10 mm ızgara yakalama kilit modunu açıp kapatır. | `testSketchOverlayDisplayedInSketchTab` |
| Hazır Dikdörtgen | `btn_sketch_preset` | Tek dokunuşla 100x50 mm kapalı dikdörtgen profil ekler. | `testPresetSketchCreationAndProfileStatus` |
| Geri Al (Eskiz) | `btn_sketch_undo` | Tuvaldeki son eklenen geometriyi siler. | `testPresetSketchCreationAndProfileStatus` |
| Tuvali Temizle | `btn_sketch_clear` | Çizilmiş tüm eskiz nesnelerini temizler. | `testPresetSketchCreationAndProfileStatus` |
| Eskizi Bitir (HUD) | `btn_sketch_finish_hud` | Eskizi bitirip kamerayı izometrik 3D'ye alır ve Ekstrüzyon panelini açar. | `testFinishSketchOpensExtrudeInspector` |

### Katı Modelleme Parametrik Denetleyicileri (Solid Inspector Cards)
| Kontrol Adı | `testTag` | Beklenen Davranış | Doğrulayan Test |
| :--- | :--- | :--- | :--- |
| Ekstrüzyon Paneli | `extrude_inspector_card` | Fusion 360 tarzı yüzen ekstrüzyon parametre kartı. | `testFinishSketchOpensExtrudeInspector` |
| Ekstrüzyon Derinliği | `slider_extrude_depth` | Derinliği 2-150 mm arasında canlı 3D mesh güncellemesiyle ayarlar. | `testFinishSketchOpensExtrudeInspector` |
| Derinlik Göstergesi | `text_extrude_depth` | Seçili ekstrüzyon mesafesini mm cinsinden gösterir. | `testFinishSketchOpensExtrudeInspector` |
| +10 Adımlayıcı | `btn_extrude_step_plus_10` | Ekstrüzyon derinliğini 10 mm artırır. | `testSolidTabToolExtrudeOpensInspector` |
| -1 Adımlayıcı | `btn_extrude_step_minus_1` | Ekstrüzyon derinliğini 1 mm azaltır. | `testSolidTabToolExtrudeOpensInspector` |
| Ekstrüzyon Tamamla | `btn_extrude_ok` | B-Rep katı gövdesini oluşturur, Browser ve Timeline'a ekler. | `testFinishSketchOpensExtrudeInspector` |
| Ekstrüzyon İptal | `btn_extrude_cancel` | Ekstrüzyon önizlemesini iptal edip paneli kapatır. | `testSolidTabToolExtrudeOpensInspector` |
| Kutu Paneli | `box_inspector_card` | Boy, En ve Yükseklik kaydırıcılarını içeren prizma paneli. | `testSolidTabBoxInspector` |
| Kutu Tamamla | `btn_box_ok` | Ayarlanan boyutlarda parametrik kutuyu oluşturur. | `testSolidTabBoxInspector` |
| Radyus Paneli | `fillet_inspector_card` | Canlı yarıçap kaydırıcısı olan radyus paneli. | `testSolidTabFilletChamferHoleInspectors` |
| Radyus Tamamla | `btn_fillet_ok` | Seçili radyus işlemini gövdeye uygular. | `testSolidTabFilletChamferHoleInspectors` |
| Pah Paneli | `chamfer_inspector_card` | Pah mesafesi kaydırıcısı olan pah paneli. | `testSolidTabFilletChamferHoleInspectors` |
| Pah Tamamla | `btn_chamfer_ok` | Seçili pah işlemini gövdeye uygular. | `testSolidTabFilletChamferHoleInspectors` |
| Delik Paneli | `hole_inspector_card` | Çap ve derinlik kaydırıcıları olan delik delme paneli. | `testSolidTabFilletChamferHoleInspectors` |
| Delik Tamamla | `btn_hole_ok` | Silindirik deliği katı üzerinden çıkarır (Boolean Cut). | `testSolidTabFilletChamferHoleInspectors` |

### Dosya & G/Ç (File / IO Tab)
| Kontrol Adı | `testTag` | Beklenen Davranış | Doğrulayan Test |
| :--- | :--- | :--- | :--- |
| Proje Kaydet | `tool_Save` | Proje durumunu JSON/DAG formatında SAF ile kaydeder. | `testFileIOTools` |
| Proje Aç | `tool_Open` | Var olan proje dosyasını SAF ile okuyup sahneye yükler. | `testFileIOTools` |
| STEP Dışa Aktar | `tool_ExportSTEP` | B-Rep katısını SAF üzerinden AP214/242 STEP dosyasına yazar. | `testFileIOTools` |
| STEP İçe Aktar | `tool_ImportSTEP` | Harici STEP dosyasını SAF ile uygulama önbelleğine alıp içe aktarır. | `testFileIOTools` |
| STL Dışa Aktar | `tool_ExportSTL` | Yüzey ağını SAF üzerinden standart ASCII/ikili STL dosyasına yazar. | `testFileIOTools` |
| STL İçe Aktar | `tool_ImportSTL` | Harici STL üçgen ağını SAF üzerinden sahneye yükler. | `testFileIOTools` |

---

## 3. Sol Ağaç Paneli (Browser Tree Panel — Tablet & Telefon)

| Kontrol Adı | `testTag` | Beklenen Davranış | Doğrulayan Test |
| :--- | :--- | :--- | :--- |
| Panel Başlığı / Kapatma | `browser_header` | Ağaç görünümünü açar veya daraltır. | `testBrowserPanelToggle` |
| Belge Ayarları Klasörü | `browser_section_doc_settings` | Ölçü birimi (mm/inç) ve tolerans ayarlarını listeler. | `testBrowserNodeExpansion` |
| İsimlendirilmiş Görünümler | `browser_section_named_views` | Top, Front, Right, Home kamera bakışlarını listeler. | `testBrowserNodeExpansion` |
| Orijin & Düzlemler | `browser_section_origin` | XY, XZ, YZ koordinat düzlemleri görünürlüğünü yönetir. | `testBrowserNodeExpansion` |
| Gövdeler Klasörü | `browser_section_bodies` | Sahnedeki tüm katı gövdeleri listeler. | `testBrowserBodiesTree` |
| Gövde Seçimi | `browser_item_body_1` | Gövdeyi seçer ve 3D sahnede vurgular. | `testBrowserSelection` |
| Gövde Görünürlük Aç/Kapa | `browser_vis_body_1` | Gövdenin render edilme görünürlüğünü gizler/gösterir. | `testBrowserVisibilityToggle` |
| Gövde Yeniden Adlandır | `browser_rename_body_1` | Gövdenin adını düzenlenebilir metinle günceller. | `testBrowserRenameItem` |
| Gövde Sil | `browser_delete_body_1` | Gövdeyi modelden ve ağaçtan güvenle kaldırır. | `testBrowserDeleteItem` |
| Eskizler Klasörü | `browser_section_sketches` | Aktif ve geçmiş 2D eskiz profillerini listeler. | `testBrowserSketchesTree` |
| İnşa Geometrisi Klasörü | `browser_section_construction` | Yardımcı eksen ve referans düzlemlerini listeler. | `testBrowserNodeExpansion` |

---

## 4. 3D Sahne & Görünüm Kontrolleri (Viewport & ViewCube)

| Kontrol Adı | `testTag` | Beklenen Davranış | Doğrulayan Test |
| :--- | :--- | :--- | :--- |
| ViewCube: ÜST | `viewcube_TOP` | Kamerayı üst ortografik düzleme taşır. | `testViewCubeNavigation` |
| ViewCube: SOL | `viewcube_LEFT` | Kamerayı sol görünüşe taşır. | `testViewCubeNavigation` |
| ViewCube: ÖN | `viewcube_FRONT` | Kamerayı ön görünüşe taşır. | `testViewCubeNavigation` |
| ViewCube: SAĞ | `viewcube_RIGHT` | Kamerayı sağ görünüşe taşır. | `testViewCubeNavigation` |
| ViewCube: ALT | `viewcube_BOTTOM` | Kamerayı alt görünüşe taşır. | `testViewCubeNavigation` |
| ViewCube: İZOMETRİK | `viewcube_ISO` | Kamerayı standart 35° izometrik başlangıç pozisyonuna alır. | `testViewCubeNavigation` |
| Yörünge Modu (Orbit) | `nav_orbit` | Tek parmak hareketini küresel yörünge rotasyonuna bağlar. | `testNavigationControls` |
| Kaydırma Modu (Pan) | `nav_pan` | Dokunma hareketini kamera düzlem kaydırmasına bağlar. | `testNavigationControls` |
| Ekrana Sığdır (Fit) | `nav_fit` | Model sınır kutusuna göre kamerayı otomatik ortalar ve odaklar. | `testNavigationControls` |
| Görselleştirme Modu | `nav_display_mode` | Gölgeli+Kenarlı, Düz Gölgeli ve Tel Kafes arasında döngü yapar. | `testNavigationControls` |

---

## 5. Zaman Çizelgesi (Parametric Timeline)

| Kontrol Adı | `testTag` | Beklenen Davranış | Doğrulayan Test |
| :--- | :--- | :--- | :--- |
| Adım Çipi | `timeline_step_0` | İlk özelliğe tıklandığında modeli o adıma geri sarar (Rollback). | `testTimelineInteraction` |
| İleri Sar / Canlı Durum | `timeline_roll_forward` | Modeli en son geçerli durumuna yeniden hesaplayıp ileri sarar. | `testTimelineInteraction` |

---

## 6. Telefon Alt Çubuğu & Modalleri (Phone Navigation Bar & Bottom Sheets)

| Kontrol Adı | `testTag` | Beklenen Davranış | Doğrulayan Test |
| :--- | :--- | :--- | :--- |
| Ağaç Modali | `phone_nav_tree` | Telefondan sol tarayıcı (Browser) ağaç panelini alttan açar. | `testPhoneBottomSheets` |
| Zaman Çizelgesi Modali | `phone_nav_timeline` | Parametrik geçmiş ve geri alma listesini alttan açar. | `testPhoneBottomSheets` |
| Özellikler Modali | `phone_nav_props` | Seçili elemanın geometrik ölçü ve özellik panelini açar. | `testPhoneBottomSheets` |
| Modal Kapatma Butonu | `phone_sheet_close` | Açık olan alt modal paneli kapatır. | `testPhoneBottomSheets` |

---

## 7. Boş Durum Karşılama Ekranı (Empty-State Screen)

| Kontrol Adı | `testTag` | Beklenen Davranış | Doğrulayan Test |
| :--- | :--- | :--- | :--- |
| Boş Durum Ekranı | `empty_state_screen` | İlk açılışta veya model boşken kullanıcıyı karşılar. | `testEmptyStatePresentation` |
| Öğreticiyi Başlat Butonu | `empty_state_btn_tutorial` | 6 adımlı etkileşimli öğretici turunu başlatır. | `testEmptyStatePresentation` |
| Örnek Tasarımları Aç | `empty_state_btn_samples` | Düzenlenebilir CAD modelleri galerisini açar. | `testEmptyStatePresentation` |
| Dosya İçe Aktar (Import) | `empty_state_btn_import` | SAF ile cihazdan STEP/STL dosyası seçtirir. | `testEmptyStatePresentation` |
| Yeni Parça Oluştur | `empty_state_btn_new_part` | Varsayılan 100x50x20mm parametrik katı blokla başlar. | `testEmptyStatePresentation` |

---

## 8. İlk Açılış Öğreticisi (Onboarding Tutorial Overlay)

| Kontrol Adı | `testTag` | Beklenen Davranış | Doğrulayan Test |
| :--- | :--- | :--- | :--- |
| Öğretici Katmanı | `tutorial_overlay` | Vurgulanan arayüz elemanının üzerinde etkileşimli ipucu gösterir. | `testTutorialFlow` |
| Geç (Skip) Butonu | `tutorial_btn_skip` | Öğreticiyi hemen kapatır ve kalıcı olarak "tamamlandı" kaydeder. | `testTutorialSkipAction` |
| İleri (Next) Butonu | `tutorial_btn_next` | Bir sonraki rehberli adıma geçer. | `testTutorialFlow` |
| Geri (Prev) Butonu | `tutorial_btn_prev` | Bir önceki öğretici adımına döner. | `testTutorialFlow` |
| Bitir (Finish) Butonu | `tutorial_btn_finish` | Son adımda öğreticiyi tamamlar ve sahneyi açar. | `testTutorialCompletion` |
| Adım Göstergesi | `tutorial_step_indicator` | "Adım X / 6" ilerleme durumunu gösterir. | `testTutorialFlow` |

---

## 9. Düzenlenebilir Örnek Tasarımlar Galerisi (Samples Gallery)

| Kontrol Adı | `testTag` | Beklenen Davranış | Doğrulayan Test |
| :--- | :--- | :--- | :--- |
| Galeri İletişim Kutusu | `samples_gallery_dialog` | Örnek tasarımların kartlarını ve küçük önizlemelerini listeler. | `testSamplesGalleryPresentation` |
| Galeri Kapatma Butonu | `samples_btn_close` | Galeri penceresini kapatır. | `testSamplesGalleryPresentation` |
| L-Braket Kartı & Aç Butonu | `sample_open_l_bracket` | Özgün L-Braket modelinin düzenlenebilir kopyasını sahneye yükler. | `testOpenSampleLBracket` |
| Flanşlı Burç Kartı & Aç | `sample_open_flanged_bushing` | Özgün döner parçalı burç modelinin kopyasını sahneye yükler. | `testOpenSampleFlangedBushing` |
| Telefon Standı Kartı & Aç | `sample_open_phone_stand` | Özgün açılı telefon tutucu modelinin kopyasını sahneye yükler. | `testOpenSamplePhoneStand` |
| Delikli Plaka Kartı & Aç | `sample_open_perforated_plate` | Dizi ve delik özellikli montaj plakasının kopyasını yükler. | `testOpenSamplePerforatedPlate` |
