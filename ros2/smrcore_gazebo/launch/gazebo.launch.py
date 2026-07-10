import os
import shutil

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import (
    DeclareLaunchArgument,
    IncludeLaunchDescription,
    OpaqueFunction,
    RegisterEventHandler,
    SetEnvironmentVariable,
)
from launch.event_handlers import OnProcessExit
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import (
    Command,
    EnvironmentVariable,
    FindExecutable,
    LaunchConfiguration,
    PathJoinSubstitution,
)
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue
from launch_ros.substitutions import FindPackageShare


GAZEBO_CONTROLLERS_FILE = "/tmp/smrcore_smri3_gazebo_controllers.yaml"
GAZEBO_MODEL_CACHE_ROOT = "/tmp/smrcore_gazebo_models"


def prepare_gazebo_controllers_file(_context):
    source = os.path.join(
        get_package_share_directory("smrcore_hardware"),
        "config",
        "smri3_gazebo_controllers.yaml",
    )
    shutil.copyfile(source, GAZEBO_CONTROLLERS_FILE)
    return []


def generate_launch_description():
    description_share = get_package_share_directory("smrcore_description")
    gazebo_pkg = FindPackageShare("smrcore_gazebo")
    world = LaunchConfiguration("world")
    gui = LaunchConfiguration("gui")
    urdf_xacro = os.path.join(description_share, "urdf", "smri3_real.urdf.xacro")
    existing_model_path = os.environ.get("GAZEBO_MODEL_PATH", "")
    os.environ["GAZEBO_MODEL_PATH"] = (
        GAZEBO_MODEL_CACHE_ROOT
        if not existing_model_path
        else f"{GAZEBO_MODEL_CACHE_ROOT}:{existing_model_path}"
    )

    robot_description_content = Command(
        [
            PathJoinSubstitution([FindExecutable(name="python3")]),
            " ",
            PathJoinSubstitution([gazebo_pkg, "scripts", "prepare_robot_description.py"]),
            " ",
            urdf_xacro,
            " ",
            "hardware_backend:=gazebo",
            " ",
            "gazebo_controllers_file:=",
            GAZEBO_CONTROLLERS_FILE,
        ]
    )
    robot_description = {
        "robot_description": ParameterValue(robot_description_content, value_type=str)
    }

    gazebo = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            PathJoinSubstitution([FindPackageShare("gazebo_ros"), "launch", "gazebo.launch.py"])
        ),
        launch_arguments={"world": world, "gui": gui}.items(),
    )

    state_publisher = Node(
        package="robot_state_publisher",
        executable="robot_state_publisher",
        parameters=[robot_description],
        output="screen",
    )

    spawn_entity = Node(
        package="gazebo_ros",
        executable="spawn_entity.py",
        arguments=["-entity", "smri3", "-topic", "robot_description", "-z", "0.008"],
        output="screen",
    )

    joint_state_spawner = Node(
        package="controller_manager",
        executable="spawner",
        arguments=["joint_state_broadcaster", "--controller-manager", "/controller_manager"],
        output="screen",
    )

    arm_controller_spawner = Node(
        package="controller_manager",
        executable="spawner",
        arguments=["arm_controller", "--controller-manager", "/controller_manager"],
        output="screen",
    )

    return LaunchDescription(
        [
            DeclareLaunchArgument(
                "world",
                default_value=PathJoinSubstitution([gazebo_pkg, "worlds", "empty.world"]),
            ),
            DeclareLaunchArgument(
                "gui",
                default_value="true",
                description='Set to "false" to run Gazebo server without gzclient.',
            ),
            SetEnvironmentVariable(
                name="GAZEBO_MODEL_PATH",
                value=[
                    GAZEBO_MODEL_CACHE_ROOT,
                    ":",
                    EnvironmentVariable("GAZEBO_MODEL_PATH", default_value=""),
                ],
            ),
            OpaqueFunction(function=prepare_gazebo_controllers_file),
            gazebo,
            state_publisher,
            spawn_entity,
            RegisterEventHandler(
                OnProcessExit(target_action=spawn_entity, on_exit=[joint_state_spawner])
            ),
            RegisterEventHandler(
                OnProcessExit(
                    target_action=joint_state_spawner,
                    on_exit=[arm_controller_spawner],
                )
            ),
        ]
    )
