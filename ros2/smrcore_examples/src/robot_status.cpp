#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "smrcore_msgs/msg/robot_status.hpp"

class RobotStatusExample : public rclcpp::Node
{
public:
    RobotStatusExample() : Node("smrcore_robot_status_example")
    {
        topic_name_ = declare_parameter<std::string>("topic_name", "/smrcore_sdk_server/robot_status");
        subscription_ = create_subscription<smrcore_msgs::msg::RobotStatus>(
            topic_name_, rclcpp::SystemDefaultsQoS(),
            [this](const smrcore_msgs::msg::RobotStatus::SharedPtr msg) {
                RCLCPP_INFO(get_logger(),
                            "connected=%s q=[%.3f %.3f %.3f %.3f %.3f %.3f] cartesian_valid=%s errors=%zu warnings=%zu",
                            msg->connected ? "true" : "false",
                            msg->positions[0], msg->positions[1], msg->positions[2],
                            msg->positions[3], msg->positions[4], msg->positions[5],
                            msg->cartesian_valid ? "true" : "false",
                            msg->latched_error_codes.size(), msg->active_warning_codes.size());
            });
    }

private:
    std::string topic_name_;
    rclcpp::Subscription<smrcore_msgs::msg::RobotStatus>::SharedPtr subscription_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<RobotStatusExample>());
    rclcpp::shutdown();
    return 0;
}
