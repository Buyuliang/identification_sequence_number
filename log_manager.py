import subprocess
from tkinter import END

class LogManager:
    def __init__(self):
        """ 初始化 LogManager 类 """

    def upload_file(self, local_file_path, oss_path):
        """
        上传文件到 OSS
        Args:
            local_file_path: 本地文件路径
            oss_path: OSS 目标路径
        Returns:
            bool: 上传是否成功
        """

        try:
            # 构建上传命令
            command = f"ossutil cp -f {local_file_path} {oss_path}"
            result = subprocess.run(command, shell=True, check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
            # print("Upload successful")
            return True  # 上传成功

        except subprocess.CalledProcessError as e:
            return False  # 上传失败

    def download_file(self, oss_path, local_file_path):
        """
        从 OSS 下载文件
        Args:
            oss_path: OSS 文件路径
            local_file_path: 本地保存路径
        Returns:
            bool: 下载是否成功
        """

        try:
            # 构建下载命令
            command = f"ossutil cp -f {oss_path} {local_file_path}"
            result = subprocess.run(command, shell=True, check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
            # print("Download successful")
            return True  # 下载成功

        except subprocess.CalledProcessError as e:
            print("Error during download:", e.stderr)
            return False  # 下载失败
