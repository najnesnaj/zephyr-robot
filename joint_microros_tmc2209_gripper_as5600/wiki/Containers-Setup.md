# Containers Setup

Two main containers are used for the ROS 2 Jazzy-based robot control system:

- `docker.io/library/ros:jazzy` (ROS 2 Jazzy desktop/full image)
- `docker.io/microros/micro-ros-agent:jazzy` (micro-ROS Agent)

The setup uses **Podman** (commands are compatible with Docker as well).

## 1. micro-ROS Agent Container

The microcontroller uses **USB CDC** for communication. The micro-ROS agent bridges the microcontroller with the ROS 2 network.

```bash
podman run -it --rm \
  --net=host \
  --privileged \
  -v /dev:/dev \
  microros/micro-ros-agent:jazzy \
  serial --dev /dev/ttyACM0 -b 115200 -v6
```

**Flags:**
- `--net=host`: Shares the host network namespace (required for ROS 2)
- `--privileged`: Grants full access to hardware devices
- `-v /dev:/dev`: Mounts the host's device directory (access to `/dev/ttyACM0`)

Leave this container running in its terminal.

## 2. ROS 2 Jazzy Container

Assuming the container is already running with the name `frosty_tharp`:

```bash
podman exec -it frosty_tharp bash
```

Always source the ROS 2 environment in every new shell:

```bash
source /opt/ros/jazzy/setup.bash
```

## Basic ROS 2 Commands

**List all available topics:**
```bash
ros2 topic list
```

Expected topics:
- `/encoder1/angle`
- `/encoder2/angle`
- `/encoder3/angle`
- `/gripper_cmd`
- `/joint_commands`
- `/micro_ros/log`
- `/parameter_events`
- `/rosout`

## Controlling the Robot

**Command joint positions:**
```bash
ros2 topic pub /joint_commands sensor_msgs/msg/JointState \
  "{name: ['joint0','joint1','joint2'], position: [0.1, 0.1, 0.0]}" --once
```

**Open the gripper (180°):**
```bash
ros2 topic pub /gripper_cmd std_msgs/msg/Int32 "{data: 180}" --once
```

**Close the gripper (0°):**
```bash
ros2 topic pub /gripper_cmd std_msgs/msg/Int32 "{data: 0}" --once
```

## Monitoring

**View micro-ROS logs:**
```bash
ros2 topic echo /micro_ros/log
```

**Read encoder angles:**
```bash
ros2 topic echo /encoder1/angle
ros2 topic echo /encoder2/angle
ros2 topic echo /encoder3/angle
```

## Tips

- Always source the setup file after opening a new shell inside the Jazzy container
- Keep the micro-ROS agent container running while working with ROS 2
- You can run multiple terminals into the same Jazzy container using `podman exec`
