#ifndef CAMERATHREAD_H
#define CAMERATHREAD_H

#include <QThread>
#include <QString>
#include <QPixmap>
#include <opencv2/opencv.hpp>  // Assuming you're using OpenCV
#include <QMutex>
#include <QMutexLocker> 

class CameraThread : public QThread
{
    Q_OBJECT

public:
    explicit CameraThread(QObject *parent = nullptr);
    ~CameraThread();

    // 设置摄像头设备路径
    void setDevice(const QString &devicePath);
    cv::Mat captureFrame();

signals:
    // 发射新帧信号，将 pixmap 传递给 UI
    void newFrameSignal(const QPixmap &pixmap);
    
    // 发射错误信号
    void errorSignal(const QString &errorMessage);

public slots:
    void stopCamera() {
        stop();  // 在槽函数中调用 stop()
    }

protected:
    void run() override;  // 重写 run 方法
    void stop();          // 停止摄像头线程

private:
    QString devicePath;  // 摄像头设备路径
    bool isRunning;      // 线程运行状态
    QMutex frameMutex;
    cv::Mat currentFrame;
};

#endif // CAMERATHREAD_H
