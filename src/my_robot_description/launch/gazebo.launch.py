"""
启动 Gazebo 仿真环境并生成机器人

启动节点:
  1. gazebo (空世界或自定义世界)
  2. robot_state_publisher (读 URDF, 发 /robot_description)
  3. spawn_entity (把机器人放进 Gazebo)

依赖: sudo apt install ros-humble-gazebo-ros-pkgs
"""

import os
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, ExecuteProcess
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory


def generate_launch_description():
    pkg_dir = get_package_share_directory('my_robot_description')
    urdf_path = os.path.join(pkg_dir, 'urdf', 'my_robot.urdf')

    return LaunchDescription([

        # ---- 1. 启动 Gazebo (空世界) ----
        ExecuteProcess(
            cmd=['gazebo', '--verbose', '-s', 'libgazebo_ros_factory.so'],
            output='screen'
        ),

        # ---- 2. robot_state_publisher ----
        Node(
            package='robot_state_publisher',
            executable='robot_state_publisher',
            parameters=[{'robot_description': open(urdf_path).read()}]
        ),

        # ---- 3. 生成机器人到 Gazebo 中 ----
        Node(
            package='gazebo_ros',
            executable='spawn_entity.py',
            arguments=[
                '-entity', 'my_robot',       # 实体名
                '-file', urdf_path,           # URDF 路径
                '-x', '0.0', '-y', '0.0', '-z', '0.1',  # 初始位置
            ],
            output='screen'
        ),

        # ---- 4. 键盘遥控 (可选，手动运行) ----
        # 使用方法: ros2 run teleop_twist_keyboard teleop_twist_keyboard --ros-args -r cmd_vel:=/cmd_vel
        #
        # 如果装了 teleop_twist_keyboard 包，取消下面注释可自动启动:
        # Node(
        #     package='teleop_twist_keyboard',
        #     executable='teleop_twist_keyboard',
        #     remappings=[('/cmd_vel', '/cmd_vel')],
        #     output='screen',
        #     prefix='xterm -e'  # 打开新终端窗口
        # ),
    ])
