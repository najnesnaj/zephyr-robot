microros on zephyr
==================

.. meta::
   :description: Guide to integrating and using micro-ROS on Zephyr RTOS
                 (adapted for direct module usage in Zephyr 4.3)

Overview
--------

This document describes how to use micro-ROS with Zephyr RTOS version 4.3,
based on the official micro_ros_zephyr_module but with modifications for
standalone integration.

The original module was designed primarily for use inside a ROS 2 capable
Docker container. The adapted version removes this dependency and integrates
cleanly as a regular Zephyr module, allowing it to be used like any other
external module in a standard Zephyr workspace.

Key Changes from the Original
-----------------------------

- Integrated directly into a Zephyr 4.3 environment.
- Added ``src/clock_gettime.c`` implementation (required for POSIX time functions).
- Updated configuration defaults for more practical use:
  - Increased nodes from 1 → 4
  - Increased publishers from 1 → 4
  - Added support for 1 subscriber
- Adjusted ``prj.conf`` (removed some conflicting or unnecessary options
  like ``CONFIG_PTHREAD_IPC=n`` and ``CONFIG_POSIX_CLOCK=y`` where handled
  differently).
- Minor source adjustments in ``main.c`` for header compatibility
  (``<time.h>`` instead of Zephyr-specific POSIX includes in some contexts).
- Built and configured ``libmicroros.a`` and supporting files locally.

Setup Instructions
------------------

1. Add the Module to Your Zephyr Project
   Place the ``micro_ros_zephyr_module`` directory inside your Zephyr workspace
   (typically under a ``modules`` folder or directly in the project root) and
   ensure it is discovered by Zephyr.

   In your project's ``west.yml`` (if using west manifests), add it as an
   external module:

   .. code-block:: yaml

      manifest:
        projects:
          - name: micro-ros-zephyr
            path: modules/libmicroros/micro_ros_zephyr_module
            url: <your-fork-or-local-path>
            revision: main  # or your branch

   Or simply copy the module into ``modules/`` and reference it appropriately.

2. Configuration (``prj.conf``)
   Example base configuration:

   .. code-block:: kconfig

      CONFIG_NEWLIB_LIBC=y
      CONFIG_NEWLIB_LIBC_NANO=n

      CONFIG_POSIX_API=y
      CONFIG_STDOUT_CONSOLE=y
      CONFIG_LOG=y

      # micro-ROS specific (via Kconfig)
      CONFIG_MICROROS=y
      # Further options available under Modules -> micro-ROS support in menuconfig

3. Building

   .. code-block:: bash

      west build -b <your_board> -p auto

   Example for a supported board (adjust as needed):

   .. code-block:: bash

      west build -b disco_l475_iot1 -p auto

4. Configuration Menu
   Tune parameters via:

   .. code-block:: bash

      west build -t menuconfig

   Navigate to **Modules → micro-ROS support** to adjust:
   - Number of nodes, publishers, subscribers, etc.
   - Transport options (serial, UDP, etc.)

Running the micro-ROS Agent
---------------------------

You can run a micro-ROS agent on your host using the official Docker images
(no ROS 2 container required for the device side):

**Serial Agent:**

.. code-block:: bash

   docker run -it --rm -v /dev:/dev --privileged --net=host \
     microros/micro-ros-agent:kilted serial --dev /dev/ttyACM0 -v6

**UDPv4 Agent:**

.. code-block:: bash

   docker run -it --rm --net=host \
     microros/micro-ros-agent:kilted udp4 --port 8888 -v6

(Replace the image tag and device/port as needed.)

Sample Application
------------------

The repository includes a basic sample in ``src/main.c`` that demonstrates
node initialization, publisher, etc.

You can extend it with your own publishers, subscribers, services, etc.,
using the standard micro-ROS C API.

Differences and Notes
---------------------

- No Docker dependency on the build side.
- ``clock_gettime`` implementation provided via ``clock_gettime.c``.
- Default limits increased for small multi-node or multi-topic use cases.
- Tested with Zephyr 4.3 (adjustments may be needed for other versions).
- Transport and middleware configuration happens through the libmicroros
  build (``colcon.meta`` adjustments).

Troubleshooting
---------------

Ensure colcon and dependencies are installed if rebuilding libmicroros:

.. code-block:: bash

   pip3 install catkin_pkg lark-parser empy colcon-common-extensions

- Check for missing POSIX functions and include the provided ``clock_gettime.c``.
- Verify board-specific transport (UART, Ethernet, etc.) is enabled in Zephyr.

Resources
---------

- Original repository: https://github.com/micro-ROS/micro_ros_zephyr_module
- micro-ROS documentation: https://micro-ros.github.io/
- Zephyr Project: https://zephyrproject.org/

License
-------

This follows the original Apache-2.0 license. See LICENSE and
3rd-party-licenses.txt for details.

.. note::
   This is an adapted guide. Always test thoroughly before deployment in
   safety-critical applications.
