#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QLabel>
#include <QCheckBox>
#include <QFrame>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMutex>
#include <QMutexLocker>
#include <QThread>
#include <QImage>
#include <QPixmap>

#include "Detection.h"
#include "CameraThread.h"
#include "DeviceManager.h"

class CameraThread;
class Detection;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    // UI components
    QWidget *centralWidget;
    QHBoxLayout *mainLayout;
    QFrame *leftFrame, *rightFrame;
    QLabel *videoLabel;
    QComboBox *deviceVar;
    QLineEdit *snInput;
    QLineEdit *vendorInput;
    QLineEdit *modelInput;
    QPushButton *startButton;
    QCheckBox *autoDetect;
    QTextEdit *resultText, *statusText, *logText;
    QCheckBox *editVendorModel;  // 用于显示前三码和后四码的复选框
    QString currentDevice;

    // DeviceManager 用于管理设备
    DeviceManager *deviceManager;

    // Camera thread
    CameraThread *cameraThread;

    Detection *detection;

    // Mutex for thread safety
    QMutex mutex;

    // Private methods to update UI
    void updateResultText(const QString &text, const QColor &color = Qt::black, int fontSize = 24);
    void updateLogText(const QString &text, const QColor &color = Qt::black, int fontSize = 24);
    void appendLogText(const QString &text, const QColor &color = Qt::black, int fontSize = 12);
    void updateStatusText(const QString &text, const QColor &color = Qt::black, int fontSize = 12);
    void updateVideoFrame(const QPixmap &pixmap);
    void populateDeviceList();  // 填充设备列表
    void updateSNText();
    void clearfile();

private slots:
    void toggleVendorModel();
    void onAutoStartDetection();
    void updateDeviceList();  // 更新设备列表
    void onDeviceSelected(int index);
    void onStartDetection();
};

#endif // MAINWINDOW_H
