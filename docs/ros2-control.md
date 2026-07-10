# ros2_control

`smrcore_hardware/SMRcoreSystemInterface` 实现
`hardware_interface::SystemInterface`，用于把 ROS 2 controller 的关节目标透传到
SMRcore SDK。

## 启动真机链路

构建并加载工作区后启动：

```bash
source install/setup.bash
ros2 launch smrcore_bringup robot.launch.py robot_ip:=192.168.1.100
```

`robot.launch.py` 会启动：

- `controller_manager/ros2_control_node`
- `robot_state_publisher`
- `joint_state_broadcaster`
- `arm_controller`

`arm_controller` 是 `joint_trajectory_controller/JointTrajectoryController`，action
入口为：

```text
/arm_controller/follow_joint_trajectory
```

## Launch 参数

| 参数 | 默认值 | 说明 |
|---|---:|---|
| `robot_ip` | `""` | 传给 `rcore::sdk::Robot::Initialize()`；空值表示本机/仿真模式 |
| `hardware_backend` | `real` | `real`、`mock` 或 `gazebo` |
| `model` | `smri3_real.urdf.xacro` | 机器人 xacro 模型 |
| `use_rviz` | `false` | 是否同时打开 RViz |
| `log_passthrough_commands` | `false` | 是否低频打印 JointPassthrough 下发目标 |

无真机时可使用 mock 后端验证 controller：

```bash
ros2 launch smrcore_bringup robot.launch.py hardware_backend:=mock use_rviz:=true
```

## 发送轨迹

启动 bringup 后，在另一个终端运行标准轨迹示例：

```bash
source install/setup.bash
ros2 launch smrcore_examples follow_joint_trajectory.launch.py
```

示例默认发送 6 轴目标角度 `[0, -90, -90, 0, 0, 0]`，单位为 degree，执行时间 5 秒。
可通过 launch 参数修改：

```bash
ros2 launch smrcore_examples follow_joint_trajectory.launch.py \
  target_degrees:="[0.0, -60.0, -80.0, 0.0, 0.0, 0.0]" \
  duration_seconds:=6.0
```

示例参数详见 `examples/README.md`。

## 生命周期行为

- `on_init`: 校验六关节单臂 hardware description。
- `on_configure`: 初始化 `rcore::sdk::Robot`。
- `on_activate`: 读取最新 SDK 状态，并把 command buffer 对齐到当前位置。
- `read`: 把 SDK `RobotStatus` 映射到 position、velocity、effort state interfaces。
- `write`: 通过 `JointPassthrough` 发送 position 和 velocity command。

`joint_trajectory_controller` 已经完成轨迹插值，因此硬件插件不使用 `ServoJ()`。

## 注意事项

- 真机控制前确认目标点、速度、工具、负载和工作空间安全。
- `hardware_backend:=real` 和 SDK server 运动入口不要默认同时控制同一台机器人。
- Gazebo 仿真使用 `smrcore_gazebo` 启动，不需要手动以 `hardware_backend:=gazebo`
  调用 `robot.launch.py`。
