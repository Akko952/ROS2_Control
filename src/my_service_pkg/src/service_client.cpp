// service_client.cpp
// 第二课：Service 客户端
// 比喻：打电话的人，拨号发两个数字，等对方算完返回结果
//
// 跟 subscriber 的区别：
// subscriber 是"开着收音机等消息"
// client 是"主动打一个电话，等对方回话"

#include <rclcpp/rclcpp.hpp>
#include <example_interfaces/srv/add_two_ints.hpp>

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);

    // client 不需要继承 Node，直接创建一个节点就行
    auto node = rclcpp::Node::make_shared("calculator_client");

    // 创建客户端：连接到 "add_two_ints" 这个服务
    // 就像查到对方的电话号码
    auto client = node->create_client<example_interfaces::srv::AddTwoInts>("add_two_ints");

    // 等服务端上线（最多等 1 秒）
    // 就像等对方开机，没人接就打不通
    if (!client->wait_for_service(std::chrono::seconds(1))) {
        RCLCPP_ERROR(node->get_logger(), "服务没上线，打不通！");
        return 1;
    }

    // 准备请求：填两个数字
    auto request = std::make_shared<example_interfaces::srv::AddTwoInts::Request>();
    request->a = 4;
    request->b = 9;

    RCLCPP_INFO(node->get_logger(), "拨号中: %ld + %ld = ?", request->a, request->b);

    // 发送请求，等结果回来
    auto result_future = client->async_send_request(request);

    // 阻塞等待对方回话
    if (rclcpp::spin_until_future_complete(node, result_future) ==
        rclcpp::FutureReturnCode::SUCCESS)
    {
        RCLCPP_INFO(node->get_logger(), "收到回复: %ld", result_future.get()->sum);
    } else {
        RCLCPP_ERROR(node->get_logger(), "打电话失败，没收到回复");
    }

    rclcpp::shutdown();
    return 0;
}
