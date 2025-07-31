#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// --- Pin Definitions ---
// Hazard Sensors (Input)
const int FLAME_SENSOR_PIN = 27; // Digital input dari sensor api
const int LPG_SENSOR_PIN   = 34; // Analog input (AO) dari MQ-6
const int SMOKE_SENSOR_PIN = 35; // Analog input (AO) dari MQ-2

// Pump Control System (Input/Output)
const int AC_VOLTAGE_SENSOR_PIN   = 32; // Input digital dari sensor deteksi tegangan AC
const int WATER_PRESENCE_SENSOR_PIN = 13; // Input dari Capacitive Liquid Level Sensor (NPN)
const int FLOW_SENSOR_PIN         = 12; // Input pulsa dari water flow sensor
const int PUMP_RELAY_PIN        = 26; // Output untuk mengontrol relay pompa

// Alarm System (Output)
const int SPEAKER_PIN = 25;      // Pin untuk Speaker Pasif

// --- Alarm Thresholds ---
// CATATAN: Nilai ini HARUS dikalibrasi sesuai lingkungan Anda!
// Nilai yang lebih tinggi berarti konsentrasi gas/asap lebih banyak.
const int LPG_THRESHOLD   = 2200; // Nilai baru setelah kalibrasi (Normal Anda ~1500)
const int SMOKE_THRESHOLD = 2200; // Nilai baru setelah kalibrasi (Normal Anda ~1500)

// --- Pump Control Configuration ---
const unsigned long PUMP_FLOW_TEST_DURATION_MS = 8000;       // 8 detik untuk verifikasi aliran
const unsigned long PUMP_LOCKOUT_DURATION_MS   = 900000;     // 15 menit jika gagal
const int FLOW_PULSE_THRESHOLD               = 10;         // Minimal 10 pulsa untuk dianggap ada aliran
const float FLOW_CALIBRATION_FACTOR          = 450.0;      // Pulsa per liter untuk YF-S201

// --- WiFi & Telegram Configuration ---
// Ganti dengan kredensial WiFi Anda
const char* WIFI_SSID = "NAMA_WIFI_ANDA";
const char* WIFI_PASSWORD = "PASSWORD_WIFI_ANDA";
const char* TELEGRAM_BOT_TOKEN = "TOKEN_BOT_TELEGRAM_ANDA";
const char* TELEGRAM_CHAT_ID = "CHAT_ID_GRUP_ANDA"; // Ganti dengan ID grup (diawali dengan tanda minus '-')

// --- Intermittent Alarm Configuration ---
const int PUMP_ALARM_TONE = 750; // Nada khas untuk masalah pompa
const unsigned long BEEP_ON_DURATION_MS = 200;  // Durasi bunyi beep (ms)
const unsigned long BEEP_OFF_DURATION_MS = 1800; // Durasi diam di antara beep (ms)

// --- State Machines ---
// Hazard Alarm State
enum AlarmType {
  ALARM_OFF,
  ALARM_FIRE,
  ALARM_LPG,
  ALARM_SMOKE
};
AlarmType currentAlarm = ALARM_OFF;

// Pump Control State
enum PumpState {
  PUMP_IDLE,
  PUMP_WAITING_FOR_WATER,
  PUMP_TESTING,
  PUMP_RUNNING,
  PUMP_LOCKED_OUT
};
PumpState currentPumpState = PUMP_IDLE;

// --- Global Variables ---
volatile int flowPulseCounter = 0;   // Dihitung oleh interrupt, harus volatile
unsigned long pumpStateTimer = 0;    // Timer untuk state pompa
unsigned long flowRateTimer = 0;     // Timer untuk menghitung laju aliran
int lastSecondPulseCount = 0;        // Pulsa yang dihitung dalam 1 detik terakhir
unsigned long serialPrintTimer = 0;  // Timer untuk membatasi output serial
bool isIntermittentAlarmActive = false; // Flag untuk mengaktifkan alarm putus-putus
bool isBeeping = false;                 // Status saat ini dari beep (on/off)
unsigned long intermittentAlarmTimer = 0; // Timer untuk logika beep

// --- Network Objects ---
WiFiClientSecure wifiClient;
UniversalTelegramBot bot(TELEGRAM_BOT_TOKEN, wifiClient);

