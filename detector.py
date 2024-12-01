# 导入 UIComponents 和 CameraHandler
import threading, cv2, csv, os, glob, time, subprocess
import numpy as np
from datetime import datetime
from time import sleep
from ui_components import UIComponents
from camera import CameraHandler
from typing import Callable
from log_manager import LogManager

class Detection:
    def __init__(self, ui_component: UIComponents, camera_handler: CameraHandler, logs_manager: LogManager):
        """
        :param ui_component: UI 组件实例，用于更新界面上的结果
        :param camera_handler: 摄像头管理器，负责初始化、获取帧、停止等操作
        :param logs_manager: 上传下载日志
        """
        self.ui = ui_component  # UI 组件实例，将被用于更新界面上的检测结果
        self.camera_handler = camera_handler  # 摄像头管理器实例
        self._logs_manager = logs_manager  # 初始化 LogManager

    def _start_detection(self, device):
        """
        启动检测的流程，获取当前帧并保存为图片，执行 shell 命令进行检测，下载日志文件
        :param device: 摄像头设备名
        """
        # 从 UI 文本框获取 SN 号
        self.ui.set_edit_vendor_model(0)
        sn = self.ui.get_sn()
        if not sn:
            self.ui.update_log_text("请输入设备 SN 号！")
            return

        vendor = self.ui.get_vendor()
        if not sn:
            self.ui.update_log_text("请输入设备 vendor 号！前三码")
            return

        model = self.ui.get_model()
        if not sn:
            self.ui.update_log_text("请输入设备 model 号 后四码！")
            return

        if device == "无可用设备":
            self.ui.update_log_text("请选择一个有效的设备！")
            return

        # 如果摄像头未初始化，则初始化摄像头
        if self.camera_handler.cap is None:
            self.camera_handler.init_camera(device)
            if not self.camera_handler.cap:
                self.ui.update_log_text("无法打开摄像头设备！")
                return
        self.ui.clear_sn_input()
        # 获取当前帧
        frame = self.camera_handler.get_current_frame()
        if frame is not None:
            # 获取当前时间，生成文件名
            timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
            image_filename = f"{sn}_{timestamp}.png"
            image_path = os.path.join("./pic/", image_filename)

            # 保存图片
            cv2.imwrite(image_path, frame)
            
            # 执行 shell 命令开始检测
            os.environ["LD_LIBRARY_PATH"] = "./lib"  # 修改为你的 lib 路径
            # command = ["./rknn_ppocr_system_demo", "model/ppocrv4_det.rknn", "model/ppocrv4_rec.rknn", image_path]
            command = ["echo", image_path]
            try:
                result = subprocess.run(command, capture_output=True, text=True, check=True)
                self.ui.update_log_text(f"检测结果:\n{result.stdout}\n")
                results = []
                # 显示 OCR 结果
                if os.path.exists("text.txt"):
                    with open("text.txt", "r") as file:
                        lines = file.readlines()

                    # 查找包含 'RK3288' 的行
                    for i, line in enumerate(lines):
                        if "RK3288" in line:
                            if i + 1 < len(lines):  # 确保有下一行
                                next_line = "".join(lines[i + 1].split())
                                # 检查字符匹配
                                if not (next_line[:3] == vendor and next_line[-4:] == model):
                                    self.ui.append_log_text("检测失败：OCR 结果中，'RK3288' 下一行字符与输入不匹配。\n")
                                    start_progress()
                                    self.ui.update_result_text("FAIL")
                                    return

                                # 保存数据到列表
                                results.append([sn, next_line, vendor, model, timestamp])
                                if not self._logs_manager.download_file("oss://az05/checkCpu/", "results.csv"):
                                    self.ui.append_log_text("日志下载失败...\n")

                                # 检查 CSV 文件是否存在
                                file_exists = os.path.exists("results.csv")

                                # 打开或创建 CSV 文件进行写入
                                with open("results.csv", 'a', newline='', encoding='utf-8') as csv_file:
                                    writer = csv.writer(csv_file)

                                    # 如果文件不存在，写入标题行
                                    if not file_exists:
                                        writer.writerow(["SN号", "content", "Vendor", "Model", "date"])
                                    
                                    # 写入数据
                                    writer.writerows(results)
                                # print(f"CSV 文件已保存到: {csv_output_path}")
                                self.ui.append_log_text("CSV 文件已保存到本地: results.csv\n")
                                if not self._logs_manager.upload_file("results.csv", "oss://az05/checkCpu/"):
                                    self.ui.append_log_text("日志上传失败...\n")
                                break
                    else:
                        self.ui.append_log_text("检测失败：OCR 结果中未找到 'RK3288'")
                        self.ui.update_result_text("FAIL")
                else:
                    # print("no found test.txt")
                    self.ui.append_log_text(f"no found test.txt\n")
            except subprocess.CalledProcessError as e:
                self.ui.append_log_text(f"检测失败: {e.stderr}\n")
        else:
            self.ui.append_log_text("无法获取摄像头帧！")

    def start_detection(self, device):
        """ 启动检测 """
        detection_thread = threading.Thread(target=self._start_detection, args=(device,))
        detection_thread.start()

    def auto_start_detection(self, device):
        """ 启动自动检测 """
        if self.ui.get_auto_detect_state():
            # 创建并启动一个新的子线程来执行自动检测任务
            detection_thread = threading.Thread(target=self._auto_detect_task, args=(device,))
            detection_thread.start()

    def _auto_detect_task(self, device):
        """ 子线程任务: 持续执行检测 """
        while self.ui.get_auto_detect_state():
            self._start_detection(device)
            sleep(1)
