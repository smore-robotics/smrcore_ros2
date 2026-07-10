#include <chrono>
#include <cmath>
#include <memory>
#include <string>
#include <vector>

#include "control_msgs/action/follow_joint_trajectory.hpp"
#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "trajectory_msgs/msg/joint_trajectory_point.hpp"

class FollowJointTrajectoryExample : public rclcpp::Node
{
public:
    using FollowJointTrajectory = control_msgs::action::FollowJointTrajectory;
    using GoalHandle = rclcpp_action::ClientGoalHandle<FollowJointTrajectory>;

    FollowJointTrajectoryExample() : Node("smrcore_follow_joint_trajectory_example")
    {
        action_name_ = declare_parameter<std::string>(
            "action_name", "/arm_controller/follow_joint_trajectory");
        target_degrees_ = declare_parameter<std::vector<double>>(
            "target_degrees", {0.0, -90.0, -90.0, 0.0, 0.0, 0.0});
        duration_seconds_ = declare_parameter<double>("duration_seconds", 5.0);
        client_ = rclcpp_action::create_client<FollowJointTrajectory>(
            this, action_name_);
    }

    void Run()
    {
        if (!client_->wait_for_action_server(std::chrono::seconds(5)))
        {
            RCLCPP_ERROR(get_logger(), "等待 action server 超时: %s",
                         action_name_.c_str());
            return;
        }

        if (target_degrees_.size() != kNumJoints)
        {
            RCLCPP_ERROR(get_logger(), "target_degrees 必须包含 6 个关节目标");
            return;
        }

        FollowJointTrajectory::Goal goal;
        goal.trajectory.joint_names = {"base_joint", "shoulder_joint", "elbow_joint",
                                       "wrist1_joint", "wrist2_joint", "wrist3_joint"};

        trajectory_msgs::msg::JointTrajectoryPoint start;
        start.positions = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
        start.velocities = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
        start.time_from_start = rclcpp::Duration::from_seconds(0.0);

        trajectory_msgs::msg::JointTrajectoryPoint target;
        target.positions.reserve(kNumJoints);
        for (const auto degrees : target_degrees_)
        {
            target.positions.push_back(DegreesToRadians(degrees));
        }
        target.velocities = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
        target.time_from_start = rclcpp::Duration::from_seconds(duration_seconds_);

        goal.trajectory.points = {start, target};

        rclcpp_action::Client<FollowJointTrajectory>::SendGoalOptions options;
        options.result_callback = [this](const GoalHandle::WrappedResult &result) {
            if (result.result)
            {
                RCLCPP_INFO(get_logger(),
                            "轨迹执行结束，action=%s，trajectory_error_code=%d",
                            ResultCodeName(result.code), result.result->error_code);
            }
            else
            {
                RCLCPP_INFO(get_logger(), "轨迹执行结束，action=%s",
                            ResultCodeName(result.code));
            }
            rclcpp::shutdown();
        };

        RCLCPP_INFO(get_logger(), "发送 FollowJointTrajectory 示例目标，时长 %.3f s",
                    duration_seconds_);
        client_->async_send_goal(goal, options);
    }

private:
    static constexpr std::size_t kNumJoints = 6;

    static double DegreesToRadians(double degrees)
    {
        return degrees * M_PI / 180.0;
    }

    static const char *ResultCodeName(rclcpp_action::ResultCode code)
    {
        switch (code)
        {
        case rclcpp_action::ResultCode::SUCCEEDED:
            return "SUCCEEDED";
        case rclcpp_action::ResultCode::ABORTED:
            return "ABORTED";
        case rclcpp_action::ResultCode::CANCELED:
            return "CANCELED";
        case rclcpp_action::ResultCode::UNKNOWN:
        default:
            return "UNKNOWN";
        }
    }

    std::string action_name_;
    std::vector<double> target_degrees_;
    double duration_seconds_ = 5.0;
    rclcpp_action::Client<FollowJointTrajectory>::SharedPtr client_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<FollowJointTrajectoryExample>();
    node->Run();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
