# IoT Environmental Monitoring & Edge Alert Gateway

A production-grade, fault-tolerant IoT Weather Station engineered using the **ESP32 microcontroller**, built on an **asynchronous non-blocking architecture**, and fully integrated with an **external Time-Series Cloud Database (ThingSpeak)** via standard REST APIs.
🔗 **Live Cloud Analytics Dashboard:** [View My Real-Time ThingSpeak Charts](https://thingspeak.mathworks.com/channels/3465318)
## 🚀 Live Demonstration
*   **Virtual Hardware Workspace:** [Insert Your Wokwi Project Share Link Here]
*   **Live Cloud Analytics Dashboard:** [Insert Your Public ThingSpeak Channel Link Here]

## 🛠️ System Architecture & Engineering Features
*   **Asynchronous Cooperative Multitasking:** Replaced standard thread-blocking `delay()` operations with a firmware-level software timer architecture utilizing native `millis()` counters. This allows simultaneous sensor sampling, pixel-buffer display refreshing, and web transmission handling.
*   **Resilient Self-Healing Transceiver Link:** Built an automated connection health-monitoring daemon that intercepts network signal loss, proactively force-disconnecting and restoring localized Wi-Fi sockets without freezing the edge execution loop.
*   **Data Integrity Mapping:** Handled hardware sensor validation checks (`isnan` floating-point filtering) to prevent sending corrupt or dead data entries over the cloud channel pipeline.
*   **Localized Safety Real-Time Processing:** Embedded a hardware-interrupt simulation style conditional logic block to activate a high-luminance emergency indicator LED and fire visual OLED display alert bars immediately upon exceeding a critical temperature boundary (30°C).

## ☁️ Cloud Data Ingestion & Analytics (ThingSpeak)

The edge gateway utilizes the official **MathWorks ThingSpeak SDK** to pipe local environmental telemetry into a cloud-hosted time-series database. 

*   **Ingestion Optimization:** Data transmission is rate-limited to an asynchronous 15-second uplink window to comply with API throttling limits without interrupting local execution loops.
*   **Data Channel Payload Mapping:**
    *   **Field 1:** Ambient Temperature (°C) — Triggers localized physical alarm routines if thresholds breach 30°C.
    *   **Field 2:** Relative Humidity (%) — Continuously mapped for microclimate analysis.
*   **API Management:** Managed through strict separation of `myChannelNumber` identifiers and private alphanumeric `Write API Keys` directly within the hardware's TCP socket wrapper layer.


## 📊 Hardware Composition
*   **Processor Core:** ESP32-WROOM-32E NodeMCU development board
*   **Sensor Layer:** DHT22 High-Precision Digital Ambient Humidity & Temperature sensor
*   **Visual Interface:** 128x64 Pixel I2C SSD1306 OLED Dot-Matrix Display module
*   **Physical Indicators:** 5mm Red Indicator LED paired with a 220Ω current-limiting resistor

## 🔌 Hardware Interconnection Map (`diagram.json`)
*   **DHT22 (DATA) \rightarrow GPIO 17**
*   **OLED Display (SDA) \rightarrow GPIO 21**
*   **OLED Display (SCL) \rightarrow GPIO 22**
*   **Alert LED (Anode) \rightarrow GPIO 2**
*   **System Ground \rightarrow ESP32 Shared GND Grid**

## 💻 Firmware Installation Requirements
To compile the `sketch.ino` layout file, you must include the following library dependencies inside your development environment:
1. `ThingSpeak` by MathWorks (Official Cloud Middleware Wrapper)
2. `DHT sensor library` by Adafruit 
3. `Adafruit SSD1306` & `Adafruit GFX Library`
