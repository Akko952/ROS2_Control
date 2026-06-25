cmake_minimum_required(VERSION 3.8)
project(my_action_pkg)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(example_interfaces REQUIRED)

# Action 服务端：数数机器人，数到指定数字
add_executable(action_server src/action_server.cpp)
ament_target_dependencies(action_server rclcpp rclcpp_action example_interfaces)

# Action 客户端：下指令，让机器人数到 10
add_executable(action_client src/action_client.cpp)
ament_target_dependencies(action_client rclcpp rclcpp_action example_interfaces)

install(TARGETS action_server action_client
  DESTINATION lib/${PROJECT_NAME})

ament_package()
