#include <Arduino.h>
// --- Pin Definitions ---
// Hazard Sensors (Input)
const int FLAME_SENSOR_PIN = 27; // Digital input dari sensor api
const int LPG_SENSOR_PIN = 34;   // Analog input (AO) dari MQ-6
const int SMOKE_SENSOR_PIN = 35; // Analog input (AO) dari MQ-2

// Pump Control System (Input/Output)
const int AC_VOLTAGE_SENSOR_PIN = 32;     // Input digital dari sensor deteksi tegangan AC
const int WATER_PRESENCE_SENSOR_PIN = 13; // Input dari Capacitive Liquid Level Sensor (NPN)
const int FLOW_SENSOR_PIN = 12;           // Input pulsa dari water flow sensor
const int PUMP_RELAY_PIN = 26;            // Output untuk mengontrol relay pompa

// Alarm System (Output)
const int SPEAKER_PIN = 25;       // Pin untuk Speaker Pasif (Alarm Pompa)
const int ACTIVE_BUZZER_PIN = 14; // Pin untuk Active Buzzer 12V (Alarm Bahaya) via Relay/MOSFET

// --- Alarm Thresholds ---
// CATATAN: Nilai ini HARUS dikalibrasi sesuai lingkungan Anda!
// Nilai yang lebih tinggi berarti konsentrasi gas/asap lebih banyak.
const int LPG_THRESHOLD = 2200;   // Nilai baru setelah kalibrasi (Normal Anda ~1500)
const int SMOKE_THRESHOLD = 2200; // Nilai baru setelah kalibrasi (Normal Anda ~1500)

// --- Pump Control Configuration ---
const unsigned long STARTUP_DELAY_MS = 3000;           // 3 detik untuk stabilisasi saat startup
const unsigned long PUMP_FLOW_TEST_DURATION_MS = 8000; // 8 detik untuk verifikasi aliran awal
const unsigned long PUMP_LOCKOUT_DURATION_MS = 900000; // 15 menit jika gagal
const unsigned long MAX_NO_FLOW_DURATION_MS = 10000;   // Toleransi 10 detik tanpa aliran saat RUNNING
const int FLOW_PULSE_THRESHOLD = 10;                   // Minimal 10 pulsa untuk dianggap ada aliran
const float FLOW_CALIBRATION_FACTOR = 450.0;           // Pulsa per liter untuk YF-S201

// --- Optional Features ---
const bool ENABLE_WATER_PRESENCE_CHECK = true; // Set ke 'false' untuk menonaktifkan sensor keberadaan air

// --- Debounce Configuration ---
const unsigned long DEBOUNCE_DELAY_MS = 50; // 50ms untuk mengabaikan getaran sinyal

// --- Intermittent Alarm Configuration ---
const int PUMP_ALARM_TONE = 750;                 // Nada khas untuk masalah pompa
const unsigned long BEEP_ON_DURATION_MS = 200;   // Durasi bunyi beep (ms)
const unsigned long BEEP_OFF_DURATION_MS = 1800; // Durasi diam di antara beep (ms)

// --- State Machines ---
// Hazard Alarm State
enum AlarmType
{
  ALARM_OFF,
  ALARM_FIRE,
  ALARM_LPG,
  ALARM_SMOKE
};
AlarmType currentAlarm = ALARM_OFF;

// Pump Control State
enum PumpState
{
  PUMP_STARTUP,
  PUMP_IDLE,
  PUMP_WAITING_FOR_WATER,
  PUMP_TESTING,
  PUMP_RUNNING,
  PUMP_LOCKED_OUT
};
PumpState currentPumpState = PUMP_STARTUP;

// --- Global Variables ---
// Sensor States (with debounce)
bool pumpRequestState = false;      // Status permintaan tandon yang stabil
bool waterPresenceState = false;    // Status keberadaan air yang stabil
int lastPumpRequestReading = HIGH;  // Pembacaan terakhir pin permintaan
int lastWaterPresenceReading = LOW; // Pembacaan terakhir pin air
unsigned long debounceTimer = 0;    // Timer untuk debounce

