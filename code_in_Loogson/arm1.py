import socket

# 配置TCP连接参数
HOST = '192.168.43.238'  # 修改为ESP8266的IP地址
PORT = 8000            # ESP8266的TCP端口

def send_tcp_command(command):
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect((HOST, PORT))
            print(f"发送命令: {command}")
            s.sendall((command + ';').encode())
            response = s.recv(1024).decode().strip()
            print(f"收到响应: {response}")
            return response
    except Exception as e:
        print(f"错误: {e}")
        return None

# 发送命令"1"
send_tcp_command("1")