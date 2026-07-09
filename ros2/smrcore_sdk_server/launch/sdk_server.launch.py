from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    return LaunchDescription(
        [
            DeclareLaunchArgument(
                "robot_ip",
                default_value="",
                description="机器人 IP；空值表示本机或仿真模式",
            ),
            DeclareLaunchArgument(
                "status_rate_hz",
                default_value="20.0",
                description="SDK 状态 topic 发布频率",
            ),
            Node(
                package="smrcore_sdk_server",
                executable="smrcore_sdk_server",
                name="smrcore_sdk_server",
                output="screen",
                parameters=[
                    {"robot_ip": LaunchConfiguration("robot_ip")},
                    {"status_rate_hz": LaunchConfiguration("status_rate_hz")},
                ],
            ),
        ]
    )
