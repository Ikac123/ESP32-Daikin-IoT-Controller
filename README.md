# IoT Smart AC Controller (ESP32 & Android Integration)

## 📌 Project Overview
This project is an **IoT Home Automation** gateway designed to control legacy Air Conditioning units (Daikin models) using an **ESP32** microcontroller. The system bridges a dedicated mobile application with the AC unit via Infrared (IR) signals and a local web server.

> **Note:** The integrated Android application (UI/UX) is developed in **Serbian**, while the backend firmware and documentation are provided in English to meet international engineering standards.

## 🛠 Key Technical Features
* **IR Protocol Emulation:** Precise 128-bit signal encoding using the `IRremoteESP8266` library to manage temperature, fan speed, and modes (Powerful, Quiet, etc.).
* **NTP Time Synchronization:** Fetches real-time data from `pool.ntp.org` via UDP, enabling autonomous scheduling without an external RTC module.
* **Smart Scheduling Engine:** Uses C++ `struct` data types to handle up to 10 independent, recurring (daily/weekly) timer events.
* **Web Server Integration:** An onboard HTTP server (Port 80) that parses custom headers from the mobile app for bi-directional communication.
* **OTA (Over-The-Air) Support:** Capability for wireless firmware updates using `BaseOTA.h`.

## 📡 Hardware Configuration (Pinout)
The system is optimized for the ESP32 platform with the following GPIO mapping:

| Component | Pin | Function |
| :--- | :--- | :--- |
| **IR Receiver** | `GPIO 15` | Captures manual remote signals for state synchronization. |
| **IR Transmitter** | `GPIO 4` | High-power IR LED for command transmission. |
| **Digital Output** | `GPIO 27` | Output for system status LED or relay control. |
| **Connectivity** | `Wi-Fi` | Integrated 2.4GHz Wi-Fi for network communication. |



## 💻 Software Logic Flow
1. **Startup:** Connects to Wi-Fi, initializes sensors, and synchronizes time with the NTP server.
2. **Monitoring:** Polls **Pin 15** to detect if the physical remote was used, ensuring the app stays in sync.
3. **Parsing:** Analyzes incoming HTTP requests for commands like `onTimer` or `offTimer`.
4. **Execution:** The `mojTimer` function monitors the schedule and triggers the `ac.send()` sequence automatically when the time matches.

## 📂 Repository Contents
* `ESP32_RemoteControl.ino`: Main firmware (C++).
* `BaseOTA.h`: Wireless update configuration.
* `Kontrola_Klime_v1.0.apk`: Android application (Serbian interface).
