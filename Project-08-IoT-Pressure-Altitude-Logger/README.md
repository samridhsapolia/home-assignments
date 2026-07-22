# Project 08 - IoT Pressure & Altitude Logger

## Objective
Develop an IoT-based pressure and altitude monitoring system using an ESP32 and BMP180 sensor. The system measures atmospheric pressure and altitude, displays the readings on an OLED display, and logs the data through the Serial Monitor.

## Components Used
- ESP32 Dev Board
- BMP180 Pressure Sensor
- OLED SSD1306 Display
- Breadboard
- Jumper Wires

## Wiring Summary
- BMP180 SDA → GPIO21
- BMP180 SCL → GPIO22
- OLED SDA → GPIO21
- OLED SCL → GPIO22
- BMP180 VCC → 3.3V
- BMP180 GND → GND
- OLED VCC → 3.3V
- OLED GND → GND

## Features
- Atmospheric pressure measurement
- Altitude calculation
- OLED display output
- Serial Monitor logging
- Continuous real-time updates

## Software Used
- Arduino IDE
- Wokwi Simulator

## Expected Output
- Pressure displayed in hPa
- Altitude displayed in meters
- Live OLED updates
- Continuous Serial Monitor output

## Author
Samridh Sapolia
