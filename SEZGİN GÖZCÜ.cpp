/*
 * ======================================================================================
 * PROJE ADI: Sezgin Gözcü 
 * LİSANS: GNU GPL V3
 *  LİSANS BİLGİSİ İÇİN https://www.gnu.org/licenses/gplv3-the-program.tr.html ziyaret edin)
 * KATKIDA BULUNANLAR:
 * - BURAK YAKUB GÜÇER : Yazılım ve Devre 
 * - EMİR KAAN TOPAL   : Tasarım ve Afiş 
 * - ZEKİ DAMAK        : Devre
 * 
 * AÇIKLAMA:
 * Bu kod, ultrasonik sensörden gelen verileri işleyerek nesne mesafesine göre
 * buzzer ve titreşim motorunun şiddetini PWM (Sinyal Genişlik Modülasyonu) ile
 * dinamik olarak değiştirir. Ayrıca hatalı okumaları önlemek için veri yumuşatma
 * algoritması içerir.
 * ======================================================================================
 */

// --- PIN TANIMLAMALARI ---
const int trigPin   = 9;  // Ultrasonik tetikleme pini
const int echoPin   = 10; // Ultrasonik yankı pini
const int buzzerPin = 11; // Buzzer (PWM destekli olmalı)
const int motorPin  = 6;  // Titreşim Motoru (PWM destekli olmalı)

// --- PARAMETRELER VE SABİTLER ---
const int MAX_DISTANCE = 100; // Maksimum algılama mesafesi (cm)
const int MIN_DISTANCE = 5;   // Minimum güvenli mesafe (cm)
const int ALERT_THRESHOLD = 40; // Uyarı vermeye başlama sınırı (cm)

// --- DEĞİŞKENLER ---
long  duration;
int   distance;
int   smoothDistance;
int   feedbackIntensity;

// Filtreleme için dizi (Daha kararlı sonuçlar için)
const int numReadings = 5;
int readings[numReadings];      
int readIndex = 0;              
int total = 0;                  
int average = 0;                

// --- FONKSİYON PROTOTİPLERİ ---
void getSensorData();
void processFeedback(int dist);
void printDebugInfo();
int smoothData(int newReading);

// ======================================================================================
// SETUP: SİSTEM BAŞLATMA
// ======================================================================================
void setup() {
    // Pin modlarını ayarla
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
    pinMode(buzzerPin, OUTPUT);
    pinMode(motorPin, OUTPUT);

    // Seri haberleşmeyi başlat
    Serial.begin(9600);
    
    // Açılış testi (Sistemin çalıştığını belirtmek için kısa bir bip)
    digitalWrite(buzzerPin, HIGH);
    digitalWrite(motorPin, HIGH);
    delay(200);
    digitalWrite(buzzerPin, LOW);
    digitalWrite(motorPin, LOW);
    
    // Filtre dizisini sıfırla
    for (int i = 0; i < numReadings; i++) {
        readings[i] = 0;
    }

    Serial.println("========================================");
    Serial.println("   AKILLI BASTON SISTEMI AKTIF EDILDI   ");
    Serial.println("========================================");
}

// ======================================================================================
// MAIN LOOP: ANA DÖNGÜ
// ======================================================================================
void loop() {
    // 1. Sensörden ham veriyi al
    getSensorData();

    // 2. Veriyi filtrele (Dalgalanmaları önlemek için)
    smoothDistance = smoothData(distance);

    // 3. Mesafeye göre geri bildirimi (Buzzer/Motor) hesapla ve uygula
    processFeedback(smoothDistance);

    // 4. Bilgisayara veri gönder (Hata ayıklama için)
    printDebugInfo();

    // Sistemin kararlılığı için kısa bir bekleme
    delay(30); 
}

// ======================================================================================
// FONKSİYON: SENSÖR VERİ OKUMA
// ======================================================================================
void getSensorData() {
    // Tetikleme pinini temizle
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);

    // 10 mikrosaniye boyunca tetikle
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    // Yankı süresini oku
    duration = pulseIn(echoPin, HIGH);

    // Mesafeyi cm cinsinden hesapla
    // Ses hızı: 340 m/s -> 0.034 cm/us
    distance = duration * 0.034 / 2;

    // Hatalı veya aşırı uzak okumaları sınırla
    if (distance > MAX_DISTANCE || distance <= 0) {
        distance = MAX_DISTANCE;
    }
}

// ======================================================================================
// FONKSİYON: GERİ BİLDİRİM İŞLEME (PWM KONTROLÜ)
// ======================================================================================
void processFeedback(int dist) {
    /* 
     * Mesafe 40 cm'den küçükse uyarı başlar.
     * Yaklaştıkça (mesafe azaldıkça) şiddet (PWM) artar.
     */
    if (dist <= ALERT_THRESHOLD) {
        
        // Mesafe 40 ile 5 arasındaysa şiddeti 100 ile 255 arasında ölçeklendir
        // dist: 40 -> intensity: 100
        // dist: 5  -> intensity: 255
        feedbackIntensity = map(dist, MIN_DISTANCE, ALERT_THRESHOLD, 255, 100);
        
        // Değerin PWM sınırları dışına çıkmadığından emin ol
        feedbackIntensity = constrain(feedbackIntensity, 0, 255);

        // Donanıma aktar
        analogWrite(buzzerPin, feedbackIntensity);
        analogWrite(motorPin, feedbackIntensity);
        
    } else {
        // Güvenli mesafedeyse her şeyi kapat
        analogWrite(buzzerPin, 0);
        analogWrite(motorPin, 0);
        feedbackIntensity = 0;
    }
}

// ======================================================================================
// FONKSİYON: VERİ YUMUŞATMA (MOVING AVERAGE FILTER)
// ======================================================================================
int smoothData(int newReading) {
    // Eski toplamdan en eski veriyi çıkar
    total = total - readings[readIndex];
    // Yeni veriyi oku
    readings[readIndex] = newReading;
    // Toplama ekle
    total = total + readings[readIndex];
    // İndeksi ilerlet
    readIndex = readIndex + 1;

    // Dizinin sonuna geldiysek başa dön
    if (readIndex >= numReadings) {
        readIndex = 0;
    }

    // Ortalamayı hesapla
    average = total / numReadings;
    return average;
}

// ======================================================================================
// FONKSİYON: SERİ PORT ÇIKTISI
// ======================================================================================
void printDebugInfo() {
    Serial.print("Ham Mesafe: ");
    Serial.print(distance);
    Serial.print(" cm | ");
    
    Serial.print("Filtrelenmiş: ");
    Serial.print(smoothDistance);
    Serial.print(" cm | ");
    
    Serial.print("Uyarı Şiddeti: %");
    Serial.println(map(feedbackIntensity, 0, 255, 0, 100));
}

/*
 * NOTLAR:
 * 1. Motor Pin (6) ve Buzzer Pin (11) Arduino Uno üzerinde ~ (Tilde) işaretli olmalıdır (PWM).
 * 2. Eğer buzzer pasif (passive) değilse analogWrite ile ton kontrolü yapılamaz, 
 *    bu durumda 'tone()' fonksiyonuna geçiş yapılmalıdır.
 * 3. Mesafe sınırları kullanıcının yürüyüş hızına göre ALERT_THRESHOLD kısmından ayarlanabilir.
 * 
 */