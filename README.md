# Multi-Sensor Home Security System with ESP32

## 🎯 Project Vision

This project is an *open-source* initiative to transform a conventional house into a smart home by developing practical and useful automated modules. The goal is to create a system that is reliable, easily expandable, and adaptable to various home automation needs.

**Current Status:** `Functional Prototype`

---

## ✨ Key Features

- **Comprehensive Hazard Detection:**
  - **Fire:** Rapid detection using an infrared sensor.
  - **LPG Gas:** Gas leak detection using an MQ-6 sensor.
  - **Smoke:** Smoke detection from potential fires using an MQ-2 sensor.
- **Smart Water Pump Control:**
  - **Two-Step Verification:** Prevents the pump from running dry by first checking for **water presence** in the pipe, then verifying the **water flow** after the pump starts.
  - **Flow Monitoring:** Measures and displays the water flow rate (Liters per Minute) while the pump is operating.
- **Intelligent Alert System:**
  - **Hazard Alarms:** Distinct alarm tones for each type of hazard (fire, gas, smoke).
  - **Pump Alarm:** A unique, intermittent alarm if the water tank requests water but the main supply is unavailable.
- **Professional Code Architecture:**
  - **Non-Blocking:** The entire system runs without disruptive `delay()` calls, ensuring high responsiveness.
  - **State Machines:** Efficiently and structurally manages the status of each module (alarms, pump).

---

## 📖 Installation and Usage Guide

Follow these steps sequentially to assemble and run the system.

### Step 1: Hardware Requirements

| Qty | Component | Function |
|:---:|:---|:---|
| 1x | ESP32 DevKit V4 (or compatible) | The brain of the entire system. |
| 1x | Flame Sensor | Detects open flames. |
| 1x | MQ-6 Gas Sensor | Detects LPG gas leaks. |
| 1x | MQ-2 Gas Sensor | Detects smoke. |
| 1x | Passive Speaker | Provides audible alarms. |
| 1x | AC Voltage Detection Module | Senses requests from the tank's float switch. |
| 1x | Capacitive Level Sensor (XKC-Y25-NPN) | Checks for the presence of water in the pipe. |
| 1x | Water Flow Sensor (YF-S201) | Verifies and measures water flow. |
| 1x | 1-Channel 5V Relay Module (10A min.) | Electronic switch to control the pump. |
| 1x | Breadboard & Jumper Wires | For assembling the circuit. |

### Step 2: Assembly and Wiring

#### A. Low-Voltage Connections (ESP32 to Modules)

Connect all sensors and modules to the ESP32 according to the following table. Pay attention to the `VIN` (for 5V) and `3V3` (for 3.3V) pins.

| Component | Component Pins | Connect to ESP32 Pins |
|:---|:---|:---|
| **Flame Sensor** | `VCC`, `GND`, `DO` | `3V3`, `GND`, `GPIO 27` |
| **MQ-6 Gas Sensor (LPG)** | `VCC`, `GND`, `AO` | `VIN`, `GND`, `GPIO 34` |
| **MQ-2 Gas Sensor (Smoke)** | `VCC`, `GND`, `AO` | `VIN`, `GND`, `GPIO 35` |
| **Passive Speaker** | `+`, `-` | `GPIO 25`, `GND` |
| **AC Voltage Sensor** | `VCC`, `GND`, `Signal/OUT` | `3V3`, `GND`, `GPIO 32` |
| **Capacitive Level Sensor** | `VCC (Brown)`, `GND (Blue)`, `Signal (Black)` | `VIN`, `GND`, `GPIO 13` |
| **Water Flow Sensor** | `VCC (Red)`, `GND (Black)`, `Signal (Yellow)` | `VIN`, `GND`, `GPIO 12` |
| **Relay Module** | `VCC`, `GND`, `IN` | `VIN`, `GND`, `GPIO 26` |

#### B. High-Voltage Connections (AC 220V)

> **⚠️ CRITICAL WARNING:** This section involves high-voltage electricity which is **EXTREMELY DANGEROUS**. Mistakes can lead to serious injury or death. If you are not 100% confident, **it is highly recommended to seek help from a professional electrician.** Ensure the main power source (MCB) to the pump is completely turned off before starting.

This system intercepts the signal from your mechanical float switch.

1.  **Identify Wires:**
    - **Live & Neutral Source Wires:** From your main power supply.
    - **Request Wire:** The Live wire coming *out* of your tank's float switch.

