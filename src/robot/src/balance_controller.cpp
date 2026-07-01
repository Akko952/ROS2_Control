// balance_controller.cpp
// Gazebo ModelPlugin: PD balance control via IMU pitch angle

#include "robot/balance_controller.hpp"

#include <gazebo/common/Events.hh>
#include <gazebo/physics/World.hh>
#include <ignition/math/Quaternion.hh>

#include <cmath>
#include <thread>

namespace robot
{

void BalanceController::Load(gazebo::physics::ModelPtr model, sdf::ElementPtr sdf)
{
  model_ = model;

  if (sdf->HasElement("kp")) {
    kp_ = sdf->Get<double>("kp");
  }
  if (sdf->HasElement("kd")) {
    kd_ = sdf->Get<double>("kd");
  }

  node_ = rclcpp::Node::make_shared("pinocchio_balance_controller");
  imu_sub_ = node_->create_subscription<sensor_msgs::msg::Imu>(
    "/imu/data", 10,
    std::bind(&BalanceController::ImuCallback, this, std::placeholders::_1));

  cmd_vel_pub_ = node_->create_publisher<geometry_msgs::msg::Twist>(
    "/cmd_vel", 10);

  // Spin ROS2 in background thread
  std::thread([this]() { rclcpp::spin(node_); }).detach();

  // Register Gazebo update callback
  update_connection_ = gazebo::event::Events::ConnectWorldUpdateBegin(
    std::bind(&BalanceController::OnUpdate, this, std::placeholders::_1));

  RCLCPP_INFO(node_->get_logger(),
               "BalanceController loaded (kp=%.1f, kd=%.1f)", kp_, kd_);
}

void BalanceController::ImuCallback(const sensor_msgs::msg::Imu::SharedPtr msg)
{
  double qx = msg->orientation.x;
  double qy = msg->orientation.y;
  double qz = msg->orientation.z;
  double qw = msg->orientation.w;

  last_pitch_ = std::asin(2.0 * (qw * qy - qz * qx));
}

void BalanceController::OnUpdate(const gazebo::common::UpdateInfo & info)
{
  double now = info.simTime.Double();
  if (last_time_ <= 0.0) {
    last_time_ = now;
    return;
  }
  last_time_ = now;

  double pitch_error = last_pitch_ - target_pitch_;
  double effort = kp_ * pitch_error;

  auto twist = geometry_msgs::msg::Twist();
  twist.linear.x = -effort * 0.1;
  twist.angular.z = 0.0;
  cmd_vel_pub_->publish(twist);
}

}  // namespace robot

GZ_REGISTER_MODEL_PLUGIN(robot::BalanceController)
