// sensor_subscriber.cpp
// 订阅自定义消息 SensorData 并打印

#include <string>

#include <rclcpp/rclcpp.hpp>
#include <my_first_pkg/msg/sensor_data.hpp>

class SensorSubscriber : public rclcpp::Node
{
public:
    SensorSubscriber() : Node("sensor_subscriber")
    {
        this->declare_parameter<std::string>("topic_name", "sensor_data");

        std::string topic_name;
        this->get_parameter("topic_name", topic_name);

        subscription_ = this->create_subscription<my_first_pkg::msg::SensorData>(
            topic_name, 10,
            std::bind(&SensorSubscriber::topic_callback, this, std::placeholders::_1));

        RCLCPP_INFO(this->get_logger(), "Sensor subscriber on '%s'", topic_name.c_str());
    }

private:
    void topic_callback(const my_first_pkg::msg::SensorData & msg) const
    {
        RCLCPP_INFO(this->get_logger(),
            "Received [%s]: T=%.1fC H=%.1f%% V=(%.3f, %.3f)",
            msg.sensor_id.c_str(),
            msg.temperature, msg.humidity,
            msg.vibration_x, msg.vibration_y);
    }

    rclcpp::Subscription<my_first_pkg::msg::SensorData>::SharedPtr subscription_;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<SensorSubscriber>());
    rclcpp::shutdown();
    return 0;
}
