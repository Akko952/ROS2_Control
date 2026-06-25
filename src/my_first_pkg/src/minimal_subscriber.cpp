// minimal_subscriber.cpp
// 升级版：话题名支持参数配置

#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>

#include <string>

class MinimalSubscriber : public rclcpp::Node
{
public:
    MinimalSubscriber() : Node("minimal_subscriber")
    {
        // 声明参数：话题名
        this->declare_parameter<std::string>("topic_name", "topic");

        std::string topic_name;
        this->get_parameter("topic_name", topic_name);

        subscription_ = this->create_subscription<std_msgs::msg::String>(
            topic_name, 10,
            std::bind(&MinimalSubscriber::topic_callback, this, std::placeholders::_1));

        RCLCPP_INFO(this->get_logger(),
            "Subscriber started: topic='%s'", topic_name.c_str());
    }

private:
    void topic_callback(const std_msgs::msg::String & msg) const
    {
        RCLCPP_INFO(this->get_logger(), "I heard: '%s'", msg.data.c_str());
    }

    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscription_;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<MinimalSubscriber>());
    rclcpp::shutdown();
    return 0;
}
