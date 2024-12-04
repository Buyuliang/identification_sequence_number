#ifndef DETECTION_H
#define DETECTION_H

#include <QObject>
#include <QString>
#include <atomic>
#include <thread>
#include <QThread>
#include "LogManager.h"

class DetectionWorker : public QThread
{
    Q_OBJECT

public:
    DetectionWorker(const QString &sn, const QString &vendor, const QString &model, const QString &devicePath);
    void run() override;

signals:
    void updateResultTextSignal(const QString &text);
    void appendLogTextSignal(const QString &text);
    void updateStatusTextSignal(const QString &text);

private:
    QString sn;
    QString vendor;
    QString model;
    QString devicePath;
};

class Detection : public QObject
{
    Q_OBJECT

public:
    explicit Detection(QObject *parent = nullptr);
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
};

#endif // DETECTION_H
