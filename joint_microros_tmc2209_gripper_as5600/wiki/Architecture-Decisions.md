# Architecture Decisions

## Why This Project?

Robotics will become a hot topic in the years to come. This project was born from a desire to learn by doing (and failing). The total cost was kept well below $80 by using components already on hand.

The STM32F411 has enough memory and power to run micro-ROS. Attach it with a USB cable and you can control it directly with ROS 2!

## Why Zephyr + micro-ROS Instead of Native ROS 2?

### The Problem

The official `micro_ros_zephyr_module` is intentionally built as a Zephyr module that still relies on the full ROS 2 build ecosystem (colcon, ament, Python dependencies, etc.) to generate the static libraries (libmicroros).

For **pure embedded developers** who primarily work in the Zephyr/West/Kconfig/CMake environment, this adds an unnecessary abstraction layer and toolchain complexity.

### The Solution

Extracting the core library and integrating it directly into Zephyr's build system provides several real advantages:

- **Simpler developer experience** — No need to set up a full ROS 2 workspace just to build firmware for an MCU
- **Better Zephyr-native feel** — Proper Kconfig options, native CMake integration, West workflows, devicetree overlays
- **Lighter and more maintainable for constrained targets** — Easier to strip features, tune memory usage, and integrate with Zephyr subsystems (logging, shell, networking)
- **Faster iteration** — Especially valuable for low-level drivers, real-time constraints, and debugging

### Potential Downsides

- **Upstream synchronization** — micro-ROS continues to evolve; you need a strategy to periodically rebase or merge changes
- **Feature completeness** — Some higher-level ROS 2 tooling may be easier through the official module
- **Maintenance burden** — If kept private, you carry the full maintenance load

### Recommendation

This was **definitely a good move** for this use case. Many embedded teams prefer exactly this kind of "ROS-capable but not ROS-dependent" firmware.

## Why AS5600 Encoders?

- Very low cost ($1.50–$3)
- 12-bit resolution (4096 counts per revolution)
- Contactless magnetic sensing (no wear)
- Easy I²C interface

The only limitation is the fixed I²C address (0x36), but the STM32F411 resolves this by using its three independent hardware I2C channels.

## Why TMC2209?

- Completely silent operation (stealthChop2 mode)
- Higher efficiency and lower heat
- 256 microsteps support
- Step/Dir interface compatible with Zephyr's stepper controller subsystem

## Why USB CDC ACM?

A single USB cable handles both the Zephyr console and micro-ROS transport. No external serial port dongle is required. This keeps the hardware setup minimal and clean.
