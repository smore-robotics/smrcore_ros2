from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    return LaunchDescription(
        [
            Node(
                package="smrcore_examples",
                executable="robot_status",
                output="screen",
            ),
        ]
    )