// --- Function Prototypes ---
void triggerAlarm(AlarmType type, int toneFrequency);
void stopAlarm();
void handleIntermittentAlarm();
void IRAM_ATTR countFlowPulse(); // ISR (Interrupt Service Routine)
void handlePumpLogic();
void sendTelegramMessage(String message);

void setup() {
  Serial.begin(115200);

  // Konfigurasi pin input
  pinMode(FLAME_SENSOR_PIN, INPUT_PULLUP);
  // Sensor deteksi tegangan AC biasanya aktif LOW saat mendeteksi tegangan
  pinMode(AC_VOLTAGE_SENSOR_PIN, INPUT_PULLUP);
  pinMode(WATER_PRESENCE_SENSOR_PIN, INPUT_PULLUP); // NPN sensor, gunakan PULLUP internal
  pinMode(FLOW_SENSOR_PIN, INPUT);                  // Tidak perlu PULLUP karena sensor aktif mengeluarkan sinyal
  // Pin analog (seperti 34 & 35) tidak memerlukan pinMode() untuk input.

  // Konfigurasi pin output
  pinMode(SPEAKER_PIN, OUTPUT);
  pinMode(PUMP_RELAY_PIN, OUTPUT);

  // Kondisi awal: pastikan pompa dan alarm mati
  digitalWrite(PUMP_RELAY_PIN, LOW); // LOW = Relay tidak aktif (pompa mati)
  noTone(SPEAKER_PIN);

  // Konfigurasi Interrupt untuk sensor aliran
  // Akan memanggil countFlowPulse() setiap kali ada sinyal NAIK (RISING) dari sensor
  attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), countFlowPulse, RISING);

  // Koneksi ke WiFi
  Serial.print("Menghubungkan ke WiFi: ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  wifiClient.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Menambahkan sertifikat root untuk koneksi aman
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nTerhubung ke WiFi!");
  Serial.print("Alamat IP: ");
  Serial.println(WiFi.localIP());

  sendTelegramMessage("✅ Sistem Keamanan Rumah telah aktif dan terhubung!");

  Serial.println("Sistem Keamanan Multi-Sensor Siap!");
  Serial.println("Memulai pemanasan sensor gas (tunggu beberapa menit untuk stabilisasi)...");
  Serial.println("Sistem Kontrol Pompa Air Cerdas Aktif.");
  Serial.println("-----------------------------------------------------------------------");
}

void loop() {
  // 1. Logika Alarm Bahaya (Prioritas Tertinggi)
  int flameState = digitalRead(FLAME_SENSOR_PIN);
  int lpgValue   = analogRead(LPG_SENSOR_PIN);
  int smokeValue = analogRead(SMOKE_SENSOR_PIN);
  if (flameState == LOW) {
    triggerAlarm(ALARM_FIRE, 1500); // Nada tinggi untuk api
  } else if (lpgValue > LPG_THRESHOLD) {
    triggerAlarm(ALARM_LPG, 1000);  // Nada sedang untuk gas
  } else if (smokeValue > SMOKE_THRESHOLD) {
    triggerAlarm(ALARM_SMOKE, 500);   // Nada rendah untuk asap
  } else {
    stopAlarm();
  }

  // 2. Jalankan logika kontrol pompa (yang akan mengatur flag alarm)
  handlePumpLogic();

  // 3. Jalankan logika suara alarm putus-putus (hanya jika tidak ada alarm bahaya)
  handleIntermittentAlarm();

  // 4. Tampilkan status ke Serial Monitor setiap 1 detik (non-blocking)
  if (millis() - serialPrintTimer >= 1000) {
    Serial.print("Api: "); Serial.print(flameState == LOW ? "TERDETEKSI" : "Aman");
    Serial.print(" | LPG (MQ-6): "); Serial.print(lpgValue);
    Serial.print(" | Asap (MQ-2): "); Serial.println(smokeValue);

    // Hitung dan tampilkan laju aliran setiap detik jika pompa berjalan
    if (currentPumpState == PUMP_RUNNING && millis() - flowRateTimer >= 1000) {
      // Ambil jumlah pulsa dalam 1 detik terakhir secara atomik
      noInterrupts();
      lastSecondPulseCount = flowPulseCounter;
      flowPulseCounter = 0;
      interrupts();
      float flowRateLPM = (lastSecondPulseCount * 60.0) / FLOW_CALIBRATION_FACTOR;
      Serial.print("[INFO] Laju Aliran Air: "); Serial.print(flowRateLPM); Serial.println(" L/min");
      flowRateTimer = millis();
    }
    serialPrintTimer = millis();
  }
}

