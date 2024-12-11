#ifndef DETECTION_H
#define DETECTION_H

#include <QObject>
#include <QString>
#include <atomic>
#include <thread>
#include <QThread>
#include "LogManager.h"
#include "CameraThread.h"

class Detection : public QObject
{
    Q_OBJECT

public:
    explicit Detection(CameraThread *camerathread, QObject *parent = nullptr);
    void updateStatusWithVendorModelCount();
    void loadVendorModelData();
    void saveVendorModelData();
    cv::Mat getCurrentFrame();
    ~Detection();

    void startDetection();
    void autoStartDetection();
    void stopAutoDetection();

    QMap<QString, int> vendorModelMap;  // 保存 vendor + model 拼接后的出现次数
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
    std::mutex detectMutex;  // 用于保护检测任务，确保同一时间只有一个线程执行
    std::atomic<bool> autoDetectState;  // 自动检测状态
    std::thread autoDetectThread;       // 自动检测线程
    std::shared_ptr<std::promise<void>> detectionPromise;
    void _autoDetectTask();  // 自动检测任务
    void run();
    void cleanUpAutoDetectThread(); // 用于清理自动检测线程
    void handleError(const QString& message);
};

#endif // DETECTION_H
