# ROS2 Launch Error Analysis & Correction

## Error

```
[ERROR] [launch]: Caught exception in launch: package 'easy' found at
'/urdf/install/easy', but libexec directory '/urdf/install/easy/lib/easy'
does not exist
```

## Root Causes

### 1. Broken Symlinks (all 79)
The workspace was built when the source tree was at `/urdf/`, but it is
now located at `/usr/src/microros_stm32/my_urdf_folder/`. Every symlink
under `install/easy/` points to `/urdf/...` — which does not exist.

### 2. No Executables Installed
`CMakeLists.txt` only installs `share/` content (launch, config).
`demo.launch.py` references two executables from the `easy` package
that have never been installed:

| Executable | Line | Status |
|---|---|---|
| `my_calibration_processor` | 17 | No source file exists anywhere |
| `joint_command_bridge` | 62 | Exists as `launch/joint_command_bridge.py` but is not installed |

## Fixes

### Step 1 — Rebuild from the correct path
```bash
cd /usr/src/microros_stm32/my_urdf_folder
colcon build --symlink-install
```

### Step 2 — Install the Python script as a ROS2 executable
In `CMakeLists.txt` (after line 16), add:
```cmake
install(PROGRAMS launch/joint_command_bridge.py
  DESTINATION lib/${PROJECT_NAME})
```

Make it executable:
```bash
chmod +x launch/joint_command_bridge.py
```

Then rebuild.

### Step 3 — Handle `my_calibration_processor`
**Option A** (quickest) — Remove the node from `demo.launch.py`:
- Delete lines 15–19 (`joint_calibration_node`)
- Remove `joint_calibration_node,` from line 140

**Option B** — Create the script at `scripts/my_calibration_processor.py`
and add `install(PROGRAMS scripts/my_calibration_processor.py DESTINATION lib/${PROJECT_NAME})`
to `CMakeLists.txt`.

## Verification

After applying the fixes:
```bash
colcon build --symlink-install
source install/setup.bash
ros2 launch easy demo.launch.py
```

Check that executables are discoverable:
```bash
ros2 pkg executables easy
```
