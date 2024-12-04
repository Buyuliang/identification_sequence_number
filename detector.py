# 导入 UIComponents 和 CameraHandler
import threading, cv2, csv, os, glob, time, subprocess, random, configparser
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
        self.model_counts = {}  # 记录每个 model_input 识别成功的计数
        self.model_colors = {}  # 存储每个模型对应的固定颜色

        # 读取配置文件
        self.config = self._load_config()
        self.oss_path = self.config.get('Paths', 'oss_path', fallback='oss://az05/serial_number/')
        self.csv_file = self.config.get('Paths', 'csv_file', fallback='results.csv')
        self.image_dir = self.config.get('Paths', 'image_dir', fallback='./pic')  # 读取图片存放目录

    def _load_config(self):
        """
        读取配置文件
        """
        config = configparser.ConfigParser()
        config.read('config.ini')  # 加载配置文件
        return config

    def _start_detection(self, device):
        """
        启动检测的流程，获取当前帧并保存为图片，执行 shell 命令进行检测，下载日志文件
        :param device: 摄像头设备名
        """
        # 从 UI 文本框获取 SN 号
        self.ui.set_edit_vendor_model(0)
        self.ui.clear_result_text()
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

        if model not in self.model_counts:
            self.model_counts[model] = 0

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
            image_path = os.path.join(self.image_dir, image_filename)
            # 获取绝对路径
            absolute_image_path = os.path.abspath(image_path)
            # 检查并创建目录
            directory = os.path.dirname(absolute_image_path)
            if not os.path.exists(directory):
                os.makedirs(directory)
            cv2.imwrite(absolute_image_path, frame)

            if os.path.isfile("text.txt"):
                os.remove("text.txt")
            # 执行 shell 命令开始检测
            os.environ["LD_LIBRARY_PATH"] = "./lib"  # 修改为你的 lib 路径
            command = ["./rknn_ppocr_system_demo", "model/ppocrv4_det.rknn", "model/ppocrv4_rec.rknn", image_path]
            try:
                result = subprocess.run(command, capture_output=True, text=True, check=True)
                self.ui.update_log_text(f"检测结果:\n{result.stdout}\n")
                results = []
                # 显示 OCR 结果
                if os.path.exists("text.txt"):
                    with open("text.txt", "r") as file:
                        lines = file.readlines()

                    for line in lines:
                        # 去掉行首尾空白字符并检查是否以 'vendor' 开头
                        cleaned_line = "".join(line.split())  # 去除空白字符
                        if cleaned_line.startswith(vendor) and cleaned_line.endswith(model):
                            # 保存数据到列表
                            results.append([sn, cleaned_line, vendor, model, timestamp])
                            
                            # 下载 results.csv 文件
                            if not self._logs_manager.download_file(self.oss_path + self.csv_file, self.csv_file):
                                self.ui.append_log_text("日志下载失败，可能不存在 results.csv 或检查网络连接...\n", font_color="red")

                            # 检查是否存在 results.csv 或临时文件 results.csv.tmp
                            csv_file_path = self.csv_file
                            temp_csv_file_path = self.csv_file + ".tmp"

                            file_to_write = csv_file_path if os.path.exists(csv_file_path) else temp_csv_file_path
                            file_exists = os.path.exists(file_to_write)

                            # 打开或创建文件进行写入
                            with open(file_to_write, 'a', newline='', encoding='utf-8') as csv_file:
                                writer = csv.writer(csv_file)

                                # 如果文件不存在，写入标题行
                                if not file_exists:
                                    writer.writerow(["SN", "content", "Vendor", "Model", "date"])

                                # 写入数据
                                writer.writerows(results)

                            if file_to_write == csv_file_path:
                                # 上传已存在的 results.csv 文件
                                self.ui.append_log_text(f"CSV 文件已保存到本地: {csv_file_path}\n")
                                if not self._logs_manager.upload_file(csv_file_path, self.oss_path):
                                    self.ui.append_log_text("日志上传失败...\n", font_color="red")
                                    self.ui.update_result_text("FAIL", font_size=64, font_color="red")
                            else:
                                # 临时文件不上传，仅记录日志
                                self.ui.append_log_text(f"CSV 文件已保存到本地临时文件: {temp_csv_file_path}\n")
                                self.ui.append_log_text("测试PASS\n", font_color="green")
                                self.ui.append_log_text("日志无法上传，需要检测网络连接...\n",font_color="red")
                                self.ui.update_result_text("FAIL", font_size=64, font_color="red")
                                break

                            self.ui.update_result_text("PASS", font_size=64, font_color="green")
                            self.model_counts[model] += 1
                            break
                    else:
                        # 如果没有找到匹配的行，输出错误信息
                        self.ui.append_log_text("检测失败：OCR 结果中未找到以 'vendor' 开头且以 'model' 结尾的行。\n")
                        self.ui.update_result_text("FAIL", font_size=64, font_color="red")

                else:
                    self.ui.append_log_text(f"no found text.txt\n")
            except subprocess.CalledProcessError as e:
                self.ui.append_log_text(f"检测失败: {e.stderr}\n")
        else:
            self.ui.append_log_text("无法获取摄像头帧！")
        self.update_result_with_random_colors()

    def _generate_random_color(self, model):
        """
        生成一个随机颜色并为每个模型分配一个固定颜色
        :param model: 模型名
        """
        if model not in self.model_colors:
            # 如果该模型没有颜色，则生成一个随机颜色并保存
            self.model_colors[model] = "#{:02x}{:02x}{:02x}".format(random.randint(0, 255), random.randint(0, 255), random.randint(0, 255))
        return self.model_colors[model]

    def update_result_with_random_colors(self):
        """
        更新所有模型及其计数，并显示每个模型的计数，使用固定颜色
        """
        self.ui.update_status_text("")
        for model, count in self.model_counts.items():
            color = self._generate_random_color(model)  # 使用固定颜色
            self.ui.append_status_text(f"{model}: {count}", font_color=color)

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
            # sleep(1)
