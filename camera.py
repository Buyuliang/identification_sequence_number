import threading
import cv2
from PIL import Image, ImageTk
from time import sleep

class CameraHandler:
    def __init__(self, root, ui_components):
        self.ui_components = ui_components
        self.root = root
        self.cap = None  # 摄像头对象
        self.current_frame = None  # 当前帧
        self.video_thread = None  # 用于更新视频流的线程

    def init_camera(self, device):
        """ 初始化摄像头设备 """
        if self.cap:
            self.cap.release()  # 释放当前摄像头
        self.cap = cv2.VideoCapture(device)
        if not self.cap.isOpened():
            self.ui_components.append_log_text(f"无法打开摄像头设备: {device}")
            return None

        # 启动视频流更新线程
        if self.video_thread is None or not self.video_thread.is_alive():
            self.video_thread = threading.Thread(target=self._update_video_stream)
            self.video_thread.daemon = True  # 设置为守护线程，退出时自动结束
            self.video_thread.start()

        return self.cap

    def _update_video_stream(self):
        """ 子线程方法，用于持续读取视频帧 """
        while self.cap.isOpened():
            ret, frame = self.cap.read()
            if ret:
                self.update_video_frame(frame)
            else:
                break
            # sleep(0.03)  # 控制帧率，大约每秒30帧

    def update_video_frame(self, frame):
        """ 更新视频流并保存当前帧 """
        self.current_frame = frame  # 保存当前帧
        frame_rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        img = ImageTk.PhotoImage(Image.fromarray(frame_rgb))
        self.root.after(0, self._update_label, img)

    def _update_label(self, img):
        """ 更新Tkinter的图像标签 """
        self.ui_components.video_label.imgtk = img
        self.ui_components.video_label.configure(image=img)

    def stop_video_stream(self):
        """ 停止当前视频流 """
        if self.cap:
            self.cap.release()  # 释放摄像头
            self.cap = None  # 清空当前的摄像头对象
            self.ui_components.append_log_text("视频流已停止")
        else:
            self.ui_components.append_log_text("没有活动的视频流可停止")
        
        self.current_frame = None  # 清空当前帧数据
        # 清空显示的视频内容
        self.ui_components.set_video_display()  # 清空视频标签的显示内容
        self.ui_components.append_log_text("检测已停止")

    def get_current_frame(self):
        return self.current_frame
