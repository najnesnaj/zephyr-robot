Hardware Overview
=================

This project controls a 3-DOF robotic joint with a gripper, driven by three
TMC2209 stepper motors and a PWM servo. Three AS5600 magnetic rotary encoders
provide absolute position feedback. The entire system runs on a WeAct Black
Pill F411CE (STM32F411CE) under Zephyr RTOS with micro-ROS for ROS 2
integration.

Parts List
----------

.. list-table::
   :header-rows: 1

   * - Part
     - Quantity
     - Description
     - Interface
   * - **WeAct Black Pill F411CE**
     - 1
     - STM32F411CEU6 Cortex-M4F @ 96 MHz
     - USB, GPIO, I2C, PWM, UART
   * - **TMC2209** (stepper driver)
     - 3
     - Trinamic silent stepper driver with 1/32 microstepping
     - Step/Dir GPIO + enable
   * - **AS5600** (magnetic encoder)
     - 3
     - 12-bit magnetic rotary encoder, I2C address ``0x36``
     - I2C
   * - **PWM Servo** (e.g. SG90 / MG996R)
     - 1
     - Standard RC servo for gripper actuation
     - PWM (50 Hz, 700-2300 us)
   * - **NEMA 17** (stepper motor)
     - 3
     - Bipolar stepper, 200 full steps/rev
     - Connected to TMC2209

Pin Mapping
-----------

Stepper Motors
~~~~~~~~~~~~~~

All three TMC2209 drivers share a common **enable** pin (PC13). Each has its
own Step and Direction pins:

.. list-table::
   :header-rows: 1

   * - Signal
     - Motor 0
     - Motor 1
     - Motor 2
   * - Step
     - PA6
     - PA0
     - PA3
   * - Dir
     - PA7
     - PA1
     - PA5
   * - Enable (shared)
     - PC13
     - PC13
     - PC13

AS5600 Encoders
~~~~~~~~~~~~~~~

Each AS5600 sits on a dedicated I2C bus at address ``0x36``:

.. list-table::
   :header-rows: 1

   * - Encoder
     - Bus
     - SCL
     - SDA
   * - AS5600 #1
     - I2C1
     - PB6
     - PB7
   * - AS5600 #2
     - I2C2
     - PB10
     - PB3
   * - AS5600 #3
     - I2C3
     - PA8
     - PB4

Servo
~~~~~

The gripper servo is driven by TIM4 channel 3:

.. list-table::
   :header-rows: 1

   * - Signal
     - Pin
     - Peripheral
     - Properties
   * - PWM
     - PB8
     - TIM4_CH3
     - 50 Hz, 700--2300 us pulse

USB
~~~

USB CDC ACM is used for both the Zephyr console and the micro-ROS transport:

.. list-table::
   :header-rows: 1

   * - Signal
     - Pin
   * - DM
     - PA11
   * - DP
     - PA12

USART1 Debug
~~~~~~~~~~~~

An optional debug UART can output log messages directly:

.. list-table::
   :header-rows: 1

   * - Signal
     - Pin
   * - TX
     - PA9 (USART1)
   * - RX
     - PA10 (USART1)

Schematic
---------

::

   +---------------------------------------------+
   |                STM32F411CEU6                 |
   |              (WeAct Black Pill)              |
   +---------------------------------------------+
   |                                             |
   |  PA0  --- Step (Motor 1)                    |
   |  PA1  --- Dir  (Motor 1)                    |
   |  PA3  --- Step (Motor 2)                    |
   |  PA5  --- Dir  (Motor 2)                    |
   |  PA6  --- Step (Motor 0)                    |
   |  PA7  --- Dir  (Motor 0)                    |
   |  PC13 --- EN   (all TMC2209)               |
   |                                             |
   |  PB6  --- SCL (I2C1  -> AS5600 #1)         |
   |  PB7  --- SDA (I2C1  -> AS5600 #1)         |
   |  PB10 --- SCL (I2C2  -> AS5600 #2)         |
   |  PB3  --- SDA (I2C2  -> AS5600 #2)         |
   |  PA8  --- SCL (I2C3  -> AS5600 #3)         |
   |  PB4  --- SDA (I2C3  -> AS5600 #3)         |
   |                                             |
   |  PB8  --- PWM (TIM4_CH3 -> Servo)          |
   |                                             |
   |  PA11 --- USB DM                            |
   |  PA12 --- USB DP                            |
   |                                             |
   |  PA9  --- USART1 TX (debug out)             |
   |  PA10 --- USART1 RX (debug in)              |
   +---------------------------------------------+
        |    |    |                  |
        |    |    |                  +-- [Servo]
        |    |    |                       PB8 (PWM)
        |    |    |
        |    |    +-- [AS5600 #1] 0x36    [AS5600 #2] 0x36    [AS5600 #3] 0x36
        |    |         I2C1 (PB6/PB7)       I2C2 (PB10/PB3)     I2C3 (PA8/PB4)
        |    |
        |    +-- [TMC2209 #0]  Step=PA6  Dir=PA7
        |    +-- [TMC2209 #1]  Step=PA0  Dir=PA1
        |    +-- [TMC2209 #2]  Step=PA3  Dir=PA5
        |         EN=PC13 (shared)
        |
        +-- [USB CDC ACM]  PA11/PA12
             Console + micro-ROS transport

I2C Bus Detail
~~~~~~~~~~~~~~

::

   +3.3V
     |
     +--[4k7]--+-- SCL (PB6) --+-- AS5600 #1 (0x36)
     |          |               |
     +--[4k7]--+-- SDA (PB7) --+
     |
     +--[4k7]--+-- SCL (PB10) -+-- AS5600 #2 (0x36)
     |          |               |
     +--[4k7]--+-- SDA (PB3) --+
     |
     +--[4k7]--+-- SCL (PA8) --+-- AS5600 #3 (0x36)
     |          |               |
     +--[4k7]--+-- SDA (PB4) --+

   All three buses are independent and run at 100 kHz.

TMC2209 Connections
~~~~~~~~~~~~~~~~~~~

::

   +3.3V    GND
     |       |
     +-------+--[TMC2209 #0]--+-- NEMA 17 (coils A, B)
     |       |                |
     |       |   STEP = PA6   |
     |       |   DIR  = PA7   |
     |       |   EN   = PC13  |
     |       |   VCC  = VMOT  |
     |       |
     +-------+--[TMC2209 #1]--+-- NEMA 17
     |           STEP = PA0
     |           DIR  = PA1
     |           EN   = PC13
     |
     +-------+--[TMC2209 #2]--+-- NEMA 17
                 STEP = PA3
                 DIR  = PA5
                 EN   = PC13

Servo Connection
~~~~~~~~~~~~~~~~

::

   PB8 (TIM4_CH3) ---- [Servo Signal]
   +5V  -------------- [Servo VCC]
   GND  -------------- [Servo GND]
