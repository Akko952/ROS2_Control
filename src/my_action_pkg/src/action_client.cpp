// action_client.cpp
// 第三课：Action 客户端 — 下单的人
// 比喻：外卖下单，等骑手汇报进度，收到最终结果
//
// 跟 Service client 的区别：
// Service client = 打电话，干等回话
// Action client = 下单，持续收进度，最后收结果

#include <rclcpp/rclcpp.hpp>
#include <rclcpp_action/rclcpp_action.hpp>
#include <example_interfaces/action/fibonacci.hpp>

using Fibonacci = example_interfaces::action::Fibonacci;

class CountingClient : public rclcpp::Node
{
public:
    CountingClient() : Node("counting_client")
    {
        // 创建 Action Client：连接到 "counting" 这个服务
        // 就像打开外卖 APP，找到这家店
        client_ = rclcpp_action::create_client<Fibonacci>(this, "counting");

        // 发送订单：数到 10
        send_goal(10);
    }

private:
    rclcpp_action::Client<Fibonacci>::SharedPtr client_;

    void send_goal(int target)
    {
        // 等服务端上线（最多等 2 秒）
        if (!client_->wait_for_action_server(std::chrono::seconds(2))) {
            RCLCPP_ERROR(this->get_logger(), "数数机器人没上线！");
            return;
        }

        // 填写订单：数到 target
        auto goal_msg = Fibonacci::Goal();
        goal_msg.order = target;

        RCLCPP_INFO(this->get_logger(), "下单：让机器人数到 %d", target);

        // 配置回调：
        // - 收到进度时调用 goal_response_callback
        // - 收到 feedback 时调用 feedback_callback
        // - 收到最终结果时调用 result_callback
        auto send_goal_options = rclcpp_action::Client<Fibonacci>::SendGoalOptions();
        send_goal_options.feedback_callback =
            std::bind(&CountingClient::feedback_callback, this,
                      std::placeholders::_1, std::placeholders::_2);
        send_goal_options.result_callback =
            std::bind(&CountingClient::result_callback, this,
                      std::placeholders::_1);

        // 发送订单
        client_->async_send_goal(goal_msg, send_goal_options);
    }

    // 收到进度反馈时调用
    void feedback_callback(
        rclcpp_action::ClientGoalHandle<Fibonacci>::SharedPtr,
        const std::shared_ptr<const Fibonacci::Feedback> feedback)
    {
        RCLCPP_INFO(this->get_logger(), "收到进度：已数到 %zu 个数",
                    feedback->sequence.size());
    }

    // 收到最终结果时调用
    void result_callback(
        const rclcpp_action::ClientGoalHandle<Fibonacci>::WrappedResult & result)
    {
        std::string seq_str;
        for (size_t i = 0; i < result.result->sequence.size(); ++i) {
            if (i > 0) seq_str += ", ";
            seq_str += std::to_string(result.result->sequence[i]);
        }
        RCLCPP_INFO(this->get_logger(), "最终结果：%s", seq_str.c_str());
        rclcpp::shutdown();
    }
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<CountingClient>());
    return 0;
}
