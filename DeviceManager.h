#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include <QStringList>

class DeviceManager {
public:
    DeviceManager();
    QStringList refreshDeviceList();
    QStringList getDeviceList();

private:
    QStringList deviceList;
};

#endif // DEVICEMANAGER_H
