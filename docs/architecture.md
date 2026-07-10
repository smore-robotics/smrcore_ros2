# 架构

本仓库把 SDK release 资产和 ROS 2 源码包分开管理：

- SDK release 资产下载到 `3rdparty/smrcore_sdk`。
- ROS 2 包统一放在 `ros2/`。
- 构建产物输出到仓库根目录的 `install/`。

## 模块职责

| 模块 | 职责 |
|---|---|
| `smrcore_msgs` | 定义 SDK server 使用的 action、service 和状态 message |
| `smrcore_description` | 提供 SMR-i3 xacro、mesh、关节配置和 RViz 模型查看入口 |
| `smrcore_hardware` | 实现 `hardware_interface::SystemInterface`，桥接 SDK 与 `ros2_control` |
| `smrcore_bringup` | 启动 `ros2_control_node`、`robot_state_publisher` 和控制器 spawner |
| `smrcore_sdk_server` | 将 SDK 任务运动和短操作封装为 ROS 2 action/service/topic |
| `smrcore_moveit_config` | 提供 MoveIt SRDF、运动学、规划和 controller 配置 |
| `smrcore_gazebo` | 使用 Gazebo Classic 和 `gazebo_ros2_control` 提供仿真后端 |
| `smrcore_examples` | 提供 ROS 用户侧示例节点 |

## ros2_control 链路

`smrcore_hardware/SMRcoreSystemInterface` 是实时轨迹链路中的硬件插件：

```text
FollowJointTrajectory / MoveIt
  -> joint_trajectory_controller
  -> controller_manager
  -> SMRcoreSystemInterface
  -> rcore::sdk::Robot::JointPassthrough
```

硬件插件同时导出 position 和 velocity command interfaces。`joint_trajectory_controller`
负责轨迹插值，硬件插件在 `write()` 周期把当前目标通过
`Robot::JointPassthrough(positions, velocities)` 透传给控制器。

该链路不使用 `ServoJ()` 替代透明透传。

## SDK server 链路

`smrcore_sdk_server` 独立持有一个 SDK session，并暴露：

- MoveJ、MoveP、MoveL、MoveC、MovePath action。
- enable、disable、recover、clear_error、stop_motion、IK/FK 等 service。
- robot_status、motor_status topic。

该链路适合直接复用 SDK 任务运动语义，不经过 `joint_trajectory_controller`。它和
`ros2_control` 真机链路都会连接机器人并下发运动指令，实际部署时应按场景选择一个入口。

## 后端模式

`smrcore_bringup` 通过 `hardware_backend` 区分后端：

- `real`: 使用 SDK 连接真实机器人。
- `mock`: 不连接真机，用于检查模型、controller 和 RViz。
- `gazebo`: 由 Gazebo launch 使用，通过 `gazebo_ros2_control/GazeboSystem` 驱动仿真模型。

V1 面向单臂 SMR-i3，会校验六关节 hardware description。
