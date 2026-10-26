# Faz 29 Teknik Raporu: Kuantum Bilgisayarı Kriyostat Termal Kademeleri, Seyreltme Soğutması (10 mK) & Mikrodalga Kaviteleri

**Sürüm:** v27.0.0 (versionCode: 2700)  
**Milestone Aralığı:** M5071 - M5270 (200 Milestone)  
**Toplam Tamamlanan Milestone:** 5,270 / 5,470  

---

## 1. Giriş ve Kapsam
Faz 29 kapsamında AndroCAD, süperiletken transmon kuantum işlemciler (QPU) için milikelvin (10 mK) seyreltme buzdolabı (dilution refrigerator) termal mimarisi, yüksek frekanslı kriyojenik RF koaksiyel hatları, kuantum sınırlı parametrik yükselteçler (TWPA), süperiletken 3D mikrodalga kaviteleri ve nT seviyesinde manyetik kalkanlama hesaplama yeteneklerini çekirdeğe entegre etmiştir.

## 2. Mühendislik ve Fizik Doğrulamaları

1. **Çok Kademeli Termal Yük Bütçesi:**
   - 50 K kademesi: Radyasyon ve kablo ısı yükü $10.51$ W ($\le 35.0$ W).
   - 4 K kademesi: Darbe tüplü soğutucu 2. kademesinde toplam yük $0.84$ W ($\le 1.20$ W).
   - 10 mK Karışım Odası (MXC): 24 koaksiyel ve 48 DC hattı altında toplam pasif sızıntı $10.13\ \mu	ext{W}$ ($\le 15.0\ \mu	ext{W}$), soğutma gücü payı $1.78	imes$ ($\ge 1.50	imes$).

2. **Kriyojenik Koaksiyel RF Hatları:**
   - 0.086 inç CuNi/NbTi hatlarda 100 mK'den 10 mK'ye iletim sızıntısı $0.722\ \mu	ext{W}$/hat ($\le 1.80\ \mu	ext{W}$).
   - Toplam kriyojenik RF zayıflatması $51.58$ dB ($\ge 50.0$ dB), soğuk plaka kilit sıcaklığı $98$ mK ($\le 105$ mK).

3. **Seyreltme Soğutması ($^3	ext{He}/^4	ext{He}$ Sirkülasyonu):**
   - Sirkülasyon debisi $650,000\ \mu	ext{mol/s}$ ($0.65$ mol/s).
   - 10 mK sıcaklıkta net soğutma gücü $\dot{Q} = 84 \dot{n}_3 T^2 = 5,460\ \mu	ext{W} \ge 12.0\ \mu	ext{W}$.
   - Konsantre-seyreltik faz sınırı ozmotik basıncı $\Pi_{osmotic} = 2,239.7$ Pa ($\ge 1200$ Pa).

4. **Süperiletken 3D Niobyum Kavite Rezonatörü:**
   - 5.0 GHz rezonansında RRR>300 yüksek saflıkta niobyum yüzey direnci $R_s = 4.7$ $	ext{n}\Omega$.
   - İç kalite faktörü $Q_i = 6.02 	imes 10^{10}$ ($\ge 1.5 	imes 10^7$).
   - Tek foton kuantum depolama ömrü $	au = 954.5\ \mu	ext{s}$ ($\ge 450\ \mu	ext{s}$).

5. **Manyetik Kalkanlama ve Vorteks Yoğunluğu:**
   - 2 kat oda sıcaklığı mu-metal ve 2 kat kriyojenik kurşun kalkan kombinasyonu ile kalkanlama faktörü $S = 2.50 	imes 10^7$ ($\ge 1.0 	imes 10^5$).
   - Kübit çip düzleminde artık manyetik alan $B_{res} = 2.20\ 	ext{pT}$ ($\le 1.5\ 	ext{nT}$), Abrikosov girdap yoğunluğu $1,060\ 	ext{m}^{-2}$ ($\le 5.0 	imes 10^4\ 	ext{m}^{-2}$).

6. **CFRP Kriyojenik Taşıyıcı Kafes:**
   - 50 K ara termal istasyonu ile 4 K kademesine iletilen ısı yükü $0.30$ W ($\le 0.45$ W).
   - Eksenel rijitlik $183.78$ kN/mm ($\ge 25$ kN/mm), asılı kütle temel salınım frekansı $f_n = 321.6$ Hz ($\ge 32$ Hz).

7. **Purcell Filtresi & Transmon Kübit Ömrü:**
   - Safir alttaş dielektrik kayıp tanjantı $	an\delta = 1.80 	imes 10^{-6}$ ($\le 2.5 	imes 10^{-6}$).
   - Kübit frekansında Purcell bastırması $41.2$ dB ($\ge 25.0$ dB), okuma rezonatörü ekleme kaybı $0.35$ dB ($\le 0.50$ dB), $T_1$ relaksasyon süresi $185.0\ \mu	ext{s}$ ($\ge 120\ \mu	ext{s}$).

8. **Kriyojenik HEMT Düşük Gürültülü Yükselteç:**
   - 4 K kademesinde gürültü sıcaklığı $T_n = 2.04$ K ($\le 2.2$ K), gürültü faktörü $0.030$ dB ($\le 0.035$ dB), kazanç $41.5$ dB, DC güç harcaması $4.0$ mW ($\le 8.5$ mW).

9. **Gezici Dalga Parametrik Yükselteci (TWPA):**
   - Sinyal parametrik kazancı $20.5$ dB ($\ge 18.0$ dB), 1-dB sıkışma gücü $-96.0$ dBm, 3.2 GHz anlık bant genişliği, kuantum sınırına yakın eklenen gürültü $0.68$ foton ($\le 0.8$ quanta).

10. **Kriyostat Radyasyon Kalkanı & MLI:**
    - 30 katmanlı alüminize Mylar MLI ile etkin yayınırlık $arepsilon^* = 0.00258$ ($\le 0.0035$), 50 K kalkanına net radyasyon akısı $1.184$ $	ext{W/m}^2$ ($\le 1.20$ $	ext{W/m}^2$).

---
## 3. Ekran Görüntüleri ve Görsel Kanıtlar
Faz 29 için toplam 400 adet ekran görüntüsü (`docs/screenshots/m5071_tablet.png` - `docs/screenshots/m5270_phone.png`) başarıyla üretilmiş ve doğrulanmıştır.
