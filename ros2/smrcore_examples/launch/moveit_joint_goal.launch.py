import os

import yaml
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import Command, FindExecutable, LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue
from launch_ros.substitutions import FindPackageShare


def load_yaml(package_name, relative_path):
    package_path = get_package_share_directory(package_name)
    absolute_path = os.path.join(package_path, relative_path)
    with open(absolute_path, "r", encoding="utf-8") as file:
        return yaml.safe_load(file)


def load_text(package_name, relative_path):
    package_path = get_package_share_directory(package_name)
    absolute_path = os.path.join(package_path, relative_path)
    with open(absolute_path, "r", encoding="utf-8") as file:
        return file.read()


def generate_launch_description():
    description_pkg = FindPackageShare("smrcore_description")
    hardware_backend = LaunchConfiguration("hardware_backend")
    robot_ip = LaunchConfiguration("robot_ip")
    model = PathJoinSubstitution([description_pkg, "urdf", "smri3_real.urdf.xacro"])

    robot_description = {
        "robot_description": ParameterValue(
            Command(
                [
                    PathJoinSubstitution([FindExecutable(name="xacro")]),
                    " ",
                    model,
                    " ",
                    "robot_ip:=",
                    robot_ip,
                    " ",
                    "hardware_backend:=",
                    hardware_backend,
                ]
            ),
            value_type=str,
        )
    }
    robot_description_semantic = {
        "robot_description_semantic": load_text(
            "smrcore_moveit_config", "config/smri3.srdf"
        )
    }
    robot_description_kinematics = {
        "robot_description_kinematics": load_yaml(
            "smrcore_moveit_config", "config/kinematics.yaml"
        )
    }
    joint_limits = load_yaml("smrcore_moveit_config", "config/joint_limits.yaml")

    return LaunchDescription(
        [
            DeclareLaunchArgument("robot_ip", default_value=""),
            DeclareLaunchArgument("hardware_backend", default_value="real"),
            Node(
                package="smrcore_examples",
                executable="moveit_joint_goal",
                parameters=[
                    robot_description,
                    robot_description_semantic,
                    robot_description_kinematics,
                    joint_limits,
                    {
                        "planning_group": "smri3_arm",
                        "target": [0.0, -1.5708, -1.5708, 0.0, 0.0, 0.0],
                        "velocity_scaling": 0.2,
                        "acceleration_scaling": 0.2,
                    }
                ],
                output="screen",
            )
        ]
    )
