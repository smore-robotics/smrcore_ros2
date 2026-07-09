#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "moveit/move_group_interface/move_group_interface.h"
#include "rclcpp/rclcpp.hpp"

class MoveItJointGoalExample : public rclcpp::Node
{
public:
    MoveItJointGoalExample()
        : Node("smrcore_moveit_joint_goal_example",
               rclcpp::NodeOptions().automatically_declare_parameters_from_overrides(true))
    {
    }
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);

    auto node = std::make_shared<MoveItJointGoalExample>();
    auto executor = std::make_shared<rclcpp::executors::SingleThreadedExecutor>();
    executor->add_node(node);
    std::thread spinner([executor]() { executor->spin(); });

    const auto planning_group =
        node->get_parameter_or<std::string>("planning_group", "smri3_arm");
    const auto target = node->get_parameter_or<std::vector<double>>(
        "target", {0.0, -1.5708, -1.5708, 0.0, 0.0, 0.0});

    moveit::planning_interface::MoveGroupInterface move_group(node, planning_group);
    move_group.setMaxVelocityScalingFactor(
        node->get_parameter_or<double>("velocity_scaling", 0.2));
    move_group.setMaxAccelerationScalingFactor(
        node->get_parameter_or<double>("acceleration_scaling", 0.2));

    if (!move_group.setJointValueTarget(target))
    {
        RCLCPP_ERROR(node->get_logger(), "设置 MoveIt 关节目标失败");
        executor->cancel();
        spinner.join();
        rclcpp::shutdown();
        return 1;
    }

    moveit::planning_interface::MoveGroupInterface::Plan plan;
    const bool planned =
        static_cast<bool>(move_group.plan(plan));
    if (!planned)
    {
        RCLCPP_ERROR(node->get_logger(), "MoveIt 规划失败");
        executor->cancel();
        spinner.join();
        rclcpp::shutdown();
        return 1;
    }

    const auto result = move_group.execute(plan);
    if (result != moveit::core::MoveItErrorCode::SUCCESS)
    {
        RCLCPP_ERROR(node->get_logger(), "MoveIt 执行失败");
        executor->cancel();
        spinner.join();
        rclcpp::shutdown();
        return 1;
    }

    RCLCPP_INFO(node->get_logger(), "MoveIt 关节目标执行完成");
    executor->cancel();
    spinner.join();
    rclcpp::shutdown();
    return 0;
}
