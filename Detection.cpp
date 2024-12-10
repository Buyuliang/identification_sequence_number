#include "Detection.h"
#include <iostream>
#include <chrono>
#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QProcess>
#include <QProcessEnvironment>
#include <QTextStream>
#include <QFile>
#include <QDir>

DetectionWorker::DetectionWorker(CameraThread *cameraThread, QString sn, QString vendor, QString model)
    : cameraThread(cameraThread), sn(sn), vendor(vendor), model(model)
{

}

cv::Mat DetectionWorker::getCurrentFrame()
{
    // 获取当前摄像头帧
    return cameraThread->captureFrame();
}

void DetectionWorker::loadVendorModelData()
{
    QFile file("vendor_model_data.txt");

    // 检查文件是否存在，如果不存在则创建一个新的空文件
    if (!file.exists()) {
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            // 文件创建成功，但是不写入任何内容
            file.close();
        }
    }

    // 读取文件内容
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList parts = line.split(":");
            if (parts.size() == 2) {
                QString key = parts[0].trimmed();
                int count = parts[1].toInt();
                vendorModelMap[key] = count;
            }
        }
        file.close();
    }
}

// 保存统计数据到文件
void DetectionWorker::saveVendorModelData()
{
    QFile file("vendor_model_data.txt");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        for (auto it = vendorModelMap.begin(); it != vendorModelMap.end(); ++it) {
            out << it.key() << ":" << it.value() << "\n";
        }
        file.close();
    }
}

void DetectionWorker::updateStatusWithVendorModelCount()
{
    QString statusText;

    // 拼接 vendor + model 的统计信息
    for (auto it = vendorModelMap.begin(); it != vendorModelMap.end(); ++it) {
        statusText += QString("%1: %2\n").arg(it.key()).arg(it.value());
    }

    // 发送状态更新信号
    emit updateStatusTextSignal(statusText, Qt::black, 12);
}

