# RViz 可视化

`smrcore_description` 提供 SMR-i3 的 xacro、mesh、关节配置和 RViz 配置。真实模型资源
来自 `rcore/data/model/SMR-i3-URDF-000_V260401`，已迁入：

- `urdf/smri3_real.urdf.xacro`
- `meshes/smri3/*.STL`
- `config/robot.xml`
- `rviz/view_robot.rviz`

## 只查看模型

构建并加载工作区后运行：

```bash
source install/setup.bash
ros2 launch smrcore_description view_robot.launch.py
```

该入口适合在不连接真机的情况下检查 mesh、link、joint 和 TF。

如果启动时报 `package 'joint_state_publisher_gui' not found`，先安装：

```bash
sudo apt install ros-humble-robot-state-publisher ros-humble-joint-state-publisher-gui ros-humble-rviz2
```

## 随 bringup 打开 RViz

连接真机并打开 RViz：

```bash
ros2 launch smrcore_bringup robot.launch.py robot_ip:=192.168.1.100 use_rviz:=true
```

无真机时用 mock 后端：

```bash
ros2 launch smrcore_bringup robot.launch.py hardware_backend:=mock use_rviz:=true
```

## 关节命名

当前 SMR-i3 六轴关节名统一为：

```text
base_joint
shoulder_joint
elbow_joint
wrist1_joint
wrist2_joint
wrist3_joint
```

`smrcore_hardware`、controller 配置、MoveIt 配置和示例节点都按这组关节名工作。
