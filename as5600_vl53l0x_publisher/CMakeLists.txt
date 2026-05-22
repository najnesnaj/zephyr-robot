# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

# Point to the pre-built libmicroros from micro_ros_zephyr_module
set(LIBMICROROS_DIR /home/naj/zephyrproject/micro_ros_zephyr_module/modules/libmicroros)

list(APPEND ZEPHYR_EXTRA_MODULES ${LIBMICROROS_DIR})

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(as5600_vl53l0x_publisher)

target_sources(app PRIVATE src/main.c )