void triggerAlarm(AlarmType type, int toneFrequency) {
  if (currentAlarm != type) {
    currentAlarm = type;
    String message = "";
    if (type == ALARM_FIRE) message = "🔥 BAHAYA: API TERDETEKSI!";
    if (type == ALARM_LPG) message = "💨 BAHAYA: KEBOCORAN GAS TERDETEKSI!";
    if (type == ALARM_SMOKE) message = "🌫️ BAHAYA: ASAP TERDETEKSI!";
    
    Serial.println("!!! " + message + " !!!");
    sendTelegramMessage("🚨 " + message);

    tone(SPEAKER_PIN, toneFrequency);
  }
}

void stopAlarm() {
  if (currentAlarm != ALARM_OFF) {
    currentAlarm = ALARM_OFF;
    Serial.println("Kondisi kembali aman.");
    sendTelegramMessage("✔️ Kondisi Bahaya Telah Kembali Aman.");
    noTone(SPEAKER_PIN);
  }
}

// Fungsi untuk mengelola alarm putus-putus
void handleIntermittentAlarm() {
  // Alarm bahaya (api/gas) lebih prioritas. Jika aktif, jangan bunyikan alarm pompa.
  if (currentAlarm != ALARM_OFF) {
    return;
  }

  // Jika flag alarm putus-putus tidak aktif, pastikan speaker mati.
  if (!isIntermittentAlarmActive) {
    if (isBeeping) {
      noTone(SPEAKER_PIN);
      isBeeping = false;
    }
    return;
  }

  // Jika kita sampai di sini, alarm putus-putus seharusnya berbunyi.
  unsigned long currentMillis = millis();

  if (isBeeping) {
    // Jika sedang berbunyi, cek apakah sudah waktunya untuk diam.
    if (currentMillis - intermittentAlarmTimer >= BEEP_ON_DURATION_MS) {
      noTone(SPEAKER_PIN);
      isBeeping = false;
      intermittentAlarmTimer = currentMillis;
    }
  } else {
    // Jika sedang diam, cek apakah sudah waktunya untuk berbunyi.
    if (currentMillis - intermittentAlarmTimer >= BEEP_OFF_DURATION_MS) {
      tone(SPEAKER_PIN, PUMP_ALARM_TONE);
      isBeeping = true;
      intermittentAlarmTimer = currentMillis;
    }
  }
}

// Interrupt Service Routine - Dijalankan setiap ada pulsa dari sensor aliran
// Harus secepat mungkin. Hanya menaikkan counter.
void IRAM_ATTR countFlowPulse() {
  flowPulseCounter++;
}

