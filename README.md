# Proyek Sistem Keamanan Rumah Multi-Sensor dengan ESP32

# Multi-Sensor Home Security System with ESP32

A smart home security system based on the ESP32, designed to detect various potential hazards in real-time. This project integrates multiple sensors to monitor environmental conditions and provide early warnings through a dynamic audio alarm.

## 📜 Status Proyek

## 📜 Project Status

**Status:** `Functional Prototype`

The system currently successfully detects fire, LPG gas, and smoke, and is capable of activating an audio alarm corresponding to the type of threat. Future development will focus on adding connectivity features and other sensors.

## Fitur

## Features

- **Flame Detection:** Uses an infrared (IR) sensor for rapid detection of flames.
- **LPG Leak Detection:** Utilizes an MQ-6 gas sensor optimized for detecting Propane and Butane.
- **Smoke Detection:** Employs an MQ-2 sensor to detect smoke from potential fires.
- **Smart Audio Alarm:** The speaker emits different tones for each type of hazard, allowing for quick threat identification.
- **Real-time Monitoring:** All sensor statuses can be monitored live via the Serial Monitor for debugging and calibration.
- **State Machine Logic:** Prevents the alarm from running inefficiently by only activating or deactivating it when the status changes.

## ⚙️ Hardware Requirements

- 1x ESP32 DevKit V4 (atau yang kompatibel)
- 1x Sensor Api (Flame Sensor Module, Digital Output)
- 1x Sensor Gas MQ-6 (LPG)
- 1x Sensor Gas MQ-2 (Asap)
- 1x Speaker Pasif (Passive Speaker)
- 1x Breadboard
- Kabel Jumper

## 🔌 Wiring Diagram

Pastikan semua komponen terhubung ke ESP32 sesuai dengan tabel berikut:

| Komponen | Pin Komponen | Terhubung ke Pin ESP32 | Keterangan |
| :--- | :--- | :--- | :--- |
| **Flame Sensor** | `VCC` | `3V3` | 3.3V Power Supply. |
| | `GND` | `GND` | Connected to ground. |
| | `DO` | `GPIO 27` | Digital detection signal. |
| **MQ-6 Sensor (LPG)** | `VCC` | `VIN` | **Important:** Requires 5V for the heater. |
| | `GND` | `GND` | Connected to ground. |
| | `AO` | `GPIO 34` | Analog gas concentration signal. |
| **MQ-2 Sensor (Smoke)** | `VCC` | `VIN` | Shares 5V power with MQ-6. |
| | `GND` | `GND` | Connected to ground. |
| | `AO` | `GPIO 35` | Analog smoke concentration signal. |
| **Passive Speaker** | `Positive (+)` | `GPIO 25` | Tone signal from ESP32. |
| | `Negative (-)` | `GND` | Connected to ground. |

## 🛠️ Software Requirements

- Visual Studio Code
- Ekstensi PlatformIO IDE untuk VSCode

## 🚀 Setup & Installation

1.  Clone repositori ini ke komputer Anda.
2.  Buka folder proyek menggunakan Visual Studio Code.
3.  PlatformIO will automatically detect the `platformio.ini` file and install the Arduino framework for ESP32.

## 🔬 Configuration & Calibration (Critical Step!)

Sensor gas MQ memerlukan kalibrasi untuk berfungsi dengan benar di lingkungan Anda dan menghindari alarm palsu.

1.  **Sensor Warm-up (Burn-in):** After assembling and uploading the code for the first time, let the device run undisturbed for **at least 15-30 minutes**. This is crucial for the heating element inside the gas sensors to reach a stable operating temperature.
2.  **Observe Clean Air Values:** Open the **Serial Monitor** in PlatformIO (ensure the baud rate is set to `115200`). Observe the `LPG (MQ-6)` and `Smoke (MQ-2)` values in clean, well-ventilated air. Record their average values after they have stabilized (e.g., around 1500).
3.  **Atur Ambang Batas:**
    - Buka file `src/main.cpp`.
    - Cari konstanta `LPG_THRESHOLD` dan `SMOKE_THRESHOLD`.
    - Change their values to a number **significantly higher** than the clean air values you recorded. As a safe starting point, add 500-700 to the normal value. For example, if the normal value is 1500, set the threshold to `2200`.
    ```cpp
    const int LPG_THRESHOLD   = 2200; // Adjust this value!
    const int SMOKE_THRESHOLD = 2200; // Adjust this value!
    ```
4.  **Re-upload:** Save the changes and upload the code to your ESP32 again.

## 🕹️ How to Use

1.  Pastikan semua komponen terhubung dengan benar sesuai diagram.
2.  Hubungkan ESP32 ke komputer melalui kabel USB.
3.  In VSCode, use PlatformIO to **Build** and **Upload** the project.
4.  After the upload is complete, open the **Serial Monitor** to monitor the sensor status.
5.  The system is now active. Try bringing a flame source, gas (from an unlit lighter), or smoke near the respective sensors to test each alarm.

## 💡 Potential Enhancements

- **IoT Connectivity:** Add Wi-Fi connectivity to send alarm notifications to a mobile phone via Telegram, Pushover, or MQTT.
- **Intermittent Alarm:** Create more dynamic alarm patterns (e.g., beeping sounds) using `millis()` to avoid blocking delays.
- **Local Display:** Integrate an OLED or LCD screen to display sensor status directly on the device.
- **Water Flow Sensor:** Add a water flow sensor to detect water usage, as per the original project plan.
- **Data Logging:** Save sensor data to an SD card or SPIFFS for historical analysis.

## 📄 License

This project is licensed under the MIT License.

---
*Built with a spirit of exploration and safety.*