#include "DeviceManager.h"
#include <QDir>
#include <QDebug>
#include <QStringList>

DeviceManager::DeviceManager() {}

QStringList DeviceManager::refreshDeviceList()
{
    deviceList.clear();
    QDir dir("/dev");
    
    // 获取所有符合 "video*" 模式的文件，并列出所有视频设备
    QStringList devices = dir.entryList(QStringList() << "video*", QDir::System | QDir::Files);
    
    // 打印查找到的设备路径
    qDebug() << "Found devices:";
    for (const QString &device : devices) {
        QString devicePath = "/dev/" + device;
        deviceList.append(devicePath);
        qDebug() << "Device: " << devicePath;
    }
    
    return deviceList;
}

QStringList DeviceManager::getDeviceList()
{
    return deviceList;
}
