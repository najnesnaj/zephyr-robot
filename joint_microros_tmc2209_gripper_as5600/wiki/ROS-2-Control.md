# ROS 2 Control

## Topics Overview

| Topic                | Type                     | Direction  | Rate          |
|----------------------|--------------------------|------------|---------------|
| `/joint_commands`    | `sensor_msgs/JointState` | Subscribe  | On new data    |
| `/gripper_cmd`       | `std_msgs/Int32`         | Subscribe  | On new data    |
| `/encoder1/angle`    | `std_msgs/Float32`       | Publish    | 1 Hz           |
| `/encoder2/angle`    | `std_msgs/Float32`       | Publish    | 1 Hz           |
| `/encoder3/angle`    | `std_msgs/Float32`       | Publish    | 1 Hz           |
| `/micro_ros/log`     | `rcl_interfaces/Log`     | Publish    | On each log call |

## Sending Joint Commands

The `joint_commands_callback` function receives a `sensor_msgs/JointState` message with 3 joint names and 3 positions (in radians):

```c
int32_t target_steps = ANGLE_TO_STEPS(position_rad);
int32_t delta = target_steps - current_microsteps;
stepper_ctrl_move_by(stepper_dev, delta);
```

The conversion factor is `ANGLE_TO_STEPS(angle_rad) = angle_rad * 509.3`, which accounts for:
- 200 full steps/rev (NEMA 17)
- 1/32 microstepping (TMC2209)
- Gear ratios

## Sending Gripper Commands

The `gripper_commands_callback` receives a `std_msgs/Int32` with a value from 0–180 (degrees):

```c
pulse_width = min_pulse + (data * (max_pulse - min_pulse) / 180);
pwm_set_pulse_dt(&servo, pulse_width);
```

- `0` → fully closed (700 µs pulse)
- `180` → fully open (2300 µs pulse)

## Reading Encoder Feedback

The `angle_timer_callback` runs at 1 Hz, reads all three AS5600 encoders, and publishes the angles:

```c
sensor_sample_fetch(as5600_dev);
sensor_channel_get(as5600_dev, SENSOR_CHAN_ROTATION, &value);

angle_msg.data = val1 + val2 / 1000000.0f;
rcl_publish(&angle_publisher, &angle_msg, NULL);
```

## micro-ROS Node

The firmware creates a single node called `joint_encoder_stepper_node` with:

- 2 subscribers (`/joint_commands`, `/gripper_cmd`)
- 4 publishers (3 encoder angles + 1 log)
- 1 timer (1 Hz encoder publishing)
- 1 executor with 3 handles (2 subs + 1 timer)

The main loop spins the executor at 10 ms intervals:
```c
while (1) {
    rclc_executor_spin_some(&executor, RCL_MS_TO_NS(10));
    k_sleep(K_MSEC(10));
}
```
