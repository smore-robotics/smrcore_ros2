# 示例

ROS 2 示例节点位于 `ros2/smrcore_examples`。运行示例前先完成构建并加载工作区：

```bash
source install/setup.bash
```

运动示例会下发真实运动指令。连接真机前请确认目标点、速度、负载、工具和工作空间安全。

## 示例总览

| 示例 | 后端前提 | 入口 |
|---|---|---|
| `follow_joint_trajectory` | 已启动 `smrcore_bringup` 或 Gazebo | `ros2 launch smrcore_examples follow_joint_trajectory.launch.py` |
| `sdk_movej_action` | 已启动 `smrcore_sdk_server` | `ros2 launch smrcore_examples sdk_movej_action.launch.py` |
| `sdk_movep_action` | 已启动 `smrcore_sdk_server` | `ros2 launch smrcore_examples sdk_movep_action.launch.py` |
| `sdk_movel_action` | 已启动 `smrcore_sdk_server` | `ros2 launch smrcore_examples sdk_movel_action.launch.py` |
| `robot_status` | 已启动 `smrcore_sdk_server` | `ros2 launch smrcore_examples robot_status.launch.py` |
| `recover_clear_error` | 已启动 `smrcore_sdk_server` | `ros2 launch smrcore_examples recover_clear_error.launch.py` |
| `moveit_joint_goal` | 已启动 MoveIt `move_group` | `ros2 launch smrcore_examples moveit_joint_goal.launch.py` |

`sdk_server` 后端和 `ros2_control` 真机后端都会连接机器人并下发运动指令。实际使用时按
场景选择一个入口，不要默认同时运行两套运动后端。

下文表格列出的是节点参数。已在 launch 文件中声明的参数可以通过
`ros2 launch ... 参数名:=参数值` 覆盖；未声明 launch 参数的示例需要使用
`ros2 run ... --ros-args -p 参数名:=参数值` 覆盖。

## ros2_control 轨迹示例

先启动真机 bringup：

```bash
ros2 launch smrcore_bringup robot.launch.py robot_ip:=192.168.1.100
```

无真机时可以启动 mock 后端或 Gazebo 后端：

```bash
ros2 launch smrcore_bringup robot.launch.py hardware_backend:=mock use_rviz:=true
```

发送 `FollowJointTrajectory`：

```bash
ros2 launch smrcore_examples follow_joint_trajectory.launch.py
```

参数：

| 参数 | 默认值 | 说明 |
|---|---|---|
| `action_name` | `/arm_controller/follow_joint_trajectory` | 目标 action |
| `target_degrees` | `[0.0, -90.0, -90.0, 0.0, 0.0, 0.0]` | 6 轴目标关节角，单位 degree |
| `duration_seconds` | `5.0` | 轨迹执行时长 |

示例：

```bash
ros2 launch smrcore_examples follow_joint_trajectory.launch.py \
  target_degrees:="[0.0, -60.0, -80.0, 0.0, 0.0, 0.0]" \
  duration_seconds:=6.0
```

## SDK action 示例

先启动 SDK server：

```bash
ros2 launch smrcore_sdk_server sdk_server.launch.py robot_ip:=192.168.1.100
```

### MoveJ

```bash
ros2 launch smrcore_examples sdk_movej_action.launch.py
```

参数：

| 参数 | 默认值 | 说明 |
|---|---|---|
| `action_name` | `/smrcore_sdk_server/move_j` | 目标 action |
| `target_positions` | `[0.0, -1.5708, -1.5708, 0.0, 0.0, 0.0]` | 6 轴目标关节角，单位 rad |
| `waypoint_name` | `""` | 非空时使用 SDK waypoint 名称 |
| `velocity_scale` | `0.2` | 速度缩放 |

自定义关节目标示例：

```bash
ros2 run smrcore_examples sdk_movej_action --ros-args \
  -p target_positions:="[0.0, -1.2, -1.4, 0.0, 0.0, 0.0]" \
  -p velocity_scale:=0.1
```

### MoveP

```bash
ros2 launch smrcore_examples sdk_movep_action.launch.py
```

参数：

| 参数 | 默认值 | 说明 |
|---|---|---|
| `action_name` | `/smrcore_sdk_server/move_p` | 目标 action |
| `target_xyz_rpy` | `[0.3, 0.0, 0.4, 0.0, 0.0, 0.0]` | 目标位姿，`x y z roll pitch yaw` |
| `frame_id` | `base` | 目标位姿参考坐标系 |
| `velocity_scale` | `0.2` | 速度缩放 |

自定义笛卡尔目标示例：

```bash
ros2 run smrcore_examples sdk_movep_action --ros-args \
  -p target_xyz_rpy:="[0.3, 0.0, 0.35, 0.0, 0.0, 0.0]" \
  -p velocity_scale:=0.1
```

### MoveL

```bash
ros2 launch smrcore_examples sdk_movel_action.launch.py
```

参数与 MoveP 相同，但目标 action 为 `/smrcore_sdk_server/move_l`。

## SDK service/topic 示例

订阅机器人状态：

```bash
ros2 launch smrcore_examples robot_status.launch.py
```

参数：

| 参数 | 默认值 | 说明 |
|---|---|---|
| `topic_name` | `/smrcore_sdk_server/robot_status` | 订阅的状态 topic |

依次调用 recover 和 clear_error：

```bash
ros2 launch smrcore_examples recover_clear_error.launch.py
```

参数：

| 参数 | 默认值 | 说明 |
|---|---|---|
| `recover_service` | `/smrcore_sdk_server/recover` | recover service |
| `clear_error_service` | `/smrcore_sdk_server/clear_error` | clear_error service |

## MoveIt 示例

先启动 MoveIt `move_group`，再运行：

```bash
ros2 launch smrcore_examples moveit_joint_goal.launch.py
```

默认参数在 launch 文件中配置：

| 参数 | 默认值 | 说明 |
|---|---|---|
| `planning_group` | `smri3_arm` | MoveIt planning group |
| `target` | `[0.0, -1.5708, -1.5708, 0.0, 0.0, 0.0]` | 6 轴目标关节角，单位 rad |
| `velocity_scaling` | `0.2` | MoveIt 速度缩放 |
| `acceleration_scaling` | `0.2` | MoveIt 加速度缩放 |
