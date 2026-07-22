#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define DHTPIN 4
#define DHTTYPE DHT22

#define RED_LED 25
#define GREEN_LED 26
#define BUZZER 27

DHT dht(DHTPIN, DHTTYPE);

unsigned long lastCSV = 0;

void setup() {
  Serial.begin(115200);

  dht.begin();

  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(BUZZER, LOW);

  Wire.begin(21, 22);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED failed");
    while (1);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
}

void loop() {

  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  if (isnan(temp) || isnan(hum)) {
    Serial.println("Failed to read DHT22");
    delay(2000);
    return;
  }

  String status;

  if (temp > 38 || hum > 80)
    status = "DANGER";
  else if (temp > 33 || hum > 70)
    status = "HOT";
  else if (temp >= 24 && temp <= 33)
    status = "COMFORT";
  else
    status = "COOL";

  display.clearDisplay();

  display.setCursor(0, 0);
  display.print("Temp: ");
  display.print(temp);
  display.println(" C");

  display.setCursor(0, 20);
  display.print("Hum : ");
  display.print(hum);
  display.println(" %");

  display.setCursor(0, 40);
  display.print("Status: ");
  display.println(status);

  display.display();

  if (temp > 38 || hum > 80) {
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);

    digitalWrite(BUZZER, HIGH);
    delay(1000);
    digitalWrite(BUZZER, LOW);
  } else {
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(BUZZER, LOW);
  }

  if (millis() - lastCSV >= 5000) {
    Serial.print(millis());
    Serial.print(",");
    Serial.print(temp);
    Serial.print(",");
    Serial.print(hum);
    Serial.print(",");
    Serial.println(status);

    lastCSV = millis();
  }

  delay(2000);
}
