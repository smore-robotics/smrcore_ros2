from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import PathJoinSubstitution
from launch_ros.substitutions import FindPackageShare


def generate_launch_description():
    bringup_pkg = FindPackageShare("smrcore_bringup")
    moveit_pkg = FindPackageShare("smrcore_moveit_config")

    return LaunchDescription(
        [
            IncludeLaunchDescription(
                PythonLaunchDescriptionSource(
                    PathJoinSubstitution([bringup_pkg, "launch", "robot.launch.py"])
                ),
                launch_arguments={
                    "hardware_backend": "mock",
                    "use_rviz": "false",
                }.items(),
            ),
            IncludeLaunchDescription(
                PythonLaunchDescriptionSource(
                    PathJoinSubstitution([moveit_pkg, "launch", "move_group.launch.py"])
                ),
                launch_arguments={"hardware_backend": "mock"}.items(),
            ),
            IncludeLaunchDescription(
                PythonLaunchDescriptionSource(
                    PathJoinSubstitution([moveit_pkg, "launch", "moveit_rviz.launch.py"])
                )
            ),
        ]
    )
