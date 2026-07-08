# 架构

本仓库把 SDK 发布包和 ROS 2 源码包分开管理。

- SDK release 资产下载到 `3rdparty/smrcore_sdk`。
- ROS 2 包统一放在 `ros2/`。
- `smrcore_hardware` 是 V1 唯一实时指令桥接。
- SDK task/action wrapper 后续放在硬件插件外部，保持 `ros2_control` 路径职责单一。

硬件插件同时导出 position 和 velocity command interfaces，并把 ROS controller
当前周期目标通过 `Robot::JointPassthrough(positions, velocities)` 透传给 rcore。
