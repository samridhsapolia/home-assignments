# Project 03 - Ultrasonic Parking Assistant

## Objective
Design an intelligent parking assistant using an Arduino Uno and an ultrasonic sensor to measure the distance between a vehicle and an obstacle. The system provides visual feedback on an OLED display and LED indicators along with buzzer alerts.

## Components Used
- Arduino Uno R3
- HC-SR04 Ultrasonic Sensor
- OLED SSD1306 (I2C)
- Red LED
- Yellow LED
- Green LED
- Active Buzzer
- Breadboard
- Jumper Wires

## Wiring Summary
- HC-SR04 Trig → Arduino Digital Pin
- HC-SR04 Echo → Arduino Digital Pin
- OLED SDA → A4
- OLED SCL → A5
- LEDs → Digital Pins
- Buzzer → Digital Pin

## Features
- Measures obstacle distance
- OLED displays live distance
- LED indicators for safe/caution/danger zones
- Audible buzzer warning
- Serial Monitor output

## Software Used
- Arduino IDE
- Wokwi Simulator

## Expected Output
- Distance updates continuously
- OLED shows distance
- LEDs indicate parking status
- Buzzer frequency changes with distance

## Author
Samridh Sapolia
