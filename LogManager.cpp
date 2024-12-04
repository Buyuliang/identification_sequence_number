#include "LogManager.h"
#include <QProcess>

LogManager::LogManager(QObject *parent) : QObject(parent)
{
}

LogManager::~LogManager()
{
}

bool LogManager::uploadFile(const QString &localFilePath, const QString &ossPath)
{
    // 构建上传命令
    QString command = QString("ossutil cp -f %1 %2").arg(localFilePath, ossPath);

    return executeCommand(command);  // 执行命令
}

bool LogManager::downloadFile(const QString &ossPath, const QString &localFilePath)
{
    // 构建下载命令
    QString command = QString("ossutil cp -f %1 %2").arg(ossPath, localFilePath);

    return executeCommand(command);  // 执行命令
}

bool LogManager::executeCommand(const QString &command)
{
    QProcess process;

    // 启动外部进程执行命令
    process.start(command);
    process.waitForFinished();  // 等待命令执行完毕

    if (process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0) {
        // 获取命令输出
        QByteArray output = process.readAllStandardOutput();
        qDebug() << "Command Output:" << output;
        return true;  // 命令执行成功
    } else {
        // 获取错误输出
        QByteArray errorOutput = process.readAllStandardError();
        qDebug() << "Error Output:" << errorOutput;
        return false;  // 命令执行失败
    }
}
