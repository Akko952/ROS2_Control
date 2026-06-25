"""
my_first_launch.py — 一键启动所有节点（含自定义消息的传感器节点）

用法:
    ros2 launch my_first_pkg my_first_launch.py                                    # 全部启动
    ros2 launch my_first_pkg my_first_launch.py sensor_topic:=/robot/sensors       # 自定义传感器话题
"""

from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration


def generate_launch_description():

    # ---- Hello world 节点参数 ----
    topic_arg = DeclareLaunchArgument(
        'topic_name', default_value='topic',
        description='Hello World 话题名'
    )
    pub_rate_arg = DeclareLaunchArgument(
        'publish_rate_ms', default_value='500',
        description='Hello World 发布间隔(ms)'
    )

    # ---- 传感器节点参数 ----
    sensor_topic_arg = DeclareLaunchArgument(
        'sensor_topic', default_value='sensor_data',
        description='传感器数据话题名'
    )
    sensor_rate_arg = DeclareLaunchArgument(
        'sensor_publish_rate_ms', default_value='1000',
        description='传感器发布间隔(ms)'
    )

    # ---- 节点定义 ----
    hello_pub = Node(
        package='my_first_pkg', executable='publisher',
        name='hello_publisher', output='screen',
        parameters=[{
            'topic_name': LaunchConfiguration('topic_name'),
            'publish_rate_ms': LaunchConfiguration('publish_rate_ms'),
        }]
    )

    hello_sub = Node(
        package='my_first_pkg', executable='subscriber',
        name='hello_subscriber', output='screen',
        parameters=[{
            'topic_name': LaunchConfiguration('topic_name'),
        }]
    )

    sensor_pub = Node(
        package='my_first_pkg', executable='sensor_publisher',
        name='sensor_publisher_node', output='screen',
        parameters=[{
            'topic_name': LaunchConfiguration('sensor_topic'),
            'publish_rate_ms': LaunchConfiguration('sensor_publish_rate_ms'),
        }]
    )

    sensor_sub = Node(
        package='my_first_pkg', executable='sensor_subscriber',
        name='sensor_subscriber_node', output='screen',
        parameters=[{
            'topic_name': LaunchConfiguration('sensor_topic'),
        }]
    )

    tf_node = Node(
        package='my_first_pkg', executable='tf_broadcaster',
        name='tf_broadcaster_node', output='screen',
    )

    return LaunchDescription([
        topic_arg, pub_rate_arg,
        sensor_topic_arg, sensor_rate_arg,
        hello_pub, hello_sub,
        sensor_pub, sensor_sub,
        tf_node,
    ])
