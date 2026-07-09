# SMRcore ROS 2

SMRcore 机器人 ROS 2 接入仓库。

本仓库是公开包装仓库，不提交 SDK 头文件或库。`scripts/download.sh` 会把
发布版 C++ SDK 下载到 `3rdparty/smrcore_sdk`，ROS 2 包通过
`CMAKE_PREFIX_PATH` 查找 `smrcore_sdkConfig.cmake` 和 `smrcore::sdk`。

## 包结构

- `ros2/smrcore_hardware`: 基于 `rcore::sdk::Robot` 的 `ros2_control`
  硬件插件。
- `ros2/smrcore_description`: xacro 机器人描述和 ros2_control 配置。
- `ros2/smrcore_bringup`: controller manager 与 robot_state_publisher
  启动编排。
- `ros2/smrcore_examples`: ROS 用户视角示例。

V1 只支持单臂会话。真机 `ros2_control` 后端要求 SDK/控制器版本支持
`Robot::JointPassthrough(q, qd)`；正式后端不会用 `ServoJ()` 代替透明透传。

## 构建

先安装 ROS 2 依赖：

```bash
sudo apt install ros-humble-ros2-control ros-humble-ros2-controllers ros-humble-xacro
```

```bash
./scripts/build.sh
```

## 启动与示例

构建完成后先加载工作区环境：

```bash
source install/setup.bash
```

本仓库目前提供两类控制入口：

- `ros2_control` 实时轨迹链路：通过 `smrcore_hardware` 接入
  `joint_trajectory_controller`，示例发送标准
  `control_msgs/action/FollowJointTrajectory`。
- SDK action/service 链路：通过 `smrcore_sdk_server` 暴露 MoveJ/MoveP/MoveL
  等运动 action，以及 recover、clear_error、状态查询等 service/topic。

两类后端都会连接机器人并下发运动指令，实际使用时建议二选一启动，不要默认同时运行。

### ros2_control 轨迹链路

先启动机器人 bringup：

```bash
ros2 launch smrcore_bringup robot.launch.py robot_ip:=192.168.1.100
```

`robot_ip:=` 为空时表示本机/仿真模式。

`robot.launch.py` 会启动 `ros2_control_node`、`robot_state_publisher`，
并加载 `joint_state_broadcaster` 与 `arm_controller`。启动成功后，在另一个
终端加载环境并发送 FollowJointTrajectory 示例：

```bash
source install/setup.bash
ros2 launch smrcore_examples follow_joint_trajectory.launch.py
```

示例节点默认发送到 `/arm_controller/follow_joint_trajectory`，可通过参数修改目标：

```bash
ros2 launch smrcore_examples follow_joint_trajectory.launch.py \
  target_degrees:="[0.0, -90.0, -90.0, 0.0, 0.0, 0.0]" \
  duration_seconds:=5.0
```

### SDK action/service 链路

MoveJ/MoveP/MoveL 等 SDK 运动指令需要先启动 SDK server：

```bash
source install/setup.bash
ros2 launch smrcore_sdk_server sdk_server.launch.py robot_ip:=192.168.1.100
```

`robot_ip:=` 为空时表示本机/仿真模式。`sdk_server.launch.py` 还支持
`status_rate_hz:=20.0` 配置状态发布频率。

SDK server 启动后，在另一个终端加载环境并运行需要的示例节点：

```bash
# MoveJ action: /smrcore_sdk_server/move_j
ros2 launch smrcore_examples sdk_movej_action.launch.py

# MoveP action: /smrcore_sdk_server/move_p
ros2 launch smrcore_examples sdk_movep_action.launch.py

# MoveL action: /smrcore_sdk_server/move_l
ros2 launch smrcore_examples sdk_movel_action.launch.py

# 订阅 /smrcore_sdk_server/robot_status
ros2 launch smrcore_examples robot_status.launch.py

# 依次调用 /smrcore_sdk_server/recover 和 /smrcore_sdk_server/clear_error
ros2 launch smrcore_examples recover_clear_error.launch.py
```

更多示例说明见 `examples/README.md`。
