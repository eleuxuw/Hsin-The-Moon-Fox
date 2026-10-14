# Hsin Interactive Shadow Lamp

## Project Overview

An Arduino-based interactive shadow lamp inspired by Hsin from Wuthering Waves.

The project combines a paper-cut silhouette, RGB lighting, and a photoresistor to create a decorative lamp that responds to user input and surrounding light conditions.

## Features

1. Automatic Night Detection

   The photoresistor detects ambient light levels and automatically activates the lamp when the environment becomes dark.

2. Four Color Modes

   The lamp features four selectable RGB color palettes:
   - Astral Moonlight
   - Phantom Orchid
   - Celestial Jade
   - Vermilion Foxfire

3. Lighting Animations

   Three animation modes are available:
   - Breathing
   - Steady
   - Heartbeat

4. Automatic Color Cycling

   Smoothly transitions between the four color palettes.

5. Button Controls

   Multiple functions can be accessed through a single push button.

## Button Controls

| Input | Function |
|---|---|
| Single Click | Change color |
| Double Click | Toggle power |
| Triple Click | Change animation |
| Quadruple Click | Toggle night detection |
| Hold 1 Second | Toggle automatic color cycling |
| Hold 3 Seconds | Reset settings |

## Hardware Components

- Arduino Nano
- Common-anode RGB LED
- Photoresistor module
- Push button
- Jumper wires
- Paper-cut silhouette and frame

## Pin Configuration

| Component | Arduino Pin |
|---|---|
| Red LED | D3 |
| Green LED | D5 |
| Blue LED | D4 |
| Push Button | D2 |
| Photoresistor | A0 |

## Night Detection

The photoresistor measures ambient brightness through analog input A0.

The lamp uses the following calibrated thresholds:

| Sensor Reading | Behavior |
|---|---|
| 900–1023 | Turn on |
| 891–899 | Maintain previous state |
| 0–890 | Turn off |

The threshold gap prevents rapid switching caused by small changes in surrounding brightness.

## Programming

The Arduino program includes:

- Analog sensor reading
- Button debouncing
- Multi-click detection
- RGB color interpolation
- Non-blocking lighting animations
- Timer interrupt-based PWM control
- Serial Monitor debugging

## Video Demonstration

Due to the limited video duration, only two features were demonstrated:

1. Manual color changing using the push button.
2. Automatic night detection using the photoresistor.

The remaining functions, including lighting animations, automatic color cycling, and additional button controls, are implemented in the program but were not demonstrated in the video.

## Conclusion

This project explores the integration of Arduino programming, environmental sensing, and interactive lighting into a decorative shadow lamp.

The result is a customizable lighting system that responds to both physical input and changes in ambient brightness.
