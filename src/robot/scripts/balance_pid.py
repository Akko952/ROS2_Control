#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Twist
from sensor_msgs.msg import Imu
import math

class BalancePIDNode(Node):
    def __init__(self):
        super().__init__('balance_pid_node')
        # PID 参数
        self.kp = 12.0
        self.kd = 0.8
        self.prev_error = 0.0

        self.publisher_ = self.create_publisher(Twist, '/cmd_vel', 10)
        # 👇 这里改成了刚才 ros2 topic list 里查到的真实话题名
        self.subscription = self.create_subscription(Imu, '/imu_sensor_broadcaster/imu', self.imu_callback, 10)

    def imu_callback(self, msg):
        quat = msg.orientation
        sinp = 2 * (quat.w * quat.y - quat.z * quat.x)
        if abs(sinp) >= 1:
            pitch = math.copysign(math.pi / 2, sinp)
        else:
            pitch = math.asin(sinp)
        
        error = -pitch  
        derivative = error - self.prev_error
        self.prev_error = error
        
        correction = self.kp * error + self.kd * derivative
        if correction > 0.3: correction = 0.3
        if correction < -0.3: correction = -0.3

        twist = Twist()
        twist.linear.x = correction
        self.publisher_.publish(twist)

def main(args=None):
    rclpy.init(args=args)
    node = BalancePIDNode()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()