#include <array>
#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "geometry_msgs/msg/pose_stamped.hpp"
#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "sdk_session.hpp"
#include "smrcore_msgs/action/move_c.hpp"
#include "smrcore_msgs/action/move_j.hpp"
#include "smrcore_msgs/action/move_l.hpp"
#include "smrcore_msgs/action/move_p.hpp"
#include "smrcore_msgs/action/move_path.hpp"
#include "smrcore_msgs/msg/cartesian_waypoint.hpp"
#include "smrcore_msgs/msg/motor_status.hpp"
#include "smrcore_msgs/msg/robot_status.hpp"
#include "smrcore_msgs/srv/clear_error.hpp"
#include "smrcore_msgs/srv/clear_payload.hpp"
#include "smrcore_msgs/srv/continue_motion.hpp"
#include "smrcore_msgs/srv/disable.hpp"
#include "smrcore_msgs/srv/enable.hpp"
#include "smrcore_msgs/srv/e_stop.hpp"
#include "smrcore_msgs/srv/forward_kinematics.hpp"
#include "smrcore_msgs/srv/get_motor_status.hpp"
#include "smrcore_msgs/srv/get_robot_info.hpp"
#include "smrcore_msgs/srv/get_robot_status.hpp"
#include "smrcore_msgs/srv/inverse_kinematics.hpp"
#include "smrcore_msgs/srv/pause_motion.hpp"
#include "smrcore_msgs/srv/recover.hpp"
#include "smrcore_msgs/srv/set_payload.hpp"
#include "smrcore_msgs/srv/stop_motion.hpp"

namespace
{
constexpr double kDefaultVelocityScale = -1.0;

} // namespace

namespace smrcore_sdk_server
{

PoseData ToPoseData(const geometry_msgs::msg::Pose &pose)
{
    PoseData out;
    out.position = {pose.position.x, pose.position.y, pose.position.z};
    out.orientation = {pose.orientation.x, pose.orientation.y,
                       pose.orientation.z, pose.orientation.w};
    return out;
}

geometry_msgs::msg::Pose ToRosPose(const PoseData &pose)
{
    geometry_msgs::msg::Pose out;
    out.position.x = pose.position[0];
    out.position.y = pose.position[1];
    out.position.z = pose.position[2];
    out.orientation.x = pose.orientation[0];
    out.orientation.y = pose.orientation[1];
    out.orientation.z = pose.orientation[2];
    out.orientation.w = pose.orientation[3];
    return out;
}

class SdkServerNode : public rclcpp::Node
{
public:
    using MoveJ = smrcore_msgs::action::MoveJ;
    using MoveP = smrcore_msgs::action::MoveP;
    using MoveL = smrcore_msgs::action::MoveL;
    using MoveC = smrcore_msgs::action::MoveC;
    using MovePath = smrcore_msgs::action::MovePath;

    SdkServerNode() : Node("smrcore_sdk_server")
    {
        robot_ip_ = declare_parameter<std::string>("robot_ip", "");
        const double status_rate_hz = declare_parameter<double>("status_rate_hz", 20.0);
        robot_ = LoadSdkSession();
        if (!robot_->Initialize(robot_ip_))
        {
            robot_.reset();
            throw std::runtime_error("SMRcore SDK 初始化失败: robot_ip='" + robot_ip_ + "'");
        }

        robot_status_pub_ = create_publisher<smrcore_msgs::msg::RobotStatus>(
            "~/robot_status", rclcpp::SystemDefaultsQoS());
        motor_status_pub_ = create_publisher<smrcore_msgs::msg::MotorStatus>(
            "~/motor_status", rclcpp::SystemDefaultsQoS());
        const auto period = std::chrono::duration<double>(1.0 / std::max(status_rate_hz, 1.0));
        status_timer_ = create_wall_timer(
            std::chrono::duration_cast<std::chrono::milliseconds>(period),
            std::bind(&SdkServerNode::PublishStatus, this));

        CreateActionServers();
        CreateServices();
        RCLCPP_INFO(get_logger(), "SMRcore SDK server started, robot_ip='%s'", robot_ip_.c_str());
    }

