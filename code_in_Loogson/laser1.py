import serial
import threading
import time

class RaspberryPiSerial:
    def __init__(self, port='/dev/ttyUSB0', baudrate=115200):
        """
        初始化串口
        :param port: 串口设备 (树莓派通常为 /dev/ttyS0 或 /dev/ttyAMA0)
        :param baudrate: 波特率 (默认9600)
        """
        self.port = port
        self.baudrate = baudrate
        self.serial = None
        self.running = False

    def open(self):
        """打开串口连接"""
        try:
            self.serial = serial.Serial(
                port=self.port,
                baudrate=self.baudrate,
                timeout=1
            )
            self.running = True
            # 启动接收线程
            threading.Thread(target=self._receive_thread, daemon=True).start()
            print(f"串口已打开 {self.port}@{self.baudrate}bps")
            return True
        except serial.SerialException as e:
            print(f"无法打开串口: {e}")
            return False

    def close(self):
        """关闭串口连接"""
        if self.serial and self.serial.is_open:
            self.running = False
            self.serial.close()
            print("串口已关闭")

    def send(self, message):
        """发送消息"""
        if self.serial and self.serial.is_open:
            try:
                self.serial.write(message.encode('utf-8'))
                print(f"发送: {message}")
            except Exception as e:
                print(f"发送失败: {e}")
        else:
            print("串口未打开")

    def _receive_thread(self):
        """接收数据的线程"""
        while self.running and self.serial and self.serial.is_open:
            try:
                # 检查是否有数据可读
                if self.serial.in_waiting > 0:
                    # 读取数据
                    data = self.serial.read(self.serial.in_waiting)
                    if data:
                        print(f"接收: {data.decode('utf-8').strip()}")
            except Exception as e:
                print(f"接收错误: {e}")
                break


if __name__ == "__main__":
    # 创建串口实例
    serial_com = RaspberryPiSerial(port='/dev/ttyUSB0', baudrate=115200)

    # 打开串口
    if not serial_com.open():
        exit(1)

    try:
        # print("串口通信测试 (输入 'quit' 退出)")
        # while True:
        #     # 获取用户输入
        #     user_input = input("输入要发送的消息: ")+"\r\n"
        #
        #     if user_input.lower() == 'quit\r\n':
        #         break

            # 发送消息
            time.sleep(2)
            serial_com.send("M3 S1000.000\nG1 F1000\n$J=G91X5.0F300\n$J=G91Y5.0F300\n$J=G91X-5.0F300\n$J=G91Y-5.0F300\r\n")
            time.sleep(6)
            serial_com.send("M5 S0\nG0\r\n")
    finally:
        # 确保程序退出时关闭串口
        serial_com.close()