# RViz 可视化

真实 SMR-i3 描述资源来自 `rcore/data/model/SMR-i3-URDF-000_V260401`，已迁入
`smrcore_description`：

- `urdf/smri3_real.urdf.xacro`
- `meshes/smri3/*.STL`
- `config/robot.xml`

无机器人时检查模型：

```bash
ros2 launch smrcore_description view_robot.launch.py
```

启动 `ros2_control` 并打开 RViz：

```bash
ros2 launch smrcore_bringup robot.launch.py robot_ip:=192.168.1.10 use_rviz:=true
```

无真机时可用 mock 后端验证 controller 和 RViz：

```bash
ros2 launch smrcore_bringup robot.launch.py hardware_backend:=mock use_rviz:=true
```

关节名统一为 `base_joint`、`shoulder_joint`、`elbow_joint`、`wrist1_joint`、
`wrist2_joint`、`wrist3_joint`。
