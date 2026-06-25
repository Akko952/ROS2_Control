cmake_minimum_required(VERSION 3.8)
project(my_first_pkg)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 查找依赖
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(rosidl_default_generators REQUIRED)  # 消息代码生成器
find_package(tf2 REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(geometry_msgs REQUIRED)

# ===== 第一步: 生成自定义消息的 C++ 代码 =====
rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/SensorData.msg"
  DEPENDENCIES std_msgs
)

# ===== 第二步: 编译原有节点 (仍用 std_msgs::String) =====
add_executable(publisher src/minimal_publisher.cpp)
ament_target_dependencies(publisher rclcpp std_msgs)

add_executable(subscriber src/minimal_subscriber.cpp)
ament_target_dependencies(subscriber rclcpp std_msgs)

# ===== 第三步: 编译新节点 (用自定义消息) =====
add_executable(sensor_publisher src/sensor_publisher.cpp)
ament_target_dependencies(sensor_publisher rclcpp std_msgs)
# 链接自定义消息的头文件
rosidl_target_interfaces(sensor_publisher ${PROJECT_NAME} "rosidl_typesupport_cpp")

add_executable(sensor_subscriber src/sensor_subscriber.cpp)
ament_target_dependencies(sensor_subscriber rclcpp std_msgs)
rosidl_target_interfaces(sensor_subscriber ${PROJECT_NAME} "rosidl_typesupport_cpp")

# ===== 第四步: TF2 广播节点 =====
add_executable(tf_broadcaster src/tf_broadcaster.cpp)
ament_target_dependencies(tf_broadcaster rclcpp tf2 tf2_ros geometry_msgs)

# ===== 安装 =====
install(TARGETS publisher subscriber sensor_publisher sensor_subscriber tf_broadcaster
  DESTINATION lib/${PROJECT_NAME})

# 安装 launch 和 msg 目录
install(DIRECTORY launch msg
  DESTINATION share/${PROJECT_NAME})

ament_package()
