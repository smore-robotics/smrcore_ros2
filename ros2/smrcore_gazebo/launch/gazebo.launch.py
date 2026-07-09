from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription, RegisterEventHandler
from launch.event_handlers import OnProcessExit
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import Command, FindExecutable, LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue
from launch_ros.substitutions import FindPackageShare


def generate_launch_description():
    description_pkg = FindPackageShare("smrcore_description")
    gazebo_pkg = FindPackageShare("smrcore_gazebo")
    world = LaunchConfiguration("world")

    robot_description_content = Command(
        [
            PathJoinSubstitution([FindExecutable(name="xacro")]),
            " ",
            PathJoinSubstitution([description_pkg, "urdf", "smri3_real.urdf.xacro"]),
            " ",
            "hardware_backend:=gazebo",
        ]
    )
    robot_description = {
        "robot_description": ParameterValue(robot_description_content, value_type=str)
    }

    gazebo = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            PathJoinSubstitution([FindPackageShare("gazebo_ros"), "launch", "gazebo.launch.py"])
        ),
        launch_arguments={"world": world}.items(),
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
        arguments=["-entity", "smri3", "-topic", "robot_description"],
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
