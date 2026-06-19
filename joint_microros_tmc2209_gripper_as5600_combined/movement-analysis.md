# Movement Analysis — Why Steppers Don't Move

## System Architecture

```
┌──────────────────────────────────────────────────────────────────────────┐
│  ROS2 Control PC (docker-ros2-jazzy)                                    │
│                                                                          │
│  demo.launch.py  ───┬── my_calibration_processor  (broken symlink)      │
│                     ├── robot_state_publisher                           │
│                     ├── move_group  (MoveIt)                            │
│                     ├── rviz2                                          │
│                     ├── ros2_control_node (plugin missing)              │
│                     ├── joint_state_broadcaster_spawner                 │
│                     └── arm_controller_spawner  (ebamk2_controller)     │
│                                                                          │
│  joint_command_bridge  ──→ /joint_commands  ─────┐                     │
│  (broken symlink, never starts)                   │                     │
│                                                   │                     │
│  /joint_states  (raw encoder)                     │                     │
│       ↓                                           │                     │
│  my_calibration_processor                         │                     │
│       ↓                                           │                     │
│  /micro_ros/calibrated_states ─→ move_group       │                     │
│                                                   │                     │
└───────────────────────────────────────────────────┼─────────────────────┘
                                                    │ USB CDC (serial)
                                                    ▼
┌──────────────────────────────────────────────────────────────────────────┐
│  STM32F411 Black Pill (Zephyr RTOS)                                     │
│                                                                          │
│  main.c:                                                                 │
│    ┌──────────────────────┐    ┌──────────────────────┐                 │
│    │ angle_timer_callback │───→│ /joint_states pub    │                 │
│    │ (AS5600 encoders)    │    │ (raw, uncalibrated)  │                 │
│    └──────────────────────┘    └──────────────────────┘                 │
│                                                                          │
│    ┌──────────────────────┐    ┌──────────────────────┐                 │
│    │ /joint_commands sub  │───→│ joint_commands_cb    │                 │
│    │ (expects 5 positions)│    │ → stepper_move_by()  │                 │
│    └──────────────────────┘    └──┬───────────────────┘                 │
│                                   │                                     │
│                                   ▼                                     │
│    TMC2209 #1 (PA3=Step, PA5=Dir)  ← Stepper 0                         │
│    TMC2209 #2 (PA0=Step, PA1=Dir)  ← Stepper 1                         │
│    TMC2209 #3 (PA6=Step, PA7=Dir)  ← Stepper 2                         │
│    All EN=PC13 (active LOW)                                             │
│                                                                          │
│    Servo (PB8, TIM4_CH3)  ← Gripper (callback commented out)            │
└──────────────────────────────────────────────────────────────────────────┘
```

---

## CRITICAL ISSUES (directly prevent movement)

### 1. Broken xacro include — no robot description (config/ebamk2.urdf.xacro:6)

```xml
<xacro:include filename="/urdf/ebamk2_mesh.urdf" />
```

**Problem:** The path `/urdf/ebamk2_mesh.urdf` does not exist. The `ebamk2_mesh.urdf` file is located in `config/`, not at `/urdf/`. This is the original build path from a previous workspace location that was moved.

**Impact:** `xacro.process_file()` in `demo.launch.py:47` fails or produces an incomplete `robot_description`. Since `robot_description` is the foundation for `robot_state_publisher`, `move_group`, and `rviz`:
- MoveIt cannot initialize its planning scene
- No kinematics chains are defined
- No trajectories can be planned
- No commands are ever sent down the pipeline

**Fix:** Change to a relative path:
```xml
<xacro:include filename="ebamk2_mesh.urdf" />
```

---

### 2. Broken mesh paths — no visual geometry (config/ebamk2_mesh.urdf)

All 8 mesh references use `file:///urdf/meshes/<name>.STL` (e.g., line 11, 42, 65, 115, 139, 160, 190, 199). The `/urdf/meshes/` directory does not exist. Meshes are at `config/../meshes/`.

**Impact:** RViz cannot render the robot model, making visual planning difficult. However, this alone doesn't prevent motion — MoveIt can plan without visual meshes.

---

### 3. `topic_based_ros2_control` plugin not installed (config/ebamk2.ros2_control.xacro:8)

```xml
<plugin>topic_based_ros2_control/TopicBasedSystem</plugin>
```

**Problem:** The `topic_based_ros2_control` package is not present in the ROS2 environment. This hardware plugin is supposed to bridge `ros2_control` commands to the `/joint_commands` topic and read state from `/joint_states`.

**Impact:** The `ros2_control_node` fails to load the hardware interface. Without it:
- `joint_state_broadcaster` cannot spawn (or spawns with no data)
- `ebamk2_controller` cannot spawn (no hardware interface available)
- The `/ebamk2_controller/controller_state` topic never appears
- The `joint_command_bridge` (which subscribes to `controller_state`) has nothing to relay

