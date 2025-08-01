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

| Component | Component Pins | Connect to ESP32 Pins | Notes |
|:---|:---|:---|:---|
| **Flame Sensor** | `VCC`, `GND`, `DO` | `3V3`, `GND`, `GPIO 27` | |
| **MQ-6 Gas Sensor (LPG)** | `VCC`, `GND`, `AO` | `VIN`, `GND`, `GPIO 34` | |
| **MQ-2 Gas Sensor (Smoke)** | `VCC`, `GND`, `AO` | `VIN`, `GND`, `GPIO 35` | |
| **Passive Speaker** | `+`, `-` | `GPIO 25`, `GND` | |
| **AC Voltage Sensor** | `VCC`, `GND`, `Signal/OUT` | `3V3`, `GND`, `GPIO 32` | |
| **Capacitive Level Sensor** | `VCC (Brown)`, `GND (Blue)`, `Signal (Black)` | `VIN`, `GND`, `GPIO 13` | The Yellow (Mode) wire does not need to be connected. |
| **Water Flow Sensor** | `VCC (Red)`, `GND (Black)`, `Signal (Yellow)` | `VIN`, `GND`, `GPIO 12` | |
| **Relay Module** | `VCC`, `GND`, `IN` | `VIN`, `GND`, `GPIO 26` | |

#### B. High-Voltage Connections (AC 220V)

> **⚠️ CRITICAL WARNING:** This section involves high-voltage electricity which is **EXTREMELY DANGEROUS**. Mistakes can lead to serious injury or death. If you are not 100% confident, **it is highly recommended to seek help from a professional electrician.** Ensure the main power source (MCB) to the pump is completely turned off before starting.

This system intercepts the single command wire coming from the automatic float switch in your water tank.

1.  **Identify Wires:**
    - **Command Wire:** The single Live/Phase wire that comes from your tank's float switch and goes to the pump.
    - **Neutral & Ground Wires:** The wires that go directly from the power source to the pump.

2.  **Cut the Command Wire:** Cut the `Command Wire` at a convenient location. You now have two ends:
    - **End A:** The end coming **FROM** the tank's float switch.
    - **End B:** The end going **TO** the pump.

3.  **Connect End A (The Command Source):**
    - Connect `End A` to the **`COM` (Common)** terminal on the Relay Module.
    - Branch/jumper from `End A` and connect it to **one of the AC input terminals** on the AC Voltage Detection Sensor.

4.  **Complete the AC Voltage Sensor Circuit:**
    - Connect the **other AC input terminal** on the sensor to the main **Neutral Wire**. This is required for the sensor to operate.

5.  **Connect End B (To the Pump):**
    - Connect `End B` to the **`NO` (Normally Open)** terminal on the Relay Module.

This way, when the tank is empty, `End A` becomes live. The AC sensor detects this and informs the ESP32. If the system logic allows, the ESP32 activates the relay, connecting `COM` to `NO`, and sending power to the pump via `End B`.

### Step 3: Software Setup

1.  **Installation:**
    - Install **Visual Studio Code**.
    - Install the **PlatformIO IDE** extension from within VS Code.
2.  **Open Project:**
    - Clone this repository.
    - Open the project folder in VS Code (`File > Open Folder...`).
    - PlatformIO will automatically install all required dependencies.

### Step 4: Gas Sensor Calibration (Mandatory!)

Gas sensors will not be accurate without calibration.

1.  **Warm-up (Burn-in):** After assembling and uploading the code, let the device run for **15-30 minutes** for the gas sensors to stabilize.
2.  **Observe Normal Values:**
