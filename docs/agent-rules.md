# AndroCAD — Ajan Geliştirme Kuralları (Kalıcı)

1. **Yerel Araç Zinciri Yok:** Derleme ve test yalnızca GitHub Actions üzerinde yapılır.
2. **Güvenlik ve Gizlilik:** Token, keystore veya kimlik bilgilerini asla dosyalara, loglara ya da commit'lere yazma.
3. **Kimlik Doğrulama Koruması:** `git` (SSH) ana kimlik doğrulama kaynağıdır; `git` kimlik doğrulaması başarısız olursa DUR ve kullanıcıya bildir (`gh` CLI doğrudan kullanılmıyorsa göz ardı edilebilir).
4. **Fikri Mülkiyet ve Lisanslama:**
   - Kodumuz tescillidir (`// Copyright (c) 2026 dursuntokgoz. All rights reserved.`), tüm hakları saklıdır.
   - `LICENSE` ve `THIRD_PARTY_NOTICES.md` dosyalarına dokunulmaz.
   - Uygulama çekirdeğinde GPL kod barındırılamaz.
   - Autodesk varlığı veya telifli öğesi kullanılamaz.
5. **Dallanma ve PR Süreci:**
   - Her düzeltme ve geliştirme alanı için ayrı branch açılır, `main` dalına PR gönderilir.
   - Yalnızca GitHub Actions CI yeşil olduğunda birleştirilir (merge edilir).
   - Anlaşılır commit mesajları yazılır. Her anlamlı iyileştirmeden sonra GitHub'a pushlanır.
6. **Performans Metrikleri:**
   - Performans rakamları kesin olarak "hedef" (target) veya "ölçülen" (measured) şeklinde etiketlenir.
7. **Uygulama Adı:**
   - Uygulamanın adı değiştirilemez (`AndroCAD`).
