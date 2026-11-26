# Arduino Hexapod Robot

This repository contains the Arduino source code for my custom-built 3D-printed hexapod robot.

The robot was designed and assembled as a personal robotics project, with a focus on mechanical design, 3D printing and electronics integration. It features six legs with three degrees of freedom each, giving a total of 18 MG996R servo motors.

The robot is controlled by an Arduino Nano, which communicates with two PCA9685 16-channel servo driver boards. A wireless PS2 controller is used to control the robot's movements and additional functions.

## Features

- Six-legged walking system with 18 servo motors.
- Wireless control using a PS2 controller.
- Movement in different directions: forward, backward, left and right.
- Turning and body positioning.
- Adjustable movement speed.
- Greeting animation.
- Raising and lowering the robot's body.
- Returning the legs to their neutral position.
- Individual servo calibration and configuration.

## Hardware

The code was developed for the following hardware configuration:

- Arduino Nano – main controller.
- 2 × PCA9685 – 16-channel servo driver boards.
- 18 × MG996R – servo motors.
- Wireless PS2 controller and receiver.
- 6V SBEC for servo power.
- 5V DC-DC converter for control electronics.

## Configuration and Calibration

The servo configuration is located in `output.cpp`. This file contains the channel assignments for both PCA9685 boards, individual servo offsets and other settings related to the leg positions.

Depending on your mechanical assembly and wiring, you may need to adjust:

- **Servo channels (`ch`)** – PCA9685 channel assignments.
- **Angle offsets (`angleOffset`)** – individual servo calibration.
- **Movement limits (`minAngle`, `maxAngle`)** – minimum and maximum servo angles.
- **Mirrored (`mirrored`)** – configuration for mirrored leg assemblies.
- **Mount angle (`mountAngle`)** – orientation of each leg relative to the body.

These values may differ depending on how the servos and mechanical parts are installed.

## Installation

1. Download or clone this repository.
2. Open the Arduino project in Arduino IDE.
3. Install the required libraries.
4. Select Arduino Nano and the correct COM port.
5. Check the servo channel assignments and calibration values in `output.cpp`.
6. Upload the code to the Arduino Nano.

Before operating the robot, make sure all servo connections are correct and that the legs are properly calibrated.

## Project Resources

Additional files and documentation for the project will be available through the following links:

- **Instructables – Build Guide:** [LINK TO INSTRUCTABLES]
- **Bambu Lab MakerWorld – 3D Models and Print Profiles:** [LINK TO BAMBU LAB PROJECT]
- **GitHub – Source Code:** [LINK TO THIS REPOSITORY]
- **Additional Resources:** [ADD LINKS IF NEEDED]

## Original Code and Credits

This project is based on the original work by **[ORIGINAL AUTHOR'S NAME]**.

- **Original repository:** [LINK TO ORIGINAL REPOSITORY]
- **Original author:** [AUTHOR'S NAME]
- **License:** MIT License

The original code has been modified and adapted for my hexapod robot. The original author's copyright notice and license terms are preserved in the LICENSE file.

## Build Instructions

The complete build documentation, including 3D printing, mechanical assembly, electronics installation, wiring diagrams and photographs, will be available in the Instructables guide linked above.