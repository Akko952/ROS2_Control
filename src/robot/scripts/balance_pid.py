#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Twist
from sensor_msgs.msg import Imu
import math

class BalancePIDNode(Node):
    def __init__(self):
        super().__init__('balance_pid_node')
        self.kp = 18
        self.kd = 0.8
        self.prev_error = 0.0

        self.publisher_ = self.create_publisher(Twist, '/diff_drive_controller/cmd_vel', 10)
        self.subscription = self.create_subscription(Imu, '/imu_sensor_broadcaster/imu', self.imu_callback, 10)

    def imu_callback(self, msg):
        quat = msg.orientation
        sinp = 2 * (quat.w * quat.y - quat.z * quat.x)
        if abs(sinp) >= 1:
            pitch = math.copysign(math.pi / 2, sinp)
        else:
            pitch = math.asin(sinp)
        
        error = pitch  
        derivative = error - self.prev_error
        self.prev_error = error
        
        correction = self.kp * error + self.kd * derivative
        
        if correction > 0.8: correction = 0.8
        if correction < -0.8: correction = -0.8

        # 👇 如果你想看数据，可以取消下面这行的注释
        self.get_logger().info(f'Pitch: {pitch:.3f}, Correction: {correction:.3f}')

        twist = Twist()
        # ===== 👇 终极关键：加上负号，让电机反向输出 👇 =====
        twist.linear.x = correction  
        # =================================================
        self.publisher_.publish(twist)

def main(args=None):
    rclpy.init(args=args)
    node = BalancePIDNode()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()