# ROS2 第二课：Service（服务）— 一问一答

## 场景
客户端发两个数字 4 和 9，服务端算加法，返回 13。

## 步骤 1：把代码复制到 WSL

```bash
cp -r /mnt/c/Users/Akko/WorkBuddy/2026-06-22-23-55-56/ros2_ws/src/my_service_pkg ~/ros2_ws/src/
```

## 步骤 2：安装依赖

example_interfaces 是 ROS2 自带的消息库，但可能没装：

```bash
sudo apt install -y ros-humble-example-interfaces
```

## 步骤 3：编译

```bash
cd ~/ros2_ws
colcon build --packages-select my_service_pkg
source install/setup.bash
```

## 步骤 4：运行 server（终端 1）

```bash
source /opt/ros/humble/setup.bash
source ~/ros2_ws/install/setup.bash
ros2 run my_service_pkg server
```

预期输出：
```
[INFO] [calculator_server]: 计算器服务已启动，等待电话...
```
（然后停住，等电话）

## 步骤 5：运行 client（终端 2）

新开一个 WSL 终端：
```bash
source /opt/ros/humble/setup.bash
source ~/ros2_ws/install/setup.bash
ros2 run my_service_pkg client
```

预期输出：
```
[INFO] [calculator_client]: 拨号中: 4 + 9 = ?
[INFO] [calculator_client]: 收到回复: 13
```

同时终端 1 的 server 会打印：
```
[INFO] [calculator_server]: 收到请求: 4 + 9 = 13
```

## 步骤 6：用命令行手动测试

不开 client，直接用 ROS2 命令行调用：

```bash
# 终端 1 先跑着 server
# 终端 2 执行：
ros2 service call /add_two_ints example_interfaces/srv/AddTwoInts "{a: 10, b: 20}"
```

预期输出：
```
result:
  sum: 30
```

## 成功标准

- [ ] server 启动后停在"等待电话"
- [ ] client 发出请求并收到 13
- [ ] server 打印出 "收到请求: 4 + 9 = 13"
- [ ] `ros2 service call` 也能正常调用

## 代码结构解读

```
my_service_pkg/
├── CMakeLists.txt
├── package.xml
└── src/
    ├── service_server.cpp   ← 接电话的（计算器）
    └── service_client.cpp   ← 打电话的（用户）
```

### 和第一课的对比

| | 第一课 Topic | 第二课 Service |
|---|---|---|
| server/publisher | 对讲机+闹钟，定时喊 | 电话，等别人打来 |
| client/subscriber | 收音机，被动听 | 电话，主动打过去 |
| 触发方式 | 闹钟定时 / 消息到达 | 客户端主动调用 |
| 有没有返回值 | 没有 | 有 |
| 谁先启动 | 都行 | server 必须先启动 |
| 通信次数 | 持续不断 | 调用一次回一次 |

### 核心模式

server 跟 publisher 骨架一模一样（继承 Node + main 里 spin），只是构造函数里创建的是 Service 而不是 Publisher。

client 不需要继承 Node，因为它是一次性调用完就退出，不需要持续运行。

## 下一步

成功后可以尝试：
1. 改 client 里的数字（request->a 和 request->b），重新编译跑
2. 用 `ros2 service list` 查看所有可用服务
3. 用 `ros2 service type /add_two_ints` 查看服务类型
