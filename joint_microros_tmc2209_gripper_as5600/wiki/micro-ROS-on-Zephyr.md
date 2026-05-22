# micro-ROS on Zephyr

## Overview

This project uses micro-ROS with Zephyr RTOS 4.3, based on the official `micro_ros_zephyr_module` but with modifications for standalone integration.

The original module was designed primarily for use inside a ROS 2 Docker container. The adapted version removes this dependency and integrates cleanly as a regular Zephyr module.

## Key Changes from the Original

- Integrated directly into a Zephyr 4.3 environment
- Added `clock_gettime` implementation (required for POSIX time functions)
- Increased default limits:
  - Nodes: 1 → 4
  - Publishers: 1 → 4
  - Subscribers: 0 → 1
- Adjusted `prj.conf` for compatibility
- Built and configured `libmicroros.a` and supporting files locally

## Build Configuration

Key `prj.conf` settings:

```kconfig
CONFIG_NEWLIB_LIBC=y
CONFIG_NEWLIB_LIBC_NANO=n
CONFIG_POSIX_API=y
CONFIG_STDOUT_CONSOLE=y
CONFIG_LOG=y
CONFIG_MICROROS=y
```

## Setup Instructions

1. Add the `micro_ros_zephyr_module` to your Zephyr workspace:

   ```yaml
   manifest:
     projects:
       - name: micro-ros-zephyr
         path: modules/libmicroros/micro_ros_zephyr_module
         url: <your-fork-or-local-path>
         revision: main
   ```

2. Build:
   ```bash
   west build -b blackpill_f411ce -p auto
   ```

3. Tune parameters via `west build -t menuconfig` → **Modules → micro-ROS support**

## micro-ROS Agent

Run the agent on the host:

```bash
docker run -it --rm --net=host --privileged -v /dev:/dev \
  microros/micro-ros-agent:jazzy serial --dev /dev/ttyACM0 -v6
```

## Resources

- [Original micro_ros_zephyr_module](https://github.com/micro-ROS/micro_ros_zephyr_module)
- [micro-ROS documentation](https://micro-ros.github.io/)
- [Zephyr Project](https://zephyrproject.org/)