void DetectionWorker::run()
{
    emit updateLogTextSignal("");
    emit updateResultTextSignal("");
    qDebug() << "DetectionWorker sn:" << sn << "vendor:" << vendor << "model:" << model;
    LogManager logManager;
    // 获取当前时间戳作为文件名
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString imagePath;
    cv::Mat image;
    bool downloadSuccess, uploadSuccess, detectResult;
    QString results;  // 存储符合条件的数据

    // 设置环境变量 LD_LIBRARY_PATH
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("LD_LIBRARY_PATH", "./lib");  // 修改为你的 lib 路径

    // 创建 QProcess 对象
    QProcess process;
    QByteArray output;
    QByteArray errorOutput;
    // 设置命令和参数
    QStringList arguments;

    // 显示 OCR 结果
    QString filePath = "text.txt";  // 文件路径
    QFile file;
    QDir dir;

    // SN vendor model 规则确认
    if (sn.isEmpty() || sn.length() != 9) {
        emit appendLogTextSignal("sn is NULL or not 9 characters long!");
        goto failout;
    }
    if (vendor.isEmpty()) {
        emit appendLogTextSignal("vendor is NULL!");
        goto failout;
    }
    if (model.isEmpty()) {
        emit appendLogTextSignal("model is NULL!");
        goto failout;
    }

    // 使用 OpenCV 保存图像
    image = getCurrentFrame();
    if (!dir.exists("./pic")) {
        if (!dir.mkpath("./pic")) {
            qDebug() << "Failed to create directory 'pic'.";
        }
    }

    imagePath = "./pic/" + sn + "_" + timestamp + ".jpg";
    if (!image.empty()) {
        cv::imwrite(imagePath.toStdString(), image);
        emit appendLogTextSignal("Saved image to: " + imagePath);
    } else {
        emit appendLogTextSignal("Image is NULL!");
        goto failout;
    }

    file.setFileName(filePath);
    if (file.exists()) {
        if (file.remove()) {
            qDebug() << "File removed successfully.";
        } else {
            qDebug() << "Failed to remove file. Error:" << file.errorString();
        }
    } else {
        qDebug() << "File does not exist.";
    }

    // 设置 QProcess 使用的环境
    process.setProcessEnvironment(env);
    arguments << "model/ppocrv4_det.rknn" << "model/ppocrv4_rec.rknn" << imagePath;  // 修改为实际路径

    // 启动程序
    process.start("./rknn_ppocr_system_demo", arguments);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    // 等待程序执行完成
    if (!process.waitForStarted()) {
        qDebug() << "Error: " << process.errorString();
    }

    // 等待程序结束并输出结果
    if (!process.waitForFinished()) {
        qDebug() << "Error: " << process.errorString();
    }

    // 获取输出内容
    output = process.readAllStandardOutput();
    errorOutput = process.readAllStandardError();

    qDebug() << "Output:" << output;
    qDebug() << "Error Output:" << errorOutput;

    // 从 OSS 下载文件
    downloadSuccess = logManager.downloadFile("oss://az05/serial_number/results.csv", "results.csv");
    if (downloadSuccess) {
        emit appendLogTextSignal("File downloaded successfully!");
    } else {
        emit appendLogTextSignal("File download failed!");
        goto failout;
    }
    detectResult = false;
    if (file.exists()) {
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            while (!in.atEnd()) {
                QString line = in.readLine();
                // 去掉行首尾的空白字符
                QString cleanedLine = line.remove(" ");

                // 检查是否以 vendor 开头并以 model 结尾
                if (cleanedLine.startsWith(vendor) && cleanedLine.endsWith(model)) {
                    // 保存数据到列表
                    detectResult = true;
                    results = sn + "," + cleanedLine + "," + vendor + "," + model + "," + timestamp;
                    // qDebug() << "Updated results list: " << results;
                }
            }
            file.close();
        } else {
            qDebug() << "无法打开文件:" << filePath;
            file.close();
            goto failout;
        }
    } else {
        qDebug() << "文件不存在:" << filePath;
        goto failout;
    }
    if (detectResult) {
        QFile resultfile("results.csv");
        if (resultfile.exists()) {
            if (resultfile.open(QIODevice::Append | QIODevice::Text)) {
                QTextStream out(&resultfile);
                out << results << "\n";  // 将每行结果添加到文件中
                resultfile.close();
            }
        } else {
            qDebug() << "resultfile write failed";
            resultfile.close();
            goto failout;
        }
        qDebug() << "检测成功上传日志";
        // 上传文件到 OSS
        uploadSuccess = logManager.uploadFile("results.csv", "oss://az05/serial_number/results.csv");
        if (uploadSuccess) {
            QString vendorModelKey = vendor + model;  // 拼接 vendor + model
            loadVendorModelData();
            // 更新统计 map
            vendorModelMap[vendorModelKey]++;
            saveVendorModelData();
            // 每次更新状态文本
            updateStatusWithVendorModelCount();
            emit appendLogTextSignal("File uploaded successfully!");
            goto passout;
        } else {
            emit appendLogTextSignal("File upload failed!");
            goto failout;
        }
    }

failout:
    emit updateResultTextSignal("FAILL", Qt::red, 24);
    emit clearSNInputSignal();
    return;

passout:
    emit updateResultTextSignal("PASS", Qt::green, 24);
    emit clearSNInputSignal();
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

void Detection::startDetection()
{
    updateSNTextSignal();
    qDebug() << "更新SN:" << this->sn;
    worker = new DetectionWorker(cameraThread, sn, vendor, model);
    
    // 连接信号和槽
    connect(worker, &DetectionWorker::updateResultTextSignal, this, &Detection::updateResultTextSignal);
    connect(worker, &DetectionWorker::appendLogTextSignal, this, &Detection::appendLogTextSignal);
    connect(worker, &DetectionWorker::updateStatusTextSignal, this, &Detection::updateStatusTextSignal);
    connect(worker, &DetectionWorker::updateSNTextSignal, this, &Detection::updateSNTextSignal);
    connect(worker, &DetectionWorker::clearSNInputSignal, this, &Detection::clearSNInputSignal);
    
    // 启动检测线程
    worker->start();
}

void Detection::autoStartDetection()
{
    // 启动自动检测功能
    if (autoDetectState.load()) {
        return;  // 如果自动检测已经启动，则不再启动新线程
    }

    // 设置为启动状态
    autoDetectState.store(true);

    // 创建并启动自动检测线程
    autoDetectThread = std::thread(&Detection::_autoDetectTask, this);
}

void Detection::_autoDetectTask()
{
    // 持续执行自动检测任务
    while (autoDetectState.load()) {
        std::cout << "Starting detection for device: " << devicePath.toStdString() << std::endl;

        // 调用 startDetection 方法进行检测
        startDetection();

        // 模拟检测间隔（可以根据需求调整）
        std::this_thread::sleep_for(std::chrono::milliseconds(500)); // 每隔500ms执行一次检测
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
