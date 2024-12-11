#include "Detection.h"
#include "UplodMes.h"
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

Detection::Detection(CameraThread *camerathread, QObject *parent)  // No default argument here
    : QObject(parent), autoDetectState(false), cameraThread(camerathread)
{

}

cv::Mat Detection::getCurrentFrame()
{
    // 获取当前摄像头帧
    return cameraThread->captureFrame();
}

void Detection::loadVendorModelData()
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
void Detection::saveVendorModelData()
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

void Detection::updateStatusWithVendorModelCount()
{
    QString statusText;

    // 拼接 vendor + model 的统计信息
    for (auto it = vendorModelMap.begin(); it != vendorModelMap.end(); ++it) {
        statusText += QString("%1: %2\n").arg(it.key()).arg(it.value());
    }

    // 发送状态更新信号
    emit updateStatusTextSignal(statusText, Qt::black, 12);
}

void Detection::handleError(const QString& message) {
    emit appendLogTextSignal(message);
    emit updateResultTextSignal("FAILL", Qt::red, 48);
    emit clearSNInputSignal();
}

void Detection::run() {
    emit updateLogTextSignal("");
    emit updateResultTextSignal("");

    LogManager logManager;
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString imagePath;
    cv::Mat image;
    QString ocrFilePath = "text.txt";
    QString configFile = "config.txt";
    QFile ocrFile;
    QDir dir;
    QMap<QString, QString> configMap;
    QProcess process;
    QStringList arguments;
    QString results;
    bool detectResult = false;

    // 设置环境变量 LD_LIBRARY_PATH
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("LD_LIBRARY_PATH", "./lib");

    // 读取配置文件
    if (!readConfig(configFile, configMap)) {
        emit appendLogTextSignal("Failed to read configuration.");
        return;
    }

    // 打印配置文件内容
    for (const auto &key : configMap.keys()) {
        qDebug() << key << ":" << configMap[key];
    }

    // 检查 SN 和其他输入参数
    if (sn.length() != 12 || !sn.startsWith("156")) {
        emit appendLogTextSignal("Invalid SN. Must be 12 characters and start with '156'.");
        return;
    }
    if (vendor.isEmpty() || model.isEmpty()) {
        emit appendLogTextSignal(vendor.isEmpty() ? "Vendor is NULL!" : "Model is NULL!");
        return;
    }

    // 保存图像
    image = getCurrentFrame();
    if (!dir.exists("./pic") && !dir.mkpath("./pic")) {
        handleError("Failed to create directory 'pic'.");
        return;
    }
    imagePath = "./pic/" + sn + "_" + timestamp + ".jpg";
    if (image.empty() || !cv::imwrite(imagePath.toStdString(), image)) {
        handleError("Failed to save image!");
        return;
    }
    emit appendLogTextSignal("Saved image to: " + imagePath);

    // 删除旧 OCR 文件
    ocrFile.setFileName(ocrFilePath);
    if (ocrFile.exists() && !ocrFile.remove()) {
        qDebug() << "Failed to remove existing OCR file: " << ocrFile.errorString();
    }

    // 启动外部进程
    process.setProcessEnvironment(env);
    arguments << "model/ppocrv4_det.rknn" << "model/ppocrv4_rec.rknn" << imagePath;
    process.start("./rknn_ppocr_system_demo", arguments);
    if (!process.waitForStarted() || !process.waitForFinished()) {
        handleError("Process error: " + process.errorString());
        return;
    }

    // 处理 OCR 文件
    if (!ocrFile.exists() || !ocrFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        handleError("Failed to open OCR file.");
        return;
    }

    QTextStream in(&ocrFile);
    while (!in.atEnd()) {
        QString line = in.readLine().remove(" ");
        if (line.startsWith(vendor) && line.endsWith(model)) {
            detectResult = true;
            results = sn + "," + line + "," + vendor + "," + model + "," + timestamp;
        }
    }
    ocrFile.close();

    if (!detectResult) {
        handleError("OCR detection failed.");
        return;
    }

    // 写入结果文件
    QFile resultFile(configMap.value("log_file"));
    if (!resultFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        handleError("Failed to open result file for writing.");
        return;
    }
    QTextStream out(&resultFile);
    out << results << "\n";
    resultFile.close();

    // 上传日志
    if (!logManager.appendFile(configMap.value("log_file"), configMap.value("oss_path")) ||
        !uploadLogToMes(sn, timestamp, true, results, configMap)) {
        handleError("Upload failed!");
        return;
    }

    // 更新统计
    loadVendorModelData();
    vendorModelMap[vendor + model]++;
    saveVendorModelData();
    updateStatusWithVendorModelCount();
    emit updateResultTextSignal("PASS", Qt::green, 48);
    emit clearSNInputSignal();
}

void Detection::startDetection()
{
    updateSNTextSignal();
    run();
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
    while (autoDetectState.load()) {

        // 调用检测任务
        // startDetection();
        updateSNTextSignal();
        run();
        // 添加检测间隔
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void Detection::stopAutoDetection()
{
    autoDetectState.store(false); // 停止自动检测
    cleanUpAutoDetectThread(); // 清理自动检测线程
}

void Detection::cleanUpAutoDetectThread()
{
    // 确保自动检测线程退出
    if (autoDetectThread.joinable()) {
        autoDetectThread.join(); // 等待自动检测线程结束
    }
}

Detection::~Detection()
{
    cleanUpAutoDetectThread(); // 清理自动检测线程
}