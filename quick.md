

start de omgeving voor RVIZ
----------------------------

podman run -it --rm   --net=host   --ipc=host   --privileged   -e DISPLAY=$DISPLAY   -e QT_X11_NO_MITSHM=1   -v /tmp/.X11-unix:/tmp/.X11-unix:rw -v /usr/src/microros_stm32/my_urdf_folder:/urdf   -v $XAUTHORITY:/root/.Xauthority   docker.io/moveit/moveit2:jazzy-release   bash

in de container :  folder /urdf

colcon build --symlink-install
source install/setup.bash 
ros2 launch easy demo.launch.py




start communicatie met de robot
-------------------------------
podman run -it --rm --net=host --privileged -v /dev:/dev microros/micro-ros-agent:jazzy serial --dev /dev/ttyACM0 -b 115200 -v6



robotarm doen bewegen
----------------------

start een sessie  : podman exec -it <name container> bash


ros2 topic pub /joint_commands sensor_msgs/msg/JointState "{name: ['joint_1','joint_2','joint_3','joint_4', 'joint_6'], position: [0.3, 0.4, 0.4, 0.0, 0.0]}" --once


controle
--------

in de container moet apt install -y ros-jazzy-ros2-control ros-jazzy-ros2-controllers dit geinstalleerd zijn!!

In ros2 topic list :/ebamk2_controller/joint_trajectory (de Action-interface waar MoveIt mee praat)
/ebamk2_controller/controller_state/dynamic_joint_states




logging 
-----------

1) seriele console : minicom -D /dev/ttyUSB0

2) ros2 logging : start een sessie  : podman exec -it <name container> bash
ros2 topic echo /micro_ros/log    
