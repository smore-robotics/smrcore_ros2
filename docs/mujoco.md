# MuJoCo 仿真

SMRcore 支持通过 MuJoCo 仿真器运行 SDK 和 ROS 2 控制链路。MuJoCo 仿真器由 SDK
在本机连接，因此 ROS 2 侧的启动方式与真机相同；区别仅在于不传 `robot_ip`，使其使用
默认空值。

## 下载并启动仿真器

从 [SMRcore SDK Releases](https://github.com/smore-robotics/smrcore_sdk/releases)
下载与当前 SDK **对应版本**的 MuJoCo 仿真器，并按照该发布包中的说明在本机启动仿真器。

启动 ROS 2 节点前，确认 MuJoCo 仿真器已经运行。仿真器版本必须与
`3rdparty/smrcore_sdk` 中使用的 SDK 版本匹配。

## 启动 ROS 2 控制链路

MuJoCo 运行后，使用与真机相同的 `ros2_control` bringup 命令，但不要传入机器人 IP：

```bash
source install/setup.bash
ros2 launch smrcore_bringup robot.launch.py
```

随后可按真机相同方式发送轨迹：

```bash
ros2 launch smrcore_examples follow_joint_trajectory.launch.py
```

## 启动 SDK server 链路

如需使用 SDK 的 MoveJ、MoveP、MoveL 等 action/service，同样不要传入 `robot_ip`：

```bash
source install/setup.bash
ros2 launch smrcore_sdk_server sdk_server.launch.py
```

`ros2_control` 和 SDK server 都可连接 MuJoCo 仿真器。它们与真机时一样都是运动入口，
默认不要同时启动，以免同时向同一仿真器下发运动指令。

## 与其他无真机场景的区别

- MuJoCo：运行 SDK 的本机仿真器，不传 `robot_ip`（使用默认空值），ROS 2 启动链路与真机相同。
- Gazebo：使用 `smrcore_gazebo` 的独立 Gazebo Classic 后端。
- mock：仅在 ROS 2 内部回显关节状态和命令，不运行物理仿真器。
