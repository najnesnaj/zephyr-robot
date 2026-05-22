# 3-DOF Robotic Joint with Gripper — Zephyr + micro-ROS

[![Zephyr](https://img.shields.io/badge/RTOS-Zephyr-6C2BD9)](https://zephyrproject.org)
[![ROS 2](https://img.shields.io/badge/ROS%202-Jazzy-22314E)](https://docs.ros.org/en/jazzy)
[![MCU](https://img.shields.io/badge/MCU-STM32F411CE-03234B)](https://www.st.com/en/microcontrollers-microprocessors/stm32f411ce.html)

A Zephyr RTOS firmware for an educational **3-DOF robotic arm with gripper**, driving three TMC2209 stepper motors and a PWM servo, with absolute position feedback from three AS5600 magnetic encoders. Communication with ROS 2 is handled by **micro-ROS over USB CDC ACM**.

## Hardware

- **MCU:** WeAct Black Pill F411CE (STM32F411CEU6, Cortex-M4F @ 96 MHz)
- **Motors:** 3× NEMA 17 + 3× TMC2209 stepper drivers (Step/Dir, shared enable PC13)
- **Gripper:** Standard RC servo (PWM, 50 Hz, 700–2300 µs on TIM4_CH3 / PB8)
- **Encoders:** 3× AS5600 magnetic encoders (12-bit, I²C), one on each I2C bus
- **Transport:** USB CDC ACM (console + micro-ROS)

### Pin Mapping

| Signal       | Motor 0  | Motor 1  | Motor 2  |
|--------------|----------|----------|----------|
| **Step**     | PA6      | PA0      | PA3      |
| **Dir**      | PA7      | PA1      | PA5      |
| **Enable**   | PC13     | PC13     | PC13     |

| Encoder | Bus  | SCL   | SDA   |
|---------|------|-------|-------|
| AS5600 #1 | I2C1 | PB6  | PB7  |
| AS5600 #2 | I2C2 | PB10 | PB3  |
| AS5600 #3 | I2C3 | PA8  | PB4  |

## Software Stack

- **Zephyr RTOS 4.3** — real-time kernel with Kconfig, devicetree, west build
- **micro-ROS** (adapted as a native Zephyr module) — ROS 2 client library on MCU
- **ROS 2 Jazzy** — host-side control, RViz2, and MoveIt 2 integration

## ROS 2 Topics

| Topic | Type | Direction |
|-------|------|-----------|
| `/joint_commands` | `sensor_msgs/JointState` | Subscribe |
| `/gripper_cmd` | `std_msgs/Int32` (0–180°) | Subscribe |
| `/encoder[1-3]/angle` | `std_msgs/Float32` | Publish (1 Hz) |
| `/micro_ros/log` | `rcl_interfaces/Log` | Publish (on event) |

## Build & Flash

```bash
west build -p auto -b blackpill_f411ce /path/to/joint_microros_tmc2209_gripper_as5600
west flash
```

## Getting Started

1. Flash the firmware to the Black Pill.
2. Run the micro-ROS agent:
   ```bash
   podman run -it --rm --net=host --privileged -v /dev:/dev \
     microros/micro-ros-agent:jazzy serial --dev /dev/ttyACM0 -v6
   ```
3. In a ROS 2 Jazzy container, send commands:
   ```bash
   ros2 topic pub /joint_commands sensor_msgs/msg/JointState \
     "{name: ['j0','j1','j2'], position: [0.1, 0.1, 0.0]}" --once
   ros2 topic pub /gripper_cmd std_msgs/msg/Int32 "{data: 90}" --once
   ```

## License

Apache 2.0
