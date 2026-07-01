cmake_minimum_required(VERSION 3.8)
project(robot)

# Dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(gazebo_ros REQUIRED)
find_package(gazebo REQUIRED)

include_directories(
  include
  ${gazebo_INCLUDE_DIRS}
)

# ===== Balance Node (ROS2 executable) =====
add_executable(balance_node
  src/balance_node.cpp
)
ament_target_dependencies(balance_node
  rclcpp
  sensor_msgs
  geometry_msgs
)
install(
  TARGETS balance_node
  DESTINATION lib/${PROJECT_NAME}
)

# ===== Balance Controller (Gazebo plugin) =====
add_library(balance_controller SHARED
  src/balance_controller.cpp
)
ament_target_dependencies(balance_controller
  rclcpp
  sensor_msgs
  geometry_msgs
  gazebo_ros
)
target_link_libraries(balance_controller
  ${gazebo_LIBRARIES}
)
install(
  TARGETS balance_controller
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

# ===== Install resource directories =====
install(
  DIRECTORY urdf meshes launch config worlds
  DESTINATION share/${PROJECT_NAME}
)

# ===== Install package.xml =====
install(
  FILES package.xml
  DESTINATION share/${PROJECT_NAME}
)

ament_package()
