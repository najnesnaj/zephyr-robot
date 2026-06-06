cmake_minimum_required(VERSION 3.22)
project(easy)

find_package(ament_cmake REQUIRED)

ament_package()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/launch")
  install(
    DIRECTORY launch
    DESTINATION share/${PROJECT_NAME}
    PATTERN "setup_assistant.launch" EXCLUDE)
endif()

install(DIRECTORY config DESTINATION share/${PROJECT_NAME})
install(FILES .setup_assistant DESTINATION share/${PROJECT_NAME})
install(PROGRAMS launch/joint_command_bridge
  DESTINATION lib/${PROJECT_NAME})
install(PROGRAMS launch/my_calibration_processor
  DESTINATION lib/${PROJECT_NAME})

