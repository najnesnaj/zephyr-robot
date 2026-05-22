# 3D Printed Robot: EEZYbotARM MK2 Plus

## Custom STM32F411CE Blackpill Version

This robot arm is based on the **EEZYbotARM MK2 Plus** design, heavily modified from the original Instructables / Thingiverse design.

- **Original Design:** EEZYbotARM MK2 Plus by jackyltle
- [Instructables Guide](https://www.instructables.com/Robot-Arm-MK2-Plus-Stepper-Motor-Used/)
- **Original Control:** Arduino + CNC Shield + A4988/DRV8825 drivers

## Modifications & Upgrades

### Main Controller
- **MCU:** STM32F411CE Blackpill development board
- **RTOS:** Zephyr RTOS
- **Firmware Features:**
  - micro-ROS integration (USB CDC communication)
  - Stepper motor control (via TMC2209)
  - Encoder reading (joint position feedback)
  - Real-time control loops

### Stepper Drivers
- **Drivers:** TMC2209 (3 units)
- **Advantages:**
  - Completely silent operation (stealthChop2 mode)
  - Higher efficiency and lower heat
  - 256 microsteps support

### Motor Driver Board
- **Board:** CNC Shield V3.0 (originally designed for Arduino)
- **Connection:** STM32 Blackpill connected to the CNC Shield using Dupont jumper wires
- **Power:** 12–24 V external PSU connected to the CNC Shield power input

### Mechanical Structure
- 3D printed parts from the original **Robot Arm MK2 Plus** design
- Uses NEMA 17 stepper motors (3 for the main joints)
- Ball bearings and hardware as specified in the original build
- Kinematic linkage system (scaled-down ABB IRB460 style)

### Power Supply
- Logic: 3.3 V (from STM32 / regulator)
- Motors: 12–24 V (recommended 12–19 V for TMC2209)
- Separate power for motors and logic recommended

## Key Advantages of This Build

- Silent operation thanks to TMC2209
- Closed-loop feedback with encoders
- Modern ROS 2 integration via micro-ROS
- Real-time performance with Zephyr RTOS
- Better reliability and precision compared to original servo or open-loop stepper versions

## Assembly Steps

1. Print all 3D parts following the original Instructables guide
2. Assemble the mechanical structure
3. Mount TMC2209 drivers on the CNC Shield (pay attention to orientation and heatsinks)
4. Wire the STM32 Blackpill to the CNC Shield
5. Connect encoders to the STM32
6. Flash the Zephyr + micro-ROS firmware
7. Use ROS 2 Jazzy to control the arm

## Safety Notes

- Always use appropriate heatsinks on TMC2209 drivers
- Double-check motor power polarity and voltage
- Start with low current limits when tuning TMC2209
- Ensure proper grounding between STM32, CNC Shield, and power supply
