#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_BMP085.h>
#include <DHT.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_BMP085 bmp;

#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#define LDR_PIN 34

const char* ssid = "Wokwi-GUEST";
const char* password = "";

WebServer server(80);

float temperature = 0;
float humidity = 0;
float pressure = 0;
float altitude = 0;
int lightLevel = 0;

String webpage() {

String page = R"rawliteral(
<!DOCTYPE html>
<html>

<head>

<meta charset="UTF-8">
<meta http-equiv="refresh" content="2">

<title>ESP32 Weather Dashboard</title>

<style>

body{

background:#0f172a;
font-family:Arial;
text-align:center;
color:white;

}

.container{

width:360px;
margin:auto;
margin-top:30px;
background:#1e293b;
padding:20px;
border-radius:15px;

}

h1{

color:#38bdf8;

}

.card{

background:#334155;
padding:12px;
margin:10px;
border-radius:10px;
font-size:22px;

}

</style>

</head>

<body>

<div class="container">

<h1>ESP32 Weather Station</h1>

<div class="card">
🌡 Temperature : %TEMP% °C
</div>

<div class="card">
💧 Humidity : %HUM% %
</div>

<div class="card">
📈 Pressure : %PRESS% hPa
</div>

<div class="card">
⛰ Altitude : %ALT% m
</div>

<div class="card">
☀ Light : %LIGHT%
</div>

</div>

</body>
</html>
)rawliteral";

page.replace("%TEMP%", String(temperature,1));
page.replace("%HUM%", String(humidity,1));
page.replace("%PRESS%", String(pressure,1));
page.replace("%ALT%", String(altitude,1));
page.replace("%LIGHT%", String(lightLevel));

return page;

}
void setup() {

  Serial.begin(115200);

  Wire.begin(21,22);

  dht.begin();

  if (!bmp.begin()) {
    Serial.println("BMP180 NOT FOUND!");
    while (1);
  }

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED FAILED!");
    while (1);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  display.setCursor(0,0);
  display.println("Connecting WiFi...");
  display.display();

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi Connected!");

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  display.clearDisplay();
  display.setCursor(0,0);
  display.println("WiFi Connected!");
  display.setCursor(0,15);
  display.print("IP:");
  display.println(WiFi.localIP());
  display.display();

  delay(3000);

  // Web page
  server.on("/", []() {
    server.send(200, "text/html", webpage());
  });

  server.begin();

  Serial.println("HTTP Server Started");

}
void loop() {

  // Read sensors
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();

  pressure = bmp.readPressure() / 100.0;
  altitude = bmp.readAltitude();

  lightLevel = analogRead(LDR_PIN);

  // Handle invalid DHT readings
  if (isnan(temperature) || isnan(humidity)) {
    temperature = 0;
    humidity = 0;
  }

  // Print to Serial Monitor
  Serial.println("-----------------------------");

  Serial.print("Temperature : ");
  Serial.print(temperature);
  Serial.println(" C");

  Serial.print("Humidity    : ");
  Serial.print(humidity);
  Serial.println(" %");

  Serial.print("Pressure    : ");
  Serial.print(pressure);
  Serial.println(" hPa");

  Serial.print("Altitude    : ");
  Serial.print(altitude);
  Serial.println(" m");

  Serial.print("Light Level : ");
  Serial.println(lightLevel);

  // OLED Display
  display.clearDisplay();

  display.setTextSize(1);

  display.setCursor(0,0);
  display.print("Temp : ");
  display.print(temperature,1);
  display.println(" C");

  display.setCursor(0,12);
  display.print("Hum  : ");
  display.print(humidity,1);
  display.println(" %");

  display.setCursor(0,24);
  display.print("Pres : ");
  display.print(pressure,1);
  display.println(" hPa");

  display.setCursor(0,36);
  display.print("Alt  : ");
  display.print(altitude,0);
  display.println(" m");

  display.setCursor(0,48);
  display.print("Light: ");
  display.println(lightLevel);

  display.display();

  // Handle Web Requests
  server.handleClient();

  delay(2000);

}
