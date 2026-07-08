from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import PathJoinSubstitution
from launch_ros.substitutions import FindPackageShare


def generate_launch_description():
    # 硬件包内保留一个入口，实际编排统一委托给 smrcore_bringup。
    return LaunchDescription(
        [
            IncludeLaunchDescription(
                PythonLaunchDescriptionSource(
                    PathJoinSubstitution(
                        [
                            FindPackageShare("smrcore_bringup"),
                            "launch",
                            "robot.launch.py",
                        ]
                    )
                )
            )
        ]
    )
