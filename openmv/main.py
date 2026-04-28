# Edge Impulse - OpenMV Object Detection Example

import sensor, image, time, os, tf, math, uos, gc
from pyb import UART

# 摄像头设置
sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)
sensor.set_windowing((240, 240))
sensor.skip_frames(time=2000)

# 串口设置
uart = UART(3, 9600)

# 模型加载
net = None
labels = None
min_confidence = 0.5

try:
    net = tf.load("trained.tflite", load_to_fb=uos.stat('trained.tflite')[6] > (gc.mem_free() - (64*1024)))
except Exception as e:
    raise Exception('Failed to load "trained.tflite": ' + str(e))

try:
    labels = [line.rstrip('\n') for line in open("labels.txt")]
except Exception as e:
    raise Exception('Failed to load "labels.txt": ' + str(e))

# 配色（用于画框）
colors = [
    (255, 0, 0), (0, 255, 0), (255, 255, 0),
    (0, 0, 255), (255, 0, 255), (0, 255, 255),
    (255, 255, 255),
]

# 播报状态记录
label_flags = {}  # 每个标签是否已播报
last_send_time = time.ticks_ms()
send_interval = 2000  # 每次播报间隔不少于 2 秒（防抖）

clock = time.clock()

while True:
    clock.tick()
    img = sensor.snapshot()
    current_time = time.ticks_ms()

    detected_this_frame = set()

    for i, detection_list in enumerate(net.detect(img, thresholds=[(math.ceil(min_confidence * 255), 255)])):
        if i == 0 or len(detection_list) == 0:
            continue

        label = labels[i]
        detected_this_frame.add(label)

        # 如果标签未播报过，且距离上次播报超过设定时间
        if (not label_flags.get(label, False)) and time.ticks_diff(current_time, last_send_time) > send_interval:
            uart.write(label + '\n')  # 发送标签字符串给 51 单片机
            print("识别并播报标签：", label)
            label_flags[label] = True  # 标记为已播报
            last_send_time = current_time

        # 可视化画图（每个检测框中心画圆）
        for d in detection_list:
            x, y, w, h = d.rect()
            center_x = math.floor(x + w / 2)
            center_y = math.floor(y + h / 2)
            img.draw_circle((center_x, center_y, 12), color=colors[i % len(colors)], thickness=2)

    # 如果某个标签这一帧没有检测到，重置其播报状态
    for label in label_flags:
        if label not in detected_this_frame:
            label_flags[label] = False  # 允许下次重新播报

    # 输出帧率
    print(clock.fps(), "fps")
