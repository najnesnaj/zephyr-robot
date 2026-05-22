# RViz2 Motion Planning

To use the Motion Planning plugin in RViz2 for your robot, you need to properly set up your URDF and launch environment.

## Setup Overview

You have placed all ROS 2 definitions (URDF, SRDF, kinematics, etc.) inside a folder called `my_urdf_folder`. You are using the official MoveIt 2 Docker image:

- **Image:** `docker.io/moveit/moveit2:jazzy-release`

## Launching the Container

Because you need graphical capabilities (RViz2), the container must be launched with specific flags to enable display forwarding:

```bash
podman run -it --rm \
  --net=host \
  --ipc=host \
  --privileged \
  -e DISPLAY=$DISPLAY \
  -e QT_X11_NO_MITSHM=1 \
  -v /tmp/.X11-unix:/tmp/.X11-unix:rw \
  -v /usr/src/microros_stm32/my_urdf_folder:/urdf \
  -v $XAUTHORITY:/root/.Xauthority \
  docker.io/moveit/moveit2:jazzy-release \
  bash
```

### Flags Explained

| Flag | Purpose |
|------|---------|
| `--net=host` | Host network stack (required for ROS 2) |
| `--ipc=host` | Shared IPC namespace (shared memory in ROS 2) |
| `--privileged` | Extended privileges for graphics and device access |
| `-e DISPLAY=$DISPLAY` | Pass display variable for GUI forwarding |
| `-e QT_X11_NO_MITSHM=1` | Disable MIT-SHM (fixes Qt/X11 issues in containers) |
| `-v /tmp/.X11-unix:...` | Mount X11 socket for graphical output |
| `-v /urdf` mount | Mount URDF folder into container at `/urdf` |
| `-v $XAUTHORITY:...` | Mount X11 authentication file |

## Inside the Container

Source your workspace and launch MoveIt:

```bash
source /urdf/install/setup.bash
ros2 launch easy demo.launch.py
```

This opens RViz2 with the MoveIt Motion Planning plugin loaded.

## Next Steps

Once RViz2 is running:
- Select the **Interact** tool
- Drag the orange interactive marker on the end-effector
- Use the **Motion Planning** panel to plan and execute trajectories
- Tweak the end-effector size and other visualization settings as needed
