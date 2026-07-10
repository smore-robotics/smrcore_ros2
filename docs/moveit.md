# MoveIt 接入

MoveIt 配置包为 `smrcore_moveit_config`，规划组为 `smri3_arm`，末端 link 为
`tool0_link`。MoveIt controller 指向现有
`/arm_controller/follow_joint_trajectory`，执行链路为：

```text
move_group -> arm_controller -> smrcore_hardware -> JointPassthrough
```

## 依赖

```bash
sudo apt install ros-humble-moveit
```

`smrcore_examples` 中的 MoveIt C++ 示例需要 `moveit_ros_planning_interface`。如果本机
未安装该依赖，仓库仍可构建，示例可执行文件会被跳过。

## 无真机 demo

```bash
source install/setup.bash
ros2 launch smrcore_moveit_config demo.launch.py
```

该入口用于检查 SRDF、运动学、规划配置和 RViz 交互，不连接真实机器人。

## 真机执行

先启动 `ros2_control` 真机链路：

```bash
ros2 launch smrcore_bringup robot.launch.py robot_ip:=192.168.1.100
```

再启动 MoveIt：

```bash
ros2 launch smrcore_moveit_config move_group.launch.py hardware_backend:=real robot_ip:=192.168.1.100
ros2 launch smrcore_moveit_config moveit_rviz.launch.py
```

MoveIt 规划成功后会通过 `/arm_controller/follow_joint_trajectory` 执行轨迹。

## C++ 示例

启动 move_group 后运行：

```bash
ros2 launch smrcore_examples moveit_joint_goal.launch.py
```

示例默认规划组为 `smri3_arm`，目标关节角为
`[0.0, -1.5708, -1.5708, 0.0, 0.0, 0.0]`，速度和加速度缩放均为 `0.2`。

## 自碰撞说明

如果运行 demo 或示例时 `move_group` 报
`Start state appears to be in collision`，并显示 `tool0_link` 与
`distal_wrist_link` 碰撞，优先检查 SRDF 里的自碰撞矩阵。真机全 0 位姿本身应为合法
位姿；该报错通常来自末端导出 collision mesh 与腕部 link 的保守碰撞检测，而不一定是
机械臂真实发生干涉。当前配置已在 SRDF 中禁用这对 link 的自碰撞检测。
