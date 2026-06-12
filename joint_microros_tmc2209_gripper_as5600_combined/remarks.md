# Calibration Analysis — Why the YAML-based calibration doesn't work

## Architecture

Before the change:
```
Firmware (main.c): raw encoder → calibrate (hardcoded #defines) → /joint_states → RViz/MoveIt
```

After the change:
```
Firmware (main.c): raw encoder → /joint_states  (RAW, uncalibrated)

Calibration processor: /micro_ros/raw_joints → calibrate (from YAML) → /micro_ros/calibrated_states
```

## Root cause — the two paths never connect

### Issue 1: Firmware publishes to `/joint_states`, but calibration processor reads `/micro_ros/raw_joints` (CRITICAL)

- **`main.c:478`** — firmware publisher topic: `"/joint_states"`
- **`my_calibration_processor:20`** — calibration subscriber topic: `"/micro_ros/raw_joints"`
- **Result:** The calibration processor **never receives any data**. Calibration is **never applied**.

### Issue 2: Calibration processor publishes to `/micro_ros/calibrated_states`, but nobody reads it (CRITICAL)

- **`my_calibration_processor:29`** — calibration publisher topic: `"/micro_ros/calibrated_states"`
- robot_state_publisher, move_group, and RViz all read from **`/joint_states`** (default topic)
- **Result:** Even if calibration ran, the calibrated data would go nowhere.

### Issue 3: Incomplete joint output in calibration processor

In `my_calibration_processor:36-59`:
- `calibrated_msg.name = msg.name` — copies all 5 names (`joint_1`..`joint_6`)
- But `calibrated_msg.position.append()` only adds positions for joints 1-3
- **Result:** Position array has 3 elements while name array has 5 — mismatch.

### Issue 4: Hardcoded direction for commands (decoupled from YAML)

In `main.c:233`:
```c
double pos1 = -msg->position.data[1];
```
This negates the commanded position for joint_2. The YAML has `direction: -1` for joint_2 (used in feedback), but changing it in YAML does **not** affect the command path — that's still hardcoded in C.

### Issue 5: (`joint_command_bridge`) may fail due to missing `control_msgs` dependency

`joint_command_bridge:4` imports `control_msgs.msg.JointTrajectoryControllerState`. If the `control_msgs` package is not installed, this node fails at startup.

---

## How the flow should work

### Minimal fix (no firmware reflash needed)

1. Change `my_calibration_processor` to:
   - Subscribe to **`/joint_states`** (where firmware publishes raw data)
   - Calibrate joints 1-3, **pass through** joints 4 and 6 unchanged
   - Publish to **`/joint_states`** (or `/micro_ros/calibrated_states` with remapping)

2. In `demo.launch.py`, if using `/micro_ros/calibrated_states`, remap:
   ```python
   remappings=[('/joint_states', '/micro_ros/calibrated_states')]
   ```
   for robot_state_publisher and move_group nodes.

### Cleaner fix (requires firmware reflash)

1. Change firmware (`main.c:478`) to publish raw data to **`/micro_ros/raw_joints`**
2. Keep `my_calibration_processor` subscribing to `/micro_ros/raw_joints`
3. Change `my_calibration_processor` to publish to **`/joint_states`** (where downstream expects it)
4. Fix the joint-4/6 passthrough
5. No launch file changes needed.
