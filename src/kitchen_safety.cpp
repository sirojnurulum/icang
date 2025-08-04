#include "kitchen_safety.h"
#include "config.h"

// Enum untuk melacak status alarm, hanya digunakan di dalam modul ini
enum KitchenAlarmState { ALARM_OFF, ALARM_FIRE, ALARM_LPG, ALARM_SMOKE };
KitchenAlarmState currentKitchenAlarm = ALARM_OFF;

// Variabel untuk pola alarm intermiten
static unsigned long kitchenAlarmTimer = 0;
static bool kitchenBuzzerState = false;

// Variabel untuk melacak status pemanasan sensor
bool isWarmingUp = true;
unsigned long startupTime = 0;

void setupKitchenSafety() {
    // Konfigurasi pin input sensor
    pinMode(FLAME_SENSOR_PIN, INPUT_PULLUP);

    // Konfigurasi pin output alarm
    pinMode(ACTIVE_BUZZER_PIN, OUTPUT);
    digitalWrite(ACTIVE_BUZZER_PIN, LOW);

    // Catat waktu startup untuk periode pemanasan
    startupTime = millis();
    Serial.println("Modul Keamanan Dapur: Aktif. Memulai pemanasan sensor gas...");
}

void loopKitchenSafety() {
    // --- Tahap Pemanasan Sensor ---
    if (isWarmingUp) {
        if (millis() - startupTime >= SENSOR_WARMUP_TIME_MS) {
            isWarmingUp = false;
            Serial.println("Pemanasan sensor gas selesai. Sistem keamanan dapur beroperasi penuh.");
        } else {
            // Selama pemanasan, hanya cetak status tanpa memicu alarm gas/asap
            static unsigned long lastLogTime = 0;
            if (millis() - lastLogTime > 2000) {
                Serial.print("Pemanasan... LPG: ");
                Serial.print(analogRead(LPG_SENSOR_PIN));
                Serial.print(" | Asap: ");
                Serial.println(analogRead(SMOKE_SENSOR_PIN));
                lastLogTime = millis();
            }
            return; // Jangan jalankan logika alarm di bawah jika masih pemanasan
        }
    }

    // --- Logika Operasional Normal (Setelah Pemanasan) ---
    
    // 1. Baca semua sensor bahaya
    int flameState = digitalRead(FLAME_SENSOR_PIN);
    int lpgValue = analogRead(LPG_SENSOR_PIN);
    int smokeValue = analogRead(SMOKE_SENSOR_PIN);

    // 2. Tentukan status alarm berdasarkan prioritas (Api > Gas > Asap)
    KitchenAlarmState detectedAlarm = ALARM_OFF;
    if (flameState == LOW) {
        detectedAlarm = ALARM_FIRE;
    } else if (lpgValue > LPG_THRESHOLD) {
        detectedAlarm = ALARM_LPG;
    } else if (smokeValue > SMOKE_THRESHOLD) {
        detectedAlarm = ALARM_SMOKE;
    }

    // 3. Kelola status alarm & mulai/hentikan alarm
    if (detectedAlarm != currentKitchenAlarm) {
        currentKitchenAlarm = detectedAlarm;

        if (currentKitchenAlarm != ALARM_OFF) {
            String message = "";
            if (currentKitchenAlarm == ALARM_FIRE) message = "🔥 BAHAYA: API TERDETEKSI!";
            if (currentKitchenAlarm == ALARM_LPG) message = "💨 BAHAYA: KEBOCORAN GAS TERDETEKSI!";
            if (currentKitchenAlarm == ALARM_SMOKE) message = "🌫️ BAHAYA: ASAP TERDETEKSI!";
            
            Serial.println("!!! " + message + " !!!");

            // Langsung nyalakan buzzer saat alarm pertama kali terdeteksi
            digitalWrite(ACTIVE_BUZZER_PIN, HIGH);
            kitchenBuzzerState = true;
            kitchenAlarmTimer = millis();
        } else {
            Serial.println("Kondisi Dapur kembali aman.");
            digitalWrite(ACTIVE_BUZZER_PIN, LOW);
            kitchenBuzzerState = false;
        }
    }

    // 4. Kelola pola alarm intermiten jika alarm sedang aktif
    if (currentKitchenAlarm != ALARM_OFF) {
        unsigned long currentTime = millis();
        if (kitchenBuzzerState && (currentTime - kitchenAlarmTimer >= KITCHEN_ALARM_ON_MS)) {
            // Waktunya mematikan buzzer
            digitalWrite(ACTIVE_BUZZER_PIN, LOW);
            kitchenBuzzerState = false;
            kitchenAlarmTimer = currentTime;
        } else if (!kitchenBuzzerState && (currentTime - kitchenAlarmTimer >= KITCHEN_ALARM_OFF_MS)) {
            // Waktunya menyalakan buzzer
            digitalWrite(ACTIVE_BUZZER_PIN, HIGH);
            kitchenBuzzerState = true;
            kitchenAlarmTimer = currentTime;
        }
    }

    // 5. Log sensor values
    static unsigned long lastLogTime = 0;
    if (millis() - lastLogTime > 2000) {
        Serial.print("Status Sensor Dapur -> ");
        Serial.print("Api: ");
        Serial.print(flameState == LOW ? "TERDETEKSI" : "Aman");
        Serial.print(" | LPG: ");
        Serial.print(lpgValue);
        Serial.print(" | Asap: ");
        Serial.println(smokeValue);
        lastLogTime = millis();
    }
}