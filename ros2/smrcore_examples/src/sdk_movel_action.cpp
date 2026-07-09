#include <chrono>
#include <cmath>
#include <memory>
#include <string>
#include <vector>

#include "geometry_msgs/msg/pose_stamped.hpp"
#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "smrcore_msgs/action/move_l.hpp"

class SdkMoveLActionExample : public rclcpp::Node
{
public:
    using ActionT = smrcore_msgs::action::MoveL;
    using GoalHandle = rclcpp_action::ClientGoalHandle<ActionT>;

    SdkMoveLActionExample() : Node("smrcore_sdk_movel_action_example")
    {
        action_name_ = declare_parameter<std::string>("action_name", "/smrcore_sdk_server/move_l");
        target_xyz_rpy_ = declare_parameter<std::vector<double>>(
            "target_xyz_rpy", {0.3, 0.0, 0.4, 0, 0.0, 0.0});
        frame_id_ = declare_parameter<std::string>("frame_id", "base");
        velocity_scale_ = declare_parameter<double>("velocity_scale", 0.2);
        client_ = rclcpp_action::create_client<ActionT>(this, action_name_);
    }

    void Run()
    {
        if (!client_->wait_for_action_server(std::chrono::seconds(5)))
        {
            RCLCPP_ERROR(get_logger(), "等待 MoveL action server 超时: %s", action_name_.c_str());
            rclcpp::shutdown();
            return;
        }
        if (target_xyz_rpy_.size() != 6)
        {
            RCLCPP_ERROR(get_logger(), "target_xyz_rpy 必须包含 x y z roll pitch yaw");
            rclcpp::shutdown();
            return;
        }

        ActionT::Goal goal;
        goal.target_pose = MakePose(target_xyz_rpy_);
        goal.velocity_scale = velocity_scale_;

        rclcpp_action::Client<ActionT>::SendGoalOptions options;
        options.result_callback = [this](const GoalHandle::WrappedResult &result) {
            if (result.result)
            {
                RCLCPP_INFO(get_logger(), "MoveL result: success=%s error_code=%u message=%s",
                            result.result->success ? "true" : "false",
                            result.result->error_code, result.result->message.c_str());
            }
            rclcpp::shutdown();
        };

        RCLCPP_INFO(get_logger(), "发送 SDK MoveL action 目标");
        client_->async_send_goal(goal, options);
    }

private:
    geometry_msgs::msg::PoseStamped MakePose(const std::vector<double> &xyz_rpy)
    {
        geometry_msgs::msg::PoseStamped pose;
        pose.header.stamp = now();
        pose.header.frame_id = frame_id_;
        pose.pose.position.x = xyz_rpy[0];
        pose.pose.position.y = xyz_rpy[1];
        pose.pose.position.z = xyz_rpy[2];
        const double roll = xyz_rpy[3];
        const double pitch = xyz_rpy[4];
        const double yaw = xyz_rpy[5];
        const double cy = std::cos(yaw * 0.5);
        const double sy = std::sin(yaw * 0.5);
        const double cp = std::cos(pitch * 0.5);
        const double sp = std::sin(pitch * 0.5);
        const double cr = std::cos(roll * 0.5);
        const double sr = std::sin(roll * 0.5);
        pose.pose.orientation.w = cr * cp * cy + sr * sp * sy;
        pose.pose.orientation.x = sr * cp * cy - cr * sp * sy;
        pose.pose.orientation.y = cr * sp * cy + sr * cp * sy;
        pose.pose.orientation.z = cr * cp * sy - sr * sp * cy;
        return pose;
    }

    std::string action_name_;
    std::vector<double> target_xyz_rpy_;
    std::string frame_id_;
    double velocity_scale_ = 0.2;
    rclcpp_action::Client<ActionT>::SharedPtr client_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<SdkMoveLActionExample>();
    node->Run();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
