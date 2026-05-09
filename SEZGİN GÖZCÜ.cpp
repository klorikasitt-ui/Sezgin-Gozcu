/*
 * ======================================================================================
 * PROJE ADI: SEZGİN GÖZCÜ - AKILLI REHBER VE ERİŞİLEBİLİRLİK SİSTEMİ
 * VERSİYON: 2.1 (Gelişmiş Aktif Buzzer ve PWM Motor Senkronizasyonu)
 * LİSANS: GNU GPL V3 (Genel Kamu Lisansı)
 * LİSANS BİLGİSİ: https://www.gnu.org/licenses/gplv3-the-program.tr.html
 * 
 * GELİŞTİRİCİ EKİP / KATKIDA BULUNANLAR:
 * - BURAK YAKUB GÜÇER : Yazılım ve Devre
 * - EMİR KAAN TOPAL   : Tasarım ve Afiş
 * - ZEKİ DAMAK        : Devre 
 * 
 * TEKNİK ÖZET:
 * Bu yazılım, görme engelli bireylerin mobilite kabiliyetini artırmak amacıyla 
 * tasarlanmıştır. HC-SR04 ultrasonik sensörü aracılığıyla 40kHz frekansında 
 * ses dalgaları yayarak çevredeki engelleri tespit eder. Elde edilen veriler, 
 * "Moving Average Filter" (Hareketli Ortalama Filtresi) algoritmasından geçirilerek 
 * parazitlerden arındırılır. Aktif buzzer ünitesi "Kesikli Uyarı" (Intermittent Alert) 
 * mantığıyla, titreşim motoru ise PWM şiddetiyle kullanıcıya mesafe bilgisi sağlar.
 * ======================================================================================
 */

// --- KÜTÜPHANE VE MAKRO TANIMLAMALARI ---
// Standart Arduino kütüphaneleri kullanılmaktadır.
#define SOUND_SPEED 0.0343 // Standart hava sıcaklığında ses hızı (cm/us)

// --- DONANIM PİN YAPILANDIRMASI (PİNOUT) ---
const int trigPin   = 9;   // Ultrasonik sensör tetikleme çıkışı
const int echoPin   = 10;  // Ultrasonik sensör yankı girişi
const int buzzerPin = 11;  // Aktif Buzzer (Dijital kontrol)
const int motorPin  = 6;   // Titreşim Motoru (PWM Kontrolü - 490Hz)

// --- SİSTEM PARAMETRELERİ VE SINIR DEĞERLER ---
const int MAX_DISTANCE    = 120; // Algılama menzili (cm) - Üst limit
const int MIN_DISTANCE    = 4;   // Güvenli durma mesafesi (cm) - Alt limit
const int ALERT_THRESHOLD = 50;  // Geri bildirimin başlayacağı mesafe (cm)

// --- FİLTRELEME VE VERİ İŞLEME DEĞİŞKENLERİ ---
const int numReadings = 8;       // Filtre örneği sayısı (Daha fazla örnek = Daha kararlı veri)
int readings[numReadings];       // Örneklerin tutulduğu dizi
int readIndex = 0;               // Mevcut dizin göstergesi
long total    = 0;               // Aritmetik ortalama için toplam
int averageDistance = 0;         // Filtrelenmiş nihai mesafe

// --- ZAMANLAMA VE DURUM YÖNETİMİ (NON-BLOCKING) ---
unsigned long lastBuzzerMillis = 0; // Buzzer zamanlaması için milisaniye takibi
bool buzzerState = false;           // Buzzer'ın mevcut durumu (Açık/Kapalı)
int feedbackIntensity = 0;          // Motor için hesaplanan PWM değeri

// --- FONKSİYON PROTOTİPLERİ ---
void initializeSystem();
void getUltrasonicData();
void processFeedback(int dist);
int applyMovingAverage(int newReading);
void logDiagnosticData();
void performHardwareSelfTest();

// ======================================================================================
// SETUP: SİSTEMİN İLKELENDİRİLMESİ
// ======================================================================================
void setup() {
    initializeSystem();
    performHardwareSelfTest();
    
    Serial.println(F(">>> SEZGIN GOZCU SISTEMI BASLATILDI <<<"));
    Serial.println(F("Gelismiş Filtreleme ve Aktif Buzzer Kontrolü Aktif."));
}

// ======================================================================================
// MAIN LOOP: ANA YÖNETİM DÖNGÜSÜ
// ======================================================================================
void loop() {
    /* 
     * İşlem Önceliği Sıralaması:
     * 1. Sensörden ham verinin çekilmesi (I/O İşlemi)
     * 2. Ham verinin gürültüden arındırılması (Sinyal İşleme)
     * 3. Kullanıcı geri bildirimlerinin üretilmesi (Aktüatör Kontrolü)
     * 4. Tanılama verilerinin iletilmesi (Hata Ayıklama)
     */
    
    getUltrasonicData();
    
    // Geçici değişken 'distance' üzerinden filtreleme yapılır
    extern int distance; 
    averageDistance = applyMovingAverage(distance);

    processFeedback(averageDistance);

    logDiagnosticData();

    // Not: Sistemin tepki süresini (Responsiveness) korumak için delay() kullanılmamıştır.
}

