#include <array>
#include <chrono>
#include <memory>
#include <string>
#include <vector>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "smrcore_msgs/action/move_j.hpp"

class SdkMoveJActionExample : public rclcpp::Node
{
public:
    using MoveJ = smrcore_msgs::action::MoveJ;
    using GoalHandle = rclcpp_action::ClientGoalHandle<MoveJ>;

    SdkMoveJActionExample() : Node("smrcore_sdk_movej_action_example")
    {
        action_name_ = declare_parameter<std::string>("action_name", "/smrcore_sdk_server/move_j");
        target_positions_ = declare_parameter<std::vector<double>>(
            "target_positions", {0.0, -1.5708, -1.5708, 0.0, 0.0, 0.0});
        waypoint_name_ = declare_parameter<std::string>("waypoint_name", "");
        velocity_scale_ = declare_parameter<double>("velocity_scale", 0.2);
        client_ = rclcpp_action::create_client<MoveJ>(this, action_name_);
    }

    void Run()
    {
        if (!client_->wait_for_action_server(std::chrono::seconds(5)))
        {
            RCLCPP_ERROR(get_logger(), "等待 MoveJ action server 超时: %s", action_name_.c_str());
            rclcpp::shutdown();
            return;
        }
        if (waypoint_name_.empty() && target_positions_.size() != 6)
        {
            RCLCPP_ERROR(get_logger(), "target_positions 必须包含 6 个关节角，单位 rad");
            rclcpp::shutdown();
            return;
        }

        MoveJ::Goal goal;
        goal.waypoint_name = waypoint_name_;
        for (std::size_t i = 0; i < target_positions_.size() && i < 6; ++i)
        {
            goal.target_positions[i] = target_positions_[i];
        }
        goal.velocity_scale = velocity_scale_;

        rclcpp_action::Client<MoveJ>::SendGoalOptions options;
        options.feedback_callback = [this](GoalHandle::SharedPtr, const std::shared_ptr<const MoveJ::Feedback> feedback) {
            RCLCPP_INFO_THROTTLE(get_logger(), *get_clock(), 1000,
                                 "MoveJ feedback: task_id=%d status=%d progress=%.2f",
                                 feedback->task_id, feedback->status, feedback->progress);
        };
        options.result_callback = [this](const GoalHandle::WrappedResult &result) {
            if (result.result)
            {
                RCLCPP_INFO(get_logger(), "MoveJ result: success=%s error_code=%u message=%s",
                            result.result->success ? "true" : "false",
                            result.result->error_code, result.result->message.c_str());
            }
            rclcpp::shutdown();
        };

        RCLCPP_INFO(get_logger(), "发送 SDK MoveJ action 目标");
        client_->async_send_goal(goal, options);
    }

private:
    std::string action_name_;
    std::vector<double> target_positions_;
    std::string waypoint_name_;
    double velocity_scale_ = 0.2;
    rclcpp_action::Client<MoveJ>::SharedPtr client_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<SdkMoveJActionExample>();
    node->Run();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
