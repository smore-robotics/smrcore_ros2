<div align="center">

<img src="docs/assets/logo.png" alt="Smartmore Robotics" width="96" />

# smrcore_ros2

**Smartmore 机器人的 ROS 2 集成仓库。**

[![License](https://img.shields.io/badge/License-Apache%202.0-1f6feb.svg)](LICENSE)

[English](README.md) · **简体中文**

</div>

---

**Smartmore 机器人**的公开 ROS 2 集成仓库，提供机器人描述、`ros2_control` 硬件插件、
MoveIt 配置、Gazebo 与 MuJoCo 仿真、SDK action/service 封装和 ROS 用户侧示例。

本仓库是公开包装仓库，不提交 SDK 头文件或库。构建前会通过
`scripts/download.sh` 将发布版 C++ SDK 下载到 `3rdparty/smrcore_sdk`，ROS 2
包通过 `CMAKE_PREFIX_PATH` 查找 `smrcore_sdkConfig.cmake` 和 `smrcore::sdk`。

## 文档

具体安装、启动、控制链路和示例用法下放到对应文档：

- [快速开始](docs/getting-started.md)：环境依赖、SDK 下载、构建和入口选择。
- [架构](docs/architecture.md)：仓库分层、控制链路和模块职责。
- [ros2_control](docs/ros2-control.md)：真机控制链路、launch 参数和
  `FollowJointTrajectory` 使用方式。
- [SDK server](docs/sdk-server.md)：SDK action/service/topic 接口和启动方式。
- [RViz 可视化](docs/rviz.md)：机器人模型查看、mock 后端和 RViz 启动。
- [MoveIt 接入](docs/moveit.md)：MoveIt demo、真机执行链路和 C++ 示例。
- [Gazebo 仿真](docs/gazebo.md)：Gazebo 后端、中文路径处理和仿真验证。
- [MuJoCo 仿真](docs/mujoco.md)：仿真器下载，以及复用真机链路的 ROS 2 启动方式。
- [示例索引](examples/README.md)：`smrcore_examples` 示例入口与对应模块文档跳转。

## 模块

| 模块 | 职责 |
|---|---|
| `ros2/smrcore_msgs` | ROS 2 action、service、message 接口定义 |
| `ros2/smrcore_description` | SMR-i3 xacro、mesh、RViz 配置和关节命名 |
| `ros2/smrcore_hardware` | 基于 `rcore::sdk::Robot` 的 `ros2_control` 硬件插件 |
| `ros2/smrcore_bringup` | 真机或 mock 后端的 controller manager 启动编排 |
| `ros2/smrcore_sdk_server` | 将 SDK 任务运动和状态能力封装为 ROS 2 action/service/topic |
| `ros2/smrcore_moveit_config` | MoveIt 规划组、控制器和 RViz 配置 |
| `ros2/smrcore_gazebo` | Gazebo Classic 仿真启动和模型资源准备 |
| `ros2/smrcore_examples` | ROS 用户视角的控制、状态和 MoveIt 示例 |

## 控制入口

本仓库提供两类会连接机器人并下发运动指令的入口：

- `ros2_control` 链路：推荐作为 ROS 标准实时轨迹控制面，使用
  `joint_trajectory_controller` 和 `FollowJointTrajectory`。
- SDK server 链路：面向 SDK 任务运动和短操作，提供 MoveJ、MoveP、MoveL、MoveC、
  MovePath 等 action，以及 recover、clear_error、IK/FK、状态查询等 service/topic。

实际连接真机时按场景选择其中一类入口，不要默认同时启动两套运动后端控制同一台机器人。

## 安全提示

> 机器人是危险设备。运行任何运动示例前，请确认目标点对当前机器人、工具、负载和工作空间
> 都是安全的，并确认急停可触达、工作空间已清空。

## 许可证

本仓库以 [Apache License 2.0](LICENSE) 发布。预编译 SDK release 制品中的第三方组件
许可证与归属声明随对应 release 压缩包提供。
