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
