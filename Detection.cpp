#include "Detection.h"
#include <iostream>
#include <chrono>
#include <thread>

DetectionWorker::DetectionWorker(const QString &sn, const QString &vendor, const QString &model, const QString &devicePath)
    : sn(sn), vendor(vendor), model(model), devicePath(devicePath)
{
}

void DetectionWorker::run()
{
    // 在这里进行检测任务的处理
    // 假设检测结束后发送信号通知主线程
    LogManager logManager;

    // 上传文件到 OSS
    bool uploadSuccess = logManager.uploadFile("/path/to/local/file.txt", "oss://bucket/path/to/oss/file.txt");
    if (uploadSuccess) {
        qDebug() << "File uploaded successfully!";
    } else {
        qDebug() << "File upload failed!";
    }

    // 从 OSS 下载文件
    bool downloadSuccess = logManager.downloadFile("oss://bucket/path/to/oss/file.txt", "/path/to/local/file.txt");
    if (downloadSuccess) {
        qDebug() << "File downloaded successfully!";
    } else {
        qDebug() << "File download failed!";
    }

    emit updateResultTextSignal("Detection finished");
    emit appendLogTextSignal("Detection log");
    emit appendLogTextSignal("SN:" + sn + "\nvendor:" + vendor);
    emit appendLogTextSignal("Detection log");
    emit appendLogTextSignal("Detection log");
    emit updateStatusTextSignal("Status updated");
}

Detection::Detection(QObject *parent)
    : QObject(parent), worker(nullptr), autoDetectState(false)
{
}

Detection::~Detection()
{
    // 清理工作
    if (worker) {
        worker->wait(); // 等待线程结束
        delete worker;
    }

    cleanUpAutoDetectThread(); // 清理自动检测线程
}

void Detection::startDetection(const QString &sn, const QString &vendor, const QString &model, const QString &devicePath)
{
    worker = new DetectionWorker(sn, vendor, model, devicePath);
    
    // 连接信号和槽
    connect(worker, &DetectionWorker::updateResultTextSignal, this, &Detection::updateResultTextSignal);
    connect(worker, &DetectionWorker::appendLogTextSignal, this, &Detection::appendLogTextSignal);
    connect(worker, &DetectionWorker::updateStatusTextSignal, this, &Detection::updateStatusTextSignal);
    
    // 启动检测线程
    worker->start();
}

void Detection::autoStartDetection(const QString &sn, const QString &vendor, const QString &model, const QString &devicePath)
{
    // 启动自动检测功能
    if (autoDetectState.load()) {
        return;  // 如果自动检测已经启动，则不再启动新线程
    }

    // 设置为启动状态
    autoDetectState.store(true);

    // 创建并启动自动检测线程
    autoDetectThread = std::thread(&Detection::_autoDetectTask, this, sn, vendor, model, devicePath);
}

void Detection::_autoDetectTask(const QString &sn, const QString &vendor, const QString &model, const QString &devicePath)
{
    // 持续执行自动检测任务
    while (autoDetectState.load()) {
        std::cout << "Starting detection for device: " << devicePath.toStdString() << std::endl;

        // 调用 startDetection 方法进行检测
        startDetection(sn, vendor, model, devicePath);

        // 模拟检测间隔（可以根据需求调整）
        std::this_thread::sleep_for(std::chrono::seconds(1)); // 每隔1秒执行一次检测
    }
}

void Detection::stopAutoDetection()
{
    // 停止自动检测
    autoDetectState.store(false);

    cleanUpAutoDetectThread(); // 清理自动检测线程
}

void Detection::cleanUpAutoDetectThread()
{
    // 确保自动检测线程退出
    if (autoDetectThread.joinable()) {
        autoDetectThread.join(); // 等待自动检测线程结束
    }
}
