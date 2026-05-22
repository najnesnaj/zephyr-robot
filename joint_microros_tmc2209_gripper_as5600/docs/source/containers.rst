Containers Used
===============

This document explains how to use the two main containers for the ROS 2 Jazzy-based robot control system:

* ``docker.io/library/ros:jazzy`` (ROS 2 Jazzy desktop/full image)
* ``docker.io/microros/micro-ros-agent:jazzy`` (micro-ROS Agent)

The setup uses **Podman** (commands are compatible with Docker as well).

1. micro-ROS Agent Container (Communication with STM32)
-------------------------------------------------------

The microcontroller (STM32F411CE Blackpill) uses **USB CDC** for communication. The micro-ROS agent bridges the microcontroller with the ROS 2 network.

**Start the micro-ROS agent:**

.. code-block:: bash

   podman run -it --rm \
     --net=host \
     --privileged \
     -v /dev:/dev \
     microros/micro-ros-agent:jazzy \
     serial --dev /dev/ttyACM0 -b 115200 -v6

**Explanation of flags:**

* ``--net=host``: Shares the host network namespace (required for ROS 2 communication)
* ``--privileged``: Grants full access to hardware devices
* ``-v /dev:/dev``: Mounts the host's device directory (needed to access ``/dev/ttyACM0``)
* ``serial --dev /dev/ttyACM0 -b 115200``: Connects via serial at 115200 baud
* ``-v6``: Verbosity level 6 (detailed logging)

Leave this container running in its terminal.

2. ROS 2 Jazzy Container
------------------------

**Assuming the container is already running with the name** ``frosty_tharp``.

**Enter the container:**

.. code-block:: bash

   podman exec -it frosty_tharp bash

**Source the ROS 2 environment** (do this in every new shell):

.. code-block:: bash

   source /opt/ros/jazzy/setup.bash

3. Basic ROS 2 Commands
-----------------------

**List all available topics:**

.. code-block:: bash

   ros2 topic list

Expected topics include:

* ``/encoder1/angle``
* ``/encoder2/angle``
* ``/encoder3/angle``
* ``/gripper_cmd``
* ``/joint_commands``
* ``/micro_ros/log``
* ``/parameter_events``
* ``/rosout``

4. Controlling the Robot
------------------------

**Command joint positions** (example):

.. code-block:: bash

   ros2 topic pub /joint_commands sensor_msgs/msg/JointState \
     "{name: ['joint0','joint1','joint2'], position: [0.1, 0.1, 0.0]}" --once

Adjust the ``position`` values (in radians) as needed.

**Open the gripper:**

.. code-block:: bash

   ros2 topic pub /gripper_cmd std_msgs/msg/Int32 "{data: 180}" --once

**Close the gripper:**

.. code-block:: bash

   ros2 topic pub /gripper_cmd std_msgs/msg/Int32 "{data: 0}" --once

5. Monitoring
-------------

**View micro-ROS logs:**

.. code-block:: bash

   ros2 topic echo /micro_ros/log

**Read encoder angles (joint positions):**

.. code-block:: bash

   ros2 topic echo /encoder1/angle
   ros2 topic echo /encoder2/angle
   ros2 topic echo /encoder3/angle

For continuous monitoring you can use ``--once`` or let the ``echo`` command run.

6. Useful Tips
--------------

* Always source the setup file after opening a new shell inside the Jazzy container.
* Keep the micro-ROS agent container running while working with ROS 2.
* Use ``--rm`` only if you want the container to be automatically removed when stopped.
* For persistent containers, remove the ``--rm`` flag and start with ``podman start`` / ``podman exec``.
* You can run multiple terminals into the same Jazzy container using ``podman exec``.

**Example workflow summary:**

1. Start micro-ROS agent (terminal 1)
2. Enter Jazzy container (terminal 2)
3. Source ROS 2 setup
4. Use ``ros2 topic pub`` and ``ros2 topic echo`` commands

This setup enables seamless bidirectional communication between the STM32 microcontroller and the ROS 2 Jazzy environment.


