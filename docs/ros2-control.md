# ros2_control

`smrcore_hardware/SMRcoreSystemInterface` implements
`hardware_interface::SystemInterface`。

生命周期行为：

- `on_init`: 校验六关节单臂 hardware description。
- `on_configure`: 初始化 `rcore::sdk::Robot`。
- `on_activate`: 读取最新 SDK 状态，并把 command buffer 对齐到当前位置。
- `read`: 把 SDK `RobotState` 映射到 position、velocity、effort state interfaces。
- `write`: 通过 `JointPassthrough` 发送 position 和 velocity command。

后端不使用 `ServoJ()`，因为 ROS 2 trajectory controller 已经完成轨迹插值。
