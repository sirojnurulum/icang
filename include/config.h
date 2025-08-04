#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

//===================================================================
// MODUL: SISTEM KEAMANAN DAPUR (Kitchen Safety System)
//===================================================================

// --- Pin Definitions ---
const int FLAME_SENSOR_PIN = 27; // Digital input dari sensor api
const int LPG_SENSOR_PIN = 34;   // Analog input (AO) dari MQ-6
const int SMOKE_SENSOR_PIN = 35; // Analog input (AO) dari MQ-2
const int ACTIVE_BUZZER_PIN = 14; // Pin untuk Active Buzzer 12V (Alarm Bahaya)

// --- Alarm Thresholds ---
const int LPG_THRESHOLD = 3500;
const int SMOKE_THRESHOLD = 3000;

// --- Kitchen Alarm Beep Pattern ---
// Mengatur durasi nyala dan mati untuk alarm bahaya dapur (dalam milidetik)
const unsigned long KITCHEN_ALARM_ON_MS = 500; // 0.5 detik menyala
const unsigned long KITCHEN_ALARM_OFF_MS = 2000; // 2 detik mati

// --- Sensor Warm-up ---
// Waktu dalam milidetik untuk sensor gas stabil setelah dinyalakan
const unsigned long SENSOR_WARMUP_TIME_MS = 30000; // 30 detik

//===================================================================
// MODUL: SISTEM KONTROL POMPA (Pump Control System)
//===================================================================

// --- Pin Definitions ---
const int AC_VOLTAGE_SENSOR_PIN = 32;     // Input digital dari sensor deteksi tegangan AC (permintaan tandon)
const int WATER_PRESENCE_SENSOR_PIN = 13; // Input dari Capacitive Liquid Level Sensor (NPN)
const int FLOW_SENSOR_PIN = 12;           // Input pulsa dari water flow sensor
const int PUMP_RELAY_PIN = 26;            // Output untuk mengontrol relay pompa
const int SPEAKER_PIN = 25;               // Pin untuk Speaker Pasif (Alarm Pompa)

// --- Pump Control Configuration ---
const unsigned long STARTUP_DELAY_MS = 3000;           // Waktu stabilisasi saat startup
const unsigned long PUMP_FLOW_TEST_DURATION_MS = 8000; // Durasi verifikasi aliran awal
const unsigned long PUMP_LOCKOUT_DURATION_MS = 900000; // 15 menit lockout jika gagal
const unsigned long MAX_NO_FLOW_DURATION_MS = 10000;   // Toleransi 10 detik tanpa aliran saat RUNNING
const int FLOW_PULSE_THRESHOLD = 10;                   // Minimal pulsa untuk dianggap ada aliran
const float FLOW_CALIBRATION_FACTOR = 450.0;           // Pulsa per liter untuk YF-S201

// --- Optional Features ---
const bool ENABLE_WATER_PRESENCE_CHECK = true; // Set 'false' untuk menonaktifkan sensor keberadaan air

// --- Debounce Configuration ---
const unsigned long DEBOUNCE_DELAY_MS = 50; // 50ms untuk debounce input

// --- Intermittent Alarm Configuration ---
const int PUMP_ALARM_TONE = 750;                 // Nada untuk masalah pompa
const unsigned long BEEP_ON_DURATION_MS = 200;   // Durasi bunyi beep
const unsigned long BEEP_OFF_DURATION_MS = 1800; // Durasi diam antar beep

#endif // CONFIG_H