// Fungsi utama untuk logika pompa, dipanggil dari loop()
void handlePumpLogic() {
  // Baca status permintaan dari saklar tandon (melalui sensor deteksi AC)
  bool isPumpRequestActive = (digitalRead(AC_VOLTAGE_SENSOR_PIN) == LOW);
  // Baca status keberadaan air di pipa
  bool isWaterPresent = (digitalRead(WATER_PRESENCE_SENSOR_PIN) == LOW);

  // Tampilkan status pompa untuk debugging
  String pumpStatusStr;
  if (currentPumpState == PUMP_IDLE) pumpStatusStr = "IDLE";
  else if (currentPumpState == PUMP_WAITING_FOR_WATER) pumpStatusStr = "MENUNGGU AIR";
  else if (currentPumpState == PUMP_TESTING) pumpStatusStr = "TESTING";
  else if (currentPumpState == PUMP_RUNNING) pumpStatusStr = "RUNNING";
  else if (currentPumpState == PUMP_LOCKED_OUT) pumpStatusStr = "LOCKED OUT";
  Serial.print("Status Pompa: " + pumpStatusStr);
  Serial.print(" | Permintaan Tandon: ");
  Serial.print(isPumpRequestActive ? "AKTIF" : "TIDAK AKTIF");
  Serial.print(" | Air di Pipa: ");
  Serial.println(isWaterPresent ? "ADA" : "KOSONG");

  switch (currentPumpState) {
    case PUMP_IDLE:
      // Jika ada permintaan dari tandon
      if (isPumpRequestActive) {
        // Dan jika air memang tersedia di pipa (pre-check)
        if (isWaterPresent) {
          Serial.println("[POMPA] Pre-check berhasil. Memulai tes verifikasi aliran...");
          flowPulseCounter = 0; // Reset counter pulsa
          digitalWrite(PUMP_RELAY_PIN, HIGH); // Nyalakan pompa untuk tes
          pumpStateTimer = millis(); // Mulai timer tes
          currentPumpState = PUMP_TESTING;
        } else {
          Serial.println("[POMPA] Permintaan terdeteksi tapi pipa kosong. Masuk mode menunggu.");
          sendTelegramMessage("🚱 PERINGATAN: Tandon kosong tetapi tidak ada pasokan air dari sumber. Pompa tidak dinyalakan.");
          isIntermittentAlarmActive = true; // AKTIFKAN alarm putus-putus
          currentPumpState = PUMP_WAITING_FOR_WATER;
        }
      }
      break;

    case PUMP_WAITING_FOR_WATER:
      // Jika permintaan dari tandon berhenti (misal, listrik padam), kembali ke IDLE
      if (!isPumpRequestActive) {
        Serial.println("[POMPA] Permintaan dibatalkan. Kembali ke mode IDLE.");
        isIntermittentAlarmActive = false; // MATIKAN alarm putus-putus
        currentPumpState = PUMP_IDLE;
      } 
      // Jika air terdeteksi saat sedang menunggu, kembali ke IDLE untuk memulai siklus pengecekan ulang
      else if (isWaterPresent) {
        Serial.println("[POMPA] Air terdeteksi! Kembali ke mode IDLE untuk memulai siklus pengecekan.");
        isIntermittentAlarmActive = false; // MATIKAN alarm putus-putus
        currentPumpState = PUMP_IDLE;
      }
      break;

    case PUMP_TESTING:
      // Cek apakah waktu tes verifikasi aliran sudah selesai
      if (millis() - pumpStateTimer >= PUMP_FLOW_TEST_DURATION_MS) {
        Serial.print("[POMPA] Tes verifikasi selesai. Jumlah pulsa: ");
        Serial.println(flowPulseCounter);

        if (flowPulseCounter >= FLOW_PULSE_THRESHOLD) {
          Serial.println("[POMPA] Aliran terverifikasi. Melanjutkan pengisian.");
          flowRateTimer = millis(); // Mulai timer untuk kalkulasi laju aliran
          currentPumpState = PUMP_RUNNING;
        } else {
          Serial.println("[POMPA] GAGAL: Aliran tidak terverifikasi! Pompa mungkin macet. Masuk mode lockout.");
          sendTelegramMessage("❌ KRITIS: Pompa gagal memompa air setelah dinyalakan. Masuk mode penguncian selama 15 menit.");
          digitalWrite(PUMP_RELAY_PIN, LOW); // Matikan pompa
          pumpStateTimer = millis(); // Mulai timer lockout
          currentPumpState = PUMP_LOCKED_OUT;
        }
      }
      break;

    case PUMP_RUNNING:
      // Matikan pompa jika permintaan dari tandon berhenti (tandon penuh) ATAU jika air tiba-tiba hilang
      if (!isPumpRequestActive || !isWaterPresent) {
        isIntermittentAlarmActive = false; // Pastikan alarm mati
        Serial.println("[POMPA] Tandon penuh (permintaan berhenti). Mematikan pompa.");
        digitalWrite(PUMP_RELAY_PIN, LOW);
        currentPumpState = PUMP_IDLE;
      }
      break;

    case PUMP_LOCKED_OUT:
      // Cek apakah masa lockout sudah selesai
      isIntermittentAlarmActive = false; // Pastikan alarm mati di state ini
      if (millis() - pumpStateTimer >= PUMP_LOCKOUT_DURATION_MS) {
        Serial.println("[POMPA] Mode lockout selesai. Kembali ke status idle.");
        currentPumpState = PUMP_IDLE;
      }
      break;
  }
}

void sendTelegramMessage(String message) {
  if (WiFi.status() == WL_CONNECTED) {
    if (bot.sendMessage(TELEGRAM_CHAT_ID, message, "Markdown")) {
      Serial.println("Pesan Telegram terkirim: " + message);
    }
  }
}
