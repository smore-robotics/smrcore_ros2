# Gazebo 仿真

Gazebo 包为 `smrcore_gazebo`。仿真使用 `hardware_backend:=gazebo`，通过
`gazebo_ros2_control/GazeboSystem` 提供 ros2_control 后端，不连接真实 SDK。

启动空世界并加载 SMR-i3：

```bash
ros2 launch smrcore_gazebo gazebo.launch.py
```

启动后会加载：

- `robot_state_publisher`
- Gazebo empty world
- `spawn_entity.py`
- `joint_state_broadcaster`
- `arm_controller`

可复用标准轨迹示例验证控制链路：

```bash
ros2 launch smrcore_examples follow_joint_trajectory.launch.py
```

Gazebo 和真机后端互斥。仿真启动不要同时使用 `hardware_backend:=real` 的
`robot.launch.py` 连接真实机器人。
