#ifndef LOGMANAGER_H
#define LOGMANAGER_H

#include <QObject>
#include <QString>
#include <QProcess>
#include <QByteArray>
#include <QDebug>

class LogManager : public QObject
{
    Q_OBJECT

public:
    explicit LogManager(QObject *parent = nullptr);
    ~LogManager();

    bool uploadFile(const QString &localFilePath, const QString &ossPath);
    bool downloadFile(const QString &ossPath, const QString &localFilePath);

private:
    bool executeCommand(const QString &command);
};

#endif // LOGMANAGER_H
