# SDK server

`smrcore_sdk_server` 将 SMRcore SDK 的任务运动、短操作和状态查询封装为 ROS 2
action、service 和 topic。该入口适合直接复用 SDK 语义，不经过
`joint_trajectory_controller`。

## 启动

构建并加载工作区后启动：

```bash
source install/setup.bash
ros2 launch smrcore_sdk_server sdk_server.launch.py robot_ip:=192.168.1.100
```

`robot_ip:=` 为空时表示本机/仿真模式。

Launch 参数：

| 参数 | 默认值 | 说明 |
|---|---:|---|
| `robot_ip` | `""` | 传给 `rcore::sdk::Robot::Initialize()` |
| `status_rate_hz` | `20.0` | `robot_status` 和 `motor_status` 发布频率 |

连接真机时不要默认同时启动 `smrcore_bringup` 的 real 后端和 SDK server 运动入口。

## Action

节点名默认为 `smrcore_sdk_server`，action 使用私有命名空间，因此完整名称如下：

| Action | 类型 | 说明 |
|---|---|---|
| `/smrcore_sdk_server/move_j` | `smrcore_msgs/action/MoveJ` | 关节空间运动；支持关节数组或 waypoint 名称 |
| `/smrcore_sdk_server/move_p` | `smrcore_msgs/action/MoveP` | 笛卡尔 PTP 运动 |
| `/smrcore_sdk_server/move_l` | `smrcore_msgs/action/MoveL` | 笛卡尔直线运动 |
| `/smrcore_sdk_server/move_c` | `smrcore_msgs/action/MoveC` | 经由点圆弧运动 |
| `/smrcore_sdk_server/move_path` | `smrcore_msgs/action/MovePath` | 多段笛卡尔路径运动 |

同一时刻只接受一个任务运动 goal。已有任务运行时，新 goal 会被拒绝。取消 action goal
会调用 SDK `StopMotion()`。

示例：

```bash
ros2 launch smrcore_examples sdk_movej_action.launch.py
ros2 launch smrcore_examples sdk_movep_action.launch.py
ros2 launch smrcore_examples sdk_movel_action.launch.py
```

## Service

| Service | 类型 | 说明 |
|---|---|---|
| `/smrcore_sdk_server/enable` | `smrcore_msgs/srv/Enable` | 使能 |
| `/smrcore_sdk_server/disable` | `smrcore_msgs/srv/Disable` | 下使能 |
| `/smrcore_sdk_server/estop` | `smrcore_msgs/srv/EStop` | 急停 |
| `/smrcore_sdk_server/recover` | `smrcore_msgs/srv/Recover` | 恢复 |
| `/smrcore_sdk_server/clear_error` | `smrcore_msgs/srv/ClearError` | 清错 |
| `/smrcore_sdk_server/stop_motion` | `smrcore_msgs/srv/StopMotion` | 停止运动 |
| `/smrcore_sdk_server/pause_motion` | `smrcore_msgs/srv/PauseMotion` | 暂停运动 |
| `/smrcore_sdk_server/continue_motion` | `smrcore_msgs/srv/ContinueMotion` | 继续运动 |
| `/smrcore_sdk_server/set_payload` | `smrcore_msgs/srv/SetPayload` | 设置负载 |
| `/smrcore_sdk_server/clear_payload` | `smrcore_msgs/srv/ClearPayload` | 清除负载 |
| `/smrcore_sdk_server/get_robot_info` | `smrcore_msgs/srv/GetRobotInfo` | 查询机器人信息 |
| `/smrcore_sdk_server/get_motor_status` | `smrcore_msgs/srv/GetMotorStatus` | 查询电机状态 |
| `/smrcore_sdk_server/get_robot_status` | `smrcore_msgs/srv/GetRobotStatus` | 查询机器人状态 |
| `/smrcore_sdk_server/forward_kinematics` | `smrcore_msgs/srv/ForwardKinematics` | 正解 |
| `/smrcore_sdk_server/inverse_kinematics` | `smrcore_msgs/srv/InverseKinematics` | 逆解 |

恢复并清错示例：

```bash
ros2 launch smrcore_examples recover_clear_error.launch.py
```

## Topic

| Topic | 类型 | 说明 |
|---|---|---|
| `/smrcore_sdk_server/robot_status` | `smrcore_msgs/msg/RobotStatus` | 关节、笛卡尔状态、控制模式和错误码 |
| `/smrcore_sdk_server/motor_status` | `smrcore_msgs/msg/MotorStatus` | 电机使能、急停、错误和运行状态 |

状态订阅示例：

```bash
ros2 launch smrcore_examples robot_status.launch.py
```

## 速度参数

MoveJ、MoveP、MoveL、MoveC、MovePath goal 均包含 `velocity_scale`。值为 `0.0`
时，server 会转为 SDK 默认速度参数 `-1.0`；其他值原样传给 SDK。