// ======================================================================================
// FONKSİYON: DONANIM VE PİN AYARLARI
// ======================================================================================
void initializeSystem() {
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
    pinMode(buzzerPin, OUTPUT);
    pinMode(motorPin, OUTPUT);

    // Seri haberleşme hızını 9600 Baud olarak ayarla
    Serial.begin(9600);
    
    // Filtre dizisini temizle
    for (int i = 0; i < numReadings; i++) {
        readings[i] = 0;
    }
}

// ======================================================================================
// FONKSİYON: DONANIM ÖZ-TESTİ (SELF-TEST)
// ======================================================================================
void performHardwareSelfTest() {
    // Açılışta kullanıcının sistemin hazır olduğunu anlaması için haptik ve sesli uyarı
    digitalWrite(buzzerPin, HIGH);
    analogWrite(motorPin, 150);
    delay(150);
    digitalWrite(buzzerPin, LOW);
    analogWrite(motorPin, 0);
    delay(100);
    digitalWrite(buzzerPin, HIGH);
    delay(150);
    digitalWrite(buzzerPin, LOW);
}

// ======================================================================================
// FONKSİYON: SENSÖR VERİ OKUMA VE HATA AYIKLAMA
// ======================================================================================
int distance; // Global mesafe değişkeni
void getUltrasonicData() {
    long duration;

    // Trig pini düşükte olduğundan emin ol (Sinyal temizliği)
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);

    // 10 mikrosaniyelik yüksek sinyal göndererek ses dalgasını başlat
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    // Yankı süresini oku (Zaman aşımı: 25ms - Yaklaşık 4 metreye tekabül eder)
    duration = pulseIn(echoPin, HIGH, 25000);

    // Mesafe hesabı: (Süre * Ses Hızı) / 2 (Gidiş-Dönüş olduğu için ikiye bölünür)
    if (duration == 0) {
        distance = MAX_DISTANCE; // Sinyal dönmezse engel yok kabul et
    } else {
        distance = duration * SOUND_SPEED / 2;
    }

    // Mantıksal sınırlandırma (Saturasyon)
    if (distance > MAX_DISTANCE) distance = MAX_DISTANCE;
    if (distance < 0) distance = 0;
}

// ======================================================================================
// FONKSİYON: HAREKETLİ ORTALAMA FİLTRESİ (MOVING AVERAGE FILTER)
// ======================================================================================
int applyMovingAverage(int newReading) {
    // En eski veriyi toplamdan çıkar
    total = total - readings[readIndex];
    // Yeni veriyi diziye ekle
    readings[readIndex] = newReading;
    // Yeni veriyi toplama ekle
    total = total + readings[readIndex];
    // Dizin ilerletme
    readIndex++;

    // Dairesel tampon (Circular Buffer) mantığı
    if (readIndex >= numReadings) {
        readIndex = 0;
    }

    // Ortalamayı döndür
    return (int)(total / numReadings);
}

// ======================================================================================
// FONKSİYON: GERİ BİLDİRİM STRATEJİSİ (AKTİF BUZZER VE PWM MOTOR)
// ======================================================================================
void processFeedback(int dist) {
    /*
     * KRİTİK MANTIK:
     * Engel 'ALERT_THRESHOLD' değerinden yakınsa sistem alarm moduna geçer.
     * Mesafe azaldıkça:
     * 1. Titreşim motoru şiddeti (PWM) doğrusal olarak artar.
     * 2. Aktif buzzer bip sıklığı (Frequency of Beeps) logaritmik olarak artar.
     */
    
    if (dist <= ALERT_THRESHOLD && dist > 0) {
        
        // 1. MOTOR KONTROLÜ (Haptik Geri Bildirim)
        // Mesafe 50cm -> PWM 100 | Mesafe 4cm -> PWM 255
        feedbackIntensity = map(dist, MIN_DISTANCE, ALERT_THRESHOLD, 255, 100);
        feedbackIntensity = constrain(feedbackIntensity, 0, 255);
        analogWrite(motorPin, feedbackIntensity);

        // 2. AKTİF BUZZER KONTROLÜ (Sesli Geri Bildirim)
        // Yaklaştıkça bekleme süresi (Delay) kısalır, ses hızlanır.
        // 50cm'de 500ms aralık | 4cm'de 40ms aralık
        int beepInterval = map(dist, MIN_DISTANCE, ALERT_THRESHOLD, 40, 500);
        
        if (millis() - lastBuzzerMillis >= beepInterval) {
            lastBuzzerMillis = millis();
            buzzerState = !buzzerState; // Durumu tersle (Toggle)
            digitalWrite(buzzerPin, buzzerState);
        }
        
    } else {
        // Güvenli bölge: Tüm aktüatörleri durdur
        digitalWrite(buzzerPin, LOW);
        analogWrite(motorPin, 0);
        feedbackIntensity = 0;
        buzzerState = false;
    }
}

// ======================================================================================
// FONKSİYON: TELEMETRİ VE TANILAMA
// ======================================================================================
void logDiagnosticData() {
    // Seri Plotter veya Seri Monitör için verileri düzenle
    Serial.print(F("Mesafe:"));
    Serial.print(averageDistance);
    Serial.print(F("cm\t"));
    
    Serial.print(F("Motor_Gucu:%"));
    Serial.print(map(feedbackIntensity, 0, 255, 0, 100));
    Serial.print(F("\t"));

    if (averageDistance <= ALERT_THRESHOLD) {
        Serial.println(F("[TEHLIKE: ENGEL TESPIT EDILDI]"));
    } else {
        Serial.println(F("[YOL ACIK]"));
    }
}
