# ROS2 第五课：Gazebo 物理仿真——让机器人动起来

## 本课目标

- 理解 Gazebo 在 ROS2 生态中的角色：**物理引擎**
- 把 URDF 模型丢进仿真世界，让它受重力、摩擦力、碰撞约束
- 用键盘遥控机器人在仿真世界中行驶
- 体会 "Gazebo 插件" 是怎么把 `/cmd_vel` 翻译成轮子转动的

---

## 前置概念

### Gazebo 是什么

```
RViz                          Gazebo
──────                        ──────
可视化工具                     物理仿真引擎
只看不改                       真的在算力
无物理                         有重力/碰撞/摩擦
"照镜子"                       "进沙盘"
```

Gazebo 用物理引擎（ODE/Bullet/DART）计算每 1ms 物体的位置、速度、碰撞。你的机器人在里面会**真的倒下、打滑、撞墙**。

### 怎么把 URDF 和 Gazebo 连起来

```
URDF 文件
    │
    ├── <link> 的 <collision> + <inertial>  ← Gazebo 物理计算用
    │
    └── <gazebo> 插件  ← 给 Gazebo 上"驱动程序"
            │
            ├── libgazebo_ros_diff_drive.so  ← 差速驱动
            │     订阅 /cmd_vel → 算左右轮转速 → 驱动物理关节
            │     读物理状态 → 发布 /odom
            │
            └── libgazebo_ros_laser.so 等    ← 传感器插件（后续）
```

**一句话**：URDF 是结构蓝图，`<gazebo>` 插件是驱动——没有插件，Gazebo 不知道你的机器人是车还是飞机。

---

## 机器人配置速查

| 参数 | 值 |
|------|-----|
| 底盘 (base_link) | 0.4×0.3×0.15m, 5kg |
| 驱动轮 | radius=0.06m, 轮距=0.36m |
| 万向轮 (caster) | sphere r=0.03m, 低摩擦 |
| LiDAR | 固定在底盘前方 10cm 上方 12cm |
| 差速驱动插件 | 订阅 `/cmd_vel`, 发布 `/odom` |

---

## 步骤 1：安装依赖 + 复制文件

```bash
# 安装键盘遥控
sudo apt install ros-humble-teleop-twist-keyboard

# 复制更新的代码
cp -r /mnt/c/Users/Akko/WorkBuddy/2026-06-22-23-55-56/ros2_ws/src/my_robot_description ~/ros2_ws/src/
```

## 步骤 2：编译

```bash
cd ~/ros2_ws
source /opt/ros/humble/setup.bash
colcon build --packages-select my_robot_description
source install/setup.bash
```

## 步骤 3：启动 Gazebo 仿真

```bash
ros2 launch my_robot_description gazebo.launch.py
```

你会看到：
- Gazebo 窗口弹出，画面中心是你的橙色机器人
- 地面是灰色的，上面站着你的机器人
- 终端输出 `[spawn_entity]: Spawn Entity successful`

> **注意**：第一次启动 Gazebo 会比较慢（下载模型），耐心等 10-20 秒。

## 步骤 4：键盘遥控！（新开终端）

```bash
cd ~/ros2_ws
source install/setup.bash
ros2 run teleop_twist_keyboard teleop_twist_keyboard --ros-args -r cmd_vel:=/cmd_vel
```

按 `i` 前进、`,` 后退、`j` 左转、`l` 右转、`k` 停止。

你应该看到机器人在 Gazebo 里真的开起来了，轮子在转！

## 步骤 5：验证数据流

再开一个终端：

```bash
source /opt/ros/humble/setup.bash

# 看里程计数据
ros2 topic echo /odom
# 应该看到位姿和速度在变化

# 看 TF 树
ros2 run tf2_tools view_frames
# 打开生成的 frames.pdf 查看: odom → base_link → left_wheel, right_wheel, lidar_link
```

---

## 关键代码解读

### Gazebo 差速驱动插件做了什么

```xml
<gazebo>
  <plugin name="diff_drive" filename="libgazebo_ros_diff_drive.so">
    <left_joint>left_wheel_joint</left_joint>    <!-- 控制哪个关节 -->
    <right_joint>right_wheel_joint</right_joint>
    <wheel_separation>0.36</wheel_separation>    <!-- 轮距 = 0.18×2 -->
    <wheel_diameter>0.12</wheel_diameter>         <!-- 轮径 = 0.06×2 -->
    <command_topic>cmd_vel</command_topic>        <!-- 谁发控制指令 -->
    <odometry_topic>odom</odometry_topic>         <!-- 里程计发到哪里 -->
    <publish_tf>true</publish_tf>                 <!-- 自动发 odom→base_link -->
  </plugin>
</gazebo>
```

这个插件内部做的事：

```
收到 /cmd_vel (线速度v, 角速度w)
    │
    ▼
差速运动学解算
    │  v_left  = (2v - w·L) / (2r)
    │  v_right = (2v + w·L) / (2r)
    │  (L=轮距, r=轮半径)
    ▼
Gazebo 关节力控 → 物理引擎积分
    │
    ▼
读取轮子编码器 → 里程计推算
    │
    ▼
发布 /odom + odom→base_link TF
```

### launch 文件结构

| 组件 | 做什么 |
|------|--------|
| `gazebo` | 启动 Gazebo 仿真器 |
| `robot_state_publisher` | 读 URDF → 发 `/robot_description` |
| `spawn_entity.py` | 把机器人放进 Gazebo 世界 |
| `teleop_twist_keyboard` | （手动开启）键盘发 `/cmd_vel` |

---

## 成功标准

- [ ] Gazebo 正常启动，机器人站在地面上
- [ ] 按 `i` 机器人前进，`,` 后退，`j`/`l` 转向
- [ ] `ros2 topic echo /odom` 有数据输出
- [ ] `ros2 run tf2_tools view_frames` 能生成正确的 TF 树
- [ ] 能回答：`libgazebo_ros_diff_drive.so` 帮我们做了什么？

---

## 常见问题

**Q: 机器人弹飞或疯狂抖动？**

惯性参数太大或太小。检查 `<mass>` 是否合理（底盘 5kg、轮子 0.3kg），以及万向轮摩擦系数是否够低。

**Q: 机器人不动？**

1. 确认 `teleop_twist_keyboard` 的终端是**当前活跃窗口**（点一下）
2. 用 `ros2 topic echo /cmd_vel` 确认是否有数据
3. Gazebo 左下角点 ▶ 确认仿真在运行中

**Q: 两个驱动轮在碰撞中穿透地面？**

在 wheel 的 `<collision>` 里加 `<origin rpy="1.5708 0 0"/>`（和 visual 对齐），否则碰撞体和视觉体朝向不一致。

---

## 知识总结

```
URDF                    Gazebo                    RViz + Teleop
─────                   ──────                    ─────────────
静态结构                 物理世界                  交互界面
<collision>             重力、碰撞、摩擦           可视化
<inertial>              差速驱动插件              键盘遥控
<link> <joint>          发布 /odom                看 /odom 数据
```

**下一阶段**：给机器人装上激光雷达传感器，在 Gazebo 里建地图，然后用 SLAM 让机器人自己认路。这就是阶段 4 的后半部分。

准备好了告诉我。
