# 示例

ROS 2 节点示例位于 `ros2/smrcore_examples`。

## ros2_control 轨迹链路

先启动 bringup：

```bash
source install/setup.bash
ros2 launch smrcore_bringup robot.launch.py robot_ip:=192.168.1.100
```

`robot_ip:=` 为空时表示本机/仿真模式。bringup 会加载
`joint_state_broadcaster` 和 `arm_controller`，启动后在另一个终端发送一条
简单关节轨迹：

```bash
source install/setup.bash
ros2 launch smrcore_examples follow_joint_trajectory.launch.py
```

示例使用标准 `control_msgs/action/FollowJointTrajectory`，目标 action 默认为
`/arm_controller/follow_joint_trajectory`。

## SDK action/service 链路

MoveJ/MoveP/MoveL/MoveC/MovePath 等 SDK 任务运动通过 `smrcore_sdk_server` 暴露为
ROS 2 action；恢复、清错、使能、IK/FK、状态查询等短操作通过 service 或 topic 暴露。
`sdk_server` 后端和 `ros2_control` 后端建议二选一启动，不要默认同时运行两套运动入口。

启动 SDK server：

```bash
source install/setup.bash
ros2 launch smrcore_sdk_server sdk_server.launch.py robot_ip:=
```

SDK server 启动后，在另一个终端运行需要的示例。发送 SDK MoveJ action：

```bash
source install/setup.bash
ros2 launch smrcore_examples sdk_movej_action.launch.py
```

发送 SDK MoveP action：

```bash
ros2 launch smrcore_examples sdk_movep_action.launch.py
```

发送 SDK MoveL action：

```bash
ros2 launch smrcore_examples sdk_movel_action.launch.py
```

订阅 SDK 状态：

```bash
ros2 launch smrcore_examples robot_status.launch.py
```

调用 Recover + ClearError service：

```bash
ros2 launch smrcore_examples recover_clear_error.launch.py
```
