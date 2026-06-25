cmake_minimum_required(VERSION 3.8)
project(my_robot_description)

find_package(ament_cmake REQUIRED)

# 安装 URDF 文件到 share/ 目录
install(DIRECTORY urdf
  DESTINATION share/${PROJECT_NAME})
install(DIRECTORY launch
  DESTINATION share/${PROJECT_NAME})
install(DIRECTORY rviz
  DESTINATION share/${PROJECT_NAME})

ament_package()
