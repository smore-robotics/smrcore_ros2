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

## 启动

```bash
source install/setup.bash
ros2 launch smrcore_bringup robot.launch.py robot_ip:=192.168.1.100
```

`robot_ip:=` 为空时表示本机/仿真模式。
