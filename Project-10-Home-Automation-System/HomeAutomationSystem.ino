#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include <WebServer.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// ---------------- USER CONFIG ----------------
const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

const char* MQTT_BROKER   = "broker.hivemq.com";
const int   MQTT_PORT     = 1883;
const char* MQTT_TOPIC    = "iitjammu/yourname/home";   // <-- change [yourname]
const char* MQTT_CLIENT_ID = "esp32-home-hub-01";        // keep unique on shared broker

// ---------------- PIN CONFIG ----------------
#define DHTPIN      4
#define DHTTYPE     DHT11

#define MQ2_PIN     35
#define PIR_PIN     13
#define LDR_PIN     34

#define RELAY_FAN   26
#define RELAY_LIGHT 27

#define BUZZER_PIN  14
#define RED_LED     25
#define GREEN_LED   2

#define BTN_FAN     0
#define BTN_LIGHT   32

// Active-LOW relay module
#define RELAY_ON  LOW
#define RELAY_OFF HIGH

// ---------------- OLED CONFIG ----------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ---------------- GLOBAL OBJECTS ----------------
DHT dht(DHTPIN, DHTTYPE);
WebServer server(80);
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// ---------------- SENSOR STATE ----------------
float temperature = 0.0;
float humidity     = 0.0;
int   gasRaw       = 0;
float gasPercent    = 0.0;
bool  pirMotion     = false;
int   ldrRaw        = 0;
float ldrPercent    = 0.0;   // higher % = brighter (adjust mapping to your LDR wiring)

// ---------------- AUTOMATION STATE ----------------
bool fanOn        = false;
bool lightOn      = false;
bool gasAlert     = false;

bool fanManual        = false;
bool lightManual      = false;
unsigned long fanManualStart   = 0;
unsigned long lightManualStart = 0;
const unsigned long MANUAL_OVERRIDE_MS = 10UL * 60UL * 1000UL; // 10 minutes

unsigned long lastMotionTime = 0;
const unsigned long NO_MOTION_TIMEOUT_MS = 3UL * 60UL * 1000UL; // 3 minutes

// ---------------- TIMERS ----------------
unsigned long lastSensorRead   = 0;
const unsigned long SENSOR_INTERVAL_MS = 5000;

unsigned long lastMQTTPublish  = 0;
const unsigned long MQTT_INTERVAL_MS = 30000;

unsigned long lastOLEDSwitch   = 0;
const unsigned long OLED_INTERVAL_MS = 5000;
int oledScreen = 0;

// button debounce
unsigned long lastFanBtnTime   = 0;
unsigned long lastLightBtnTime = 0;
const unsigned long DEBOUNCE_MS = 250;

// ============================================================
//                      HELPER FUNCTIONS
// ============================================================

String uptimeString() {
  unsigned long ms = millis();
  unsigned long seconds = ms / 1000;
  unsigned long minutes = seconds / 60;
  unsigned long hours   = minutes / 60;
  unsigned long days    = hours / 24;

  hours   %= 24;
  minutes %= 60;

  char buf[32];
  snprintf(buf, sizeof(buf), "%lud:%luh:%lum", days, hours, minutes);
  return String(buf);
}

void setRelay(int pin, bool on) {
  digitalWrite(pin, on ? RELAY_ON : RELAY_OFF);
}

// ============================================================
//                      SENSOR READING
// ============================================================

void readSensors() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  if (!isnan(t)) temperature = t;
  if (!isnan(h)) humidity = h;

  gasRaw = analogRead(MQ2_PIN);          // 0-4095 on ESP32 ADC
  gasPercent = (gasRaw / 4095.0) * 100.0;

  ldrRaw = analogRead(LDR_PIN);
  ldrPercent = (ldrRaw / 4095.0) * 100.0; // calibrate mapping to your divider orientation

  bool motionNow = digitalRead(PIR_PIN) == HIGH;
  pirMotion = motionNow;
  if (motionNow) lastMotionTime = millis();
}

// ============================================================
//                      BUTTON HANDLING
// ============================================================

void handleButtons() {
  unsigned long now = millis();

  if (digitalRead(BTN_FAN) == LOW && (now - lastFanBtnTime > DEBOUNCE_MS)) {
    lastFanBtnTime = now;
    fanManual = true;
    fanManualStart = now;
    fanOn = !fanOn;
    setRelay(RELAY_FAN, fanOn);
  }

  if (digitalRead(BTN_LIGHT) == LOW && (now - lastLightBtnTime > DEBOUNCE_MS)) {
    lastLightBtnTime = now;
    lightManual = true;
    lightManualStart = now;
    lightOn = !lightOn;
    setRelay(RELAY_LIGHT, lightOn);
  }

  // expire manual overrides after 10 minutes -> hand control back to automation
  if (fanManual && (now - fanManualStart > MANUAL_OVERRIDE_MS)) {
    fanManual = false;
  }
  if (lightManual && (now - lightManualStart > MANUAL_OVERRIDE_MS)) {
    lightManual = false;
  }
}

