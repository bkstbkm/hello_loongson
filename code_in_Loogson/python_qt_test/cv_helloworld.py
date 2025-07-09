# This Python file uses the following encoding: utf-8

# if __name__ == "__main__":
#     pass

import cv2
import numpy as np
cap = None
frame_count = 0

def initialize():
    """初始化视频捕获"""
    global cap
    cap = cv2.VideoCapture(0)  # 使用默认摄像头
    if not cap.isOpened():
        raise RuntimeError("Could not open video device")
    return True
def process_frame():
    """处理视频帧并返回处理后的图像"""
    global cap, frame_count
    color = 0
    position = (0,0)
    detected = 0
    if cap is None:
        initialize()

    ret, frame = cap.read()
    if not ret:
        return None

    # 转换为HSV颜色空间（更适合颜色检测）
    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)

    # 定义黑色和绿色的HSV范围
    # 黑色范围
    lower_black = np.array([185, 127, 0])
    upper_black = np.array([255, 195, 57])

    # 紫色范围
    lower_green = np.array([103, 93, 081])
    upper_green = np.array([154, 238, 207])

    # 创建黑色和绿色的掩膜
    mask_black = cv2.inRange(hsv, lower_black, upper_black)
    mask_green = cv2.inRange(hsv, lower_green, upper_green)

    # 形态学操作（去除噪声）
    kernel = np.ones((5, 5), np.uint8)
    mask_black = cv2.morphologyEx(mask_black, cv2.MORPH_OPEN, kernel)
    mask_green = cv2.morphologyEx(mask_green, cv2.MORPH_OPEN, kernel)

    # 查找黑色方块的轮廓
    contours_black, _ = cv2.findContours(mask_black, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

    # 查找绿色方块的轮廓
    contours_green, _ = cv2.findContours(mask_green, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

    # 处理黑色方块
    for cnt in contours_black:
        # 计算轮廓面积，过滤掉小的噪点
        area = cv2.contourArea(cnt)
        if area < 3500:  # 调整这个值以适应你的场景
            continue

        # 计算轮廓的近似多边形（简化轮廓）
        epsilon = 0.02 * cv2.arcLength(cnt, True)
        approx = cv2.approxPolyDP(cnt, epsilon, True)

        # 如果是四边形（4个顶点）
        if len(approx) == 4:
            # 计算中心点
            M = cv2.moments(cnt)
            if M["m00"] != 0:
                cX = int(M["m10"] / M["m00"])
                cY = int(M["m01"] / M["m00"])
                position = (cX, cY)
                color = 1;
                detected = 1;
                # 绘制轮廓和中心点
                cv2.drawContours(frame, [approx], -1, (0, 0, 255), 2)
                cv2.circle(frame, (cX, cY), 5, (0, 0, 255), -1)
                cv2.putText(frame, "Black", (cX - 20, cY - 20),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 0, 255), 2)
                print(f"黑色方块中心坐标: ({cX}, {cY})")
            else:
                detected = 0
    # 处理绿色方块
    for cnt in contours_green:
        # 计算轮廓面积
        area = cv2.contourArea(cnt)
        if area < 3500:  # 调整这个值以适应你的场景
            continue

        # 计算轮廓的近似多边形
        epsilon = 0.02 * cv2.arcLength(cnt, True)
        approx = cv2.approxPolyDP(cnt, epsilon, True)

        # 如果是四边形
        if len(approx) == 4:
            # 计算中心点
            M = cv2.moments(cnt)
            if M["m00"] != 0:
                cX = int(M["m10"] / M["m00"])
                cY = int(M["m01"] / M["m00"])
                position = (cX, cY)
                color = 2;
                detected = 1;
                # 绘制轮廓和中心点
                cv2.drawContours(frame, [approx], -1, (0, 255, 0), 2)
                cv2.circle(frame, (cX, cY), 5, (0, 255, 0), -1)
                cv2.putText(frame, "Green", (cX - 20, cY - 20),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 2)
                print(f"绿色方块中心坐标: ({cX}, {cY})")
            else:
                detected = 0


    # 调整大小以适应Qt窗口
    # frame = cv2.resize(frame, (1280, 960))
    # frame1 = cv2.cvtColor(frame, cv2.COLOR_HSV2BGR)
    frame1 = cv2.resize(frame, (1280, 960))
    # 将图像转换为字节串
    return (
            frame1.tobytes(),  # 图像数据
            color,
            position,                  # (x,y)坐标
            int(detected)              # 检测标志 0/1
        )
    # return frame.tobytes()




def cleanup():
    """清理资源"""
    global cap
    if cap is not None:
        cap.release()
        cap = None
if __name__ == "__main__":
    process_frame()
