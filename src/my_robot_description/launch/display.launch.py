"""
display.launch.py — 在 RViz 中显示机器人模型

用法:
    ros2 launch my_robot_description display.launch.py
"""
import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():

    # 找到 URDF 文件路径
    pkg_dir = get_package_share_directory('my_robot_description')
    urdf_path = os.path.join(pkg_dir, 'urdf', 'my_robot.urdf')

    with open(urdf_path, 'r') as f:
        robot_desc = f.read()

    # 参数: 允许命令行传入自定义 URDF
    model_arg = DeclareLaunchArgument(
        'model',
        default_value=urdf_path,
        description='URDF 文件路径'
    )

    # robot_state_publisher: 读取 URDF → 发布 /tf 和 /robot_description
    robot_state_pub = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        name='robot_state_publisher',
        output='screen',
        parameters=[{'robot_description': robot_desc}]
    )

    # joint_state_publisher_gui: 发布 /joint_states (可拖动滑块)
    joint_state_pub = Node(
        package='joint_state_publisher_gui',
        executable='joint_state_publisher_gui',
        name='joint_state_publisher_gui',
    )

    # RViz2: 可视化
    rviz_config = os.path.join(pkg_dir, 'rviz', 'display.rviz')
    rviz = Node(
        package='rviz2',
        executable='rviz2',
        name='rviz2',
        arguments=['-d', rviz_config]
    )

    return LaunchDescription([
        model_arg,
        robot_state_pub,
        joint_state_pub,
        rviz,
    ])
