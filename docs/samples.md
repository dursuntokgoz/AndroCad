# AndroCAD — Özgün Düzenlenebilir Örnek Tasarımlar (Sample Designs)

Bu belge, AndroCAD içerisinde kullanıcılara hazır sunulan ve doğrudan düzenlenebilen özgün örnek tasarımları, matematiksel parametrelerini ve sergiledikleri CAD özelliklerini belgeler.

---

## 1. Fikri Mülkiyet ve Özgünlük Taahhüdü (Originality Statement)

AndroCAD içerisinde yer alan tüm örnek modeller **%100 özgün olarak** sıfırdan tasarlanmıştır. Hiçbir üçüncü taraf kütüphaneden, Autodesk® varlıklarından ya da dış CAD depolarından model kopyalanmamıştır. Tüm modeller AndroCAD'in kendi açık, sürümlü `.androcad` (JSON tabanlı B-Rep parametrik DAG) formatında tanımlıdır.

Modeller `assets/samples/` dizininde **salt okunur (read-only)** olarak saklanır. Kullanıcı bir modeli açtığında, model uygulama çalışma dizinine (`context.filesDir/samples/`) düzenlenebilir bir **kopya** olarak alınır; orijinal asset hiçbir koşulda bozulmaz.

---

## 2. Örnek Modeller Kataloğu

### Model 1: L-Braket (`l_bracket.androcad`)
- **Kategori:** Yapısal / Mekanik Bağlantı Elemanı
- **Tanım:** 90° yapısal montaj braketi. Çift ISO M10/M6 montaj deliği ve parametrik et kalınlığı içerir.
- **Gösterilen Özellikler:**
  - Parametrik prizmatik katı gövde (`Box`)
  - Çift silindirik delik çıkarma (`Hole`)
  - Çoklu değişkenli matematiksel ifade değerlendirme (`ExpressionParser`: `base_len`, `width`, `base_thk`, `hole_rad`)
- **Nominal Parametreler:**
  - `base_len = 80.0 mm`
  - `width = 40.0 mm`
  - `base_thk = 12.0 mm`
  - `hole_rad = 5.0 mm` (Ø10 mm cıvata delikleri, merkezler: X=20 ve X=60)
- **Beklenen Hacim (Nominal):** $\approx 36.515,04\text{ mm}^3$
- **Parametre Değişim Testi:** `base_len` parametresi 100.0 mm yapıldığında hacim doğrusal olarak $+9.600,00\text{ mm}^3$ artarak $46.115,04\text{ mm}^3$ değerine ulaşır.

---

### Model 2: Flanşlı Burç (`flanged_bushing.androcad`)
- **Kategori:** Döner Eleman / Rulman Yatağı
- **Tanım:** Mil geçişi ve yüzey sabitlemesi için tasarlanmış merkez delikli flanş gövdesi.
- **Gösterilen Özellikler:**
  - Kare flanş tabanı (`Box`)
  - Eksenel hassas şaft geçiş deliği (`Hole`)
  - Parametrik mil çapı değişimi
- **Nominal Parametreler:**
  - `flange_dx = 60.0 mm`
  - `flange_dy = 60.0 mm`
  - `flange_thk = 10.0 mm`
  - `bore_radius = 12.0 mm` (Ø24 mm mil yuvası, merkez: X=30, Y=30)
- **Beklenen Hacim (Nominal):** $\approx 31.476,11\text{ mm}^3$
- **Parametre Değişim Testi:** `bore_radius` parametresi 10.0 mm değerine düşürüldüğünde delik alanı küçülür ve net hacim $32.858,41\text{ mm}^3$ seviyesine yükselir.

---

### Model 3: Telefon Standı (`phone_stand.androcad`)
- **Kategori:** Tüketici Ürünü / Masaüstü Aksesuarı
- **Tanım:** Masaüstü akıllı telefon tutucu bloğu. Alt kısmında şarj kablosu geçiş yuvası barındırır.
- **Gösterilen Özellikler:**
  - Eğimli/Kütlesel destek gövdesi (`Box`)
  - Kablo geçiş kanalı (`Hole`)
  - Yükseklik ve derinlik orantılama
- **Nominal Parametreler:**
  - `stand_len = 70.0 mm`
  - `stand_width = 50.0 mm`
  - `stand_height = 25.0 mm`
  - `cable_slot_rad = 6.0 mm` (merkez: X=35, Y=25)
- **Beklenen Hacim (Nominal):** $\approx 84.672,57\text{ mm}^3$
- **Parametre Değişim Testi:** `stand_height` parametresi 30.0 mm yapıldığında yeni net hacim $101.607,08\text{ mm}^3$ olarak yeniden hesaplanır.

---

### Model 4: Delikli Plaka (`perforated_plate.androcad`)
- **Kategori:** Hassas Montaj Plakası / Izgara
- **Tanım:** Dört köşesinde simetrik havalandırma ve montaj delikleri bulunan temel montaj plakası.
- **Gösterilen Özellikler:**
  - Taban sacı / gövde (`Box`)
  - 2x2 düzenli ızgara delikleri (`Hole` çoklu DAG düğümleri)
  - Kalınlık ve plaka ebatlarının tek parametreyle tüm delikleri koruyarak güncellenmesi
- **Nominal Parametreler:**
  - `plate_len = 100.0 mm`
  - `plate_width = 80.0 mm`
  - `plate_thk = 8.0 mm`
  - `hole_rad = 4.0 mm` (Ø8 mm delikler, merkezler: (25, 25), (75, 25), (25, 55), (75, 55))
- **Beklenen Hacim (Nominal):** $\approx 62.391,50\text{ mm}^3$
- **Parametre Değişim Testi:** `plate_thk` parametresi 10.0 mm yapıldığında yeni net hacim $77.989,38\text{ mm}^3$ olarak hesaplanır.

---

## 3. Doğrulama ve Test Kriterleri

1. **Asset Dokunulmazlığı (Immutability):**
   Model açıldığında `context.assets.open(...)` içeriği `context.filesDir/samples/` altına kopyalanır. Düzenlemeler sadece bu yerel dosya üzerinde gerçekleşir, orijinal asset baytları değişmez.
2. **DAG Rebuild & Tolerans:**
   Her örnek hatasız açılır ve `rebuild()` çağrıldığında beklenen hacim $\%0.5$ tolerans içinde doğrulanır.
3. **Parametrik Yeniden Hesaplama:**
   `setParameter(...)` çağrıldığında ve model `rebuild()` edildiğinde alt özellikler ve yüz geometrisi anında güncellenir.
4. **SAF ve Format Gidiş-Dönüşü (Roundtrip):**
   Düzenlenen model `.androcad`, STEP ve STL formatlarında kaydedilip geri yüklendiğinde hacim ve geometri korunur.