**Result: The entire command chain from MoveIt to micro-ROS is broken at this point.**

**Fix:** Install `topic_based_ros2_control`:
```bash
apt-get install ros-jazzy-topic-based-ros2-control
```
Or configure a different hardware interface (e.g., `mock_components/GenericSystem` for testing).

---

### 4. Broken symlinks — executables cannot start

All executables in `install/easy/lib/easy/` are symlinks to `/urdf/...` paths that no longer exist:

| Symlink | Target | Status |
|---------|--------|--------|
| `joint_command_bridge` | → `/urdf/launch/joint_command_bridge` | BROKEN |
| `my_calibration_processor` | → `/urdf/launch/my_calibration_processor` | BROKEN |

**Impact:** `demo.launch.py` references `executable='joint_command_bridge'` (line 69) and `executable='my_calibration_processor'` (line 23). When launched, these nodes immediately fail because the executables are broken links.

**Without `joint_command_bridge`:**
- No translation of `controller_state` → `/joint_commands`
- Micro-ROS never receives position commands

**Without `my_calibration_processor`:**
- Raw encoder data on `/joint_states` is never calibrated
- The `/micro_ros/calibrated_states` topic is never published
- `move_group` (remapped to `/micro_ros/calibrated_states`) sees no feedback

**Fix:** Rebuild with `--symlink-install` from the correct path:
```bash
cd /usr/src/microros_stm32/my_urdf_folder
colcon build --symlink-install
```

---

### 5. Joint state remapping mismatch — no feedback loop (demo.launch.py:103)

```python
remappings=[('/joint_states', '/micro_ros/calibrated_states')],
```

**Problem:** The `move_group` node is remapped to read feedback from `/micro_ros/calibrated_states`. However:
- micro-ROS firmware publishes RAW encoder data to `/joint_states`
- The `my_calibration_processor` (if running) publishes calibrated data to `/micro_ros/calibrated_states`
- But `robot_state_publisher` reads from `/joint_states` (raw) — no remapping
- The `ros2_control_node` reads from `/joint_states` (raw) — via `topic_based_ros2_control` config

**Impact:**
- MoveIt sees calibrated feedback but robot_state_publisher and ros2_control see raw data
- TF tree is computed from raw (uncalibrated) joint positions
- If the calibration processor is broken (Issue 4), MoveIt sees NO feedback at all
- Without joint state feedback, MoveIt may refuse to execute trajectories or generate them with incorrect assumptions

---

## SECONDARY ISSUES (would block movement even after primary fixes)

### 6. Controller joint mismatch (4 vs 5 joints)

| File | Joints managed |
|------|---------------|
| `ros2_controllers.yaml:13-17` | `joint_1, joint_2, joint_3, joint_4` |
| `moveit_controllers.yaml:13-18` | `joint_1, joint_2, joint_3, joint_4, joint_6` |
| `ebamk2.ros2_control.xacro:14-48` | `joint_1, joint_2, joint_3, joint_4, joint_6` |
| `joint_command_bridge:48-53` | sends 5 positions (joint_1..4 + joint_6) |
| `main.c:225` | expects exactly 5 positions |

**Problem:** `ros2_controllers.yaml` has `joint_6` commented out. The `ebamk2_controller` (JointTrajectoryController) only manages 4 joints, while MoveIt and the hardware interface expect 5.

`allow_partial_joints_goal: true` is set, so the controller accepts 5-joint trajectories and controls only its 4 joints. However:
- `joint_6` commands (gripper) are silently dropped by the controller
- The bridge sends 5 positions (including joint_6=0.0) — micro-ROS accepts them
- But motors 0-3 (joints 1-4) should still move — IF the bridge is running

### 7. joint_command_bridge strips joint names

```python
joint_state.name = []
joint_state.position = self.latest_positions  # 5 values, no names
```

The micro-ROS firmware checks `msg->position.size != 5` and uses positional ordering. This works by matching the bridge's position order `[joint_1, joint_2, joint_3, joint_4, joint_6]` with the firmware's expected order `pos0..pos4`. But SENSITIVE to ordering changes — any reordering silently maps to wrong motors.

### 8. joint_command_bridge runs at 10Hz timer

```python
self.publish_rate = 0.1  # 10Hz timer
```

Commands are sampled from the controller state and only forwarded every 100ms via a timer. This means:
- At least 100ms delay between trajectory execution and motor response
- If trajectory waypoints are closer than 100ms apart, some are skipped
- Combined with the 2-second stepper timeout, creates a fragile system

### 9. Stepper safety timeout (2 seconds) (main.c:557)

