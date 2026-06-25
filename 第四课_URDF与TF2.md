# ROS2 第四课：URDF 机器人建模 + TF2 坐标变换

## 本课目标

- 理解 URDF 是什么、TF2 是什么、两者如何配合
- 用 URDF 描述一个四轮差速驱动机器人
- 在 RViz 中可视化机器人模型
- 手写 TF2 代码广播 / 监听坐标变换
- 体会 "URDF 自动发 TF" vs "手写 TF2" 的区别

---

## 前置概念

### URDF — 机器人哪块骨头长什么样

URDF（Unified Robot Description Format）是一个 XML 文件，回答三个问题：

```
这个机器人 → 由哪些零件组成？（links）
           → 零件之间怎么连接？（joints）
           → 每个零件多重、多大？（inertial / geometry）
```

**类比**：URDF 就是机器人的「解剖图谱」——告诉你头骨 (link)、颈椎 (joint)、肋骨 (link) 各自长什么样、怎么连的。

### TF2 — 这些骨头现在在什么位置

TF2（Transform Frame）是一个**实时坐标系广播系统**，回答：

```
"lidar 坐标系 相对于 base 坐标系，在当前这一刻，位移和旋转是多少？"
```

**类比**：URDF 是解剖图谱（静态结构），TF2 是**运动追踪器**（动态位置）。医生看图谱知道骨头的样子，但要追踪病人的动作需要传感器数据。

### 两者如何配合

```
URDF 文件
    │
    ▼
robot_state_publisher（ROS2 自带节点）
    │  读取 URDF 的 joint 定义
    │  结合 /joint_states（关节角度）
    │  自动计算所有 link 之间的坐标变换
    ▼
/tf 话题  ←── TF2 广播
    │
    ▼
RViz / tf2_echo / 你的代码  ←── TF2 监听
```

**关键理解**：URDF 定义了「可能的变换」，TF2 广播「当前的变换」。对于固定关节（fixed joint），变换永远不变；对于旋转关节（continuous joint），变换随关节角度变化。

---

## 我们的机器人长什么样

```
          ┌─────────────┐
          │   LiDAR      │ ← lidar_link（青色圆柱，固定在底盘上）
          │  (cyan)      │
          └──────┬──────┘
                 │ fixed joint
    ┌────────────┼────────────┐
    │       ┌────┴────┐      │
    │       │ 底盘     │      │ ← base_link（橙色盒子 0.4×0.3×0.15m，5kg）
    │       │(orange)  │      │
    │       └──────────┘      │
    │   ⚫              ⚫    │ ← 左右轮（深灰圆柱 radius=0.06m）
    │ left_wheel    right_wheel│   continuous joint（无限制旋转）
    └─────────────────────────┘
                    ⚪          ← caster_wheel（万向轮，前方支撑）
```

- **4 个 link**：base_link, left_wheel, right_wheel, lidar_link, caster_wheel
- **4 个 joint**：left_wheel_joint (continuous), right_wheel_joint (continuous), lidar_joint (fixed), caster_joint (fixed)

---

## 步骤 1：把代码复制到 WSL

你需要复制**两个包**到 WSL：

```bash
# 创建目录（如果还没有）
mkdir -p ~/ros2_ws/src

# 复制 my_first_pkg（含 TF2 broadcaster）
cp -r /mnt/c/Users/Akko/WorkBuddy/2026-06-22-23-55-56/ros2_ws/src/my_first_pkg ~/ros2_ws/src/

# 复制 my_robot_description（含 URDF 和 launch）
cp -r /mnt/c/Users/Akko/WorkBuddy/2026-06-22-23-55-56/ros2_ws/src/my_robot_description ~/ros2_ws/src/
```

## 步骤 2：编译全部

```bash
cd ~/ros2_ws
source /opt/ros/humble/setup.bash
colcon build --packages-select my_first_pkg my_robot_description
source ~/ros2_ws/install/setup.bash
```

## 步骤 3：在 RViz 中查看机器人模型

```bash
# 确保 source 过
source ~/ros2_ws/install/setup.bash

ros2 launch my_robot_description display.launch.py
```

这会自动打开 RViz，你应该看到：

- 橙色盒子底盘 + 深灰轮子 + 青色 LiDAR + 灰色万向球
- 左侧 `joint_state_publisher_gui` 窗口，可以拖动左右轮的关节滑块

> **注意**：如果 RViz 没显示模型，检查左下角 `Fixed Frame` 是否设为 `base_link`。

**此时发生了什么？**

```
launch 文件启动了三个节点：
┌──────────────────────────────────┐
│ robot_state_publisher            │ ← 读 URDF，发 /robot_description
│ joint_state_publisher_gui        │ ← 发 /joint_states（滑块控制关节角度）
│ rviz2                            │ ← 订阅 /robot_description，画机器人
└──────────────────────────────────┘
        ↓
robot_state_publisher 自动计算所有 TF（根据 URDF + 关节角度）
        ↓
/tf 话题上自动出现 base_link→left_wheel, base_link→lidar_link 等变换
```

## 步骤 4：验证自动 TF

在另一个终端：

```bash
source /opt/ros/humble/setup.bash

# 查看 base_link 到 lidar_link 的变换
ros2 run tf2_ros tf2_echo base_link lidar_link

# 应该看到：
# Translation: [0.100, 0.000, 0.120]
# Rotation: [0.000, 0.000, 0.000, 1.000]
```

这和你 URDF 里 `lidar_joint` 的 `<origin xyz="0.1 0.0 0.12">` 完全一致。

## 步骤 5：手写 TF2 广播器（对比理解）

关掉上一个 launch（Ctrl+C），我们来试试不靠 URDF，**纯手写代码广播 TF**：

