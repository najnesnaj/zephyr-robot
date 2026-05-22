EEZY Bot Arm Hardware
=====================


**Custom STM32F411CE Blackpill Version**

This document describes the hardware setup for the **EEZYbotARM MK2 Plus** (stepper motor version), heavily modified from the original Instructables / Thingiverse design.

Original Project
----------------
- **Design**: EEZYbotARM MK2 Plus by jackyltle (Thingiverse)
- **Link**: https://www.instructables.com/Robot-Arm-MK2-Plus-Stepper-Motor-Used/
- **Original Control**: Arduino + CNC Shield + A4988/DRV8825 drivers

Modifications & Upgrades
------------------------

**Main Controller**
  - **MCU**: STM32F411CE Blackpill development board
  - **RTOS**: Zephyr RTOS
  - **Firmware Features**:
    - micro-ROS integration (USB CDC communication)
    - Stepper motor control (via TMC2209)
    - Encoder reading (joint position feedback)
    - Real-time control loops

**Stepper Drivers**
  - **Drivers**: TMC2209 (4 units)
  - **Advantages**:
    - Completely silent operation (stealthChop2 mode)
    - Higher efficiency and lower heat
    - Advanced features (UART configuration, stallGuard, etc.)
    - 256 microsteps support

**Motor Driver Board**
  - **Board**: CNC Shield V3.0 (originally designed for Arduino)
  - **Connection**: STM32 Blackpill connected to the CNC Shield using Dupont jumper wires
  - **Power**: 12V–24V external PSU connected to the CNC Shield power input

**Mechanical Structure**
  - 3D printed parts from the original **Robot Arm MK2 Plus** design
  - Uses NEMA 17 stepper motors (typically 3 for the main joints + optional 4th for gripper/rotation)
  - Ball bearings and hardware as specified in the original build
  - Kinematic linkage system (scaled-down ABB IRB460 style)

**Electronics Connection Overview**
------------------------------------

- STM32F411CE Blackpill → Dupont cables → CNC Shield V3.0 pin headers
- TMC2209 drivers plugged into the CNC Shield slots (X, Y, Z, A)
- Stepper motors connected to the TMC2209 outputs
- Encoders (one per joint) wired to STM32 GPIO pins
- USB cable from Blackpill to host computer (for micro-ROS communication)

**Power Supply**
  - Logic: 3.3V (from STM32 / regulator)
  - Motors: 12V–24V (recommended 12V–19V for TMC2209)
  - Separate power for motors and logic recommended

**Key Advantages of This Build**
---------------------------------

- Silent operation thanks to TMC2209
- Closed-loop feedback with encoders
- Modern ROS 2 integration via micro-ROS
- Real-time performance with Zephyr RTOS
- Better reliability and precision compared to original servo or open-loop stepper versions

**Recommended Next Steps**
--------------------------

1. Print all 3D parts following the original Instructables guide.
2. Assemble the mechanical structure.
3. Mount TMC2209 drivers on the CNC Shield (pay attention to orientation and heatsinks).
4. Wire the STM32 Blackpill to the CNC Shield.
5. Connect encoders to the STM32.
6. Flash the Zephyr + micro-ROS firmware.
7. Use the ROS 2 Jazzy container (as described in previous documentation) to control the arm.

**Safety Notes**
----------------

- Always use appropriate heatsinks on TMC2209 drivers.
- Double-check motor power polarity and voltage.
- Start with low current limits when tuning TMC2209.
- Ensure proper grounding between STM32, CNC Shield, and power supply.

This hardware setup transforms the classic EEZYbotARM MK2 Plus into a modern, silent, closed-loop robotic arm fully integrated with ROS 2.
