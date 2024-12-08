#ifndef DETECTION_H
#define DETECTION_H

#include <QObject>
#include <QString>
#include <atomic>
#include <thread>
#include <QThread>
#include "LogManager.h"
#include "CameraThread.h"

class DetectionWorker : public QThread
{
    Q_OBJECT

public:
    DetectionWorker(CameraThread *cameraThread, const QString &sn, const QString &vendor, const QString &model, const QString &devicePath);
    // explicit DetectionWorker(CameraThread *cameraThread, const QString &sn, const QString &vendor, const QString &model, const QString &devicePath, QObject *parent = nullptr);
    void run() override;
    cv::Mat getCurrentFrame();

signals:
    void updateResultTextSignal(const QString &text);   // 更新结果文本信号
    void appendLogTextSignal(const QString &text);      // 日志文本信号
    void updateStatusTextSignal(const QString &text);   // 更新状态文本信号

private:
    CameraThread *cameraThread;  // 摄像头线程
    QString sn;           // 设备序列号
    QString vendor;       // 厂商信息
    QString model;        // 设备型号
    QString devicePath;   // 设备路径
};

class Detection : public QObject
{
    Q_OBJECT

public:
    explicit Detection(CameraThread *camerathread, QObject *parent = nullptr); 
    ~Detection();

    void startDetection(const QString &sn, const QString &vendor, const QString &model, const QString &devicePath);
    void autoStartDetection(const QString &sn, const QString &vendor, const QString &model, const QString &devicePath);
    void stopAutoDetection();

signals:
    void updateResultTextSignal(const QString &text);   // 更新结果文本
    void appendLogTextSignal(const QString &text);      // 日志输出
    void updateStatusTextSignal(const QString &text);   // 更新状态文本

private:
    DetectionWorker *worker; // 后台线程对象

    std::atomic<bool> autoDetectState;  // 自动检测状态
    std::thread autoDetectThread;       // 自动检测线程
    void _autoDetectTask(const QString &sn, const QString &vendor, const QString &model, const QString &devicePath);  // 自动检测任务

    void cleanUpAutoDetectThread(); // 用于清理自动检测线程

    CameraThread *cameraThread;  // 摄像头线程
};

#endif // DETECTION_H
