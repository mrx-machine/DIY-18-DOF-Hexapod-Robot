# Hexapod Robot – Arduino Code

This folder contains the Arduino source code for my 3D-printed hexapod robot.

The robot uses an Arduino Nano to control 18 MG996R servo motors through two PCA9685 servo driver boards. Movement and other functions are controlled wirelessly using a PS2 controller.

## Main Features

- Control of all six legs (3 servos per leg)
- Walking in different directions
- Turning and body positioning
- Greeting animation
- Adjustable movement speed
- Returning the legs to their neutral position
- Wireless PS2 controller support

## Configuration

Most of the servo configuration can be found in `output.cpp`.

Here you can adjust:

- **Servo channels (`ch`)** – PCA9685 channel assignments
- **Angle offsets (`angleOffset`)** – individual servo calibration
- **Movement limits (`minAngle`, `maxAngle`)** – minimum and maximum servo angles
- **Mirrored (`mirrored`)** – mirrored leg configuration
- **Mount angle (`mountAngle`)** – leg mounting orientation

These settings may need to be adjusted depending on your assembly and servo installation.

## Installation

1. Download or clone this repository.
2. Open the Arduino project in Arduino IDE.
3. Install all required libraries.
4. Select Arduino Nano and the correct COM port.
5. Check the servo configuration in `output.cpp`.
6. Upload the code to the Arduino Nano.

## Notes

The code is based on the original project by https://github.com/Sir-Kuhnhero and has been modified and adapted for my hexapod robot.

See the main repository's LICENSE file for licensing information.