// ============================================================
//                      AUTOMATION RULES
// ============================================================

void applyAutomationRules() {
  unsigned long now = millis();

  // ---- Rule 3: Gas Alert (highest priority, safety shutdown) ----
  if (gasPercent > 60.0) {
    gasAlert = true;
    digitalWrite(BUZZER_PIN, HIGH);
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);

    // force all relays OFF regardless of manual override
    fanOn = false;
    lightOn = false;
    setRelay(RELAY_FAN, false);
    setRelay(RELAY_LIGHT, false);

    return; // skip normal automation while alert is active
  } else {
    gasAlert = false;
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, HIGH);
  }

  // ---- Rule 1: Fan (hysteresis), skipped if manual override active ----
  if (!fanManual) {
    if (!fanOn && temperature > 32.0) {
      fanOn = true;
      setRelay(RELAY_FAN, true);
    } else if (fanOn && temperature < 28.0) {
      fanOn = false;
      setRelay(RELAY_FAN, false);
    }
  }

  // ---- Rule 2: Light (LDR + PIR), skipped if manual override active ----
  if (!lightManual) {
    bool noMotionTimeout = (now - lastMotionTime) > NO_MOTION_TIMEOUT_MS;

    if (!lightOn && ldrPercent < 20.0 && pirMotion) {
      lightOn = true;
      setRelay(RELAY_LIGHT, true);
    } else if (lightOn && (ldrPercent > 60.0 || noMotionTimeout)) {
      lightOn = false;
      setRelay(RELAY_LIGHT, false);
    }
  }
}

// ============================================================
//                      MQTT
// ============================================================

void connectMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Connecting to MQTT...");
    if (mqttClient.connect(MQTT_CLIENT_ID)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" retrying in 2s");
      delay(2000);
    }
  }
}

void publishMQTT() {
  if (!mqttClient.connected()) {
    connectMQTT();
  }
  mqttClient.loop();

  StaticJsonDocument<256> doc;
  doc["temp"]        = temperature;
  doc["humidity"]    = humidity;
  doc["gas"]         = gasPercent;
  doc["pir"]         = pirMotion;
  doc["light"]       = ldrPercent;
  doc["fan"]         = fanOn;
  doc["light_relay"] = lightOn;
  doc["alert"]       = gasAlert;

  char buffer[256];
  size_t n = serializeJson(doc, buffer);
  mqttClient.publish(MQTT_TOPIC, buffer, n);

  Serial.print("Published: ");
  Serial.println(buffer);
}

// ============================================================
//                      OLED DISPLAY
// ============================================================

void updateOLED() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  if (oledScreen == 0) {
    display.println("== ENVIRONMENT ==");
    display.setTextSize(2);
    display.setCursor(0, 16);
    display.print(temperature, 1);
    display.println(" C");
    display.setCursor(0, 40);
    display.print(humidity, 1);
    display.println(" %RH");
  } else if (oledScreen == 1) {
    display.println("== GAS / MOTION ==");
    display.setTextSize(1);
    display.setCursor(0, 16);
    display.print("Gas   : "); display.print(gasPercent, 1); display.println(" %");
    display.setCursor(0, 30);
    display.print("PIR   : "); display.println(pirMotion ? "MOTION" : "clear");
    display.setCursor(0, 44);
    display.print("LDR   : "); display.print(ldrPercent, 1); display.println(" %");
  } else {
    display.println("== SYSTEM STATUS ==");
    display.setCursor(0, 16);
    display.print("Fan   : "); display.println(fanOn ? "ON" : "OFF");
    display.setCursor(0, 30);
    display.print("Light : "); display.println(lightOn ? "ON" : "OFF");
    display.setCursor(0, 44);
    display.print("Alert : "); display.println(gasAlert ? "GAS!" : "OK");
    display.setCursor(0, 56);
    display.print("Up: "); display.println(uptimeString());
  }

  display.display();
}

// ============================================================
//                      WEB DASHBOARD
// ============================================================

