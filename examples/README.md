# 示例

ROS 2 节点示例位于 `ros2/smrcore_examples`。

启动 bringup 后可以发送一条简单关节轨迹：

```bash
ros2 launch smrcore_examples follow_joint_trajectory.launch.py
```

示例使用标准 `control_msgs/action/FollowJointTrajectory`，目标 action 默认为
`/arm_controller/follow_joint_trajectory`。
