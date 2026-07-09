#include "sdk_session.hpp"

#include <cmath>
#include <memory>
#include <utility>

#include "sdk/robot.hpp"

namespace smrcore_sdk_server
{
namespace
{

struct Rpy
{
    double roll = 0.0;
    double pitch = 0.0;
    double yaw = 0.0;
};

Rpy QuaternionToRpy(const std::array<double, 4> &q)
{
    Rpy out;
    const double x = q[0];
    const double y = q[1];
    const double z = q[2];
    const double w = q[3];
    const double sinr_cosp = 2.0 * (w * x + y * z);
    const double cosr_cosp = 1.0 - 2.0 * (x * x + y * y);
    out.roll = std::atan2(sinr_cosp, cosr_cosp);
    const double sinp = 2.0 * (w * y - z * x);
    out.pitch = std::abs(sinp) >= 1.0 ? std::copysign(M_PI / 2.0, sinp)
                                       : std::asin(sinp);
    const double siny_cosp = 2.0 * (w * z + x * y);
    const double cosy_cosp = 1.0 - 2.0 * (y * y + z * z);
    out.yaw = std::atan2(siny_cosp, cosy_cosp);
    return out;
}

std::array<double, 4> RpyToQuaternion(double roll, double pitch, double yaw)
{
    std::array<double, 4> q = {};
    const double cy = std::cos(yaw * 0.5);
    const double sy = std::sin(yaw * 0.5);
    const double cp = std::cos(pitch * 0.5);
    const double sp = std::sin(pitch * 0.5);
    const double cr = std::cos(roll * 0.5);
    const double sr = std::sin(roll * 0.5);
    q[3] = cr * cp * cy + sr * sp * sy;
    q[0] = sr * cp * cy - cr * sp * sy;
    q[1] = cr * sp * cy + sr * cp * sy;
    q[2] = cr * cp * sy - sr * sp * cy;
    return q;
}

rcore::sdk::Pose ToSdkPose(const PoseData &pose)
{
    std::array<double, 3> xyz = pose.position;
    const auto rpy = QuaternionToRpy(pose.orientation);
    std::array<double, 3> rvec = {rpy.roll, rpy.pitch, rpy.yaw};
    return rcore::sdk::Pose::FromEuler(
        rcore::sdk::VecView<const double, 3>(xyz.data()),
        rcore::sdk::VecView<const double, 3>(rvec.data()));
}

PoseData ToPoseData(const rcore::sdk::Pose &pose)
{
    PoseData out;
    const auto xyz = pose.tvec();
    const auto rpy = pose.rvec();
    out.position = {xyz[0], xyz[1], xyz[2]};
    out.orientation = RpyToQuaternion(rpy[0], rpy[1], rpy[2]);
    return out;
}

SdkResult ToSdkResult(rcore::sdk::Result result)
{
    SdkResult out;
    out.success = result.IsSuccess();
    out.error_code = result.GetErrorCode();
    out.message = out.success ? "success" : result.GetErrorMsg();
    return out;
}

SdkResult AcceptedResult()
{
    return {true, 0, "accepted"};
}

class SdkSessionImpl final : public SdkSession
{
public:
    bool Initialize(const std::string &robot_ip) override
    {
        return robot_.Initialize(robot_ip);
    }

    void Shutdown() override { robot_.Shutdown(); }
    bool IsConnected() override { return robot_.IsConnected(); }

    SdkResult Enable() override { return ToSdkResult(robot_.Control().Enable()); }
    SdkResult Disable() override { return ToSdkResult(robot_.Control().Disable()); }
    SdkResult EStop() override { return ToSdkResult(robot_.Control().EStop()); }
    SdkResult Recover() override { return ToSdkResult(robot_.Control().Recover()); }
    SdkResult ClearError() override { return ToSdkResult(robot_.Control().ClearError()); }
    SdkResult StopMotion() override { return ToSdkResult(robot_.Motion().StopMotion()); }
    SdkResult PauseMotion() override { return ToSdkResult(robot_.Motion().PauseMotion()); }
    SdkResult ContinueMotion() override { return ToSdkResult(robot_.Motion().ContinueMotion()); }
    SdkResult ClearPayload() override { return ToSdkResult(robot_.ClearPayload()); }

    SdkResult SetPayload(double mass,
                         const std::array<double, 3> &com) override
    {
        rcore::PayloadConfig payload;
        payload.mass = mass;
        payload.com = com;
        return ToSdkResult(robot_.SetPayload(payload));
    }

    RobotInfo GetRobotInfo() override
    {
        const auto info = robot_.GetRobotInfo();
        return {info.robot_model, info.robot_serial_number, info.sdk_version};
    }

    RobotStatus GetRobotStatus() override
    {
        const auto state = robot_.GetState();
        RobotStatus msg;
        msg.connected = robot_.IsConnected();
        msg.positions = state.positions;
        msg.velocities = state.velocities;
        msg.torques = state.torques;
        msg.cartesian_valid = state.cartesian_valid;
        msg.cartesian_position = state.cartesian_position;
        msg.cartesian_orientation = state.cartesian_orientation;
        msg.control_mode = static_cast<uint8_t>(state.control_mode);
        msg.reference_frame = state.reference_frame;
        const auto &latched_errors = state.errors.GetLatchedErrors();
        for (std::size_t i = 0; i < state.errors.GetLatchedErrorCount(); ++i)
        {
            msg.latched_error_codes.push_back(latched_errors[i].code);
        }
        const auto &warnings = state.errors.GetActiveWarnings();
        for (std::size_t i = 0; i < state.errors.GetActiveWarningCount(); ++i)
        {
            msg.active_warning_codes.push_back(warnings[i]);
        }
        const auto &commands = state.errors.GetActiveCommands();
        for (std::size_t i = 0; i < state.errors.GetActiveCommandCount(); ++i)
        {
            msg.command_error_codes.push_back(commands[i].code);
        }
        return msg;
    }

