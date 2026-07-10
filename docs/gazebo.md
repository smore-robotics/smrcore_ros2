# Gazebo 仿真

Gazebo 包为 `smrcore_gazebo`。仿真使用 `hardware_backend:=gazebo`，通过
`gazebo_ros2_control/GazeboSystem` 提供 `ros2_control` 后端，不连接真实 SDK。

## 依赖

```bash
sudo apt install ros-humble-gazebo-ros-pkgs ros-humble-gazebo-ros2-control
```

## 启动

构建并加载工作区后启动空世界并加载 SMR-i3：

```bash
source install/setup.bash
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

Gazebo 和真机后端互斥。仿真启动时不要同时使用 `hardware_backend:=real` 的
`robot.launch.py` 连接真实机器人。

## 中文路径说明

Gazebo Classic 自带的 `package://` 资源解析对非 ASCII 路径支持不好；仓库位于
`~/桌面/...` 这类路径时，模型可能已经出现在左侧模型树里，但 visual/collision mesh
加载失败。

`smrcore_gazebo` 启动时只对 Gazebo 生成一份临时 robot description：

1. 把 `meshes/smri3/*.STL` 复制到 `/tmp/smrcore_gazebo_models/smrcore_smri3_meshes/`
2. 设置 `GAZEBO_MODEL_PATH=/tmp/smrcore_gazebo_models:...`
3. 在生成给 Gazebo 的 URDF 里改写为纯 ASCII 的 `model://smrcore_smri3_meshes/...` mesh URI
4. 把控制器配置复制到 `/tmp/smrcore_smri3_gazebo_controllers.yaml`

源 xacro 仍保留 `package://smrcore_description/...`，因此 RViz、MoveIt 和常规
`robot_state_publisher` 启动不依赖这个复制过程。

如果仍遇到资源加载问题，推荐把仓库映射到纯英文路径后重新构建：

```bash
ln -s ~/桌面/core/smrcore_ros2/smrcore_ros2 ~/smrcore_ros2
cd ~/smrcore_ros2
./scripts/build.sh
```
