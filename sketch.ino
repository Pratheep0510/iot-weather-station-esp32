#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DHT.h"
#include "ThingSpeak.h"  // Bypasses manual raw HTTP bottlenecks

// --- Hardware Pin Definitions ---
#define DHTPIN          17      // Aligned directly with your GPIO 17 layout
#define DHTTYPE         DHT22   // Sensor type
#define ALERT_LED_PIN   2       // GPIO pin connected to Alert LED
#define TEMP_THRESHOLD  30.0    // Temperature alert threshold in Celsius

// --- OLED Display Parameters ---
#define SCREEN_WIDTH    128     
#define SCREEN_HEIGHT   64      
#define OLED_RESET      -1      
#define SCREEN_ADDRESS  0x3C    // Try 0x3D if the OLED goes blank

// --- Non-Blocking Asynchronous Intervals ---
const unsigned long SENSOR_INTERVAL = 2000;  
const unsigned long CLOUD_INTERVAL  = 15000; // ThingSpeak ingestion limit (15s)

unsigned long lastSensorRead = 0;
unsigned long lastCloudPush  = 0;

// --- IoT Cloud & Network Configuration ---
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// ⚠️ FILL IN THESE TWO EXACT FIELDS FROM YOUR THINGSPEAK CHANNEL PAGE
unsigned long myChannelNumber = 3465318; // e.g., 2413886 (No quotation marks)
const char * myWriteAPIKey   = "GQCUYRSAMAHDY4ZC"; // Keep quotation marks

// --- Core Object Instantiations ---
WiFiClient client; // Required for the ThingSpeak network transceiver stack
DHT dht(DHTPIN, DHTTYPE);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- Global Telemetry Buffers ---
float currentTemp = 0.0;
float currentHum  = 0.0;
bool isSensorValid = false;

// Gracefully heals broken Wi-Fi links without locking the microcontroller thread
void maintainWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[System] Link lost. Reconnecting to gateway...");
    WiFi.disconnect();
    WiFi.begin(ssid, password);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 10) {
      delay(500);
      Serial.print(".");
      attempts++;
    }
  }
}

// Samples the DHT22 and evaluates safety critical alert loops instantly
void readTelemetry() {
  currentHum = dht.readHumidity();
  currentTemp = dht.readTemperature();

  if (isnan(currentHum) || isnan(currentTemp)) {
    isSensorValid = false;
    Serial.println("[Error] Sensor telemetry corrupt.");
    return;
  }
  isSensorValid = true;

  // Immediate Alert System Check
  if (currentTemp > TEMP_THRESHOLD) {
    digitalWrite(ALERT_LED_PIN, HIGH);
  } else {
    digitalWrite(ALERT_LED_PIN, LOW);
  }
}

// Rewrites local hardware pixel frames to the SSD1306 OLED Screen
void updateDisplay() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  display.setCursor(0, 0);
  display.println("IoT EDGE GATEWAY");
  display.println("---------------------");

  if (!isSensorValid) {
    display.setCursor(0, 25);
    display.println("SENSOR FAULT!");
    display.display();
    return;
  }

  // Render Telemetry Grid
  display.setCursor(0, 20);
  display.print("Temp: "); display.setTextSize(2); display.print(currentTemp, 1); display.setTextSize(1); display.print(" C");
  display.setCursor(0, 40);
  display.print("Hum:  "); display.setTextSize(2); display.print(currentHum, 1); display.setTextSize(1); display.print(" %");

  // Dynamic Status Banner
  display.setCursor(0, 56);
  if (currentTemp > TEMP_THRESHOLD) {
    display.print("ALERT: OVERHEATING!");
  } else if (WiFi.status() == WL_CONNECTED) {
    display.print("Cloud Status: ONLINE");
  } else {
    display.print("Cloud Status: OFFLINE");
  }
  
  display.display();
}

// Packages and marshals payloads securely using native Library methods
void pushToCloud() {
  if (WiFi.status() != WL_CONNECTED || !isSensorValid) return;

  // Map values onto your ThingSpeak data fields
  ThingSpeak.setField(1, currentTemp);
  ThingSpeak.setField(2, currentHum);
  
  // Transmit payload out through open socket link
  int returnCode = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  
  if(returnCode == 200) {
    Serial.println("[Cloud] Channel update sync successful. Status: 200");
  } else {
    Serial.print("[Cloud] Transmission dropped. Library Return Code: ");
    Serial.println(returnCode);
  }
}

void setup() {
  Serial.begin(115200);
  delay(10);
  
  pinMode(ALERT_LED_PIN, OUTPUT);
  digitalWrite(ALERT_LED_PIN, LOW);

  dht.begin();

  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("[Fatal] OLED allocation failed.");
    for(;;); 
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Booting System...");
  display.println("Initializing Network...");
  display.display();

  WiFi.disconnect(true);
  delay(100);
  WiFi.mode(WIFI_STA);
  delay(100);

  WiFi.begin(ssid, password);
  Serial.print("[Network] Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n[Network] IP Address assigned successfully.");

  // Initialize the native ThingSpeak client middleware mapping
  ThingSpeak.begin(client); 
}

void loop() {
  unsigned long currentMillis = millis();

  maintainWiFi();

  // Thread 1: Asynchronous Sensor Sampling (Every 2 Seconds)
  if (currentMillis - lastSensorRead >= SENSOR_INTERVAL) {
    lastSensorRead = currentMillis;
    readTelemetry();
    updateDisplay();
    
    Serial.print("Local Log -> Temp: "); Serial.print(currentTemp, 1);
    Serial.print("°C | Hum: "); Serial.print(currentHum, 1); Serial.println("%");
  }

  // Thread 2: Asynchronous Cloud Uplink (Every 15 Seconds)
  if (currentMillis - lastCloudPush >= CLOUD_INTERVAL) {
    lastCloudPush = currentMillis;
    pushToCloud();
  }
}
