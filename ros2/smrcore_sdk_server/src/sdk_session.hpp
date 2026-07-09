#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace smrcore_sdk_server
{

struct SdkResult
{
    bool success = false;
    uint16_t error_code = 0;
    std::string message;
};

struct MotionTaskStatus
{
    int32_t task_id = 0;
    int32_t status = 0;
    bool has_active_task = false;
    uint16_t error_code = 0;
    std::string message;
};

struct RobotInfo
{
    std::string robot_model;
    std::string robot_serial_number;
    std::string sdk_version;
};

struct RobotStatus
{
    bool connected = false;
    std::array<double, 6> positions = {};
    std::array<double, 6> velocities = {};
    std::array<double, 6> torques = {};
    bool cartesian_valid = false;
    std::array<double, 3> cartesian_position = {};
    std::array<double, 3> cartesian_orientation = {};
    uint8_t control_mode = 0;
    std::string reference_frame;
    std::vector<uint16_t> latched_error_codes;
    std::vector<uint16_t> active_warning_codes;
    std::vector<uint16_t> command_error_codes;
};

struct MotorStatus
{
    bool enabled = false;
    bool estop = false;
    bool error = false;
    uint16_t status_code = 0;
    uint8_t operation_mode = 0;
    bool operational = false;
    double timestamp = 0.0;
};

struct PoseData
{
    std::array<double, 3> position = {};
    std::array<double, 4> orientation = {0.0, 0.0, 0.0, 1.0};
};

struct CartesianWaypoint
{
    PoseData pose;
    uint8_t mode = 0;
    double blend_radius = 0.0;
};

class SdkSession
{
public:
    virtual ~SdkSession() = default;

    virtual bool Initialize(const std::string &robot_ip) = 0;
    virtual void Shutdown() = 0;
    virtual bool IsConnected() = 0;

    virtual SdkResult Enable() = 0;
    virtual SdkResult Disable() = 0;
    virtual SdkResult EStop() = 0;
    virtual SdkResult Recover() = 0;
    virtual SdkResult ClearError() = 0;
    virtual SdkResult StopMotion() = 0;
    virtual SdkResult PauseMotion() = 0;
    virtual SdkResult ContinueMotion() = 0;
    virtual SdkResult ClearPayload() = 0;
    virtual SdkResult SetPayload(double mass,
                                 const std::array<double, 3> &com) = 0;

    virtual RobotInfo GetRobotInfo() = 0;
    virtual RobotStatus GetRobotStatus() = 0;
    virtual MotorStatus GetMotorStatus() = 0;
    virtual MotionTaskStatus GetMotionTaskStatus() = 0;
    virtual SdkResult WaitMotionResult() = 0;

    virtual SdkResult MoveJ(const std::array<double, 6> &target_positions,
                            const std::string &waypoint_name,
                            double velocity_scale) = 0;
    virtual SdkResult MoveP(const PoseData &pose,
                            double velocity_scale) = 0;
    virtual SdkResult MoveL(const PoseData &pose,
                            double velocity_scale) = 0;
    virtual SdkResult MoveC(const PoseData &via_pose,
                            const PoseData &goal_pose,
                            double velocity_scale) = 0;
    virtual SdkResult MovePath(const std::vector<CartesianWaypoint> &waypoints,
                               double velocity_scale) = 0;
    virtual SdkResult ForwardKinematics(const std::array<double, 6> &joints,
                                        PoseData &pose) = 0;
    virtual SdkResult InverseKinematics(const PoseData &pose,
                                        std::array<double, 6> &joints) = 0;
};

using CreateSdkSessionFn = SdkSession *(*)();
using DestroySdkSessionFn = void (*)(SdkSession *);

std::shared_ptr<SdkSession> LoadSdkSession();

} // namespace smrcore_sdk_server
