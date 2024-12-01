import tkinter as tk
from tkinter import Button, Label, Text, Entry, Checkbutton

class UIComponents:
    def __init__(self, root):
        # 主框架（容器）
        self.frame = tk.Frame(root)
        self.frame.place(x=0, y=0, relwidth=1, relheight=1)  # 使用 place 设置固定位置与尺寸

        # 左侧区域 - 摄像头显示
        self.left_frame = tk.Frame(self.frame, width=800, height=600)
        self.left_frame.place(x=10, y=10)  # 左侧框架，放置在 (10, 10) 位置
        self.video_label = tk.Label(self.left_frame, relief="solid", bg="black")
        self.video_label.place(x=0, y=0, width=800, height=600)  # 视频显示框，填充左侧区域

        # 右侧区域 - 控件和输入框
        self.right_frame = tk.Frame(self.frame, relief="solid")
        self.right_frame.place(x=820, y=10)  # 右侧框架，放置在 (820, 10) 位置

        # 右侧区域分为两列
        self.left_column_frame = tk.Frame(self.right_frame)
        self.left_column_frame.grid(row=0, column=0, sticky="n")  # 左侧列区域
        self.right_column_frame = tk.Frame(self.right_frame)
        self.right_column_frame.grid(row=0, column=1, sticky="n")  # 右侧列区域

        # 摄像头设备选择菜单（最顶部）
        self.device_var = tk.StringVar()
        self.device_var.set("选择设备")  # 默认选项
        self.device_menu = tk.OptionMenu(self.left_column_frame, self.device_var, "无可用设备")  # 初始值为 "无可用设备"
        self.device_menu.grid(row=0, column=0, padx=10, pady=10, sticky="w")  # 放置设备选择框

        # SN 号文本框
        self.sn_label = Label(self.left_column_frame, text="SN 号", relief="solid", width=15, anchor="w")
        self.sn_label.grid(row=1, column=0, padx=10, pady=5, sticky="w")  # SN 号标签
        self.sn_input = Entry(self.left_column_frame, width=30)
        self.sn_input.grid(row=1, column=1, padx=10, pady=5, sticky="w")  # SN 号输入框

        # 前三码和后四码输入框
        self.vendor_label = Label(self.left_column_frame, text="前三码", relief="solid", width=15, anchor="w")
        self.vendor_label.grid(row=2, column=0, padx=10, pady=5, sticky="w")  # 前三码标签
        self.vendor_input = Entry(self.left_column_frame, width=30, state="disabled")
        self.vendor_input.grid(row=2, column=1, padx=10, pady=5, sticky="w")  # 前三码输入框

        self.model_label = Label(self.left_column_frame, text="后四码", relief="solid", width=15, anchor="w")
        self.model_label.grid(row=3, column=0, padx=10, pady=5, sticky="w")  # 后四码标签
        self.model_input = Entry(self.left_column_frame, width=30, state="disabled")
        self.model_input.grid(row=3, column=1, padx=10, pady=5, sticky="w")  # 后四码输入框

        # 添加复选框来控制前三码和后四码输入框的状态
        self.edit_vendor_model = tk.IntVar(value=0)  # 默认不选中，前三码和后四码不可编辑

        # 单一复选框控制两个输入框的编辑状态
        self.editable_checkbutton = Checkbutton(self.left_column_frame, text="编辑前三码/后四码", variable=self.edit_vendor_model, command=self.toggle_vendor_model)
        self.editable_checkbutton.grid(row=4, column=0, columnspan=2, padx=10, pady=10, sticky="w")  # 放置复选框，宽度填满整个列

        # 上移的控件：开始检测按钮和启用自动检测复选框
        self.start_button = Button(self.left_column_frame, text="开始检测", width=20, height=2)
        self.start_button.grid(row=5, column=0, columnspan=2, padx=10, pady=10)  # 设置按钮位置，正常排列

        self.auto_detect = tk.IntVar(value=0)  # 默认值：不启用自动检测
        self.auto_detect_checkbutton = Checkbutton(self.left_column_frame, text="启用自动检测", variable=self.auto_detect, command=self.set_detector_button_disable)
        self.auto_detect_checkbutton.grid(row=6, column=0, columnspan=2, padx=10, pady=10)  # 启用自动检测复选框，放置在按钮下方

        # 识别结果文本框，放在启用自动检测下方，设置 columnspan=2 让其占两列
        self.result_label = Label(self.left_column_frame, text="识别结果", relief="solid", width=15, anchor="w")
        self.result_label.grid(row=7, column=0, padx=10, pady=10, sticky="w")  # 识别结果标签
        self.result_text = Text(self.left_column_frame, width=50, height=13, wrap="word", bd=2, relief="sunken")
        self.result_text.grid(row=8, column=0, columnspan=2, padx=10, pady=10, sticky="w")  # 识别结果文本框，跨两列显示

        # 右侧列 - 状态和日志输出区域
        self.status_label = Label(self.right_column_frame, text="状态", relief="solid", width=15, anchor="w")
        self.status_label.grid(row=0, column=0, padx=10, pady=10, sticky="w")  # 状态标签
        self.status_text = Text(self.right_column_frame, width=40, height=14, wrap="word", bd=2, relief="sunken")
        self.status_text.grid(row=1, column=0, padx=10, pady=10, sticky="w")  # 状态文本框

        self.log_label = Label(self.right_column_frame, text="日志输出", relief="solid", width=15, anchor="w")
        self.log_label.grid(row=2, column=0, padx=10, pady=10, sticky="w")  # 日志输出标签
        self.log_text = Text(self.right_column_frame, width=40, height=14, wrap="word", bd=2, relief="sunken")
        self.log_text.grid(row=3, column=0, padx=10, pady=10, sticky="w")  # 日志输出文本框

        # 更新窗口尺寸
        root.update_idletasks()  # 更新窗口，计算所有控件的尺寸

        # 计算窗口的总宽度和高度，考虑到所有控件的边距
        frame_width = self.left_frame.winfo_width() + self.right_frame.winfo_width() + 20  # 左右两边框架宽度和间距
        frame_height = self.left_frame.winfo_height() + 20  # 左侧框架高度和间距

        window_width = frame_width + 20  # 加上窗口边框
        window_height = frame_height + 10  # 加上额外的边距空间

        # 调整窗口大小
        print(f"Calculated window_width: {window_width}, window_height: {window_height}")
        root.geometry(f"{window_width}x{window_height}")  # 设置窗口大小，确保所有控件都能显示

    def toggle_vendor_model(self):
        """ 切换前三码和后四码输入框的编辑状态 """
        if self.edit_vendor_model.get() == 1:
            self.vendor_input.config(state="normal")  # 可编辑状态
            self.model_input.config(state="normal")   # 可编辑状态
        else:
            self.vendor_input.config(state="disabled")  # 禁用状态
            self.model_input.config(state="disabled")   # 禁用状态

    def set_edit_vendor_model(self, value):
        """手动设置 edit_vendor_model 为 1 或 0"""
        self.edit_vendor_model.set(value)
        self.toggle_vendor_model()  # 立即更新输入框状态

    def set_detector_button_disable(self):
        """ 切换检测按键的编辑状态 """
        if self.auto_detect.get() == 0:
            self.start_button.config(state="normal")  # 可编辑状态
        else:
            self.start_button.config(state="disabled")  # 禁用状态

    # 获取输入框中的文本内容
    def get_vendor(self):
        """ 获取 SN 输入框内容 """
        return self.vendor_input.get()

    # 获取输入框中的文本内容
    def get_model(self):
        """ 获取 SN 输入框内容 """
        return self.model_input.get()

    # 获取输入框中的文本内容
    def get_sn(self):
        """ 获取 SN 输入框内容 """
        return self.sn_input.get()

    # 清除 SN 输入框内容
    def clear_sn_input(self):
        self.sn_input.delete(0, tk.END)

    # 光标设置到 SN 输入框
    def focus_on_sn_input(self):
        self.sn_input.focus()

    # 设置复选框状态
    def set_editable_checkbutton(self, state):
        self.edit_vendor_model.set(state)

    # 更新识别结果文本框内容
    def update_result_text(self, text):
        self.result_text.delete(1.0, tk.END)
        self.result_text.insert(tk.END, text)

    # 清空日志输出
    def clear_log_text(self):
        self.log_text.delete(1.0, tk.END)

    # 追加日志输出
    def append_log_text(self, text):
        self.log_text.insert(tk.END, text + "\n")

    # 更新状态文本框内容
    def update_log_text(self, text):
        self.log_text.delete(1.0, tk.END)
        self.log_text.insert(tk.END, text)

    # 更新状态文本框内容
    def update_status_text(self, text):
        self.status_text.delete(1.0, tk.END)
        self.status_text.insert(tk.END, text)

    # 获取自动检测复选框状态
    def get_auto_detect_state(self):
        return self.auto_detect.get()

    def set_video_display(self, img=None):
        """ 
        设置视频标签的显示内容
        如果 img 为 None，则清空视频标签内容，否则显示传入的图像
        """
        if img is None:
            # 如果没有传入图像，则清空视频标签
            self.video_label.configure(image='')  # 清空视频标签的显示内容
        else:
            # 否则，显示传入的图像
            self.video_label.imgtk = img
            self.video_label.configure(image=img)
