// balance_controller.hpp
// Gazebo ModelPlugin for Pinocchio balance control

#ifndef ROBOT_BALANCE_CONTROLLER_HPP_
#define ROBOT_BALANCE_CONTROLLER_HPP_

#include <gazebo/common/Plugin.hh>
#include <gazebo/physics/Model.hh>

#include <memory>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/imu.hpp>
#include <geometry_msgs/msg/twist.hpp>

namespace robot
{

class BalanceController : public gazebo::ModelPlugin
{
public:
  BalanceController() = default;
  virtual ~BalanceController() = default;

  void Load(gazebo::physics::ModelPtr model, sdf::ElementPtr sdf) override;

private:
  void OnUpdate(const gazebo::common::UpdateInfo & info);
  void ImuCallback(const sensor_msgs::msg::Imu::SharedPtr msg);

  rclcpp::Node::SharedPtr node_;
  rclcpp::Subscription<sensor_msgs::msg::Imu>::SharedPtr imu_sub_;
  rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_pub_;
  gazebo::physics::ModelPtr model_;
  gazebo::event::ConnectionPtr update_connection_;

  double kp_{50.0};
  double kd_{5.0};
  double target_pitch_{0.0};
  double last_pitch_{0.0};
  double last_time_{0.0};
};

}  // namespace robot

#endif  // ROBOT_BALANCE_CONTROLLER_HPP_
