import socket
import struct
import pandas as pd

# 初始化一个空的DataFrame，用于存储数据
data_frame = pd.DataFrame()

# 解析基站到电脑的报文
def parse_message(message):
    header_expected = 0xAA
    min_length_required = 12  # 1 byte for header + 4 bytes for uint32 timestamp + 3*2 bytes for uint16 x, y, z

    if len(message) < min_length_required:
        print(f"Received message too short: expected at least {min_length_required}, got {len(message)}")
        return None

    header = message[0]
    if header != header_expected:
        print(f"Incorrect header: expected {header_expected}, got {header}")
        return None
    
    data_points = []
    offset = 1
    
    for i in range(5):  # 循环解析5组（时间戳 + ADC 数据）
        if offset + 10 > len(message):
            break  # 如果剩余数据不足以解析，跳出循环
        try:
            timestamp, x, y, z = struct.unpack('<Ihhh', message[offset:offset+10])
        except struct.error:
            continue  # 如果数据不完整或有错误，继续下一轮解析

        data_points.append((timestamp, x, y, z))
        offset += 10

    return data_points


# 更新DataFrame
def update_dataframe(data_points):
    global data_frame
    # 遍历数据点并添加到全局DataFrame中
    for timestamp, gyro_x, gyro_y, gyro_z in data_points:
        # 创建一个新的DataFrame来存储当前数据点
        print(f"Timestamp: {timestamp}, Gyro X: {gyro_x}, Gyro Y: {gyro_y}, Gyro Z: {gyro_z}")

        new_row = pd.DataFrame({
            'Timestamp': [timestamp],
            'Gyro X': [gyro_x],
            'Gyro Y': [gyro_y],
            'Gyro Z': [gyro_z]
        })
        # 将新数据追加到全局DataFrame中
        data_frame = pd.concat([data_frame, new_row], ignore_index=True)
    
    # 对全局DataFrame按时间戳进行排序
    data_frame.sort_values(by='Timestamp', inplace=True)
    # 实时保存DataFrame到Excel文件
    data_frame.to_excel("output.xlsx", index=False)


# 从UDP读取并解析数据
def read_from_udp(udp_port):
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind(('', udp_port))  # 绑定端口
    sock.settimeout(1)  # 设置1秒超时以允许定期检查是否应该退出
    print(f"Listening on UDP port {udp_port}")

    running = True  # 控制循环的变量
    try:
        while running:  # 无限循环，持续监听UDP
            try:
                data, addr = sock.recvfrom(300)  
                if data:
                    parsed_data = parse_message(data)
                    if parsed_data:
                        update_dataframe(parsed_data)
            except socket.timeout:
                pass  # 捕获超时，不做任何事情，继续循环
    except KeyboardInterrupt:
        print("\nProgram terminated by user (Ctrl + C).")
        running = False  # 设置为False，以结束循环
    finally:
        sock.close()  # 确保退出时关闭套接字
        print("Socket closed.")
        print("Data saved to Excel.")


if __name__ == "__main__":
    udp_port = 12345  # 定义UDP端口号
    # 读取UDP数据并解析
    read_from_udp(udp_port)
