// service_server.cpp
// 第二课：Service 服务端
// 比喻：一个"接电话的"计算器，收到两个数字就返回它们的和
//
// 跟 publisher 的骨架一模一样：继承 Node → 构造函数里准备工具 → main 里 spin
// 唯一区别：准备的不是"对讲机+闹钟"，而是"电话"

#include <rclcpp/rclcpp.hpp>
#include <example_interfaces/srv/add_two_ints.hpp>

class CalculatorServer : public rclcpp::Node
{
public:
    CalculatorServer() : Node("calculator_server")
    {
        // 创建 Service：服务名叫 "add_two_ints"，收到请求就执行 add_callback
        // 就像登记一个分机号，别人打这个号你就接
        service_ = this->create_service<example_interfaces::srv::AddTwoInts>(
            "add_two_ints",
            std::bind(&CalculatorServer::add_callback, this,
                      std::placeholders::_1, std::placeholders::_2));

        RCLCPP_INFO(this->get_logger(), "计算器服务已启动，等待电话...");
    }

private:
    // 这个函数在"接到电话"时自动执行
    // request = 对方说的话（两个数字）
    // response = 你要回的话（计算结果）
    void add_callback(
        const std::shared_ptr<example_interfaces::srv::AddTwoInts::Request> request,
        std::shared_ptr<example_interfaces::srv::AddTwoInts::Response> response)
    {
        // 算加法
        response->sum = request->a + request->b;

        RCLCPP_INFO(this->get_logger(),
            "收到请求: %ld + %ld = %ld",
            request->a, request->b, response->sum);
    }

    rclcpp::Service<example_interfaces::srv::AddTwoInts>::SharedPtr service_;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<CalculatorServer>());
    rclcpp::shutdown();
    return 0;
}
