# Project 09 - Stepper Motor Precision Positioner

## Objective
Build a precision position control system using an Arduino Uno, 28BYJ-48 stepper motor, ULN2003 driver, potentiometer, push buttons, and OLED display. The system allows accurate angular positioning similar to CNC machines and 3D printers.

## Components Used
- Arduino Uno R3
- 28BYJ-48 Stepper Motor
- ULN2003 Driver Board
- 10kΩ Potentiometer
- Push Buttons ×3
- OLED SSD1306 Display
- LED
- Breadboard
- Jumper Wires

## Wiring Summary
- ULN2003 IN1–IN4 → Arduino Pins 8, 9, 10, 11
- Potentiometer Output → A0
- Push Buttons → Digital Pins
- OLED SDA → A4
- OLED SCL → A5
- LED → Digital Pin (through resistor)

## Features
- Precision stepper motor control
- Potentiometer-controlled target angle
- Clockwise and Counter-Clockwise movement
- Home position reset
- OLED displays current angle, target angle, direction, and steps
- LED status indication

## Software Used
- Arduino IDE
- Wokwi Simulator

## Expected Output
- Stepper motor rotates accurately
- OLED updates angle and position
- Buttons control motor direction
- Home button resets logical position
- Serial Monitor displays motor status

## Author
Samridh Sapolia