```bash
source ~/ros2_ws/install/setup.bash

# 启动手写 TF 广播器
ros2 run my_first_pkg tf_broadcaster
```

在另一个终端验证：

```bash
ros2 run tf2_ros tf2_echo base_link lidar_link
```

效果完全一样！但这回是**你的代码**在发 /tf，而不是 robot_state_publisher。

---

## 代码解读

### URDF 核心结构

```xml
<!-- 每个 link 描述一个刚体 -->
<link name="base_link">
    <visual>      <!-- RViz 中长什么样 -->
    <collision>   <!-- Gazebo 物理碰撞用 -->
    <inertial>    <!-- 质量和转动惯量（仿真用） -->
</link>

<!-- 每个 joint 描述两个 link 的连接关系 -->
<joint name="lidar_joint" type="fixed">    <!-- fixed = 焊死不动 -->
    <parent link="base_link"/>             <!-- 父坐标系 -->
    <child  link="lidar_link"/>            <!-- 子坐标系 -->
    <origin xyz="0.1 0.0 0.12"/>          <!-- 子相对于父的位置 -->
</joint>

<joint name="left_wheel_joint" type="continuous">  <!-- continuous = 无限旋转 -->
    <axis xyz="0 1 0"/>                   <!-- 绕 Y 轴转 -->
</joint>
```

**joint 类型速查**：

| 类型 | 含义 | 用途 |
|------|------|------|
| `fixed` | 固定不动 | 传感器支架、万向轮 |
| `continuous` | 无限制旋转 | 驱动轮 |
| `revolute` | 有限角度旋转 | 机械臂关节 |
| `prismatic` | 直线滑动 | 推杆、气缸 |

### TF2 手写广播器（逐行注释）

代码在 `my_first_pkg/src/tf_broadcaster.cpp`，核心逻辑：

```cpp
class TFBroadcasterNode : public rclcpp::Node
{
    // 1. 创建 TransformBroadcaster 对象
    tf_broadcaster_ = std::make_shared<tf2_ros::TransformBroadcaster>(this);

    // 2. 定时器每 100ms 触发一次 publish_tf()
    timer_ = this->create_wall_timer(100ms, ...);

    void publish_tf() {
        TransformStamped tf;
        tf.header.frame_id = "base_link";    // 从哪出发
        tf.child_frame_id  = "lidar_link";   // 到哪去
        tf.transform.translation.x = 0.1;    // 前方 10cm
        tf.transform.translation.z = 0.12;   // 上方 12cm
        tf.transform.rotation.w = 1.0;       // 无旋转（四元数单位值）

        tf_broadcaster_->sendTransform(tf);  // 发到 /tf 话题
    }
};
```

**为什么用四元数？**
欧拉角 (roll, pitch, yaw) 有万向节死锁问题。ROS2 统一用四元数 (x, y, z, w) 表示旋转。无旋转时：`x=0, y=0, z=0, w=1`。

### Launch 文件解读

`display.launch.py` 启动了三个节点：

| 节点 | 做什么 |
|------|--------|
| `robot_state_publisher` | 读 URDF + 订阅 `/joint_states` → 计算所有 TF → 发 `/tf` |
| `joint_state_publisher_gui` | GUI 滑块 → 发 `/joint_states` |
| `rviz2` | 订阅 `/robot_description` 和 `/tf` → 渲染 3D 模型 |

---

## 成功标准

- [ ] `ros2 launch my_robot_description display.launch.py` 后 RViz 显示完整的四轮机器人
- [ ] `joint_state_publisher_gui` 拖动滑块，轮子在 RViz 中跟着转
- [ ] `ros2 run tf2_ros tf2_echo base_link lidar_link` 输出正确位移 `[0.1, 0.0, 0.12]`
- [ ] `ros2 run my_first_pkg tf_broadcaster` 后 `tf2_echo` 同样能看到变换
- [ ] 能回答：URDF 和 TF2 各负责什么？fixed joint 和 continuous joint 有什么区别？

---

## 知识总结

```
URDF（静态）              TF2（动态）
─────────────            ─────────────
机器人结构定义            实时坐标变换广播
XML 文件                 /tf 话题
只写一次                 持续发布
robot_state_publisher    手写 TransformBroadcaster
     读取                      或
      ↓                  robot_state_publisher
/tf 变换                      自动发布
```

**一句话**：URDF 描述「谁连着谁、长什么样」，TF2 广播「谁现在在哪」。

---

## 常见问题

**Q: 为什么 left_wheel 的 visual 里要 `<origin rpy="1.5708 0.0 0.0"/>`？**

因为圆柱默认竖着（沿 Z 轴），但轮子要横着（沿 Y 轴）。`rpy="1.5708 0 0"` 就是把圆柱绕 X 轴转 90°（π/2 ≈ 1.5708 弧度）放平。

**Q: `<origin xyz="0.0 0.18 -0.05"/>` 是什么意思？**

- `x=0.0`：轮子在底盘前后方向的中心
- `y=0.18`：轮子在底盘左侧 18cm 处
- `z=-0.05`：轮心比底盘中心低 5cm（底盘厚 15cm，轮心在底盘底面）

**Q: 什么时候用 URDF，什么时候手写 TF2？**

- 机器人**物理结构固定**（底盘、轮子、传感器支架）→ 写 URDF，让 `robot_state_publisher` 自动处理
- **动态/逻辑坐标系**（导航目标点、检测到的物体位置）→ 手写 TF2 广播

---

## 下一步

掌握了 URDF + TF2 后，你已经可以：
1. 为自己的机器人建 3D 模型并在 RViz 中可视化
2. 理解 ROS2 中所有坐标系是怎么维护的

准备好了就告诉我，下一阶段进入**Gazebo 仿真**——让你的机器人模型在物理引擎里真正动起来。
