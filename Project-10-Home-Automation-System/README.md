# Project 10 - Home Automation System

## Objective
Develop an IoT-based home automation system using ESP32 to control household appliances through Wi-Fi. The system demonstrates remote control, automation logic, and real-time device monitoring.

## Components Used
- ESP32 Dev Board
- Relay Module
- LEDs (Appliance Indicators)
- Push Buttons / Virtual Controls
- OLED SSD1306 Display (if used)
- Breadboard
- Jumper Wires

## Wiring Summary
- Relay IN1 → ESP32 GPIO
- Relay VCC → 5V
- Relay GND → GND
- LED Anodes → ESP32 GPIO Pins (through resistors)
- OLED SDA → GPIO21 (if used)
- OLED SCL → GPIO22 (if used)

## Features
- Wi-Fi connectivity
- Remote appliance control
- LED status indication
- OLED status display (if applicable)
- Serial Monitor logging

## Software Used
- Arduino IDE
- Wokwi Simulator

## Expected Output
- ESP32 connects to Wi-Fi
- Appliances/LEDs can be controlled
- Device status updates correctly
- Serial Monitor displays system status

## Author
Samridh Sapolia