// Timers and Counters
volatile int flowPulseCounter = 0;  // Dihitung oleh interrupt, harus volatile
unsigned long pumpStateTimer = 0;   // Timer untuk state pompa
unsigned long flowRateTimer = 0;    // Timer untuk menghitung laju aliran
int lastSecondPulseCount = 0;       // Pulsa yang dihitung dalam 1 detik terakhir
unsigned long noFlowTimer = 0;      // Timer untuk mendeteksi ketiadaan aliran saat RUNNING
unsigned long serialPrintTimer = 0; // Timer untuk membatasi output serial

// Alarm States
bool isIntermittentAlarmActive = false;   // Flag untuk mengaktifkan alarm putus-putus
bool isBeeping = false;                   // Status saat ini dari beep (on/off)
unsigned long intermittentAlarmTimer = 0; // Timer untuk logika beep

// --- Function Prototypes ---
void triggerAlarm(AlarmType type, int toneFrequency);
void stopAlarm();
void printSystemStatus(int flame, int lpg, int smoke);
void handleIntermittentAlarm();
void IRAM_ATTR countFlowPulse(); // ISR (Interrupt Service Routine)
void handlePumpLogic();

void setup()
{
  Serial.begin(115200);

  // Konfigurasi pin input
  pinMode(FLAME_SENSOR_PIN, INPUT_PULLUP);
  // Sensor deteksi tegangan AC biasanya aktif LOW saat mendeteksi tegangan
  pinMode(AC_VOLTAGE_SENSOR_PIN, INPUT_PULLUP);
  pinMode(WATER_PRESENCE_SENSOR_PIN, INPUT_PULLUP); // Kembalikan ke PULLUP, karena ini cara yang benar untuk NPN
  pinMode(FLOW_SENSOR_PIN, INPUT);                  // Tidak perlu PULLUP karena sensor aktif mengeluarkan sinyal
  // Pin analog (seperti 34 & 35) tidak memerlukan pinMode() untuk input.

  // Konfigurasi pin output
  pinMode(SPEAKER_PIN, OUTPUT);
  pinMode(PUMP_RELAY_PIN, OUTPUT);
  pinMode(ACTIVE_BUZZER_PIN, OUTPUT);

  // Kondisi awal: pastikan pompa dan semua alarm mati
  digitalWrite(PUMP_RELAY_PIN, LOW); // LOW = Relay tidak aktif (pompa mati)
  noTone(SPEAKER_PIN);
  digitalWrite(ACTIVE_BUZZER_PIN, LOW); // Pastikan buzzer aktif mati

  // Konfigurasi Interrupt untuk sensor aliran
  // Akan memanggil countFlowPulse() setiap kali ada sinyal NAIK (RISING) dari sensor
  attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), countFlowPulse, RISING);

  // Inisialisasi timer untuk mode startup
  pumpStateTimer = millis();

  Serial.println("Sistem Keamanan Multi-Sensor Siap!");
  Serial.println("Memulai pemanasan sensor gas (tunggu beberapa menit untuk stabilisasi)...");
  Serial.println("Sistem Kontrol Pompa Air Cerdas Aktif.");
  Serial.println("-----------------------------------------------------------------------");
}

