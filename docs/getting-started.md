# 快速开始

先安装 ROS 2 Humble 和 `ros2_control` 相关包，然后在仓库根目录构建：

```bash
sudo apt install ros-humble-ros2-control ros-humble-ros2-controllers ros-humble-xacro
```

```bash
./scripts/build.sh
```

启动真机控制链路：

```bash
source install/setup.bash
ros2 launch smrcore_bringup robot.launch.py robot_ip:=192.168.1.100
```

`robot_ip` 参数会传给 `rcore::sdk::Robot::Initialize()`。空值表示本机/仿真模式。
