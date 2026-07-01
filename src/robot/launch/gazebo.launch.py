# gazebo.launch.py
# One-click: gzserver + robot + ros2_control

import launch
import launch_ros
from launch.actions import ExecuteProcess, RegisterEventHandler
from launch.event_handlers import OnProcessExit
from launch.substitutions import Command, PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare


def generate_launch_description():
    pkg = FindPackageShare('robot')
    urdf = PathJoinSubstitution([pkg, 'urdf', 'robot.xacro'])
    config = PathJoinSubstitution([pkg, 'config', 'ros2_control.yaml'])

    robot_desc = {'robot_description': Command(
        ['xacro ', urdf, ' config_path:=', config]
    )}

    spawn_entity = Node(
        package='gazebo_ros',
        executable='spawn_entity.py',
        arguments=['-topic', 'robot_description', '-entity', 'pinocchio'],
        output='screen'
    )

    joint_state_broadcaster = Node(
        package='controller_manager',
        executable='spawner',
        arguments=[
            'joint_state_broadcaster',
            '--controller-manager', '/controller_manager',
            '--controller-manager-timeout', '180',
        ],
        output='screen'
    )

    diff_drive_controller = Node(
        package='controller_manager',
        executable='spawner',
        arguments=[
            'diff_drive_controller',
            '--controller-manager', '/controller_manager',
            '--controller-manager-timeout', '180',
        ],
        output='screen'
    )

    return launch.LaunchDescription([
        ExecuteProcess(
            cmd=['gzserver', '--verbose',
                 '-s', 'libgazebo_ros_init.so',
                 '-s', 'libgazebo_ros_factory.so'],
            output='screen'
        ),

        Node(
            package='robot_state_publisher',
            executable='robot_state_publisher',
            parameters=[robot_desc],
            output='screen'
        ),

        spawn_entity,

        RegisterEventHandler(
            OnProcessExit(
                target_action=spawn_entity,
                on_exit=[
                    joint_state_broadcaster,
                    diff_drive_controller,
                ],
            )
        ),
    ])