# Pin Mapping

## Stepper Motors

All three TMC2209 drivers share a common **enable** pin (PC13). Each has its own Step and Direction pins.

| Signal     | Motor 0 | Motor 1 | Motor 2 |
|------------|---------|---------|---------|
| **Step**   | PA6     | PA0     | PA3     |
| **Dir**    | PA7     | PA1     | PA5     |
| **Enable** | PC13    | PC13    | PC13    |

## AS5600 Encoders

Each AS5600 sits on a dedicated I2C bus at address `0x36`:

| Encoder    | Bus  | SCL   | SDA   |
|------------|------|-------|-------|
| AS5600 #1  | I2C1 | PB6   | PB7   |
| AS5600 #2  | I2C2 | PB10  | PB3   |
| AS5600 #3  | I2C3 | PA8   | PB4   |

## Servo

| Signal | Pin   | Peripheral  | Properties              |
|--------|-------|-------------|-------------------------|
| PWM    | PB8   | TIM4_CH3    | 50 Hz, 700–2300 µs pulse |

## USB CDC ACM

| Signal | Pin   |
|--------|-------|
| DM     | PA11  |
| DP     | PA12  |

## USART1 Debug (optional)

| Signal | Pin    |
|--------|--------|
| TX     | PA9    |
| RX     | PA10   |

## TMC2209 Connections

```
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
```
