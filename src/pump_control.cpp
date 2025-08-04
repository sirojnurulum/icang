#include "pump_control.h"
#include "config.h"

// State machine untuk pompa
enum PumpState {
    PUMP_STARTUP,
    PUMP_IDLE,
    PUMP_WAITING_FOR_WATER,
    PUMP_TESTING,
    PUMP_RUNNING,
    PUMP_LOCKED_OUT
};

// Variabel global khusus untuk modul pompa
PumpState currentPumpState = PUMP_STARTUP;
bool pumpRequestState = false;
bool waterPresenceState = false;

// Variabel internal (private to this file)
namespace {
    // Debouncing
    int lastPumpRequestReading = HIGH;
    int lastWaterPresenceReading = LOW;
    unsigned long debounceTimer = 0;

    // Timers and Counters
    volatile int flowPulseCounter = 0;
    unsigned long pumpStateTimer = 0;
    unsigned long noFlowTimer = 0;
    unsigned long statusPrintTimer = 0;

    // Intermittent Alarm
    bool isIntermittentAlarmActive = false;
    bool isBeeping = false;
    unsigned long intermittentAlarmTimer = 0;
}

// --- Interrupt Service Routine ---
void IRAM_ATTR countFlowPulse() {
    flowPulseCounter++;
}

// --- Fungsi Internal --- 

void handleIntermittentAlarm() {
    if (!isIntermittentAlarmActive) {
        if (isBeeping) {
            noTone(SPEAKER_PIN);
            isBeeping = false;
        }
        return;
    }

    unsigned long currentMillis = millis();
    if (isBeeping) {
        if (currentMillis - intermittentAlarmTimer >= BEEP_ON_DURATION_MS) {
            noTone(SPEAKER_PIN);
            isBeeping = false;
            intermittentAlarmTimer = currentMillis;
        }
    } else {
        if (currentMillis - intermittentAlarmTimer >= BEEP_OFF_DURATION_MS) {
            tone(SPEAKER_PIN, PUMP_ALARM_TONE);
            isBeeping = true;
            intermittentAlarmTimer = currentMillis;
        }
    }
}

void printPumpStatus() {
    if (millis() - statusPrintTimer >= 2000) { // Cetak setiap 2 detik
        String pumpStatusStr;
        switch (currentPumpState) {
            case PUMP_STARTUP: pumpStatusStr = "STARTUP"; break;
            case PUMP_IDLE: pumpStatusStr = "IDLE"; break;
            case PUMP_WAITING_FOR_WATER: pumpStatusStr = "MENUNGGU AIR"; break;
            case PUMP_TESTING: pumpStatusStr = "TESTING ALIRAN"; break;
            case PUMP_RUNNING: pumpStatusStr = "RUNNING"; break;
            case PUMP_LOCKED_OUT: pumpStatusStr = "LOCKED OUT"; break;
        }

        Serial.print("Status Pompa -> ");
        Serial.print(pumpStatusStr);
        Serial.print(" | Permintaan: ");
        Serial.print(pumpRequestState ? "AKTIF" : "-");
        Serial.print(" | Air Pipa: ");
        Serial.println(waterPresenceState ? "ADA" : "-");

        if (currentPumpState == PUMP_RUNNING) {
            noInterrupts();
            int pulseCount = flowPulseCounter;
            flowPulseCounter = 0;
            interrupts();
            float flowRateLPM = (pulseCount * 30.0) / FLOW_CALIBRATION_FACTOR; // Dihitung per 2 detik
            Serial.print("[INFO] Laju Aliran Air: ");
            Serial.print(flowRateLPM);
            Serial.println(" L/min");
        }
        statusPrintTimer = millis();
    }
}

// --- Fungsi Publik --- 

void setupPumpControl() {
    // Konfigurasi pin input
    pinMode(AC_VOLTAGE_SENSOR_PIN, INPUT_PULLUP);
    pinMode(WATER_PRESENCE_SENSOR_PIN, INPUT_PULLUP);
    pinMode(FLOW_SENSOR_PIN, INPUT);

    // Konfigurasi pin output
    pinMode(PUMP_RELAY_PIN, OUTPUT);
    pinMode(SPEAKER_PIN, OUTPUT);

    // Kondisi awal
    digitalWrite(PUMP_RELAY_PIN, LOW);
    noTone(SPEAKER_PIN);

    // Konfigurasi interrupt
    attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), countFlowPulse, RISING);

    pumpStateTimer = millis();
    Serial.println("Modul Kontrol Pompa: Aktif.");
}

