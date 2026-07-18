#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

// Relay Pins
#define RELAY1 26
#define RELAY2 27

// Buzzer
#define BUZZER 25

// Push Buttons
#define BUTTON1 32
#define BUTTON2 33

// Bluetooth Status LED
#define BT_LED 2

bool light1 = false;
bool light2 = false;

void beep() {
  digitalWrite(BUZZER, HIGH);
  delay(200);
  digitalWrite(BUZZER, LOW);
}

void sendStatus() {
  SerialBT.print("L1:");
  SerialBT.print(light1 ? "ON" : "OFF");
  SerialBT.print(" L2:");
  SerialBT.println(light2 ? "ON" : "OFF");
}

void setup() {

  Serial.begin(115200);

  SerialBT.begin("IIT_IoT_HomeCtrl");

  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);

  pinMode(BT_LED, OUTPUT);

  // Relay OFF (Active LOW)
  digitalWrite(RELAY1, HIGH);
  digitalWrite(RELAY2, HIGH);

  digitalWrite(BUZZER, LOW);
  digitalWrite(BT_LED, LOW);

  Serial.println("Bluetooth Home Controller Ready");
}

void loop() {

  // Bluetooth Connection LED
  if (SerialBT.hasClient()) {
    digitalWrite(BT_LED, HIGH);
  } else {
    digitalWrite(BT_LED, LOW);
  }

  // Bluetooth Commands
  if (SerialBT.available()) {

    char c = SerialBT.read();

    switch (c) {

      case '1':
        light1 = true;
        digitalWrite(RELAY1, LOW);
        beep();
        sendStatus();
        break;

      case '2':
        light1 = false;
        digitalWrite(RELAY1, HIGH);
        beep();
        sendStatus();
        break;

      case '3':
        light2 = true;
        digitalWrite(RELAY2, LOW);
        beep();
        sendStatus();
        break;

      case '4':
        light2 = false;
        digitalWrite(RELAY2, HIGH);
        beep();
        sendStatus();
        break;

      case '5':
        light1 = true;
        light2 = true;
        digitalWrite(RELAY1, LOW);
        digitalWrite(RELAY2, LOW);
        beep();
        sendStatus();
        break;

      case '6':
        light1 = false;
        light2 = false;
        digitalWrite(RELAY1, HIGH);
        digitalWrite(RELAY2, HIGH);
        beep();
        sendStatus();
        break;

      case '?':
        sendStatus();
        break;
    }
  }

  // Push Button 1
  if (digitalRead(BUTTON1) == LOW) {

    light1 = !light1;

    digitalWrite(RELAY1, light1 ? LOW : HIGH);

    beep();

    sendStatus();

    while (digitalRead(BUTTON1) == LOW);
    delay(50);
  }

  // Push Button 2
  if (digitalRead(BUTTON2) == LOW) {

    light2 = !light2;

    digitalWrite(RELAY2, light2 ? LOW : HIGH);

    beep();

    sendStatus();

    while (digitalRead(BUTTON2) == LOW);
    delay(50);
  }
}
