Logging System
==============

The firmware implements a **triple-output logging architecture**, sending
diagnostic information to three independent backends simultaneously. This
design allows monitoring via the ROS 2 ecosystem, an external serial debug
terminal, and the Zephyr console over USB.

The three layers
----------------

.. list-table::
   :header-rows: 1

   * - Layer
     - Destination
     - Mechanism
     - ROS 2 Level
   * - 1
     - Zephyr LOG subsystem
     - ``LOG_INF`` / ``LOG_ERR`` / ``LOG_WRN``
     - —
   * - 2
     - ``/micro_ros/log`` topic
     - ``rcl_interfaces/Log`` publisher
     - 20 (INFO) / 30 (WARN) / 40 (ERROR)
   * - 3
     - USART1 (PA9) direct UART
     - ``uart_poll_out()`` polling
     - —

Custom macros
-------------

Three convenience macros unify all three layers into single-line calls:

``LOG_INF_PUBLISH(fmt, ...)``
  1. Calls ``LOG_INF`` → Zephyr UART console (CDC ACM)
  2. Publishes to ``/micro_ros/log`` with level ``20`` (INFO)
  3. Echoes to USART1 via ``uart_poll_out``

``LOG_ERR_PUBLISH(fmt, ...)``
  1. Calls ``LOG_ERR``
  2. Publishes to ``/micro_ros/log`` with level ``40`` (ERROR)
  3. Echoes to USART1

``LOG_WRN_PUBLISH(fmt, ...)``
  1. Calls ``LOG_WRN``
  2. Publishes to ``/micro_ros/log`` with level ``30`` (WARN)
  3. **Does NOT** echo to USART1 (reserved for more verbose warnings)

Macro implementation
--------------------

The macros are defined in ``src/main.c``. The pattern for
``LOG_INF_PUBLISH`` is::

   #define LOG_INF_PUBLISH(...) do {                     \
       char buf[256];                                    \
       snprintf(buf, sizeof(buf), __VA_ARGS__);          \
       LOG_INF(__VA_ARGS__);                             \
       micro_ros_log_publish(buf, 20);                   \
       if (usart1_dev && device_is_ready(usart1_dev)) {  \
           for (int i = 0; buf[i] != '\\0'; i++) {       \
               uart_poll_out(usart1_dev, buf[i]);        \
           }                                             \
           uart_poll_out(usart1_dev, '\\r');              \
           uart_poll_out(usart1_dev, '\\n');              \
       }                                                 \
   } while(0)

The message is first formatted into a local ``buf[256]``, then consumed by
Zephyr's ``LOG_INF``, published to ROS 2, and finally bit-banged out of
USART1 character-by-character.

micro-ROS log publisher
-----------------------

The function ``micro_ros_log_init()`` creates a publisher on
``/micro_ros/log`` with message type ``rcl_interfaces/msg/Log``::

   void micro_ros_log_init(rcl_node_t *node)
   {
       log_msg.msg.capacity = 128;
       log_msg.msg.data = (char *)malloc(log_msg.msg.capacity * sizeof(char));
       log_msg.msg.size = 0;

       rcl_publisher_options_t pub_opts = rcl_publisher_get_default_options();
       rcl_publisher_init(&log_publisher, node,
           ROSIDL_GET_MSG_TYPE_SUPPORT(rcl_interfaces, msg, Log),
           "/micro_ros/log",
           &pub_opts);
   }

Each message contains:
- ``stamp`` — ROS 2 timestamp from ``rmw_uros_epoch_nanos()``
- ``level`` — 20 (INFO), 30 (WARN), 40 (ERROR)
- ``msg`` — the formatted log string (up to 128 bytes)

The publisher can be subscribed to from any ROS 2 node (e.g. ``ros2 topic
echo /micro_ros/log``).

Zephyr LOG configuration
------------------------

From ``prj.conf``:

.. code-block:: kconfig

   CONFIG_LOG=y
   CONFIG_LOG_BACKEND_UART=y
   CONFIG_LOG_MODE_DEFERRED=y

- **LOG_BACKEND_UART**: routes log output to the console UART (the CDC ACM
  virtual COM port in this build).
- **LOG_MODE_DEFERRED**: log messages are queued and processed by a
  background thread, reducing ISR latency.

The module is registered with INFO-level filtering::

   LOG_MODULE_REGISTER(joint_stepper_microros, LOG_LEVEL_INF);

Data flow diagram
-----------------

::

   +-------------------+       +-------------------+
   | Application code  | ----> | LOG_INF_PUBLISH   |
   | (main.c)          |       | LOG_ERR_PUBLISH   |
   +-------------------+       | LOG_WRN_PUBLISH   |
                               +--------+----------+
                                        |
                         +--------------+--------------+
                         |              |              |
                   Zephyr LOG      snprintf        snprintf
                   subsystem        into buf        into buf
                         |              |              |
                         v              v              v
                   +----------+  +-----------+  +------------+
                   | LOG_BE-  |  | micro-ROS |  | USART1     |
                   | CKEND_   |  | publisher |  | (PA9 TX)   |
                   | UART     |  | /micro_   |  | polling    |
                   |          |  | ros/log   |  |            |
                   +----------+  +-----------+  +------------+
                         |              |              |
                   USB CDC ACM    ROS 2 DDS     Debug serial
                   (virtual COM)  network        terminal
                   (9600 baud)    (any node)     (115200 baud)

Log message flow
----------------

1. Application calls ``LOG_INF_PUBLISH("Servo set to %d", angle)``.
2. The macro formats the string into a 256-byte local buffer.
3. ``LOG_INF`` sends the message to the Zephyr logging subsystem, which
   buffers it and eventually outputs it to the CDC ACM UART console.
4. ``micro_ros_log_publish()`` writes the timestamp, level, and message into
   an ``rcl_interfaces/msg/Log`` struct and publishes it to the DDS
   network via the micro-ROS transport (also over USB CDC ACM).
5. (For INFO and ERROR) The macro iterates over each character of the
   buffer and writes it to USART1 via ``uart_poll_out()``, appending
   CR+LF. This provides an independent hardware serial debug output.

ROS 2 topics reference
----------------------

.. list-table::
   :header-rows: 1

   * - Direction
     - Topic
     - Type
     - Rate
   * - Subscribe
     - ``/joint_commands``
     - ``sensor_msgs/JointState``
     - On new data
   * - Subscribe
     - ``/gripper_cmd``
     - ``std_msgs/Int32``
     - On new data
   * - Publish
     - ``/encoder1/angle``
     - ``std_msgs/Float32``
     - 1 Hz
   * - Publish
     - ``/encoder2/angle``
     - ``std_msgs/Float32``
     - 1 Hz
   * - Publish
     - ``/encoder3/angle``
     - ``std_msgs/Float32``
     - 1 Hz
   * - Publish
     - ``/micro_ros/log``
     - ``rcl_interfaces/Log``
     - On each log call

Viewing logs
------------

Zephyr console (USB CDC ACM)
  Connect to the virtual COM port with a serial terminal (e.g.
  ``screen /dev/ttyACM0 115200``). All Zephyr LOG output appears here.

ROS 2
  .. code-block:: bash

     ros2 topic echo /micro_ros/log

USART1 debug
  Connect a USB-UART adapter to PA9 (TX) and GND. Open the port at 115200
  baud. INFO and ERROR messages appear here.
