#pragma once

#include <array>
#include <memory>
#include <string>
#include <vector>

#include "hardware_interface/handle.hpp"
#include "hardware_interface/hardware_info.hpp"
#include "hardware_interface/system_interface.hpp"
#include "hardware_interface/types/hardware_interface_return_values.hpp"
#include "rclcpp/duration.hpp"
#include "rclcpp/logger.hpp"
#include "rclcpp/time.hpp"
#include "rclcpp_lifecycle/state.hpp"

#include "sdk/robot.hpp"

namespace smrcore_hardware
{

class SMRcoreSystemInterface : public hardware_interface::SystemInterface
{
public:
    static constexpr std::size_t kNumJoints = 6;

    hardware_interface::CallbackReturn
    on_init(const hardware_interface::HardwareInfo &info) override;

    std::vector<hardware_interface::StateInterface>
    export_state_interfaces() override;

    std::vector<hardware_interface::CommandInterface>
    export_command_interfaces() override;

    hardware_interface::CallbackReturn
    on_configure(const rclcpp_lifecycle::State &previous_state) override;

    hardware_interface::CallbackReturn
    on_activate(const rclcpp_lifecycle::State &previous_state) override;

    hardware_interface::CallbackReturn
    on_deactivate(const rclcpp_lifecycle::State &previous_state) override;

    hardware_interface::return_type
    read(const rclcpp::Time &time, const rclcpp::Duration &period) override;

    hardware_interface::return_type
    write(const rclcpp::Time &time, const rclcpp::Duration &period) override;

    hardware_interface::return_type
    prepare_command_mode_switch(const std::vector<std::string> &start_interfaces,
                                const std::vector<std::string> &stop_interfaces)
        override;

    hardware_interface::return_type
    perform_command_mode_switch(const std::vector<std::string> &start_interfaces,
                                const std::vector<std::string> &stop_interfaces)
        override;

private:
    bool ValidateJoints(const hardware_interface::HardwareInfo &info) const;
    bool IsWholeArmPositionVelocitySet(
        const std::vector<std::string> &interfaces) const;
    void CopyStateToBuffers(const rcore::RobotState &state);
    bool CommandsAreFinite() const;

    rclcpp::Logger logger_ = rclcpp::get_logger("smrcore_hardware");
    std::unique_ptr<rcore::sdk::Robot> robot_;
    std::string robot_ip_;
    bool log_passthrough_commands_ = false;
    std::size_t write_count_ = 0;

    std::array<std::string, kNumJoints> joint_names_{};
    std::array<double, kNumJoints> state_positions_{};
    std::array<double, kNumJoints> state_velocities_{};
    std::array<double, kNumJoints> state_efforts_{};
    std::array<double, kNumJoints> command_positions_{};
    std::array<double, kNumJoints> command_velocities_{};
    bool active_ = false;
};

} // namespace smrcore_hardware
