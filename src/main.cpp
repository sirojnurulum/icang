#include <Arduino.h>

// --- Pin Definitions ---
// Input
const int FLAME_SENSOR_PIN = 27; // Digital input dari sensor api
const int LPG_SENSOR_PIN   = 34; // Analog input (AO) dari MQ-6
const int SMOKE_SENSOR_PIN = 35; // Analog input (AO) dari MQ-2

// Output
const int SPEAKER_PIN = 25;      // Pin untuk Speaker Pasif

// --- Alarm Thresholds ---
// CATATAN: Nilai ini HARUS dikalibrasi sesuai lingkungan Anda!
// Nilai yang lebih tinggi berarti konsentrasi gas/asap lebih banyak.
const int LPG_THRESHOLD   = 2200; // Nilai baru setelah kalibrasi (Normal Anda ~1500)
const int SMOKE_THRESHOLD = 2200; // Nilai baru setelah kalibrasi (Normal Anda ~1500)

// --- State Machine ---
// Menggunakan enum untuk mengelola jenis alarm agar kode lebih jelas
enum AlarmType {
  ALARM_OFF,
  ALARM_FIRE,
  ALARM_LPG,
  ALARM_SMOKE
};
AlarmType currentAlarm = ALARM_OFF;

// --- Function Prototypes ---
void triggerAlarm(AlarmType type, int toneFrequency);
void stopAlarm();

void setup() {
  Serial.begin(115200);

  // Konfigurasi pin input
  pinMode(FLAME_SENSOR_PIN, INPUT_PULLUP);
  // Pin analog (seperti 34 & 35) tidak memerlukan pinMode() untuk input.

  // Konfigurasi pin output
  pinMode(SPEAKER_PIN, OUTPUT);

  Serial.println("Sistem Keamanan Multi-Sensor Siap!");
  Serial.println("Memulai pemanasan sensor gas (tunggu beberapa menit untuk stabilisasi)...");
  Serial.println("-----------------------------------------------------------------------");
}

void loop() {
  // 1. Baca semua sensor
  int flameState = digitalRead(FLAME_SENSOR_PIN);
  int lpgValue   = analogRead(LPG_SENSOR_PIN);
  int smokeValue = analogRead(SMOKE_SENSOR_PIN);

  // 2. Tampilkan nilai sensor untuk monitoring dan kalibrasi
  Serial.print("Api: "); Serial.print(flameState == LOW ? "TERDETEKSI" : "Aman");
  Serial.print(" | LPG (MQ-6): "); Serial.print(lpgValue);
  Serial.print(" | Asap (MQ-2): "); Serial.println(smokeValue);

  // 3. Logika Alarm dengan Prioritas (Api > LPG > Asap)
  if (flameState == LOW) {
    triggerAlarm(ALARM_FIRE, 1500); // Nada tinggi untuk api
  } else if (lpgValue > LPG_THRESHOLD) {
    triggerAlarm(ALARM_LPG, 1000);  // Nada sedang untuk gas
  } else if (smokeValue > SMOKE_THRESHOLD) {
    triggerAlarm(ALARM_SMOKE, 500);   // Nada rendah untuk asap
  } else {
    // Jika tidak ada bahaya, hentikan alarm yang mungkin sedang aktif
    stopAlarm();
  }

  delay(1000); // Baca sensor setiap 1 detik
}

void triggerAlarm(AlarmType type, int toneFrequency) {
  if (currentAlarm != type) {
    currentAlarm = type;
    if (type == ALARM_FIRE) Serial.println("!!! BAHAYA: API TERDETEKSI !!!");
    if (type == ALARM_LPG) Serial.println("!!! BAHAYA: KEBOCORAN GAS TERDETEKSI !!!");
    if (type == ALARM_SMOKE) Serial.println("!!! BAHAYA: ASAP TERDETEKSI !!!");
    tone(SPEAKER_PIN, toneFrequency);
  }
}

void stopAlarm() {
  if (currentAlarm != ALARM_OFF) {
    currentAlarm = ALARM_OFF;
    Serial.println("Kondisi kembali aman.");
    noTone(SPEAKER_PIN);
  }
}