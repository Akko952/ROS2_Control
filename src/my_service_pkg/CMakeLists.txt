cmake_minimum_required(VERSION 3.8)
project(my_service_pkg)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(example_interfaces REQUIRED)

# 服务端：接电话，算加法
add_executable(server src/service_server.cpp)
ament_target_dependencies(server rclcpp example_interfaces)

# 客户端：打电话，发两个数字
add_executable(client src/service_client.cpp)
ament_target_dependencies(client rclcpp example_interfaces)

install(TARGETS server client
  DESTINATION lib/${PROJECT_NAME})

ament_package()
