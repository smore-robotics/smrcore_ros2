#include <chrono>
#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "smrcore_msgs/srv/clear_error.hpp"
#include "smrcore_msgs/srv/recover.hpp"

class RecoverClearErrorExample : public rclcpp::Node
{
public:
    RecoverClearErrorExample() : Node("smrcore_recover_clear_error_example")
    {
        recover_service_ = declare_parameter<std::string>("recover_service", "/smrcore_sdk_server/recover");
        clear_error_service_ = declare_parameter<std::string>("clear_error_service", "/smrcore_sdk_server/clear_error");
        recover_client_ = create_client<smrcore_msgs::srv::Recover>(recover_service_);
        clear_error_client_ = create_client<smrcore_msgs::srv::ClearError>(clear_error_service_);
    }

    void Run()
    {
        CallRecover();
        CallClearError();
        rclcpp::shutdown();
    }

private:
    void CallRecover()
    {
        if (!recover_client_->wait_for_service(std::chrono::seconds(5)))
        {
            RCLCPP_ERROR(get_logger(), "等待 recover service 超时: %s", recover_service_.c_str());
            return;
        }
        auto future = recover_client_->async_send_request(std::make_shared<smrcore_msgs::srv::Recover::Request>());
        if (rclcpp::spin_until_future_complete(shared_from_this(), future) == rclcpp::FutureReturnCode::SUCCESS)
        {
            const auto response = future.get();
            RCLCPP_INFO(get_logger(), "Recover: success=%s error_code=%u message=%s",
                        response->success ? "true" : "false", response->error_code, response->message.c_str());
        }
    }

    void CallClearError()
    {
        if (!clear_error_client_->wait_for_service(std::chrono::seconds(5)))
        {
            RCLCPP_ERROR(get_logger(), "等待 clear_error service 超时: %s", clear_error_service_.c_str());
            return;
        }
        auto future = clear_error_client_->async_send_request(std::make_shared<smrcore_msgs::srv::ClearError::Request>());
        if (rclcpp::spin_until_future_complete(shared_from_this(), future) == rclcpp::FutureReturnCode::SUCCESS)
        {
            const auto response = future.get();
            RCLCPP_INFO(get_logger(), "ClearError: success=%s error_code=%u message=%s",
                        response->success ? "true" : "false", response->error_code, response->message.c_str());
        }
    }

    std::string recover_service_;
    std::string clear_error_service_;
    rclcpp::Client<smrcore_msgs::srv::Recover>::SharedPtr recover_client_;
    rclcpp::Client<smrcore_msgs::srv::ClearError>::SharedPtr clear_error_client_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<RecoverClearErrorExample>();
    node->Run();
    rclcpp::shutdown();
    return 0;
}
