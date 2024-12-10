#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QMutexLocker>
#include <QFrame>
#include <QPixmap>
#include <QDebug>
#include <QFile>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      deviceManager(new DeviceManager()),  // 初始化 DeviceManager
    //   detection(new Detection()),            // 初始化 Detection 类
      currentDevice("")  // 初始化当前设备为空
{
    setWindowTitle("UI Components");
    setGeometry(100, 100, 1400, 600);

    // Central widget and layout
    centralWidget = new QWidget(this);
    mainLayout = new QHBoxLayout(centralWidget);
    setCentralWidget(centralWidget);

    // Left frame for video display
    leftFrame = new QFrame(this);
    leftFrame->setFrameShape(QFrame::StyledPanel);
    leftFrame->setMinimumWidth(800);
    leftFrame->setMaximumHeight(600);
    mainLayout->addWidget(leftFrame);

    videoLabel = new QLabel(leftFrame);
    videoLabel->setAlignment(Qt::AlignCenter);
    videoLabel->setStyleSheet("background-color: black;");
    QVBoxLayout *leftLayout = new QVBoxLayout(leftFrame);
    leftLayout->addWidget(videoLabel);

    // Right frame for controls
    rightFrame = new QFrame(this);
    rightFrame->setFrameShape(QFrame::StyledPanel);
    rightFrame->setMinimumWidth(300);
    mainLayout->addWidget(rightFrame);

    QHBoxLayout *rightLayout = new QHBoxLayout(rightFrame);

    // Left column: device info
    QVBoxLayout *leftColumnFrame = new QVBoxLayout();
    rightLayout->addLayout(leftColumnFrame);

    // Right column: status/logs
    QVBoxLayout *rightColumnFrame = new QVBoxLayout();
    rightLayout->addLayout(rightColumnFrame);

    // Device selection dropdown
    deviceVar = new QComboBox(this);
    deviceVar->addItem("选择设备");
    // deviceVar->setStyleSheet("background-color: white;");
    deviceVar->setStyleSheet(
        "QComboBox {"
        "   background-color: white;"
        "}"
        "QComboBox::item {"
        "   background-color: white;"
        "}"
        "QComboBox::item:hover {"
        "   background-color: lightblue;"  // 悬停时的背景色
        "}"
        "QComboBox::item:selected {"
        "   background-color: lightgreen;"  // 选中时的背景色
        "}"
    );

    leftColumnFrame->addWidget(deviceVar);

    // Refresh device button
    QPushButton *refreshButton = new QPushButton("刷新设备", this);
    leftColumnFrame->addWidget(refreshButton);

    // SN input field
    QLabel *snLabel = new QLabel("SN 号", this);
    snInput = new QLineEdit(this);
    QHBoxLayout *snLayout = new QHBoxLayout();
    snLayout->addWidget(snLabel);
    snLayout->addWidget(snInput);
    snLayout->setStretch(0, 1);
    snLayout->setStretch(1, 2);
    leftColumnFrame->addLayout(snLayout);

    // Vendor/Model input fields (initially disabled)
    vendorInput = new QLineEdit(this);
    vendorInput->setDisabled(true);
    modelInput = new QLineEdit(this);
    modelInput->setDisabled(true);

    // Create a vertical layout for Vendor/Model input
    QVBoxLayout *vendorModelLayout = new QVBoxLayout();

    // First row: Vendor
    QHBoxLayout *vendorLayout = new QHBoxLayout();
    vendorLayout->addWidget(new QLabel("前三码"));
    vendorLayout->addWidget(vendorInput);
    vendorLayout->setStretch(0, 1);
    vendorLayout->setStretch(1, 2);
    vendorModelLayout->addLayout(vendorLayout);

    // Second row: Model
    QHBoxLayout *modelLayout = new QHBoxLayout();
    modelLayout->addWidget(new QLabel("后四码"));
    modelLayout->addWidget(modelInput);
    modelLayout->setStretch(0, 1);
    modelLayout->setStretch(1, 2);
    vendorModelLayout->addLayout(modelLayout);

    // Add the vertical layout to the left column
    leftColumnFrame->addLayout(vendorModelLayout);

    // Checkbox to toggle vendor/model input
    editVendorModel = new QCheckBox("编辑前三码/后四码", this);
    connect(editVendorModel, &QCheckBox::stateChanged, this, &MainWindow::toggleVendorModel);
    leftColumnFrame->addWidget(editVendorModel);

    // Start detection button
    startButton = new QPushButton("开始检测", this);
    leftColumnFrame->addWidget(startButton);

    // Auto-detect checkbox
    autoDetect = new QCheckBox("启用自动检测", this);
    leftColumnFrame->addWidget(autoDetect);

    // Result text area
    QLabel *resultLabel = new QLabel("识别结果", this);
    resultText = new QTextEdit(this);
    resultText->setReadOnly(true);
    leftColumnFrame->addWidget(resultLabel);
    leftColumnFrame->addWidget(resultText);

    // Status and log text areas
    QLabel *statusLabel = new QLabel("状态", this);
    statusText = new QTextEdit(this);
    statusText->setReadOnly(true);
    rightColumnFrame->addWidget(statusLabel);
    rightColumnFrame->addWidget(statusText);

    QLabel *logLabel = new QLabel("日志输出", this);
    logText = new QTextEdit(this);
    logText->setReadOnly(true);
    rightColumnFrame->addWidget(logLabel);
    rightColumnFrame->addWidget(logText);

    // Initialize CameraThread
    cameraThread = new CameraThread();
    connect(cameraThread, &CameraThread::newFrameSignal, this, &MainWindow::updateVideoFrame);
    cameraThread->start();

    detection = new Detection(cameraThread);
    // Signal-slot connections
    connect(deviceVar, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onDeviceSelected);
    connect(refreshButton, &QPushButton::clicked, this, &MainWindow::updateDeviceList);
    connect(startButton, &QPushButton::clicked, this, &MainWindow::onStartDetection);
    connect(editVendorModel, &QCheckBox::stateChanged, this, &MainWindow::toggleVendorModel);
    connect(autoDetect, &QCheckBox::stateChanged, this, &MainWindow::onAutoStartDetection);

    // Connect signals from Detection class
    connect(detection, &Detection::updateResultTextSignal, this, &MainWindow::updateResultText);
    connect(detection, &Detection::updateLogTextSignal, this, &MainWindow::updateLogText);
    connect(detection, &Detection::appendLogTextSignal, this, &MainWindow::appendLogText);
    connect(detection, &Detection::updateStatusTextSignal, this, &MainWindow::updateStatusText);
    connect(detection, &Detection::updateSNTextSignal, this, &MainWindow::updateSNText);
    connect(detection, &Detection::clearSNInputSignal, this, &MainWindow::clearSNInput);

    // Populate device list
    updateDeviceList();
    clearfile();
}

