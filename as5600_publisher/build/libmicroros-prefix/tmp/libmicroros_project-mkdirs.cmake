# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/naj/zephyrproject/micro_ros_zephyr_module/modules/libmicroros"
  "/home/naj/zephyrproject/micro_ros_zephyr_module/modules/libmicroros"
  "/home/naj/zephyrproject/as5600_publisher/build/libmicroros-prefix"
  "/home/naj/zephyrproject/as5600_publisher/build/libmicroros-prefix/tmp"
  "/home/naj/zephyrproject/as5600_publisher/build/libmicroros-prefix/src/libmicroros_project-stamp"
  "/home/naj/zephyrproject/as5600_publisher/build/libmicroros-prefix/src"
  "/home/naj/zephyrproject/as5600_publisher/build/libmicroros-prefix/src/libmicroros_project-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/naj/zephyrproject/as5600_publisher/build/libmicroros-prefix/src/libmicroros_project-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/naj/zephyrproject/as5600_publisher/build/libmicroros-prefix/src/libmicroros_project-stamp${cfgdir}") # cfgdir has leading slash
endif()