void loop()
{
  // --- Debounce Logic for Inputs ---
  // Baca pin mentah
  int currentPumpRequestReading = digitalRead(AC_VOLTAGE_SENSOR_PIN);
  int currentWaterPresenceReading = digitalRead(WATER_PRESENCE_SENSOR_PIN);

  // Cek apakah ada perubahan pada salah satu pin
  if (currentPumpRequestReading != lastPumpRequestReading || currentWaterPresenceReading != lastWaterPresenceReading)
  {
    // Jika ada perubahan, reset debounce timer
    debounceTimer = millis();
  }

  // Jika tidak ada perubahan selama durasi debounce
  if ((millis() - debounceTimer) > DEBOUNCE_DELAY_MS)
  {
    // Update status stabil hanya jika pembacaan saat ini berbeda dari status stabil terakhir
    bool newPumpRequestState = (currentPumpRequestReading == LOW);
    if (newPumpRequestState != pumpRequestState)
    {
      pumpRequestState = newPumpRequestState;
    }

    bool newWaterPresenceState = (currentWaterPresenceReading == HIGH);
    if (newWaterPresenceState != waterPresenceState)
    {
      waterPresenceState = newWaterPresenceState;
    }
  }

  // Simpan pembacaan saat ini untuk iterasi berikutnya
  lastPumpRequestReading = currentPumpRequestReading;
  lastWaterPresenceReading = currentWaterPresenceReading;

  // 1. Logika Alarm Bahaya (Prioritas Tertinggi)
  int flameState = digitalRead(FLAME_SENSOR_PIN);
  int lpgValue = analogRead(LPG_SENSOR_PIN);
  int smokeValue = analogRead(SMOKE_SENSOR_PIN);
  if (flameState == LOW)
  {
    triggerAlarm(ALARM_FIRE, 1500); // Nada tinggi untuk api
  }
  else if (lpgValue > LPG_THRESHOLD)
  {
    triggerAlarm(ALARM_LPG, 1000); // Nada sedang untuk gas
  }
  else if (smokeValue > SMOKE_THRESHOLD)
  {
    triggerAlarm(ALARM_SMOKE, 500); // Nada rendah untuk asap
  }
  else
  {
    stopAlarm();
  }

  // 2. Jalankan logika kontrol pompa (yang akan mengatur flag alarm)
  handlePumpLogic();

  // 3. Jalankan logika suara alarm putus-putus (hanya jika tidak ada alarm bahaya)
  handleIntermittentAlarm();

  // 4. Tampilkan status ke Serial Monitor setiap 1 detik (non-blocking)
  printSystemStatus(flameState, lpgValue, smokeValue);
}

void triggerAlarm(AlarmType type, int toneFrequency)
{
  if (currentAlarm != type)
  {
    currentAlarm = type;
    String message = "";
    if (type == ALARM_FIRE)
      message = "🔥 BAHAYA: API TERDETEKSI!";
    if (type == ALARM_LPG)
      message = "💨 BAHAYA: KEBOCORAN GAS TERDETEKSI!";
    if (type == ALARM_SMOKE)
      message = "🌫️ BAHAYA: ASAP TERDETEKSI!";

    Serial.println("!!! " + message + " !!!");
    // Mengaktifkan buzzer aktif 12V untuk alarm bahaya.
    // Juga mematikan alarm pompa (speaker pasif) jika kebetulan sedang aktif.
    digitalWrite(ACTIVE_BUZZER_PIN, HIGH);
    noTone(SPEAKER_PIN);
  }
}

void stopAlarm()
{
  if (currentAlarm != ALARM_OFF)
  {
    currentAlarm = ALARM_OFF;
    Serial.println("Kondisi kembali aman.");
    // Mematikan buzzer aktif 12V
    digitalWrite(ACTIVE_BUZZER_PIN, LOW);
  }
}

// Fungsi untuk mencetak semua status ke Serial Monitor
void printSystemStatus(int flame, int lpg, int smoke)
{
  if (millis() - serialPrintTimer >= 1000)
  {
    Serial.print("Api: ");
    Serial.print(flame == LOW ? "TERDETEKSI" : "Aman");
    Serial.print(" | LPG (MQ-6): ");
    Serial.print(lpg);
    Serial.print(" | Asap (MQ-2): ");
    Serial.println(smoke);

    // Tampilkan status pompa untuk debugging
    String pumpStatusStr;
    if (currentPumpState == PUMP_STARTUP)
      pumpStatusStr = "STARTUP";
    else if (currentPumpState == PUMP_IDLE)
      pumpStatusStr = "IDLE";
    else if (currentPumpState == PUMP_WAITING_FOR_WATER)
      pumpStatusStr = "MENUNGGU AIR";
    else if (currentPumpState == PUMP_TESTING)
      pumpStatusStr = "TESTING";
    else if (currentPumpState == PUMP_RUNNING)
      pumpStatusStr = "RUNNING";
    else if (currentPumpState == PUMP_LOCKED_OUT)
      pumpStatusStr = "LOCKED OUT";
    Serial.print("Status Pompa: " + pumpStatusStr);
    Serial.print(" | Permintaan Tandon: ");
    Serial.print(pumpRequestState ? "AKTIF" : "TIDAK AKTIF");
    Serial.print(" | Air di Pipa: ");
    Serial.println(waterPresenceState ? "ADA" : "KOSONG");

    // Hitung dan tampilkan laju aliran setiap detik jika pompa berjalan
    if (currentPumpState == PUMP_RUNNING && millis() - flowRateTimer >= 1000)
    {
      // Ambil jumlah pulsa dalam 1 detik terakhir secara atomik
      noInterrupts();
      lastSecondPulseCount = flowPulseCounter;
      flowPulseCounter = 0;
      interrupts();
      float flowRateLPM = (lastSecondPulseCount * 60.0) / FLOW_CALIBRATION_FACTOR;
      Serial.print("[INFO] Laju Aliran Air: ");
      Serial.print(flowRateLPM);
      Serial.println(" L/min");
      flowRateTimer = millis();

      // Logika pengawas aliran saat pompa RUNNING
      if (flowRateLPM < 0.1)
      { // Jika aliran sangat rendah atau nol
        if (noFlowTimer == 0)
        { // Jika timer belum dimulai, mulai sekarang
          Serial.println("[PERINGATAN] Aliran berhenti saat pompa berjalan. Memulai timer pengaman...");
          noFlowTimer = millis();
        }
      }
      else
      {                  // Jika aliran normal
        noFlowTimer = 0; // Reset timer jika ada aliran
      }
    }
    Serial.println("-----------------------------------------------------------------------");
    serialPrintTimer = millis();
  }
}

