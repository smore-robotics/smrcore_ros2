# SMRcore ROS 2

SMRcore ROS 2 接入优先使用 ROS 标准控制面：

```text
MoveIt / FollowJointTrajectory
  -> joint_trajectory_controller
  -> controller_manager
  -> smrcore_hardware
  -> SMRcore SDK JointPassthrough
  -> rcore controller
```

SDK 任务运动 action/service 会作为独立后端规划，不和 `ros2_control` 真机链路在
同一个 bringup 实例里同时暴露，避免出现两条运动入口抢占同一台机器人。

## 实施文档

- [RViz 可视化](rviz.md): 使用真实 SMR-i3 模型或 mock ros2_control 后端检查模型。
- [MoveIt 接入](moveit.md): 启动 MoveIt demo、连接真机控制器和运行 MoveIt 示例。
- [Gazebo 仿真](gazebo.md): 使用 Gazebo 后端加载 SMR-i3 并复用同一 arm_controller。
- [完整 ROS 2 体验补齐计划](smrcore-ros2-completion-plan.md): 对照
  `rokae_ros2` 后补齐 MoveIt、RViz、Gazebo 和真实 SMR-i3 描述资源的实施路线。
