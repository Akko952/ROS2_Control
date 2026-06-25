// minimal_publisher.cpp
// 升级版：话题名和发布频率支持参数配置

#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>

#include <string>

using namespace std::chrono_literals;

class MinimalPublisher : public rclcpp::Node
{
public:
    MinimalPublisher() : Node("minimal_publisher"), count_(0)
    {
        // 声明参数：话题名（可从 launch 文件 / 命令行传入）
        this->declare_parameter<std::string>("topic_name", "topic");

        // 声明参数：发布间隔（毫秒）
        this->declare_parameter<int>("publish_rate_ms", 500);

        // 读取参数值
        std::string topic_name;
        this->get_parameter("topic_name", topic_name);

        int publish_rate_ms;
        this->get_parameter("publish_rate_ms", publish_rate_ms);

        // 用参数创建 publisher 和 timer
        publisher_ = this->create_publisher<std_msgs::msg::String>(topic_name, 10);

        timer_ = this->create_wall_timer(
            std::chrono::milliseconds(publish_rate_ms),
            std::bind(&MinimalPublisher::timer_callback, this));

        RCLCPP_INFO(this->get_logger(),
            "Publisher started: topic='%s', rate=%dms",
            topic_name.c_str(), publish_rate_ms);
    }

private:
    void timer_callback()
    {
        auto message = std_msgs::msg::String();
        message.data = "Hello, world! " + std::to_string(count_++);
        RCLCPP_INFO(this->get_logger(), "Publishing: '%s'", message.data.c_str());
        publisher_->publish(message);
    }

    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;
    rclcpp::TimerBase::SharedPtr timer_;
    size_t count_;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<MinimalPublisher>());
    rclcpp::shutdown();
    return 0;
}
