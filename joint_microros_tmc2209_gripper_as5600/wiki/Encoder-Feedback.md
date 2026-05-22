# Encoder: Closed-Loop Feedback with AS5600

## Why Use a Cheap AS5600 on the Back of a NEMA Stepper?

To add reliable closed-loop position feedback at very low cost, an **AS5600** magnetic encoder is mounted on the rear shaft of standard NEMA17 stepper motors.

### Key Advantages

- Very low cost ($1.50–$3)
- 12-bit resolution (4096 counts per revolution)
- Contactless magnetic sensing (no wear)
- Easy I²C interface
- Small neodymium magnet (usually 10 mm × 2–3 mm) attached directly to the motor shaft

### Mounting Method

A small diametric magnet is glued or press-fit onto the rear shaft of the stepper. The AS5600 breakout board is held in place by a custom 3D-printed bracket, maintaining an air gap of approximately 1–2 mm.

## Comparison Table

| Type                | Resolution | Cost       | Verdict                    |
|---------------------|------------|------------|----------------------------|
| AS5600 Magnetic     | 12-bit     | Very Low   | **Best choice**            |
| Optical Encoder     | 600–2000 PPR | Medium   | More fragile, alignment-sensitive |
| AS5047 / TLV493D    | 14-bit+    | Higher     | Overkill for most builds   |

## Benefits of Rear-Shaft Mounting

- Does not interfere with the front shaft or load
- Uses standard stepper motors without modification
- Easy access for wiring and maintenance
- Keeps the encoder relatively protected

## Limitations

- 12-bit resolution is sufficient for most CNC and robotics applications
- Should be kept away from strong external magnetic fields
- Requires a rigid and well-aligned mounting bracket

## Firmware Integration

The encoders are read in an RTOS timer callback at 1 Hz:

```c
sensor_sample_fetch(as5600_dev);
sensor_channel_get(as5600_dev, SENSOR_CHAN_ROTATION, &sensor_value);
```

Angles are published to `/encoder[1-3]/angle` as `std_msgs/Float32`.
