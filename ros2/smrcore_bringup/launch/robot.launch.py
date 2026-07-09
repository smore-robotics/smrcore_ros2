from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, RegisterEventHandler
from launch.conditions import IfCondition
from launch.event_handlers import OnProcessExit
from launch.substitutions import Command, FindExecutable, LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue
from launch_ros.substitutions import FindPackageShare


def generate_launch_description():
    robot_ip = LaunchConfiguration("robot_ip")
    hardware_backend = LaunchConfiguration("hardware_backend")
    model = LaunchConfiguration("model")
    log_passthrough_commands = LaunchConfiguration("log_passthrough_commands")
    use_rviz = LaunchConfiguration("use_rviz")

    description_pkg = FindPackageShare("smrcore_description")
    hardware_pkg = FindPackageShare("smrcore_hardware")

    robot_description_content = Command(
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
            " ",
            "log_passthrough_commands:=",
            log_passthrough_commands,
        ]
    )
    robot_description = {
        "robot_description": ParameterValue(robot_description_content, value_type=str)
    }

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

    rviz_node = Node(
        package="rviz2",
        executable="rviz2",
        arguments=[
            "-d",
            PathJoinSubstitution([description_pkg, "rviz", "view_robot.rviz"]),
        ],
        condition=IfCondition(use_rviz),
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
                "hardware_backend",
                default_value="real",
                description="ros2_control 后端：real、mock 或 gazebo",
            ),
            DeclareLaunchArgument(
                "model",
                default_value=PathJoinSubstitution(
                    [description_pkg, "urdf", "smri3_real.urdf.xacro"]
                ),
                description="机器人 xacro 模型文件",
            ),
            DeclareLaunchArgument(
                "use_rviz",
                default_value="false",
                description="是否自动启动 RViz",
            ),
            DeclareLaunchArgument(
                "log_passthrough_commands",
                default_value="false",
                description="是否低频打印 JointPassthrough 下发目标",
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
            rviz_node,
        ]
    )