    MotorStatus GetMotorStatus() override
    {
        const auto status = robot_.GetMotorStatus();
        return {status.enabled, status.estop, status.error, status.status_code,
                status.operation_mode, status.operational, status.timestamp};
    }

    MotionTaskStatus GetMotionTaskStatus() override
    {
        const auto status = robot_.Motion().GetMotionTaskStatus();
        return {status.task_id, status.status, status.has_active_task,
                status.error_code, status.error_msg};
    }

    SdkResult WaitMotionResult() override
    {
        if (!active_motion_result_)
        {
            return {true, 0, "success"};
        }
        auto result = ToSdkResult(std::move(*active_motion_result_));
        active_motion_result_.reset();
        return result;
    }

    SdkResult MoveJ(const std::array<double, 6> &target_positions,
                    const std::string &waypoint_name,
                    double velocity_scale) override
    {
        if (!waypoint_name.empty())
        {
            active_motion_result_ = std::make_unique<rcore::sdk::Result>(
                robot_.Motion().MoveJ(waypoint_name, true, velocity_scale));
            return AcceptedResult();
        }
        rcore::sdk::JointPositions target;
        for (std::size_t i = 0; i < target_positions.size(); ++i)
        {
            target[i] = target_positions[i];
        }
        active_motion_result_ = std::make_unique<rcore::sdk::Result>(
            robot_.Motion().MoveJ(target, true, velocity_scale));
        return AcceptedResult();
    }

    SdkResult MoveP(const PoseData &pose,
                    double velocity_scale) override
    {
        active_motion_result_ = std::make_unique<rcore::sdk::Result>(
            robot_.Motion().MoveP(ToSdkPose(pose), true, velocity_scale));
        return AcceptedResult();
    }

    SdkResult MoveL(const PoseData &pose,
                    double velocity_scale) override
    {
        active_motion_result_ = std::make_unique<rcore::sdk::Result>(
            robot_.Motion().MoveL(ToSdkPose(pose), true, velocity_scale));
        return AcceptedResult();
    }

    SdkResult MoveC(const PoseData &via_pose,
                    const PoseData &goal_pose,
                    double velocity_scale) override
    {
        active_motion_result_ = std::make_unique<rcore::sdk::Result>(
            robot_.Motion().MoveC(ToSdkPose(via_pose), ToSdkPose(goal_pose),
                                  true, velocity_scale));
        return AcceptedResult();
    }

    SdkResult MovePath(const std::vector<CartesianWaypoint> &waypoints,
                       double velocity_scale) override
    {
        std::vector<rcore::sdk::CartesianWaypoint> sdk_waypoints;
        sdk_waypoints.reserve(waypoints.size());
        for (const auto &point : waypoints)
        {
            rcore::sdk::CartesianWaypoint waypoint;
            waypoint.pose = ToSdkPose(point.pose);
            waypoint.mode = point.mode == 1 ? rcore::sdk::PathWaypointMode::Blend
                                            : rcore::sdk::PathWaypointMode::Stop;
            waypoint.blend_radius = point.blend_radius;
            sdk_waypoints.push_back(waypoint);
        }
        active_motion_result_ = std::make_unique<rcore::sdk::Result>(
            robot_.Motion().MovePath(sdk_waypoints, true, velocity_scale));
        return AcceptedResult();
    }

    SdkResult ForwardKinematics(const std::array<double, 6> &joints,
                                PoseData &pose) override
    {
        rcore::sdk::JointPositions sdk_joints;
        for (std::size_t i = 0; i < joints.size(); ++i)
        {
            sdk_joints[i] = joints[i];
        }
        rcore::sdk::Pose sdk_pose;
        auto result = robot_.Motion().ForwardKinematics(sdk_joints, sdk_pose);
        pose = ToPoseData(sdk_pose);
        return ToSdkResult(std::move(result));
    }

    SdkResult InverseKinematics(const PoseData &pose,
                                std::array<double, 6> &joints) override
    {
        rcore::sdk::JointPositions sdk_joints;
        auto result = robot_.Motion().InverseKinematics(ToSdkPose(pose),
                                                        sdk_joints);
        for (std::size_t i = 0; i < joints.size(); ++i)
        {
            joints[i] = sdk_joints[i];
        }
        return ToSdkResult(std::move(result));
    }

private:
    rcore::sdk::Robot robot_;
    std::unique_ptr<rcore::sdk::Result> active_motion_result_;
};

} // namespace
} // namespace smrcore_sdk_server

extern "C" smrcore_sdk_server::SdkSession *CreateSdkSession()
{
    return new smrcore_sdk_server::SdkSessionImpl();
}

extern "C" void DestroySdkSession(smrcore_sdk_server::SdkSession *session)
{
    delete session;
}
