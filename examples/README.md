# 示例索引

ROS 2 示例节点位于 `ros2/smrcore_examples`。本页只作为示例入口索引，具体启动顺序、
参数说明和注意事项见对应模块文档。

运行示例前先完成构建并加载工作区：

```bash
source install/setup.bash
```

运动示例会下发真实运动指令。连接真机前请确认目标点、速度、负载、工具和工作空间安全。
`ros2_control` 真机后端和 SDK server 运动入口不要默认同时控制同一台机器人。

| 示例 | 入口 | 详细说明 |
|---|---|---|
| `follow_joint_trajectory` | `ros2 launch smrcore_examples follow_joint_trajectory.launch.py` | [ros2_control 发送轨迹](../docs/ros2-control.md#发送轨迹)；Gazebo 中复用见 [Gazebo 仿真](../docs/gazebo.md) |
| `sdk_movej_action` | `ros2 launch smrcore_examples sdk_movej_action.launch.py` | [SDK server / MoveJ 示例](../docs/sdk-server.md#movej-示例) |
| `sdk_movep_action` | `ros2 launch smrcore_examples sdk_movep_action.launch.py` | [SDK server / MoveP 示例](../docs/sdk-server.md#movep-示例) |
| `sdk_movel_action` | `ros2 launch smrcore_examples sdk_movel_action.launch.py` | [SDK server / MoveL 示例](../docs/sdk-server.md#movel-示例) |
| `robot_status` | `ros2 launch smrcore_examples robot_status.launch.py` | [SDK server / 状态订阅示例](../docs/sdk-server.md#状态订阅示例) |
| `recover_clear_error` | `ros2 launch smrcore_examples recover_clear_error.launch.py` | [SDK server / 恢复清错示例](../docs/sdk-server.md#恢复清错示例) |
| `moveit_joint_goal` | `ros2 launch smrcore_examples moveit_joint_goal.launch.py` | [MoveIt C++ 示例](../docs/moveit.md#c-示例) |

示例源码可直接查看 `ros2/smrcore_examples/src/`，对应 launch 文件在
`ros2/smrcore_examples/launch/`。
