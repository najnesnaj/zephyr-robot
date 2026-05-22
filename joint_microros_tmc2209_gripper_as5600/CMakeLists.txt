cmake_minimum_required(VERSION 3.20.0)

set(LIBMICROROS_DIR /home/naj/zephyrproject/micro_ros_zephyr_module/modules/libmicroros)
list(APPEND ZEPHYR_EXTRA_MODULES ${LIBMICROROS_DIR})

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(joint_microros_tmc2209_gripper_as5600)

target_sources(app PRIVATE src/main.c)
