# SMRcore ROS 2

SMRcore ROS 2 接入仓库面向三类使用场景：

- 使用 `ros2_control` 和 `joint_trajectory_controller` 接入 ROS 标准轨迹控制。
- 使用 `smrcore_sdk_server` 调用 SDK 任务运动、恢复、清错、IK/FK 和状态查询能力。
- 使用 RViz、MoveIt、Gazebo、MuJoCo 完成模型查看、规划执行和仿真验证。

推荐优先使用 ROS 标准控制面：

```text
MoveIt / FollowJointTrajectory
  -> joint_trajectory_controller
  -> controller_manager
  -> smrcore_hardware
  -> SMRcore SDK JointPassthrough
  -> rcore controller
```

SDK action/service 是独立入口，适合复用 SDK 任务运动接口。连接真机时不要默认同时启动
`ros2_control` 真机链路和 SDK server 运动链路，避免两条运动入口抢占同一台机器人。

## 文档索引

- [快速开始](getting-started.md): 安装依赖、下载 SDK、构建工作区和选择入口。
- [架构](architecture.md): 仓库分层、控制链路和模块职责。
- [ros2_control](ros2-control.md): 真机 bringup、mock 后端、控制器和轨迹示例。
- [SDK server](sdk-server.md): action、service、topic 接口和示例入口。
- [RViz 可视化](rviz.md): 模型查看、RViz 参数和关节命名。
- [MoveIt 接入](moveit.md): MoveIt demo、真机执行链路和 MoveIt 示例。
- [Gazebo 仿真](gazebo.md): Gazebo 后端、资源路径处理和仿真控制。
- [MuJoCo 仿真](mujoco.md): MuJoCo 仿真器下载，以及复用真机链路的 ROS 2 启动方式。
- [完整 ROS 2 体验补齐计划](smrcore-ros2-completion-plan.md): 功能补齐过程记录。
