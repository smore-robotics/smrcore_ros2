# 快速开始

本文只覆盖从空仓库到可启动 ROS 2 节点的最短路径。各模块的详细参数和示例见对应文档。

## 前置条件

- Ubuntu + ROS 2 Humble。
- 可以访问 SMRcore SDK release 资产，或已提前准备好 C++ SDK 发布包。
- 真机控制时，运行机器需要能访问机器人控制器 IP。

安装基础依赖：

```bash
sudo apt install \
  ros-humble-ros2-control \
  ros-humble-ros2-controllers \
  ros-humble-xacro \
  ros-humble-robot-state-publisher \
  ros-humble-joint-state-publisher-gui \
  ros-humble-rviz2
```

使用 MoveIt 或 Gazebo 时再安装对应依赖：

```bash
sudo apt install \
  ros-humble-moveit \
  ros-humble-gazebo-ros-pkgs \
  ros-humble-gazebo-ros2-control
```

## 下载 SDK

`scripts/build.sh` 会在缺少 SDK 时自动调用 `scripts/download.sh`。也可以显式下载：

```bash
SDK_VERSION=0.0.7 ./scripts/download.sh
```

如果仓库根目录存在 `.sdk-version`，`download.sh` 会优先使用其中的版本号。下载完成后，
SDK 会安装到：

```text
3rdparty/smrcore_sdk
```

## 构建

在仓库根目录执行：

```bash
./scripts/build.sh
```

脚本会加载 `/opt/ros/${ROS_DISTRO:-humble}/setup.bash`，并使用 `ros2/` 作为
`colcon` 源码目录。若仓库路径包含中文等非 ASCII 字符，脚本会把 `build` 和 `log`
目录放到 `/tmp` 下，`install` 仍生成在仓库根目录。

等价的手动构建方式：

```bash
source /opt/ros/humble/setup.bash
colcon build \
  --base-paths ros2 \
  --symlink-install \
  --cmake-args -DCMAKE_PREFIX_PATH="$PWD/3rdparty/smrcore_sdk"
```

构建完成后加载工作区：

```bash
source install/setup.bash
```

## 选择入口

真机轨迹控制优先使用 `ros2_control`：

```bash
ros2 launch smrcore_bringup robot.launch.py robot_ip:=192.168.1.100
```

使用 MuJoCo 仿真器时，先启动与 SDK 对应版本的仿真器，再使用完全相同的链路，
但不传 IP 参数：

```bash
ros2 launch smrcore_bringup robot.launch.py
```

详细的下载和启动说明见 [MuJoCo 仿真](mujoco.md)。

无真机时可以使用 mock 后端检查控制器和 RViz：

```bash
ros2 launch smrcore_bringup robot.launch.py hardware_backend:=mock use_rviz:=true
```

需要 SDK 任务运动或短操作接口时启动 SDK server：

```bash
ros2 launch smrcore_sdk_server sdk_server.launch.py robot_ip:=192.168.1.100
```

`robot_ip:=` 为空时表示本机/仿真模式。连接真机时，`ros2_control` 真机链路和
SDK server 运动链路建议二选一启动。

## 下一步

- 标准轨迹控制见 [ros2_control](ros2-control.md)。
- SDK action/service 见 [SDK server](sdk-server.md)。
- 示例入口索引见仓库根目录的 `examples/README.md`。
- MoveIt、Gazebo 和 MuJoCo 分别见 [MoveIt 接入](moveit.md)、[Gazebo 仿真](gazebo.md)
  与 [MuJoCo 仿真](mujoco.md)。