2.  **Connect the "Request Wire" to Two Places:**
    - **To the AC Voltage Sensor:** Connect the `Request Wire` to one AC input terminal on the sensor. Connect the `Neutral Source Wire` to the other AC input terminal. This tells the ESP32 that the tank is requesting water.
    - **To the Relay Module:** Branch the `Request Wire` and connect it to the **`COM` (Common)** terminal on the relay.

3.  **Connect the Relay to the Pump:**
    - Connect the **`NO` (Normally Open)** terminal on the relay to the **Live** terminal of the Water Pump.

4.  **Connect the Pump's Neutral:**
    - Connect the `Neutral Source Wire` directly to the **Neutral** terminal of the Water Pump.

### Step 3: Telegram Bot Setup (for Notifications)

To receive real-time notifications on your phone, you need to create a Telegram Bot.

1.  **Create a New Bot:**
    - In the Telegram app, search for `BotFather` (it has a blue checkmark).
    - Start a chat and type `/newbot`.
    - Follow the instructions to give your bot a name (e.g., "Smart Home Alarm") and a username (which must end in `bot`, e.g., `MySmartHomeAlertBot`).
    - **BotFather** will give you a secret **API Token**. Copy and save this token carefully.

2.  **Get Your Chat ID:**
    - **For Personal Notifications:** Search for the `userinfobot`, start a chat, and it will immediately give you your numeric **Chat ID**.
    - **For Group Notifications:**
        1. Create a new Telegram group.
        2. Add the bot you just created as a member of the group.
        3. Add `userinfobot` to the group.
        4. Type `/my_id` in the group. `userinfobot` will reply with the group's **Chat ID**. It will be a negative number (e.g., `-1001234567890`).

3.  **Update the Code:**
    - Open the `src/main.cpp` file.
    - Find the `WiFi & Telegram Configuration` section.
    - Replace the placeholder values for `WIFI_SSID`, `WIFI_PASSWORD`, `TELEGRAM_BOT_TOKEN`, and `TELEGRAM_CHAT_ID` with your actual credentials.

### Step 4: Software Setup

1.  **Installation:**
    - Install **Visual Studio Code**.
    - Install the **PlatformIO IDE** extension from within VS Code.
2.  **Open Project:**
    - Clone this repository.
    - Open the project folder in VS Code (`File > Open Folder...`).
    - PlatformIO will automatically install all required dependencies.

### Step 5: Gas Sensor Calibration (Mandatory!)

Gas sensors will not be accurate without calibration.

1.  **Warm-up (Burn-in):** After assembling and uploading the code, let the device run for **15-30 minutes** for the gas sensors to stabilize.
2.  **Observe Normal Values:** Open the **Serial Monitor** in PlatformIO (Baud Rate: `115200`). In clean air, note the average values for `LPG (MQ-6)` and `Smoke (MQ-2)`. They might be around 1500.
3.  **Set Thresholds:** Open the `src/main.cpp` file and change the `LPG_THRESHOLD` and `SMOKE_THRESHOLD` values to be higher than the normal values (e.g., normal value + 500).
4.  **Re-upload:** Save and upload the code again.

### Step 5: Usage and Monitoring

1.  **Upload Code:** Click the **Upload (→)** icon in the PlatformIO status bar.
2.  **Monitor System:** Click the **Serial Monitor (🔌)** icon to see the real-time system log.
3.  **Understanding the Pump Logs:**
    - `IDLE`: System is normal, waiting for a request.
    - `WAITING FOR WATER`: The tank is requesting water, but the main pipe is empty. The intermittent alarm will be active.
    - `TESTING`: Water is present in the pipe; the pump is running a flow verification test.
    - `RUNNING`: The test was successful; the pump is running normally. The flow rate will be displayed.
    - `LOCKED OUT`: The test failed (pump might be jammed/flow is too weak); the pump is shut down for 15 minutes.

---

## 💡 Potential Enhancements

- **IoT Connectivity:** Add notifications to Telegram or MQTT for alarms or pump failures.
- **Local Display:** Integrate an OLED screen to display status without needing a computer.
- **Data Logging:** Store event history to an SD card or internal memory.

## 🤝 Contributing

This is an *open-source* project, and contributions are highly encouraged. Feel free to create a *pull request* or open an *issue*.

## ⚠️ Disclaimer

This software is provided 'as is'. The use, modification, or sale of any system built upon this code is entirely at the user's own risk. The contributors are not liable for any damage, loss, or misuse that may arise from the use of this system.

## 📄 License

This project is licensed under the **MIT License**.

---
*Built with a spirit of exploration and safety.*