    ~SdkServerNode() override
    {
        if (!robot_)
        {
            return;
        }
        try
        {
            robot_->Shutdown();
        }
        catch (const std::exception &e)
        {
            RCLCPP_WARN(get_logger(), "SMRcore SDK shutdown failed: %s", e.what());
        }
    }

private:
    template <typename ActionT, typename StartFn>
    void CreateMoveAction(const std::string &name,
                          typename rclcpp_action::Server<ActionT>::SharedPtr &server,
                          StartFn start_fn)
    {
        server = rclcpp_action::create_server<ActionT>(
            this, name,
            [this](const rclcpp_action::GoalUUID &, std::shared_ptr<const typename ActionT::Goal>) {
                std::lock_guard<std::mutex> lock(task_mutex_);
                if (task_running_)
                {
                    return rclcpp_action::GoalResponse::REJECT;
                }
                task_running_ = true;
                return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
            },
            [](const std::shared_ptr<rclcpp_action::ServerGoalHandle<ActionT>>) {
                return rclcpp_action::CancelResponse::ACCEPT;
            },
            [this, start_fn](const std::shared_ptr<rclcpp_action::ServerGoalHandle<ActionT>> goal_handle) {
                std::thread([this, goal_handle, start_fn]() {
                    ExecuteMoveAction<ActionT>(goal_handle, start_fn);
                }).detach();
            });
    }

