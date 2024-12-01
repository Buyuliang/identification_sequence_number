import glob
import tkinter as tk
import os
from ui_components import UIComponents

class DeviceManager:
    def __init__(self, ui_components):
        """
        :param ui_components: 包含设备选择相关组件的 UI 组件实例
        """
        self.ui_components = ui_components
        self.device_list = []

    def refresh_device_list(self):
        """ 刷新设备列表 """
        self.device_list = self._get_connected_devices()
        # 清空当前设备菜单
        self.ui_components.device_menu['menu'].delete(0, 'end')

        # 将设备添加到菜单中
        for device in self.device_list:
            self.ui_components.device_menu['menu'].add_command(
                label=str(device), 
                command=tk._setit(self.ui_components.device_var, str(device))
            )

    def _get_connected_devices(self):
        """ 获取连接的设备列表，动态扫描 /dev 目录 """
        video_devices = []
        # 扫描 /dev 目录下的视频设备
        for i in range(10):  # 假设最多有10个视频设备（视频设备可能有 video0, video1, ...）
            device_path = f"/dev/video{i}"
            if os.path.exists(device_path):  # 如果设备存在，则添加到列表
                video_devices.append(device_path)
        return video_devices

    def get_selected_device(self):
        """ 获取当前选中的设备 """
        return self.ui_components.device_var.get()