MainWindow::~MainWindow()
{
    cameraThread->stopCamera();
    delete cameraThread;
    delete deviceManager;  // 清理 DeviceManager
}

void MainWindow::updateResultText(const QString &text, const QColor &color, int fontSize)
{
    QMutexLocker locker(&mutex);  // 线程锁，确保线程安全
    resultText->clear();
    // 获取 QTextEdit 当前的文本格式
    QTextCharFormat format;
    format.setForeground(QBrush(color));           // 设置字体颜色
    format.setFontPointSize(fontSize);             // 设置字体大小

    // 将格式应用到 QTextEdit
    resultText->setCurrentCharFormat(format);

    // 插入文本并保持光标位置不变
    resultText->append(text);
}

void MainWindow::updateLogText(const QString &text, const QColor &color, int fontSize)
{
    QMutexLocker locker(&mutex);  // 线程锁，确保线程安全
    logText->clear();
    QTextCharFormat format;
    format.setForeground(QBrush(color));           // 设置字体颜色
    format.setFontPointSize(fontSize);             // 设置字体大小
    logText->setCurrentCharFormat(format);
    logText->append(text);
}

void MainWindow::appendLogText(const QString &text, const QColor &color, int fontSize)
{
    QMutexLocker locker(&mutex);  // 线程锁，确保线程安全

    // 获取 QTextEdit 当前的文本格式
    QTextCharFormat format;
    format.setForeground(QBrush(color));           // 设置字体颜色
    format.setFontPointSize(fontSize);             // 设置字体大小
    logText->setCurrentCharFormat(format);
    logText->append(text);
}

