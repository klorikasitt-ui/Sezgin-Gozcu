**Sezgin Gözcü**, görme engelli bireylerin çevrelerindeki engelleri daha güvenli ve sezgisel bir şekilde fark etmelerini sağlayan, Arduino tabanlı bir akıllı rehber sistemidir. Nesne mesafesine göre dinamik (PWM) geri bildirim vererek kullanıcının engel mesafesini hissetmesini sağlar.
## 🛠️ Özellikler
 * **Dinamik Uyarı Sistemi:** Mesafe azaldıkça buzzer sesi ve titreşim motorunun şiddeti otomatik olarak artar.
 * **Veri Yumuşatma Algoritması:** "Moving Average Filter" (Hareketli Ortalama Filtresi) sayesinde sensörden kaynaklı hatalı okumalar ve titremeler engellenir.
 * **Hassas Algılama:** 5 cm ile 100 cm arasındaki engelleri milimetrik hassasiyetle takip eder.
 * **Açılış Testi:** Cihaz her açıldığında donanımın çalıştığını teyit eden kısa bir sinyal verir.
## 🔌 Devre Bağlantıları
| Bileşen | Arduino Pini | Not |
|---|---|---|
| **HC-SR04 Trig** | Pin 9 | Tetikleme Sinyali |
| **HC-SR04 Echo** | Pin 10 | Yankı Dönüşü |
| **Buzzer** | Pin 11 | PWM Destekli Pin |
| **Titreşim Motoru** | Pin 6 | PWM Destekli Pin |
| **VCC / GND** | 5V / GND | Güç Hattı |
## 🚀 Kurulum ve Kullanım
 1. Arduino IDE'yi bilgisayarınıza indirin.
 2. sezgin_gozcu.ino dosyasını açın.
 3. Arduino Uno kartınızı USB ile bağlayın.
 4. **Araçlar > Kart** menüsünden "Arduino Uno"yu seçin.
 5. **Yükle** butonuna basın.
 6. Seri Port Ekranını (9600 Baud) açarak anlık mesafe verilerini takip edebilirsiniz.
## 📜 Lisans
Bu proje **GNU GPL V3** lisansı ile korunmaktadır. Bu, yazılımı özgürce kullanabileceğiniz, paylaşabileceğiniz ve değiştirebileceğiniz anlamına gelir. Detaylı bilgi için GNU Lisans Sayfası ziyaret edilebilir.
## 👥 Katkıda Bulunanlar
 * **Burak Yakub Güçer** - Yazılım ve Devre 
 * **Emir Kaan Topal** - Tasarım ve Afiş 
 * **Zeki Damak** - Devre 
## ⚠️ Önemli Notlar
> **PWM Pinleri:** Buzzer ve motorun şiddet kontrolü için mutlaka üzerinde ~ işareti bulunan (6, 9, 10, 11 gibi) pinler kullanılmalıdır.
> **Mesafe Ayarı:** Kod içerisindeki ALERT_THRESHOLD değişkenini değiştirerek uyarının kaç santimetreden başlayacağını kendinize göre özelleştirebilirsiniz.
> 
*Bu proje, erişilebilirlik teknolojilerine katkı sağlamak amacıyla geliştirilmiştir.*
