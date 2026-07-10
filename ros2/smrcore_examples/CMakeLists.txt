cmake_minimum_required(VERSION 3.8)
project(smrcore_examples)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(control_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(moveit_ros_planning_interface QUIET)
find_package(rclcpp REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(smrcore_msgs REQUIRED)
find_package(trajectory_msgs REQUIRED)

add_executable(follow_joint_trajectory src/follow_joint_trajectory.cpp)
target_compile_features(follow_joint_trajectory PUBLIC cxx_std_17)
ament_target_dependencies(follow_joint_trajectory
  control_msgs
  rclcpp
  rclcpp_action
  trajectory_msgs
)

if(moveit_ros_planning_interface_FOUND)
  add_executable(moveit_joint_goal src/moveit_joint_goal.cpp)
  target_compile_features(moveit_joint_goal PUBLIC cxx_std_17)
  ament_target_dependencies(moveit_joint_goal
    moveit_ros_planning_interface
    rclcpp
  )
else()
  message(WARNING "moveit_ros_planning_interface not found; skipping moveit_joint_goal example")
endif()

foreach(example_target
  sdk_movej_action
  sdk_movep_action
  sdk_movel_action
  robot_status
  recover_clear_error
)
  add_executable(${example_target} src/${example_target}.cpp)
  target_compile_features(${example_target} PUBLIC cxx_std_17)
  ament_target_dependencies(${example_target}
    geometry_msgs
    rclcpp
    rclcpp_action
    smrcore_msgs
  )
endforeach()

set(example_targets
  follow_joint_trajectory
  sdk_movej_action
  sdk_movep_action
  sdk_movel_action
  robot_status
  recover_clear_error
)
if(TARGET moveit_joint_goal)
  list(APPEND example_targets moveit_joint_goal)
endif()

install(TARGETS ${example_targets} DESTINATION lib/${PROJECT_NAME})
install(DIRECTORY launch DESTINATION share/${PROJECT_NAME})

ament_package()