void MainWindow::updateStatusText(const QString &text, const QColor &color, int fontSize)
{
    QMutexLocker locker(&mutex);  // 线程锁，确保线程安全
    statusText->clear();
    // 获取 QTextEdit 当前的文本格式
    QTextCharFormat format;
    format.setForeground(QBrush(color));           // 设置字体颜色
    format.setFontPointSize(fontSize);             // 设置字体大小
    statusText->setCurrentCharFormat(format);
    statusText->append(text);
}

void MainWindow::updateSNText()
{
    QMutexLocker locker(&mutex);
    detection->sn = snInput->text();
    detection->vendor = vendorInput->text();
    detection->model = modelInput->text();
}

void MainWindow::updateVideoFrame(const QPixmap &pixmap)
{
    videoLabel->setPixmap(pixmap);
}

// void MainWindow::toggleVendorModel()
// {
//     if (editVendorModel->isChecked()) {
//         vendorInput->setEnabled(true);
//         modelInput->setEnabled(true);
//     } else {
//         vendorInput->setDisabled(true);
//         modelInput->setDisabled(true);
//     }
// }

void MainWindow::toggleVendorModel(int state)
{
    // 如果 editVendorModel 被选中，启用 vendorInput 和 modelInput
    bool enableFields = (state == Qt::Checked) && !autoDetect->isChecked();
    vendorInput->setEnabled(enableFields);
    modelInput->setEnabled(enableFields);
}

void MainWindow::onAutoStartDetection()
{
    if (autoDetect->isChecked()) {
        // 获取 SN、Vendor、Model 等输入
        QString sn = snInput->text();
        QString vendor = vendorInput->text();
        QString model = modelInput->text();
        QString devicePath = currentDevice;
        startButton->setDisabled(true);
        vendorInput->setDisabled(true);
        modelInput->setDisabled(true);

        // 启动检测
        detection->autoStartDetection();
    } else {
        startButton->setEnabled(true);
        detection->stopAutoDetection();
    }
}

void MainWindow::updateDeviceList()
{
    // 获取当前选中的设备
    QString currentDevice = deviceVar->currentText();

    // 清空现有列表
    deviceVar->clear();
    deviceVar->addItem("选择设备");

    // 获取设备列表
    QStringList devices = deviceManager->refreshDeviceList();

    // 如果设备列表为空，添加提示信息
    if (devices.isEmpty()) {
        deviceVar->addItem("无可用设备");
    } else {
        for (const QString &device : devices) {
            deviceVar->addItem(device);
        }
    }
}

void MainWindow::onDeviceSelected(int index)
{
    if (index == 0) {
        return;  // "选择设备" 未选中
    }

    currentDevice = deviceVar->itemText(index);  // 获取选中的设备路径
    qDebug() << "选中的设备: " << currentDevice;

    // 更新摄像头显示或切换设备
    cameraThread->setDevice(currentDevice);
}

void MainWindow::onStartDetection()
{
    // 启动检测
    vendorInput->setDisabled(true);
    modelInput->setDisabled(true);
    detection->startDetection();
}

void MainWindow::clearfile()
{
    QFile file("vendor_model_data.txt");
    if (file.exists()) {
        file.remove();
    }
}

void MainWindow::clearSNInput() {
    snInput->clear();          // 清除文本框内容
    snInput->setFocus();       // 设置光标到该文本框
}