void loopPumpControl() {
    // 1. Debounce Inputs
    int currentPumpRequestReading = digitalRead(AC_VOLTAGE_SENSOR_PIN);
    int currentWaterPresenceReading = digitalRead(WATER_PRESENCE_SENSOR_PIN);

    if (currentPumpRequestReading != lastPumpRequestReading || currentWaterPresenceReading != lastWaterPresenceReading) {
        debounceTimer = millis();
    }

    if ((millis() - debounceTimer) > DEBOUNCE_DELAY_MS) {
        pumpRequestState = (currentPumpRequestReading == LOW);
        waterPresenceState = (currentWaterPresenceReading == HIGH);
    }

    lastPumpRequestReading = currentPumpRequestReading;
    lastWaterPresenceReading = currentWaterPresenceReading;

    // 2. State Machine Logic
    switch (currentPumpState) {
        case PUMP_STARTUP:
            if (millis() - pumpStateTimer >= STARTUP_DELAY_MS) {
                currentPumpState = PUMP_IDLE;
            }
            break;

        case PUMP_IDLE:
            if (pumpRequestState) {
                if (!ENABLE_WATER_PRESENCE_CHECK || waterPresenceState) {
                    Serial.println("[POMPA] Memulai tes aliran...");
                    flowPulseCounter = 0;
                    digitalWrite(PUMP_RELAY_PIN, HIGH);
                    pumpStateTimer = millis();
                    currentPumpState = PUMP_TESTING;
                } else {
                    Serial.println("[POMPA] Menunggu air...");
                    isIntermittentAlarmActive = true;
                    currentPumpState = PUMP_WAITING_FOR_WATER;
                }
            }
            break;

        case PUMP_WAITING_FOR_WATER:
            if (!pumpRequestState) {
                isIntermittentAlarmActive = false;
                currentPumpState = PUMP_IDLE;
            } else if (waterPresenceState) {
                isIntermittentAlarmActive = false;
                currentPumpState = PUMP_IDLE; // Kembali ke IDLE untuk memulai siklus normal
            }
            break;

        case PUMP_TESTING:
            if (millis() - pumpStateTimer >= PUMP_FLOW_TEST_DURATION_MS) {
                if (flowPulseCounter >= FLOW_PULSE_THRESHOLD) {
                    Serial.println("[POMPA] Aliran terverifikasi. Pompa berjalan.");
                    currentPumpState = PUMP_RUNNING;
                    noFlowTimer = 0; // Reset no-flow timer
                } else {
                    Serial.println("[POMPA] GAGAL: Aliran tidak terdeteksi. Masuk mode lockout.");
                    digitalWrite(PUMP_RELAY_PIN, LOW);
                    pumpStateTimer = millis();
                    currentPumpState = PUMP_LOCKED_OUT;
                }
            }
            break;

        case PUMP_RUNNING:
            if (!pumpRequestState || (ENABLE_WATER_PRESENCE_CHECK && !waterPresenceState)) {
                Serial.println("[POMPA] Permintaan berhenti atau air habis. Mematikan pompa.");
                digitalWrite(PUMP_RELAY_PIN, LOW);
                currentPumpState = PUMP_IDLE;
            }
            // Cek jika aliran berhenti saat running
            if (flowPulseCounter == 0) {
                if (noFlowTimer == 0) noFlowTimer = millis();
            } else {
                noFlowTimer = 0;
            }
            if (noFlowTimer != 0 && (millis() - noFlowTimer > MAX_NO_FLOW_DURATION_MS)) {
                 Serial.println("[POMPA] GAGAL: Aliran berhenti saat berjalan. Masuk mode lockout.");
                 digitalWrite(PUMP_RELAY_PIN, LOW);
                 pumpStateTimer = millis();
                 currentPumpState = PUMP_LOCKED_OUT;
            }
            break;

        case PUMP_LOCKED_OUT:
            isIntermittentAlarmActive = false;
            if (millis() - pumpStateTimer >= PUMP_LOCKOUT_DURATION_MS) {
                Serial.println("[POMPA] Lockout selesai. Kembali ke IDLE.");
                currentPumpState = PUMP_IDLE;
            }
            break;
    }

    // 3. Handle Alarms and Status Printing
    handleIntermittentAlarm();
    printPumpStatus();
}