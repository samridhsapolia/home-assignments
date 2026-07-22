#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Ultrasonic Pins
#define TRIG_PIN 9
#define ECHO_PIN 10

// LEDs
#define RED_LED 2
#define YELLOW_LED 3
#define GREEN_LED 4

// Buzzer
#define BUZZER 6

unsigned long previousBeep = 0;
unsigned long previousSerial = 0;
bool beepState = false;

float getDistance() {

  float total = 0;

  for (int i = 0; i < 5; i++) {

    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);

    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);

    digitalWrite(TRIG_PIN, LOW);

    long duration = pulseIn(ECHO_PIN, HIGH, 30000);

    float distance = duration * 0.034 / 2.0;

    total += distance;
  }

  return total / 5.0;
}

void setup() {

  Serial.begin(9600);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  pinMode(BUZZER, OUTPUT);

  digitalWrite(RED_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(GREEN_LED, LOW);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED Failed");
    while (1);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
}

void loop() {

  float distance = getDistance();

  String zone = "SAFE";
  int interval = 0;

  digitalWrite(RED_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(GREEN_LED, LOW);

  if (distance > 60) {

    zone = "SAFE";
    digitalWrite(GREEN_LED, HIGH);
    noTone(BUZZER);

  }

  else if (distance > 30) {

    zone = "CAUTION";
    digitalWrite(YELLOW_LED, HIGH);
    interval = 800;

  }

  else if (distance > 15) {

    zone = "CLOSE";
    digitalWrite(YELLOW_LED, HIGH);
    digitalWrite(RED_LED, HIGH);
    interval = 300;

  }

  else {

    zone = "DANGER";
    digitalWrite(RED_LED, HIGH);
    tone(BUZZER, 1000);

  }

  if (interval > 0) {

    if (millis() - previousBeep >= interval) {

      previousBeep = millis();

      if (beepState) {
        noTone(BUZZER);
      } else {
        tone(BUZZER, 1000);
      }

      beepState = !beepState;
    }
  }

  // OLED
  display.clearDisplay();

  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print(distance, 1);
  display.print("cm");

  display.setTextSize(1);
  display.setCursor(0, 28);
  display.print("Zone: ");
  display.print(zone);

  // Bar Graph
  int width = map(constrain((int)distance, 0, 100), 100, 0, 0, 120);

  display.drawRect(4, 54, 120, 8, SSD1306_WHITE);
  display.fillRect(4, 54, width, 8, SSD1306_WHITE);

  display.display();

  if (millis() - previousSerial >= 500) {

    previousSerial = millis();

    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.print(" cm   Zone: ");
    Serial.println(zone);
  }
}
