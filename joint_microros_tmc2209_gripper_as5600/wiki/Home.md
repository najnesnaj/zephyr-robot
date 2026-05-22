# Welcome to the joint_microros_tmc2209_gripper_as5600 wiki

This project is a **Zephyr RTOS firmware** for an educational 3-DOF robotic arm with gripper. It drives three TMC2209 stepper motors and a PWM servo, with absolute position feedback from three AS5600 magnetic encoders. Communication with ROS 2 is handled by micro-ROS over USB CDC ACM.

## Quick Links

- [[Hardware-Overview]]
- [[Pin-Mapping]]
- [[Boards---Blackpill-and-CNC-Shield]]
- [[Encoder-Feedback]]
- [[3D-Printed-Robot--EEZYbotARM]]
- [[micro-ROS-on-Zephyr]]
- [[Containers-Setup]]
- [[ROS-2-Control]]
- [[RViz2-Motion-Planning]]
- [[Logging-System]]
- [[Architecture-Decisions]]

## Board

**WeAct Black Pill F411CE** (STM32F411CE, Cortex-M4F @ 96 MHz)

## Build

```bash
west build -p auto -b blackpill_f411ce /path/to/joint_microros_tmc2209_gripper_as5600
```

## ROS 2 Topics

| Topic                | Type                     | Direction  |
|----------------------|--------------------------|------------|
| `/joint_commands`    | `sensor_msgs/JointState` | Subscribe  |
| `/gripper_cmd`       | `std_msgs/Int32`         | Subscribe  |
| `/encoder1/angle`    | `std_msgs/Float32`       | Publish    |
| `/encoder2/angle`    | `std_msgs/Float32`       | Publish    |
| `/encoder3/angle`    | `std_msgs/Float32`       | Publish    |
| `/micro_ros/log`     | `rcl_interfaces/Log`     | Publish    |
