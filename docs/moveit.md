# MoveIt 接入

MoveIt 配置包为 `smrcore_moveit_config`，规划组为 `smri3_arm`，末端 link 为
`tool0_link`。MoveIt controller 指向现有
`/arm_controller/follow_joint_trajectory`，执行链路为：

```text
move_group -> arm_controller -> smrcore_hardware -> JointPassthrough
```

无真机 demo：

```bash
ros2 launch smrcore_moveit_config demo.launch.py
```

真机控制时先启动 bringup，再启动 move_group：

```bash
ros2 launch smrcore_bringup robot.launch.py robot_ip:=192.168.1.10
ros2 launch smrcore_moveit_config move_group.launch.py hardware_backend:=real robot_ip:=192.168.1.10
ros2 launch smrcore_moveit_config moveit_rviz.launch.py
```

MoveIt C++ 示例：

```bash
ros2 launch smrcore_examples moveit_joint_goal.launch.py
```

如果只启动 demo 后运行示例，`move_group` 报
`Start state appears to be in collision`，并显示 `tool0_link` 与
`distal_wrist_link` 碰撞，优先检查 SRDF 里的自碰撞矩阵。真机全 0 位姿本身应为
合法位姿；该报错通常来自末端导出 collision mesh 与腕部 link 的保守碰撞检测，而不是
机械臂真实发生干涉。当前配置已在 SRDF 中禁用这对 link 的自碰撞检测。

该示例需要系统安装 `moveit_ros_planning_interface`。如果本机未安装该依赖，仓库仍可
构建，示例可执行文件会被跳过。
