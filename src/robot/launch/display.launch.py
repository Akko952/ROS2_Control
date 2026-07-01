"""display.launch.py - Unified RViz view for the robot package."""

from launch import LaunchDescription
from launch.substitutions import Command, PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare


def generate_launch_description():
    pkg = FindPackageShare('robot')
    urdf = PathJoinSubstitution([pkg, 'urdf', 'robot.xacro'])
    config = PathJoinSubstitution([pkg, 'config', 'ros2_control.yaml'])
    rviz_config = PathJoinSubstitution([pkg, 'rviz', 'display.rviz'])

    robot_desc = {'robot_description': Command(
        ['xacro ', urdf, ' config_path:=', config]
    )}

    return LaunchDescription([
        Node(
            package='robot_state_publisher',
            executable='robot_state_publisher',
            parameters=[robot_desc],
            output='screen'
        ),
        Node(
            package='rviz2',
            executable='rviz2',
            arguments=['-d', rviz_config],
            output='screen'
        ),
    ])