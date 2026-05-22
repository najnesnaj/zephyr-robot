# Boards: Blackpill STM32F411CE + CNC Header Board

## A Marriage of Convenience

This project uses a hybrid setup combining a low-cost **Blackpill STM32F411CE** development board with a custom **CNC header board** (CNC Shield V3.0). The result is a compact, powerful, and inexpensive motion control solution.

## Why This Combination?

- **Blackpill STM32F411CE**
  - Extremely cheap (~$5–8)
  - Powerful ARM Cortex-M4 @ 100 MHz with FPU
  - 512 KB Flash, 128 KB RAM
  - Excellent USB, SPI, I2C, UART, timers, and ADC peripherals
  - Small form factor

- **CNC Shield V3.0**
  - Breaks out all necessary signals for CNC/robot control
  - Standard stepper driver interfaces (STEP/DIR/ENABLE)
  - Easy to plug TMC2209 or A4988 drivers
  - Power distribution and protection
  - Originally designed for Arduino Uno

## Physical Integration

The STM32 Blackpill connects to the CNC Shield using Dupont jumper wires. The TMC2209 drivers are plugged into the CNC Shield slots (X, Y, Z, A).

**Key points:**
- Direct GPIO mapping from STM32F411 to CNC functions
- Proper power routing (3.3 V logic + 5 V tolerant inputs where needed)
- The Blackpill is mounted component-side up with the USB connector accessible

## Firmware Support

This board combination works well with:
- **Zephyr RTOS** (this project)
- **FluidNC** (recommended for CNC)
- **GRBL-STM32** port
- **Klipper** (via custom config)

## Advantages

- Very low total cost
- High performance (100 MHz + hardware timers)
- Excellent real-time capabilities
- Easy to repair / replace individual parts
- Full control over pinout and features

> **Tip:** Add a small heatsink on the STM32F411 for sustained high-speed stepper pulsing.