```c
if ((current_time - last_command_time) > 2000) {
    gpio_pin_set(gpioc_dev, STEPPER_ENABLE_PIN, 1);
    motors_enabled = false;
}
```

If no command is received for 2 seconds, the stepper drivers are disabled. With a 10Hz bridge, this is 20 missed cycles before timeout. Network glitches or momentary micro-ROS disconnections cause the motors to power down and re-enable on the next command, causing jerky movement.

### 10. Kinematics solver timeout too low (config/kinematics.yaml:3)

```yaml
kinematics_solver_timeout: 0.005  # 5 milliseconds
```

The default KDL kinematics solver typically needs 50-200ms for IK. A 5ms timeout causes nearly all IK solve attempts to fail, meaning MoveIt cannot find valid joint configurations for planned Cartesian poses. This can manifest as "no solution found" when trying to plan even simple movements.

### 11. Velocity/acceleration scaling at 10% (config/joint_limits.yaml)

```yaml
default_velocity_scaling_factor: 0.1
default_acceleration_scaling_factor: 0.1
```

MoveIt scales all motions to 10% of the joint limits. Combined with the low joint velocities (0.5 rad/s), the actual speed is very slow (0.05 rad/s ≈ 3°/s). A full joint rotation takes over 2 minutes — the arm appears to be doing nothing.

### 12. `use_sim_time: True` on ros2_control_node (demo.launch.py:134)

```python
{'use_sim_time': True}
```

This tells `ros2_control_node` to use simulation time, but micro-ROS publishes wall-clock timestamps. This can cause:
- Timestamp mismatches in the controller's trajectory timing
- The controller thinking time hasn't advanced → commands not executed
- Especially problematic if no `/clock` topic is published

### 13. Calibration formula yields zero for joints 4 & 6

In `calibration.yaml`, joints 4 and 6 have `direction: 0`. The calibration formula `((raw_pos - offset) * direction) / reduction` produces `0.0` for these joints. This is intended (they are not yet instrumented), but the calibrated output reports `0.0` radians, not the actual (uncalibrated) position.

### 14. Gripper subscriber commented out (main.c:540-542)

```c
// RCCHECK(rclc_executor_add_subscription(
//    &executor, &gripper_commands_subscriber, &gripper_cmd_msg,
//    &gripper_commands_callback, ON_NEW_DATA));
```

The gripper servo callback is defined but never registered in the executor. No gripper commands are processed.

---

## DATA FLOW DIAGNOSIS

### Expected command path (if all worked):
```
RViz "Plan & Execute"
  → MoveGroup (MoveIt)
    → FollowJointTrajectory action
      → ebamk2_controller (JointTrajectoryController)
        → topic_based_ros2_control hardware interface
          → writes to /joint_commands topic
            → micro-ROS subscriber /joint_commands
              → joint_commands_callback()
                → stepper_ctrl_move_by() for each motor
```

### ACTUAL command path (broken):
```
RViz "Plan & Execute"
  → MoveGroup (MoveIt)
    ✗ robot_description is empty (Issue 1)
    → Planning fails or produces empty trajectory
    → No FollowJointTrajectory action sent
    → No controller_state published
    → joint_command_bridge never fires
    → /joint_commands never published
    → micro-ROS receives nothing
    → Motors stay still
```

### Even if MoveIt generated a trajectory:
```
  → ebamk2_controller
    ✗ topic_based_ros2_control plugin not installed (Issue 3)
    → Controller fails to load
    → No controller_state published
    → joint_command_bridge has nothing to bridge
      (also broken — Issue 4)
```

---

## IMMEDIATE ACTION ITEMS (priority order)

1. **Fix xacro include path** — change `/urdf/ebamk2_mesh.urdf` → `ebamk2_mesh.urdf`
2. **Install topic_based_ros2_control** — `apt-get install ros-jazzy-topic-based-ros2-control` (or use mock hardware)
3. **Rebuild easy package** — `colcon build --symlink-install` from the correct directory
4. **Fix mesh paths** — update `file:///urdf/meshes/` → proper `package://easy/meshes/` paths
5. **Uncomment joint_6** in `ros2_controllers.yaml` if gripper control is needed
6. **Increase kinematics timeout** — set `kinematics_solver_timeout: 0.5` (500ms) in `kinematics.yaml`
7. **Set `use_sim_time: False`** or remove it from `ros2_control_node` parameters
8. **Increase timeout** — change the 2-second stepper timeout in `main.c:557` to at least 5 seconds

### Quick test (bypass MoveIt entirely)

To verify hardware works, publish a test command directly:
```bash
ros2 topic pub /joint_commands sensor_msgs/JointState "{position: [0.5, 0.0, 0.0, 0.0, 0.0]}" --rate 5
```

If steppers move, the firmware and micro-ROS link are working, and the problem is entirely on the ROS2/planning side.
