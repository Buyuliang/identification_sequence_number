#include "Detection.h"
#include <iostream>
#include <chrono>
#include <QDateTime>
#include <QDebug>
#include <QFile>

DetectionWorker::DetectionWorker(CameraThread *cameraThread, const QString &sn, const QString &vendor, const QString &model, const QString &devicePath)
    : cameraThread(cameraThread), sn(sn), vendor(vendor), model(model), devicePath(devicePath)
{

}

cv::Mat DetectionWorker::getCurrentFrame()
{
    // 获取当前摄像头帧
    return cameraThread->captureFrame();
}

void DetectionWorker::run()
{
    // 在这里进行检测任务的处理
    // 假设检测结束后发送信号通知主线程
    LogManager logManager;

    // 上传文件到 OSS
    bool uploadSuccess = logManager.uploadFile("/path/to/local/file.txt", "oss://bucket/path/to/oss/file.txt");
    if (uploadSuccess) {
        emit appendLogTextSignal("File uploaded successfully!");
    } else {
        emit appendLogTextSignal("File upload failed!");
    }

    // 从 OSS 下载文件
    bool downloadSuccess = logManager.downloadFile("oss://bucket/path/to/oss/file.txt", "/path/to/local/file.txt");
    if (downloadSuccess) {
        emit appendLogTextSignal("File downloaded successfully!");
    } else {
        emit appendLogTextSignal("File download failed!");
    }

    emit updateResultTextSignal("Detection finished");
    emit appendLogTextSignal("Detection log");
    emit appendLogTextSignal("SN:" + sn + "\nvendor:" + vendor);
    emit appendLogTextSignal("Detection log");
    emit appendLogTextSignal("Detection log");
    emit updateStatusTextSignal("Status updated");

    // 获取当前时间戳作为文件名
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString imagePath = sn + "_" + timestamp + ".jpg";

    // 使用 OpenCV 保存图像
    cv::Mat image = getCurrentFrame();
    if (!image.empty()) {
        cv::imwrite(imagePath.toStdString(), image);
        emit appendLogTextSignal("Saved image to: " + imagePath);
    } else {
        emit appendLogTextSignal("Image is NULL!");
    }

    QList<QList<QString>> results;  // 存储符合条件的数据

    // 显示 OCR 结果
    QString filePath = "text.txt";  // 文件路径
    QFile file(filePath);

    if (file.exists()) {
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            while (!in.atEnd()) {
                QString line = in.readLine();
                // 去掉行首尾的空白字符
                QString cleanedLine = line.simplified();

                // 检查是否以 vendor 开头并以 model 结尾
                if (cleanedLine.startsWith(vendor) && cleanedLine.endsWith(model)) {
                    // 保存数据到列表
                    QList<QString> result;
                    result.append(sn);
                    result.append(vendor);
                    result.append(model);
                    result.append(timestamp);
                    results.append(result);
                }
            }
            file.close();
        } else {
            qDebug() << "无法打开文件:" << filePath;
        }
    } else {
        qDebug() << "文件不存在:" << filePath;
    }

}

Detection::Detection(CameraThread *camerathread, QObject *parent)  // No default argument here
    : QObject(parent), worker(nullptr), autoDetectState(false), cameraThread(camerathread)
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
    worker = new DetectionWorker(cameraThread, sn, vendor, model, devicePath);
    
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
