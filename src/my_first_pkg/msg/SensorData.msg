# 传感器数据消息
# 每条号冒号前是类型，冒号后是字段名

string sensor_id           # 传感器编号，如 "front_lidar"
float64 temperature        # 温度 (摄氏度)
float64 humidity           # 湿度 (0-100)
float64 vibration_x        # X方向振动 (m/s²)
float64 vibration_y        # Y方向振动 (m/s²)
