#include "CameraThread.h"
#include <QImage>
#include <QPixmap>
#include <QMutex>
#include <opencv2/opencv.hpp>  // Assuming you're using OpenCV

CameraThread::CameraThread(QObject *parent)
    : QThread(parent), isRunning(true)
{
    // 不再直接初始化摄像头，而是在调用 setDevice 时再初始化
}

CameraThread::~CameraThread()
{
    stop();
}

void CameraThread::run()
{
    // 使用设备路径来初始化摄像头
    cv::VideoCapture cap(devicePath.toStdString());  // 使用设备路径来打开摄像头
    if (!cap.isOpened()) {
        devicePath.clear();  // 设置 devicePath 为空
        emit errorSignal("无法打开摄像头！");  // 如果设备路径无效，发送错误信号
        return;
    }

    while (isRunning) {
        cv::Mat frame;
        cap >> frame;
        if (frame.empty()) {
            break;  // 如果读取的帧为空，跳出循环
        }

        // 转换为RGB格式以便显示
        cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
        QImage img(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888);
        QPixmap pixmap = QPixmap::fromImage(img);

        // 发送新帧信号
        emit newFrameSignal(pixmap);
    }
}

void CameraThread::stop()
{
    isRunning = false;
    wait();  // 等待线程结束
}

void CameraThread::setDevice(const QString &devicePath)
{
    // 保存设备路径
    this->devicePath = devicePath;

    // 停止当前线程
    isRunning = false;
    wait();

    // 重启线程
    isRunning = true;
    start();  // 重新启动线程并加载新设备
}
