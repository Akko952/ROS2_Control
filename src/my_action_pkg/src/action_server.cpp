// action_server.cpp
// 第三课：Action 服务端 — 数数机器人
// 比喻：外卖骑手，接到订单后每秒汇报进度，数完返回结果
//
// 跟之前两课的骨架一样：继承 Node → 构造函数里准备工具 → main 里 spin
// 区别：这次准备的是"接订单系统"（Action Server）

#include <rclcpp/rclcpp.hpp>
#include <rclcpp_action/rclcpp_action.hpp>
#include <example_interfaces/action/fibonacci.hpp>

// 用 Fibonacci action 来做数数练习
// 它的结构：
//   Goal:     order（数到第几个）
//   Feedback: sequence（当前数到哪了）
//   Result:   sequence（最终完整序列）

using Fibonacci = example_interfaces::action::Fibonacci;

class CountingServer : public rclcpp::Node
{
public:
    CountingServer() : Node("counting_server")
    {
        // 创建 Action Server：服务名叫 "counting"
        // 就像开通一个外卖接单账号
        action_server_ = rclcpp_action::create_server<Fibonacci>(
            this,
            "counting",
            std::bind(&CountingServer::handle_goal, this,
                      std::placeholders::_1, std::placeholders::_2),
            std::bind(&CountingServer::handle_cancel, this,
                      std::placeholders::_1),
            std::bind(&CountingServer::handle_accepted, this,
                      std::placeholders::_1));

        RCLCPP_INFO(this->get_logger(), "数数机器人已上线，等待下单...");
    }

private:
    rclcpp_action::Server<Fibonacci>::SharedPtr action_server_;

    // 回调1：有人下单了，接不接？
    // 返回 ACCEPT 表示接单，REJECT 表示拒单
    rclcpp_action::GoalResponse handle_goal(
        const rclcpp_action::GoalUUID &,
        std::shared_ptr<const Fibonacci::Goal> goal)
    {
        RCLCPP_INFO(this->get_logger(), "收到订单：数到 %d", goal->order);
        return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
    }

    // 回调2：客户要取消订单，同意吗？
    rclcpp_action::CancelResponse handle_cancel(
        const std::shared_ptr<rclcpp_action::ServerGoalHandle<Fibonacci>>)
    {
        RCLCPP_INFO(this->get_logger(), "客户取消了订单");
        return rclcpp_action::CancelResponse::ACCEPT;
    }

    // 回调3：接单成功，开始干活
    void handle_accepted(
        const std::shared_ptr<rclcpp_action::ServerGoalHandle<Fibonacci>> goal_handle)
    {
        // 开一个新线程去数数，不阻塞主线程
        std::thread{std::bind(&CountingServer::execute, this, std::placeholders::_1), goal_handle}.detach();
    }

    // 实际干活的函数：数数 + 汇报进度
    void execute(const std::shared_ptr<rclcpp_action::ServerGoalHandle<Fibonacci>> goal_handle)
    {
        auto goal = goal_handle->get_goal();
        auto feedback = std::make_shared<Fibonacci::Feedback>();
        auto result = std::make_shared<Fibonacci::Result>();

        // 数数过程：每数一个，汇报一次进度
        feedback->sequence = {0, 1};
        for (int i = 1; i < goal->order; ++i) {
            // 检查是否被取消了
            if (goal_handle->is_canceling()) {
                result->sequence = feedback->sequence;
                goal_handle->canceled(result);
                RCLCPP_INFO(this->get_logger(), "订单已取消");
                return;
            }

            // 算下一个数
            int next = feedback->sequence[i] + feedback->sequence[i - 1];
            feedback->sequence.push_back(next);

            // 汇报进度（发 feedback）
            goal_handle->publish_feedback(feedback);
            RCLCPP_INFO(this->get_logger(), "进度汇报：已数到第 %d 个，当前值 %d",
                        i + 1, next);

            // 间隔 1 秒，模拟干活需要时间
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        // 数完了，返回最终结果
        result->sequence = feedback->sequence;
        goal_handle->succeed(result);
        RCLCPP_INFO(this->get_logger(), "订单完成！");
    }
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<CountingServer>());
    rclcpp::shutdown();
    return 0;
}
