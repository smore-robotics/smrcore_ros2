from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    return LaunchDescription(
        [
            DeclareLaunchArgument(
                "target_degrees",
                default_value="[0.0, -90.0, -90.0, 0.0, 0.0, 0.0]",
                description="6 轴目标关节角，单位 degree",
            ),
            DeclareLaunchArgument(
                "duration_seconds",
                default_value="5.0",
                description="轨迹执行时长，单位 second",
            ),
            Node(
                package="smrcore_examples",
                executable="follow_joint_trajectory",
                parameters=[
                    {
                        "target_degrees": LaunchConfiguration("target_degrees"),
                        "duration_seconds": LaunchConfiguration("duration_seconds"),
                    }
                ],
                output="screen",
            )
        ]
    )
