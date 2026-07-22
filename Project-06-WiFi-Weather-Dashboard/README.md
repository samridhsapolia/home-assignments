# Project 06 - Wi-Fi Weather Dashboard

## Objective
Build an IoT-based weather monitoring system using ESP32 that measures environmental parameters and hosts a web dashboard over Wi-Fi.

## Components Used
- ESP32 Dev Board
- DHT22 Temperature & Humidity Sensor
- BMP180 Pressure Sensor
- LDR Module
- OLED SSD1306 Display
- Breadboard
- Jumper Wires

## Wiring Summary
- DHT22 Data → GPIO4
- BMP180 SDA → GPIO21
- BMP180 SCL → GPIO22
- OLED SDA → GPIO21
- OLED SCL → GPIO22
- LDR A0 → GPIO34
- All VCC → 3.3V
- All GND → GND

## Features
- Displays temperature and humidity
- Displays pressure and altitude
- Displays light intensity
- OLED live display
- Wi-Fi connectivity
- Embedded web dashboard
- Serial Monitor logging

## Software Used
- Arduino IDE
- Wokwi Simulator

## Expected Output
- OLED updates sensor values
- ESP32 connects to Wi-Fi
- HTTP server starts successfully
- Sensor data available through the web interface when deployed on an ESP32 connected to a Wi-Fi network

## Author
Samridh Sapolia
