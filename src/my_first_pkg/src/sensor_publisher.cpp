// sensor_publisher.cpp
// 用自定义消息 SensorData 发布模拟传感器数据

#include <chrono>
#include <cmath>
#include <string>

#include <rclcpp/rclcpp.hpp>
#include <my_first_pkg/msg/sensor_data.hpp>   // 自动生成的头文件

using namespace std::chrono_literals;

class SensorPublisher : public rclcpp::Node
{
public:
    SensorPublisher() : Node("sensor_publisher"), count_(0)
    {
        // 声明可配置参数
        this->declare_parameter<std::string>("topic_name", "sensor_data");
        this->declare_parameter<int>("publish_rate_ms", 1000);

        std::string topic_name;
        int rate;
        this->get_parameter("topic_name", topic_name);
        this->get_parameter("publish_rate_ms", rate);

        // 用自定义消息类型创建发布器
        publisher_ = this->create_publisher<my_first_pkg::msg::SensorData>(topic_name, 10);

        timer_ = this->create_wall_timer(
            std::chrono::milliseconds(rate),
            std::bind(&SensorPublisher::timer_callback, this));

        RCLCPP_INFO(this->get_logger(), "Sensor publisher on '%s'", topic_name.c_str());
    }

private:
    void timer_callback()
    {
        auto msg = my_first_pkg::msg::SensorData();

        // 像填结构体一样填字段
        msg.sensor_id    = "front_lidar";
        msg.temperature  = 25.0 + 2.0 * std::sin(count_ * 0.1);  // 模拟波动
        msg.humidity     = 42.0 + std::cos(count_ * 0.15);
        msg.vibration_x  = 0.03 * std::sin(count_ * 0.2);
        msg.vibration_y  = 0.01 * std::cos(count_ * 0.25);

        RCLCPP_INFO(this->get_logger(),
            "[%s] T=%.1fC H=%.1f%% V=(%.3f, %.3f)",
            msg.sensor_id.c_str(),
            msg.temperature, msg.humidity,
            msg.vibration_x, msg.vibration_y);

        publisher_->publish(msg);
        count_++;
    }

    rclcpp::Publisher<my_first_pkg::msg::SensorData>::SharedPtr publisher_;
    rclcpp::TimerBase::SharedPtr timer_;
    size_t count_;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<SensorPublisher>());
    rclcpp::shutdown();
    return 0;
}
