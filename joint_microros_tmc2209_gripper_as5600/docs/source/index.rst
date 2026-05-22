A Zephyr RTOS firmware project for a 3-DOF robotic joint with gripper.
=======================================================================

A Zephyr RTOS firmware project for a 3-DOF robotic joint with gripper,
driving three TMC2209 stepper motors and a PWM servo, with absolute
position feedback from three AS5600 magnetic encoders. Communication
with ROS 2 is handled by micro-ROS over USB CDC ACM.

.. toctree::
   :maxdepth: 2
   :caption: Contents:

   why
   intro
   goodidea
   3Dprinted_robot
   hardware
   microros_zephyr
   logging
   containers
   boards
   encoder
   rviz
   rviz_pictures

Board
  WeAct Black Pill F411CE (STM32F411CE, Cortex-M4F @ 96 MHz)

Build
  .. code-block:: bash

     west build -p auto -b blackpill_f411ce \\
       /home/naj/zephyrproject/joint_microros_tmc2209_gripper_as5600

Topics overview
---------------

.. list-table::
   :header-rows: 1

   * - Topic
     - Type
     - Direction
   * - ``/joint_commands``
     - ``sensor_msgs/JointState``
     - Subscribe
   * - ``/gripper_cmd``
     - ``std_msgs/Int32``
     - Subscribe
   * - ``/encoder[1-3]/angle``
     - ``std_msgs/Float32``
     - Publish (1 Hz)
   * - ``/micro_ros/log``
     - ``rcl_interfaces/Log``
     - Publish (on event)

Indices and tables
==================

* :ref:`genindex`
* :ref:`search`