// Fungsi untuk mengelola alarm putus-putus
void handleIntermittentAlarm()
{
  // Alarm bahaya (api/gas) lebih prioritas. Jika aktif, jangan bunyikan alarm pompa.
  if (currentAlarm != ALARM_OFF)
  {
    return;
  }

  // Jika flag alarm putus-putus tidak aktif, pastikan speaker mati.
  if (!isIntermittentAlarmActive)
  {
    if (isBeeping)
    {
      noTone(SPEAKER_PIN);
      isBeeping = false;
    }
    return;
  }

  // Jika kita sampai di sini, alarm putus-putus seharusnya berbunyi.
  unsigned long currentMillis = millis();

  if (isBeeping)
  {
    // Jika sedang berbunyi, cek apakah sudah waktunya untuk diam.
    if (currentMillis - intermittentAlarmTimer >= BEEP_ON_DURATION_MS)
    {
      noTone(SPEAKER_PIN);
      isBeeping = false;
      intermittentAlarmTimer = currentMillis;
    }
  }
  else
  {
    // Jika sedang diam, cek apakah sudah waktunya untuk berbunyi.
    if (currentMillis - intermittentAlarmTimer >= BEEP_OFF_DURATION_MS)
    {
      tone(SPEAKER_PIN, PUMP_ALARM_TONE);
      isBeeping = true;
      intermittentAlarmTimer = currentMillis;
    }
  }
}

// Interrupt Service Routine - Dijalankan setiap ada pulsa dari sensor aliran
// Harus secepat mungkin. Hanya menaikkan counter.
void IRAM_ATTR countFlowPulse()
{
  flowPulseCounter++;
}

