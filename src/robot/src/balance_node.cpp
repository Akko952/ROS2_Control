// balance_node.cpp
// ROS2 node: reads IMU, computes PD balance control, publishes cmd_vel
// Usage: ros2 run robot balance_node

#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/imu.hpp"
#include "geometry_msgs/msg/twist.hpp"

#include <cmath>

using std::placeholders::_1;

class BalanceNode : public rclcpp::Node
{
public:
  BalanceNode()
  : Node("pinocchio_balance_node")
  {
    declare_parameter<double>("kp", 50.0);
    declare_parameter<double>("kd", 5.0);
    declare_parameter<double>("target_pitch", 0.0);

    kp_ = get_parameter("kp").as_double();
    kd_ = get_parameter("kd").as_double();
    target_pitch_ = get_parameter("target_pitch").as_double();

    imu_sub_ = this->create_subscription<sensor_msgs::msg::Imu>(
      "/imu/data", 10,
      std::bind(&BalanceNode::imu_callback, this, _1));

    cmd_vel_pub_ = this->create_publisher<geometry_msgs::msg::Twist>(
      "/cmd_vel", 10);

    RCLCPP_INFO(this->get_logger(),
                   "BalanceNode started (kp=%.1f, kd=%.1f)", kp_, kd_);
  }

private:
  void imu_callback(const sensor_msgs::msg::Imu::SharedPtr msg)
  {
    // Extract pitch from quaternion
    double qx = msg->orientation.x;
    double qy = msg->orientation.y;
    double qz = msg->orientation.z;
    double qw = msg->orientation.w;

    double pitch = std::asin(2.0 * (qw * qy - qz * qx));
    double pitch_rate = msg->angular_velocity.y;  // y-axis = pitch rate

    // PD control
    double pitch_error = pitch - target_pitch_;
    double effort = kp_ * pitch_error + kd_ * pitch_rate;

    // Publish velocity command
    auto twist = geometry_msgs::msg::Twist();
    twist.linear.x = -effort * 0.1;   // scale to m/s
    twist.angular.z = 0.0;
    cmd_vel_pub_->publish(twist);
  }

  rclcpp::Subscription<sensor_msgs::msg::Imu>::SharedPtr imu_sub_;
  rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_pub_;
  double kp_;
  double kd_;
  double target_pitch_;
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<BalanceNode>());
  rclcpp::shutdown();
  return 0;
}
