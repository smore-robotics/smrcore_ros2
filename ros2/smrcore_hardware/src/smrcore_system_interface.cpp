#include "smrcore_hardware/smrcore_system_interface.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

#include "hardware_interface/types/hardware_interface_type_values.hpp"
#include "pluginlib/class_list_macros.hpp"
#include "rclcpp/rclcpp.hpp"

namespace smrcore_hardware
{
namespace
{

bool InterfaceNameEquals(const std::string &full_name,
                         const std::string &joint_name,
                         const std::string &interface_name)
{
    return full_name == joint_name + "/" + interface_name;
}

} // namespace

hardware_interface::CallbackReturn
SMRcoreSystemInterface::on_init(const hardware_interface::HardwareInfo &info)
{
    if (hardware_interface::SystemInterface::on_init(info) !=
        hardware_interface::CallbackReturn::SUCCESS)
    {
        return hardware_interface::CallbackReturn::ERROR;
    }

    if (!ValidateJoints(info))
    {
        return hardware_interface::CallbackReturn::ERROR;
    }

    robot_ip_.clear();
    const auto it = info.hardware_parameters.find("robot_ip");
    if (it != info.hardware_parameters.end())
    {
        robot_ip_ = it->second;
    }

    for (std::size_t i = 0; i < kNumJoints; ++i)
    {
        joint_names_[i] = info.joints[i].name;
    }

    return hardware_interface::CallbackReturn::SUCCESS;
}

std::vector<hardware_interface::StateInterface>
SMRcoreSystemInterface::export_state_interfaces()
{
    std::vector<hardware_interface::StateInterface> interfaces;
    interfaces.reserve(kNumJoints * 3);
    for (std::size_t i = 0; i < kNumJoints; ++i)
    {
        interfaces.emplace_back(joint_names_[i],
                                hardware_interface::HW_IF_POSITION,
                                &state_positions_[i]);
        interfaces.emplace_back(joint_names_[i],
                                hardware_interface::HW_IF_VELOCITY,
                                &state_velocities_[i]);
        interfaces.emplace_back(joint_names_[i], hardware_interface::HW_IF_EFFORT,
                                &state_efforts_[i]);
    }
    return interfaces;
}

std::vector<hardware_interface::CommandInterface>
SMRcoreSystemInterface::export_command_interfaces()
{
    std::vector<hardware_interface::CommandInterface> interfaces;
    interfaces.reserve(kNumJoints * 2);
    for (std::size_t i = 0; i < kNumJoints; ++i)
    {
        interfaces.emplace_back(joint_names_[i],
                                hardware_interface::HW_IF_POSITION,
                                &command_positions_[i]);
        interfaces.emplace_back(joint_names_[i],
                                hardware_interface::HW_IF_VELOCITY,
                                &command_velocities_[i]);
    }
    return interfaces;
}

hardware_interface::CallbackReturn SMRcoreSystemInterface::on_configure(
    const rclcpp_lifecycle::State &)
{
    robot_ = std::make_unique<rcore::sdk::Robot>();
    if (!robot_->Initialize(robot_ip_))
    {
        RCLCPP_ERROR(logger_, "SMRcore SDK 初始化失败，robot_ip='%s'",
                     robot_ip_.c_str());
        robot_.reset();
        return hardware_interface::CallbackReturn::ERROR;
    }

    RCLCPP_INFO(logger_, "SMRcore SDK 初始化完成，robot_ip='%s'",
                robot_ip_.c_str());
    return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn SMRcoreSystemInterface::on_activate(
    const rclcpp_lifecycle::State &)
{
    if (!robot_)
    {
        return hardware_interface::CallbackReturn::ERROR;
    }

    try
    {
        const auto state = robot_->GetState();
        CopyStateToBuffers(state);
        command_positions_ = state_positions_;
        command_velocities_ = state_velocities_;
    }
    catch (const std::exception &e)
    {
        RCLCPP_ERROR(logger_, "读取激活初始状态失败: %s", e.what());
        return hardware_interface::CallbackReturn::ERROR;
    }

    active_ = true;
    return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn SMRcoreSystemInterface::on_deactivate(
    const rclcpp_lifecycle::State &)
{
    active_ = false;
    if (robot_)
    {
        auto result = robot_->StopMotion();
        if (!result.IsSuccess())
        {
            RCLCPP_WARN(logger_, "StopMotion 失败: %s",
                        result.GetErrorMsg().c_str());
        }
    }
    return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::return_type
SMRcoreSystemInterface::read(const rclcpp::Time &, const rclcpp::Duration &)
{
    if (!robot_)
    {
        return hardware_interface::return_type::ERROR;
    }

    try
    {
        CopyStateToBuffers(robot_->GetState());
    }
    catch (const std::exception &e)
    {
        RCLCPP_ERROR(logger_, "读取 SMRcore 状态失败: %s", e.what());
        return hardware_interface::return_type::ERROR;
    }

    return hardware_interface::return_type::OK;
}

hardware_interface::return_type
SMRcoreSystemInterface::write(const rclcpp::Time &, const rclcpp::Duration &)
{
    if (!active_ || !robot_)
    {
        return hardware_interface::return_type::OK;
    }

    if (!CommandsAreFinite())
    {
        RCLCPP_ERROR(logger_, "关节透传目标包含非有限值，丢弃本周期指令");
        return hardware_interface::return_type::ERROR;
    }

    rcore::sdk::JointPositions positions;
    rcore::sdk::JointVelocities velocities;
    for (std::size_t i = 0; i < kNumJoints; ++i)
    {
        positions[i] = command_positions_[i];
        velocities[i] = command_velocities_[i];
    }

    robot_->JointPassthrough(positions, velocities);
    return hardware_interface::return_type::OK;
}

hardware_interface::return_type SMRcoreSystemInterface::prepare_command_mode_switch(
    const std::vector<std::string> &start_interfaces,
    const std::vector<std::string> &)
{
    if (start_interfaces.empty())
    {
        return hardware_interface::return_type::OK;
    }

    return IsWholeArmPositionVelocitySet(start_interfaces)
               ? hardware_interface::return_type::OK
               : hardware_interface::return_type::ERROR;
}

hardware_interface::return_type SMRcoreSystemInterface::perform_command_mode_switch(
    const std::vector<std::string> &start_interfaces,
    const std::vector<std::string> &)
{
    if (start_interfaces.empty())
    {
        return hardware_interface::return_type::OK;
    }

    return IsWholeArmPositionVelocitySet(start_interfaces)
               ? hardware_interface::return_type::OK
               : hardware_interface::return_type::ERROR;
}

bool SMRcoreSystemInterface::ValidateJoints(
    const hardware_interface::HardwareInfo &info) const
{
    if (info.joints.size() != kNumJoints)
    {
        RCLCPP_ERROR(logger_, "SMRcore V1 只支持 6 轴单臂，当前 joint 数量为 %zu",
                     info.joints.size());
        return false;
    }

    for (const auto &joint : info.joints)
    {
        const auto has_command = [&](const std::string &name) {
            return std::any_of(joint.command_interfaces.begin(),
                               joint.command_interfaces.end(),
                               [&](const auto &iface) { return iface.name == name; });
        };
        const auto has_state = [&](const std::string &name) {
            return std::any_of(joint.state_interfaces.begin(),
                               joint.state_interfaces.end(),
                               [&](const auto &iface) { return iface.name == name; });
        };

        if (!has_command(hardware_interface::HW_IF_POSITION) ||
            !has_command(hardware_interface::HW_IF_VELOCITY) ||
            !has_state(hardware_interface::HW_IF_POSITION) ||
            !has_state(hardware_interface::HW_IF_VELOCITY) ||
            !has_state(hardware_interface::HW_IF_EFFORT))
        {
            RCLCPP_ERROR(logger_,
                         "joint '%s' 必须声明 position/velocity command 和 "
                         "position/velocity/effort state interface",
                         joint.name.c_str());
            return false;
        }
    }

    return true;
}

bool SMRcoreSystemInterface::IsWholeArmPositionVelocitySet(
    const std::vector<std::string> &interfaces) const
{
    if (interfaces.size() != kNumJoints * 2)
    {
        return false;
    }

    for (const auto &joint_name : joint_names_)
    {
        const auto has_position = std::any_of(
            interfaces.begin(), interfaces.end(), [&](const std::string &name) {
                return InterfaceNameEquals(name, joint_name,
                                           hardware_interface::HW_IF_POSITION);
            });
        const auto has_velocity = std::any_of(
            interfaces.begin(), interfaces.end(), [&](const std::string &name) {
                return InterfaceNameEquals(name, joint_name,
                                           hardware_interface::HW_IF_VELOCITY);
            });
        if (!has_position || !has_velocity)
        {
            return false;
        }
    }

    return true;
}

void SMRcoreSystemInterface::CopyStateToBuffers(const rcore::RobotState &state)
{
    for (std::size_t i = 0; i < kNumJoints; ++i)
    {
        state_positions_[i] = state.positions[i];
        state_velocities_[i] = state.velocities[i];
        state_efforts_[i] = state.torques[i];
    }
}

bool SMRcoreSystemInterface::CommandsAreFinite() const
{
    for (std::size_t i = 0; i < kNumJoints; ++i)
    {
        if (!std::isfinite(command_positions_[i]) ||
            !std::isfinite(command_velocities_[i]))
        {
            return false;
        }
    }
    return true;
}

} // namespace smrcore_hardware

PLUGINLIB_EXPORT_CLASS(smrcore_hardware::SMRcoreSystemInterface,
                       hardware_interface::SystemInterface)