// Fungsi utama untuk logika pompa, dipanggil dari loop()
void handlePumpLogic()
{
  switch (currentPumpState)
  {
  case PUMP_STARTUP:
    // Tunggu beberapa detik agar semua sensor stabil
    if (millis() - pumpStateTimer >= STARTUP_DELAY_MS)
    {
      Serial.println("[INFO] Mode startup selesai. Sistem siap beroperasi.");
      currentPumpState = PUMP_IDLE;
    }
    break;

  case PUMP_IDLE:
    // Jika ada permintaan dari tandon
    if (pumpRequestState)
    {
      // Pemeriksaan ini hanya dilakukan jika fiturnya diaktifkan
      if (!ENABLE_WATER_PRESENCE_CHECK || waterPresenceState)
      {
        noFlowTimer = 0; // Pastikan timer pengaman direset sebelum tes
        Serial.println("[POMPA] Pre-check berhasil. Memulai tes verifikasi aliran...");
        flowPulseCounter = 0;               // Reset counter pulsa
        digitalWrite(PUMP_RELAY_PIN, HIGH); // Nyalakan pompa untuk tes
        pumpStateTimer = millis();          // Mulai timer tes
        currentPumpState = PUMP_TESTING;
      }
      // Blok ini hanya akan berjalan jika pemeriksaan diaktifkan DAN air tidak ada
      else
      {
        Serial.println("[POMPA] Permintaan terdeteksi tapi pipa kosong. Masuk mode menunggu.");
        isIntermittentAlarmActive = true; // AKTIFKAN alarm putus-putus
        currentPumpState = PUMP_WAITING_FOR_WATER;
      }
    }
    break;

  case PUMP_WAITING_FOR_WATER:
    // Jika permintaan dari tandon berhenti (misal, listrik padam), kembali ke IDLE
    if (!pumpRequestState)
    {
      Serial.println("[POMPA] Permintaan dibatalkan. Kembali ke mode IDLE.");
      isIntermittentAlarmActive = false; // MATIKAN alarm putus-putus
      currentPumpState = PUMP_IDLE;
    }
    // Jika air terdeteksi saat sedang menunggu, kembali ke IDLE untuk memulai siklus pengecekan ulang yang aman
    else if (waterPresenceState)
    {
      Serial.println("[POMPA] Air terdeteksi! Kembali ke mode IDLE untuk memulai siklus pengecekan.");
      isIntermittentAlarmActive = false; // MATIKAN alarm putus-putus
      currentPumpState = PUMP_IDLE;
    }
    break;

  case PUMP_TESTING:
    // Cek apakah waktu tes verifikasi aliran sudah selesai
    if (millis() - pumpStateTimer >= PUMP_FLOW_TEST_DURATION_MS)
    {
      Serial.print("[POMPA] Tes verifikasi selesai. Jumlah pulsa: ");
      Serial.println(flowPulseCounter);

      if (flowPulseCounter >= FLOW_PULSE_THRESHOLD)
      {
        Serial.println("[POMPA] Aliran terverifikasi. Melanjutkan pengisian.");
        flowRateTimer = millis(); // Mulai timer untuk kalkulasi laju aliran
        currentPumpState = PUMP_RUNNING;
      }
      else
      {
        Serial.println("[POMPA] GAGAL: Aliran tidak terverifikasi! Pompa mungkin macet. Masuk mode lockout.");
        digitalWrite(PUMP_RELAY_PIN, LOW); // Matikan pompa
        pumpStateTimer = millis();         // Mulai timer lockout
        currentPumpState = PUMP_LOCKED_OUT;
      }
    }
    break;

  case PUMP_RUNNING:
    // Matikan pompa jika permintaan dari tandon berhenti (tandon penuh) ATAU jika air tiba-tiba hilang
    // Pemeriksaan keberadaan air hanya dilakukan jika fiturnya diaktifkan
    if (!pumpRequestState || (ENABLE_WATER_PRESENCE_CHECK && !waterPresenceState))
    {
      noFlowTimer = 0;                   // Reset timer pengaman
      isIntermittentAlarmActive = false; // Pastikan alarm mati
      Serial.println("[POMPA] Tandon penuh (permintaan berhenti) atau pasokan air hilang. Mematikan pompa.");
      digitalWrite(PUMP_RELAY_PIN, LOW);
      currentPumpState = PUMP_IDLE;
    }

    // Cek pengawas aliran. Jika tidak ada aliran terlalu lama, masuk ke lockout.
    if (noFlowTimer != 0 && (millis() - noFlowTimer > MAX_NO_FLOW_DURATION_MS))
    {
      Serial.println("[POMPA] GAGAL: Aliran berhenti total saat pompa berjalan! Masuk mode lockout.");
      digitalWrite(PUMP_RELAY_PIN, LOW); // Matikan pompa
      pumpStateTimer = millis();         // Mulai timer lockout
      currentPumpState = PUMP_LOCKED_OUT;
      noFlowTimer = 0; // Reset timer pengaman
      isIntermittentAlarmActive = false;
    }
    break;

  case PUMP_LOCKED_OUT:
    // Cek apakah masa lockout sudah selesai
    isIntermittentAlarmActive = false; // Pastikan alarm mati di state ini
    if (millis() - pumpStateTimer >= PUMP_LOCKOUT_DURATION_MS)
    {
      Serial.println("[POMPA] Mode lockout selesai. Kembali ke status idle.");
      currentPumpState = PUMP_IDLE;
    }
    break;
  }
}
