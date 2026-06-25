# ROS2 第三课：Action（动作）— 数数机器人

## 场景
客户端下单"数到 10"，服务端每秒数一个数并汇报进度，数完返回完整结果。

（用 Fibonacci action 做演示，它有 order/sequence/feedback 三个字段，正好对应 goal/result/feedback）

## 步骤 1：把代码复制到 WSL

```bash
cp -r /mnt/c/Users/Akko/WorkBuddy/2026-06-22-23-55-56/ros2_ws/src/my_action_pkg ~/ros2_ws/src/
```

## 步骤 2：编译

```bash
cd ~/ros2_ws
source /opt/ros/humble/setup.bash
colcon build --packages-select my_action_pkg
source install/setup.bash
```

## 步骤 3：运行 server（终端 1）

```bash
source /opt/ros/humble/setup.bash
source ~/ros2_ws/install/setup.bash
ros2 run my_action_pkg action_server
```

预期输出：
```
[INFO] [counting_server]: 数数机器人已上线，等待下单...
```
（然后停住，等订单）

## 步骤 4：运行 client（终端 2）

新开一个 WSL 终端：
```bash
source /opt/ros/humble/setup.bash
source ~/ros2_ws/install/setup.bash
ros2 run my_action_pkg action_client
```

预期输出（client 终端）：
```
[INFO] [counting_client]: 下单：让机器人数到 10
[INFO] [counting_client]: 收到进度：已数到 2 个数
[INFO] [counting_client]: 收到进度：已数到 3 个数
[INFO] [counting_client]: 收到进度：已数到 4 个数
...
[INFO] [counting_client]: 收到进度：已数到 10 个数
[INFO] [counting_client]: 最终结果：0, 1, 1, 2, 3, 5, 8, 13, 21, 34
```

同时 server 终端会打印：
```
[INFO] [counting_server]: 收到订单：数到 10
[INFO] [counting_server]: 进度汇报：已数到第 2 个，当前值 1
[INFO] [counting_server]: 进度汇报：已数到第 3 个，当前值 1
...
[INFO] [counting_server]: 订单完成！
```

## 成功标准

- [ ] server 启动后停在"等待下单"
- [ ] client 发出订单后持续收到进度反馈
- [ ] 大约 10 秒后 client 收到最终结果
- [ ] server 也同步打印进度和完成信息

## 三种通信方式对比

| | Topic | Service | Action |
|---|---|---|---|
| 比喻 | 广播电台 | 打电话 | 外卖下单 |
| 方向 | 单向 | 一问一答 | 下单+进度+结果 |
| 有反馈 | 没有 | 没有 | 有（持续汇报） |
| 可取消 | - | - | 可以 |
| 适合 | 数据流 | 快速操作 | 长时间任务 |
| 代码复杂度 | 最简单 | 中等 | 最复杂 |

## 代码结构

```
my_action_pkg/
├── CMakeLists.txt
├── package.xml
└── src/
    ├── action_server.cpp    ← 接订单 + 干活 + 汇报进度
    └── action_client.cpp    ← 下单 + 收进度 + 收结果
```

### Server 的三个回调

Action server 比 Topic/Service 多了几个回调，对应"外卖接单"的完整流程：

| 回调 | 什么时候触发 | 比喻 |
|------|-------------|------|
| handle_goal | 收到订单时 | 接不接单 |
| handle_cancel | 客户取消时 | 同意取消吗 |
| handle_accepted | 接单后 | 开始干活 |
| execute（在 handle_accepted 里调用） | 干活过程 | 边干边汇报 |

### Client 的两个回调

| 回调 | 什么时候触发 | 比喻 |
|------|-------------|------|
| feedback_callback | 收到进度时 | 看到骑手位置更新 |
| result_callback | 收到最终结果时 | 外卖送到 |

## 下一步

成功后可以尝试：
1. 改 client 里的数字（send_goal(10) 改成 send_goal(5)），重新编译跑
2. 用命令行测试：`ros2 action list` 和 `ros2 action send_goal`
