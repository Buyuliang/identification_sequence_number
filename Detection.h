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
    DetectionWorker(CameraThread *cameraThread, QString sn, QString vendor, QString model);
    void updateStatusWithVendorModelCount();
    void loadVendorModelData();
    void saveVendorModelData();
    void run() override;
    cv::Mat getCurrentFrame();

    QMap<QString, int> vendorModelMap;  // 保存 vendor + model 拼接后的出现次数
    CameraThread *cameraThread;  // 摄像头线程
    QString sn;           // 设备序列号
    QString vendor;       // 厂商信息
    QString model;        // 设备型号
    QString devicePath;   // 设备路径

signals:
    void updateResultTextSignal(const QString &text, const QColor &color = Qt::black, int fontSize = 12);   // 更新结果文本信号
    void updateLogTextSignal(const QString &text, const QColor &color = Qt::black, int fontSize = 12);      // 更新结日志文本信号
    void appendLogTextSignal(const QString &text, const QColor &color = Qt::black, int fontSize = 12);      // 日志文本信号
    void updateStatusTextSignal(const QString &text, const QColor &color = Qt::black, int fontSize = 12);   // 更新状态文本信号
    void updateSNTextSignal();
    void clearSNInputSignal();
};

class Detection : public QObject
{
    Q_OBJECT

public:
    explicit Detection(CameraThread *camerathread, QObject *parent = nullptr); 
    ~Detection();

    void startDetection();
    void autoStartDetection();
    void stopAutoDetection();

    CameraThread *cameraThread;  // 摄像头线程
    QString sn;           // 设备序列号
    QString vendor;       // 厂商信息
    QString model;        // 设备型号
    QString devicePath;   // 设备路径

signals:
    void updateResultTextSignal(const QString &text, const QColor &color = Qt::black, int fontSize = 12);   // 更新结果文本
    void updateLogTextSignal(const QString &text, const QColor &color = Qt::black, int fontSize = 12);      // 更新结日志文本信号
    void appendLogTextSignal(const QString &text, const QColor &color = Qt::black, int fontSize = 12);      // 日志输出
    void updateStatusTextSignal(const QString &text, const QColor &color = Qt::black, int fontSize = 12);   // 更新状态文本
    void updateSNTextSignal();
    void clearSNInputSignal();

private:
    DetectionWorker *worker; // 后台线程对象

    std::atomic<bool> autoDetectState;  // 自动检测状态
    std::thread autoDetectThread;       // 自动检测线程
    void _autoDetectTask();  // 自动检测任务

    void cleanUpAutoDetectThread(); // 用于清理自动检测线程
};

#endif // DETECTION_H
