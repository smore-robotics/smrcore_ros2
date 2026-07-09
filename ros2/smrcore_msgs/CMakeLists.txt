cmake_minimum_required(VERSION 3.8)
project(smrcore_msgs)

find_package(ament_cmake REQUIRED)
find_package(action_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(rosidl_default_generators REQUIRED)
find_package(std_msgs REQUIRED)

rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/CartesianWaypoint.msg"
  "msg/MotorStatus.msg"
  "msg/RobotStatus.msg"
  "action/MoveC.action"
  "action/MoveJ.action"
  "action/MoveL.action"
  "action/MoveP.action"
  "action/MovePath.action"
  "srv/ClearError.srv"
  "srv/ClearPayload.srv"
  "srv/ContinueMotion.srv"
  "srv/Disable.srv"
  "srv/EStop.srv"
  "srv/Enable.srv"
  "srv/ForwardKinematics.srv"
  "srv/GetMotorStatus.srv"
  "srv/GetRobotInfo.srv"
  "srv/GetRobotStatus.srv"
  "srv/InverseKinematics.srv"
  "srv/PauseMotion.srv"
  "srv/Recover.srv"
  "srv/SetPayload.srv"
  "srv/StopMotion.srv"
  DEPENDENCIES action_msgs geometry_msgs std_msgs
)

ament_export_dependencies(rosidl_default_runtime)
ament_package()