    template <typename ActionT, typename StartFn>
    void ExecuteMoveAction(const std::shared_ptr<rclcpp_action::ServerGoalHandle<ActionT>> goal_handle,
                           StartFn start_fn)
    {
        auto result_msg = std::make_shared<typename ActionT::Result>();
        try
        {
            SdkResult sdk_result = start_fn(*goal_handle->get_goal());
            if (!sdk_result.success)
            {
                result_msg->success = false;
                result_msg->error_code = sdk_result.error_code;
                result_msg->message = sdk_result.message;
                goal_handle->abort(result_msg);
                ReleaseTask();
                return;
            }

            bool saw_active_task = false;
            int inactive_samples_without_task = 0;
            MotionTaskStatus last_status;
            while (rclcpp::ok())
            {
                if (goal_handle->is_canceling())
                {
                    auto stop_result = robot_->StopMotion();
                    result_msg->success = stop_result.success;
                    result_msg->error_code = stop_result.error_code;
                    result_msg->message = result_msg->success ? "canceled" : stop_result.message;
                    goal_handle->canceled(result_msg);
                    ReleaseTask();
                    return;
                }

                auto status = robot_->GetMotionTaskStatus();
                last_status = status;
                auto feedback = std::make_shared<typename ActionT::Feedback>();
                feedback->task_id = status.task_id;
                feedback->status = status.status;
                feedback->progress = status.has_active_task ? 0.5F : (saw_active_task ? 1.0F : 0.0F);
                feedback->error_code = status.error_code;
                feedback->message = status.message;
                goal_handle->publish_feedback(feedback);

                saw_active_task = saw_active_task || status.has_active_task;
                if (!status.has_active_task && status.task_id == 0 && !saw_active_task)
                {
                    ++inactive_samples_without_task;
                }
                else
                {
                    inactive_samples_without_task = 0;
                }

                if ((saw_active_task && !status.has_active_task) || status.status == 3 ||
                    status.status == 4 || inactive_samples_without_task >= 20)
                {
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }

            auto final_result = robot_->WaitMotionResult();
            result_msg->success = final_result.success;
            result_msg->error_code = final_result.error_code;
            result_msg->message = final_result.message;
            if (last_status.status == 4)
            {
                result_msg->success = false;
                result_msg->error_code = last_status.error_code;
                result_msg->message = last_status.message;
            }
            if (result_msg->success)
            {
                goal_handle->succeed(result_msg);
            }
            else
            {
                goal_handle->abort(result_msg);
            }
        }
        catch (const std::exception &e)
        {
            result_msg->success = false;
            result_msg->error_code = 1;
            result_msg->message = e.what();
            goal_handle->abort(result_msg);
        }
        ReleaseTask();
    }

    void ReleaseTask()
    {
        std::lock_guard<std::mutex> lock(task_mutex_);
        task_running_ = false;
    }

    void CreateActionServers()
    {
        CreateMoveAction<MoveJ>(
            "~/move_j", move_j_server_,
            [this](const MoveJ::Goal &goal) {
                const double scale = NormalizeVelocityScale(goal.velocity_scale);
                if (!goal.waypoint_name.empty())
                {
                    return robot_->MoveJ({}, goal.waypoint_name, scale);
                }
                std::array<double, 6> target = {};
                for (std::size_t i = 0; i < 6; ++i)
                {
                    target[i] = goal.target_positions[i];
                }
                return robot_->MoveJ(target, "", scale);
            });

        CreateMoveAction<MoveP>(
            "~/move_p", move_p_server_,
            [this](const MoveP::Goal &goal) {
                return robot_->MoveP(ToPoseData(goal.target_pose.pose),
                                     NormalizeVelocityScale(goal.velocity_scale));
            });

        CreateMoveAction<MoveL>(
            "~/move_l", move_l_server_,
            [this](const MoveL::Goal &goal) {
                return robot_->MoveL(ToPoseData(goal.target_pose.pose),
                                     NormalizeVelocityScale(goal.velocity_scale));
            });

        CreateMoveAction<MoveC>(
            "~/move_c", move_c_server_,
            [this](const MoveC::Goal &goal) {
                return robot_->MoveC(ToPoseData(goal.via_pose.pose),
                                     ToPoseData(goal.goal_pose.pose),
                                     NormalizeVelocityScale(goal.velocity_scale));
            });

        CreateMoveAction<MovePath>(
            "~/move_path", move_path_server_,
            [this](const MovePath::Goal &goal) {
                std::vector<CartesianWaypoint> waypoints;
                waypoints.reserve(goal.waypoints.size());
                for (const auto &point : goal.waypoints)
                {
                    CartesianWaypoint waypoint;
                    waypoint.pose = ToPoseData(point.pose.pose);
                    waypoint.mode = point.mode;
                    waypoint.blend_radius = point.blend_radius;
                    waypoints.push_back(waypoint);
                }
                return robot_->MovePath(waypoints,
                                        NormalizeVelocityScale(goal.velocity_scale));
            });
    }

    double NormalizeVelocityScale(double velocity_scale) const
    {
        return velocity_scale == 0.0 ? kDefaultVelocityScale : velocity_scale;
    }

    template <typename ServiceT, typename Fn>
    void CreateResultService(const std::string &name, Fn fn)
    {
        auto service = create_service<ServiceT>(
            name,
            [this, fn](const std::shared_ptr<typename ServiceT::Request>,
                       std::shared_ptr<typename ServiceT::Response> response) {
                SdkResult result = fn();
                response->success = result.success;
                response->error_code = result.error_code;
                response->message = result.message;
            });
        result_services_.push_back(service);
    }

    void CreateServices()
    {
        CreateResultService<smrcore_msgs::srv::Enable>("~/enable", [this]() { return robot_->Enable(); });
        CreateResultService<smrcore_msgs::srv::Disable>("~/disable", [this]() { return robot_->Disable(); });
        CreateResultService<smrcore_msgs::srv::EStop>("~/estop", [this]() { return robot_->EStop(); });
        CreateResultService<smrcore_msgs::srv::Recover>("~/recover", [this]() { return robot_->Recover(); });
        CreateResultService<smrcore_msgs::srv::ClearError>("~/clear_error", [this]() { return robot_->ClearError(); });
        CreateResultService<smrcore_msgs::srv::StopMotion>("~/stop_motion", [this]() { return robot_->StopMotion(); });
        CreateResultService<smrcore_msgs::srv::PauseMotion>("~/pause_motion", [this]() { return robot_->PauseMotion(); });
        CreateResultService<smrcore_msgs::srv::ContinueMotion>("~/continue_motion", [this]() { return robot_->ContinueMotion(); });
        CreateResultService<smrcore_msgs::srv::ClearPayload>("~/clear_payload", [this]() { return robot_->ClearPayload(); });

        set_payload_service_ = create_service<smrcore_msgs::srv::SetPayload>(
            "~/set_payload",
            [this](const std::shared_ptr<smrcore_msgs::srv::SetPayload::Request> request,
                   std::shared_ptr<smrcore_msgs::srv::SetPayload::Response> response) {
                auto result = robot_->SetPayload(request->mass, request->center_of_mass);
                response->success = result.success;
                response->error_code = result.error_code;
                response->message = result.message;
            });

        get_robot_info_service_ = create_service<smrcore_msgs::srv::GetRobotInfo>(
            "~/get_robot_info",
            [this](const std::shared_ptr<smrcore_msgs::srv::GetRobotInfo::Request>,
                   std::shared_ptr<smrcore_msgs::srv::GetRobotInfo::Response> response) {
                const auto info = robot_->GetRobotInfo();
                response->success = true;
                response->message = "success";
                response->robot_model = info.robot_model;
                response->robot_serial_number = info.robot_serial_number;
                response->sdk_version = info.sdk_version;
            });

        get_motor_status_service_ = create_service<smrcore_msgs::srv::GetMotorStatus>(
            "~/get_motor_status",
            [this](const std::shared_ptr<smrcore_msgs::srv::GetMotorStatus::Request>,
                   std::shared_ptr<smrcore_msgs::srv::GetMotorStatus::Response> response) {
                response->success = true;
                response->message = "success";
                response->status = ToMotorStatusMsg(robot_->GetMotorStatus());
            });

        get_robot_status_service_ = create_service<smrcore_msgs::srv::GetRobotStatus>(
            "~/get_robot_status",
            [this](const std::shared_ptr<smrcore_msgs::srv::GetRobotStatus::Request>,
                   std::shared_ptr<smrcore_msgs::srv::GetRobotStatus::Response> response) {
                response->success = true;
                response->message = "success";
                response->status = ToRobotStatusMsg(robot_->GetRobotStatus());
            });

        fk_service_ = create_service<smrcore_msgs::srv::ForwardKinematics>(
            "~/forward_kinematics",
            [this](const std::shared_ptr<smrcore_msgs::srv::ForwardKinematics::Request> request,
                   std::shared_ptr<smrcore_msgs::srv::ForwardKinematics::Response> response) {
                std::array<double, 6> joints = {};
                for (std::size_t i = 0; i < 6; ++i)
                {
                    joints[i] = request->joint_positions[i];
                }
                PoseData pose;
                auto result = robot_->ForwardKinematics(joints, pose);
                response->success = result.success;
                response->error_code = result.error_code;
                response->message = result.message;
                response->pose.header.stamp = now();
                response->pose.header.frame_id = "base";
                response->pose.pose = ToRosPose(pose);
            });

        ik_service_ = create_service<smrcore_msgs::srv::InverseKinematics>(
            "~/inverse_kinematics",
            [this](const std::shared_ptr<smrcore_msgs::srv::InverseKinematics::Request> request,
                   std::shared_ptr<smrcore_msgs::srv::InverseKinematics::Response> response) {
                std::array<double, 6> joints = {};
                auto result = robot_->InverseKinematics(ToPoseData(request->pose.pose), joints);
                response->success = result.success;
                response->error_code = result.error_code;
                response->message = result.message;
                for (std::size_t i = 0; i < 6; ++i)
                {
                    response->joint_positions[i] = joints[i];
                }
            });
    }

    smrcore_msgs::msg::MotorStatus ToMotorStatusMsg(const MotorStatus &status) const
    {
        smrcore_msgs::msg::MotorStatus msg;
        msg.enabled = status.enabled;
        msg.estop = status.estop;
        msg.error = status.error;
        msg.status_code = status.status_code;
        msg.operation_mode = status.operation_mode;
        msg.operational = status.operational;
        msg.timestamp = status.timestamp;
        return msg;
    }

    smrcore_msgs::msg::RobotStatus ToRobotStatusMsg(const RobotStatus &state)
    {
        smrcore_msgs::msg::RobotStatus msg;
        msg.header.stamp = now();
        msg.connected = state.connected;
        msg.positions = state.positions;
        msg.velocities = state.velocities;
        msg.torques = state.torques;
        msg.cartesian_valid = state.cartesian_valid;
        msg.cartesian_position = state.cartesian_position;
        msg.cartesian_orientation = state.cartesian_orientation;
        msg.control_mode = state.control_mode;
        msg.reference_frame = state.reference_frame;
        msg.latched_error_codes = state.latched_error_codes;
        msg.active_warning_codes = state.active_warning_codes;
        msg.command_error_codes = state.command_error_codes;
        return msg;
    }

    void PublishStatus()
    {
        try
        {
            robot_status_pub_->publish(ToRobotStatusMsg(robot_->GetRobotStatus()));
            motor_status_pub_->publish(ToMotorStatusMsg(robot_->GetMotorStatus()));
        }
        catch (const std::exception &e)
        {
            RCLCPP_WARN_THROTTLE(get_logger(), *get_clock(), 2000,
                                 "发布 SDK 状态失败: %s", e.what());
        }
    }

    std::string robot_ip_;
    std::shared_ptr<SdkSession> robot_;
    std::mutex task_mutex_;
    bool task_running_ = false;

    rclcpp::Publisher<smrcore_msgs::msg::RobotStatus>::SharedPtr robot_status_pub_;
    rclcpp::Publisher<smrcore_msgs::msg::MotorStatus>::SharedPtr motor_status_pub_;
    rclcpp::TimerBase::SharedPtr status_timer_;

    rclcpp_action::Server<MoveJ>::SharedPtr move_j_server_;
    rclcpp_action::Server<MoveP>::SharedPtr move_p_server_;
    rclcpp_action::Server<MoveL>::SharedPtr move_l_server_;
    rclcpp_action::Server<MoveC>::SharedPtr move_c_server_;
    rclcpp_action::Server<MovePath>::SharedPtr move_path_server_;

    std::vector<rclcpp::ServiceBase::SharedPtr> result_services_;
    rclcpp::Service<smrcore_msgs::srv::SetPayload>::SharedPtr set_payload_service_;
    rclcpp::Service<smrcore_msgs::srv::GetRobotInfo>::SharedPtr get_robot_info_service_;
    rclcpp::Service<smrcore_msgs::srv::GetMotorStatus>::SharedPtr get_motor_status_service_;
    rclcpp::Service<smrcore_msgs::srv::GetRobotStatus>::SharedPtr get_robot_status_service_;
    rclcpp::Service<smrcore_msgs::srv::ForwardKinematics>::SharedPtr fk_service_;
    rclcpp::Service<smrcore_msgs::srv::InverseKinematics>::SharedPtr ik_service_;
};

} // namespace smrcore_sdk_server

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    try
    {
        rclcpp::spin(std::make_shared<smrcore_sdk_server::SdkServerNode>());
    }
    catch (const std::exception &e)
    {
        RCLCPP_FATAL(rclcpp::get_logger("smrcore_sdk_server"), "%s", e.what());
        rclcpp::shutdown();
        return 1;
    }
    rclcpp::shutdown();
    return 0;
}
