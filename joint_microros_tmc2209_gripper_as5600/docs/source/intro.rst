=========================================
Educational EEZYuBotArm with Micro-ROS
=========================================

Introduction
============

This project showcases the creation and programming of a 3D-printed EEZYuBotArm, specifically redesigned to fit NEMA 17 stepper motors. Unlike traditional designs, these motors drive the robot arm directly through gears, completely eliminating the need to fiddle with belts. The hardware is designed for straightforward fabrication: simply print the parts and assemble them, primarily using M2 bolts. 

.. note::
   This robot is intended for educational use only. It is mechanically not rigid or solid enough for real-world industrial work.

Hardware and Teach-In Capabilities
----------------------------------

Because the joints are equipped with AS5600 magnetic encoders, the robot arm can be easily manipulated by hand. This setup allows the arm to function as an intuitive input or teaching device. 

All hardware components are selected to be highly cost-effective:

* **NEMA 17 Stepper Motors:** Provide reliable, belt-free gear-driven motion.
* **AS5600 Encoders:** Deliver accurate position tracking for manual manipulation.
* **STM32F411 "Black Pill":** Serves as the central, low-cost microcontroller unit.

Although the AS5600 encoder has a fixed I2C address, the STM32F411 resolves this limitation easily by utilizing its three independent hardware I2C channels. 

Advanced Software Integration with Micro-ROS and Zephyr
-------------------------------------------------------

The core achievement of this project lies in the advanced programming of the STM32F411 Black Pill. The entire software stack runs directly on the microcontroller, managing stepper motors, a servo, and AS5600 encoder readings simultaneously.

Key software features include:

* **Zephyr RTOS & Micro-ROS:** The Micro-ROS library has been specifically adapted to work natively with Zephyr 4.3.
* **ROS2 Jazzy Compatibility:** The software architecture is fully updated to support the ROS2 Jazzy release.
* **Native USB Logging:** While a hardware serial port is present for debugging, all primary ROS2 messages and logs pass directly through a single USB cable. No external serial port dongle is required.
* **Closed-Loop Control:** Thanks to the accuracy of the AS5600 encoders, a configurable control mechanism is in place to drive the stepper motors based on encoder feedback.

Virtualization and RViz2
------------------------

To bridge the physical and digital worlds, the project includes complete integration with RViz2. This virtualization allows users to easily simulate the robot's movement on screen, or conversely, read the physical robot's manual movements and display them in the virtual environment in real time.

