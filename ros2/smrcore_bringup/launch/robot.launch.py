from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, RegisterEventHandler
from launch.event_handlers import OnProcessExit
from launch.substitutions import Command, FindExecutable, LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare


def generate_launch_description():
    robot_ip = LaunchConfiguration("robot_ip")

    description_pkg = FindPackageShare("smrcore_description")
    hardware_pkg = FindPackageShare("smrcore_hardware")

    robot_description_content = Command(
        [
            PathJoinSubstitution([FindExecutable(name="xacro")]),
            " ",
            PathJoinSubstitution([description_pkg, "urdf", "smri3.urdf.xacro"]),
            " ",
            "robot_ip:=",
            robot_ip,
        ]
    )
    robot_description = {"robot_description": robot_description_content}

    controllers_file = PathJoinSubstitution(
        [hardware_pkg, "config", "smri3_controllers.yaml"]
    )

    control_node = Node(
        package="controller_manager",
        executable="ros2_control_node",
        parameters=[robot_description, controllers_file],
        output="screen",
    )

    state_publisher = Node(
        package="robot_state_publisher",
        executable="robot_state_publisher",
        parameters=[robot_description],
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
                "robot_ip",
                default_value="",
                description="机器人 IP；空值表示本机或仿真模式",
            ),
            DeclareLaunchArgument(
                "use_rviz",
                default_value="false",
                description="预留参数，V1 暂不自动启动 RViz",
            ),
            control_node,
            state_publisher,
            joint_state_spawner,
            RegisterEventHandler(
                event_handler=OnProcessExit(
                    target_action=joint_state_spawner,
                    on_exit=[arm_controller_spawner],
                )
            ),
        ]
    )
