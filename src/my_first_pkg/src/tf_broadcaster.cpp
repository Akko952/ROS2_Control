/**
 * tf_broadcaster.cpp — 手写 TF2 坐标广播
 *
 * 不靠 URDF, 不用 robot_state_publisher.
 * 直接用代码告诉 ROS2: "lidar 在底盘前方10cm,上方12cm"
 *
 * 运行:
 *   ros2 run my_first_pkg tf_broadcaster
 *
 * 验证:
 *   ros2 run tf2_ros tf2_echo base_link lidar_link
 */

#include <chrono>
#include <functional>
#include <memory>

#include <rclcpp/rclcpp.hpp>
#include <tf2_ros/transform_broadcaster.h>
#include <geometry_msgs/msg/transform_stamped.hpp>

using namespace std::chrono_literals;

class TFBroadcasterNode : public rclcpp::Node
{
public:
    TFBroadcasterNode()
    : Node("tf_broadcaster_node")
    {
        // 创建 TF2 广播器 — 负责把坐标变换发到 /tf 话题
        tf_broadcaster_ = std::make_shared<tf2_ros::TransformBroadcaster>(this);

        // 定时器: 每 100ms (10Hz) 发布一次 TF
        timer_ = this->create_wall_timer(
            100ms,
            std::bind(&TFBroadcasterNode::publish_tf, this));

        RCLCPP_INFO(this->get_logger(), "TF broadcaster started");
    }

private:
    void publish_tf()
    {
        // 构造一条变换: base_link -> lidar_link
        geometry_msgs::msg::TransformStamped tf;

        // 头信息
        tf.header.stamp    = this->now();
        tf.header.frame_id = "base_link";    // 从哪个坐标系出发

        // 目标坐标系
        tf.child_frame_id = "lidar_link";    // 到哪个坐标系

        // 位移: 雷达在底盘前方10cm,正中间,上方12cm
        tf.transform.translation.x = 0.1;   // 前方
        tf.transform.translation.y = 0.0;   // 居中
        tf.transform.translation.z = 0.12;  // 上方

        // 旋转: 不转 (四元数默认就是无旋转)
        tf.transform.rotation.x = 0.0;
        tf.transform.rotation.y = 0.0;
        tf.transform.rotation.z = 0.0;
        tf.transform.rotation.w = 1.0;

        // 广播!
        tf_broadcaster_->sendTransform(tf);
    }

    std::shared_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;
    rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<TFBroadcasterNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