String buildDashboardHTML() {
  String html = "<!DOCTYPE html><html><head><meta charset='utf-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<title>Home Hub</title>";
  html += "<style>";
  html += "body{font-family:Arial, sans-serif; background:#1e1e2f; color:#eee; margin:0; padding:20px;}";
  html += "h1{color:#7fd; }";
  html += ".card{background:#2b2b40; border-radius:10px; padding:16px; margin:10px 0;}";
  html += ".row{display:flex; justify-content:space-between; margin:6px 0;}";
  html += ".alert{background:#7a1e1e;}";
  html += "button{padding:10px 16px; border:none; border-radius:6px; background:#4b7bec; color:#fff; font-size:15px; margin-right:8px;}";
  html += "button.off{background:#555;}";
  html += "</style></head><body>";
  html += "<h1>ESP32 Home Automation Hub</h1>";

  html += String("<div class='card") + (gasAlert ? " alert" : "") + "'>";
  html += "<div class='row'><span>Gas Alert</span><b>" + String(gasAlert ? "ACTIVE - SHUTDOWN" : "OK") + "</b></div>";
  html += "</div>";

  html += "<div class='card'>";
  html += "<div class='row'><span>Temperature</span><b>" + String(temperature, 1) + " &deg;C</b></div>";
  html += "<div class='row'><span>Humidity</span><b>" + String(humidity, 1) + " %</b></div>";
  html += "<div class='row'><span>Gas Level</span><b>" + String(gasPercent, 1) + " %</b></div>";
  html += "<div class='row'><span>Motion (PIR)</span><b>" + String(pirMotion ? "Detected" : "None") + "</b></div>";
  html += "<div class='row'><span>Light Level (LDR)</span><b>" + String(ldrPercent, 1) + " %</b></div>";
  html += "<div class='row'><span>Uptime</span><b>" + uptimeString() + "</b></div>";
  html += "</div>";

  html += "<div class='card'>";
  html += "<div class='row'><span>Fan Relay</span><b>" + String(fanOn ? "ON" : "OFF") + (fanManual ? " (manual)" : " (auto)") + "</b></div>";
  html += "<a href='/toggle/fan'><button class='" + String(fanOn ? "" : "off") + "'>Toggle Fan</button></a>";
  html += "<div class='row' style='margin-top:14px;'><span>Light Relay</span><b>" + String(lightOn ? "ON" : "OFF") + (lightManual ? " (manual)" : " (auto)") + "</b></div>";
  html += "<a href='/toggle/light'><button class='" + String(lightOn ? "" : "off") + "'>Toggle Light</button></a>";
  html += "</div>";

  html += "<script>setTimeout(()=>location.reload(), 5000);</script>";
  html += "</body></html>";
  return html;
}

void handleRoot() {
  server.send(200, "text/html", buildDashboardHTML());
}

void handleApiData() {
  StaticJsonDocument<256> doc;
  doc["temp"]        = temperature;
  doc["humidity"]    = humidity;
  doc["gas"]         = gasPercent;
  doc["pir"]         = pirMotion;
  doc["light"]       = ldrPercent;
  doc["fan"]         = fanOn;
  doc["light_relay"] = lightOn;
  doc["alert"]       = gasAlert;
  doc["uptime"]      = uptimeString();
  String out;
  serializeJson(doc, out);
  server.send(200, "application/json", out);
}

void handleToggleFan() {
  fanManual = true;
  fanManualStart = millis();
  fanOn = !fanOn;
  setRelay(RELAY_FAN, fanOn);
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleToggleLight() {
  lightManual = true;
  lightManualStart = millis();
  lightOn = !lightOn;
  setRelay(RELAY_LIGHT, lightOn);
  server.sendHeader("Location", "/");
  server.send(303);
}

void setupWebServer() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/data", HTTP_GET, handleApiData);
  server.on("/toggle/fan", HTTP_GET, handleToggleFan);
  server.on("/toggle/light", HTTP_GET, handleToggleLight);
  server.begin();
}

// ============================================================
//                      SETUP / LOOP
// ============================================================

void setup() {
  Serial.begin(115200);

  pinMode(PIR_PIN, INPUT);
  pinMode(RELAY_FAN, OUTPUT);
  pinMode(RELAY_LIGHT, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BTN_FAN, INPUT_PULLUP);
  pinMode(BTN_LIGHT, INPUT_PULLUP);

  setRelay(RELAY_FAN, false);
  setRelay(RELAY_LIGHT, false);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, HIGH);

  dht.begin();

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED init failed");
  } else {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 20);
    display.println("Home Hub Booting...");
    display.display();
  }

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(400);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("WiFi connected, IP: ");
  Serial.println(WiFi.localIP());

  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  connectMQTT();

  setupWebServer();

  // initial sensor read so the dashboard isn't empty on first load
  readSensors();
  applyAutomationRules();
}

void loop() {
  unsigned long now = millis();

  server.handleClient();   // must be called every loop - WebServer is synchronous, not async

  handleButtons();

  if (now - lastSensorRead > SENSOR_INTERVAL_MS) {
    readSensors();
    applyAutomationRules();
    lastSensorRead = now;
  }

  if (now - lastMQTTPublish > MQTT_INTERVAL_MS) {
    publishMQTT();
    lastMQTTPublish = now;
  } else {
    if (!mqttClient.connected()) connectMQTT();
    mqttClient.loop();
  }

  if (now - lastOLEDSwitch > OLED_INTERVAL_MS) {
    oledScreen = (oledScreen + 1) % 3;
    updateOLED();
    lastOLEDSwitch = now;
  }
}
