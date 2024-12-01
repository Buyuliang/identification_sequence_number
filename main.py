import tkinter as tk
from camera import CameraHandler
from device_manager import DeviceManager
from ui_components import UIComponents
from detector import Detection  # 导入 Detection 类
from log_manager import LogManager

class AIApp(tk.Tk):
    def __init__(self):
        super().__init__()

        self.title("AI Camera Detection")
        # self.geometry("1200x800")

        # 初始化 UI 组件
        self.ui_components = UIComponents(self)

        # 初始化设备管理器，并依赖注入 ui_components
        self.device_manager = DeviceManager(self.ui_components)

        # 初始化摄像头管理器，并依赖注入 ui_components
        self.camera_handler = CameraHandler(self, self.ui_components)

        self.log_manager = LogManager()

        # 创建检测类实例，并将 UI 组件实例、摄像头管理器和结果处理器传递给它
        self.detection = Detection(self.ui_components, self.camera_handler, self.log_manager)

        # 刷新设备列表
        self.device_manager.refresh_device_list()

        # 设置设备选择变化的事件处理
        self.ui_components.device_var.trace_add('write', self.on_device_select)

        self.ui_components.start_button.config(command=self.start_detection)
        self.ui_components.auto_detect.trace_add('write', self.auto_start_detection)

        # 默认初始化第一个设备
        self.init_camera()

    def init_camera(self):
        """ 自动初始化摄像头并开始视频流 """
        device = self.device_manager.get_selected_device()
        if device is not None:
            # 如果是设备路径，直接传递给摄像头初始化
            self.camera_handler.init_camera(device)
            self.update_video_frame()

    def on_device_select(self, *args):
        """ 当选择设备时更新视频流 """
        # 获取选择的设备路径
        device = self.device_manager.get_selected_device()
        if device is not None:
            # 停止当前视频流
            self.camera_handler.stop_video_stream()

            # 重新初始化并更新视频流
            self.camera_handler.init_camera(device)
            self.update_video_frame()

    def update_video_frame(self, *args):
        """ 实时更新视频帧 """
        if not self.camera_handler.cap:
            return
        ret, frame = self.camera_handler.cap.read()
        if ret:
            self.camera_handler.update_video_frame(frame)

    def start_detection(self, *args):
        """ 启动检测流程 """
        device = self.device_manager.get_selected_device()
        if device:
            self.detection.start_detection(device)

    def auto_start_detection(self, *args):
        """ 启动检测流程 """
        device = self.device_manager.get_selected_device()
        if device:
            self.detection.auto_start_detection(device)

if __name__ == "__main__":
    app = AIApp()
    app.mainloop()
