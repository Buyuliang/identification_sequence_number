#ifndef UPLODMES_H
#define UPLODMES_H

#include <QUrlQuery>
#include <QFile>
#include <QEventLoop>
#include <QTextStream>
#include <QString>
#include <QMap>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>

// Function to read configuration from a file
bool readConfig(const QString &configFile, QMap<QString, QString> &configMap) {
    QFile file(configFile);
    if (!file.exists()) {
        qWarning() << "Config file does not exist:" << configFile;
        return false;
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open config file:" << configFile;
        return false;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith("#")) {
            continue; // Skip empty lines or comments
        }

        QStringList parts = line.split('=');
        if (parts.size() == 2) {
            QString key = parts[0].trimmed();
            QString value = parts[1].trimmed();
            configMap[key] = value;
        }
    }

    file.close();

    // Check required keys
    if (!configMap.contains("token") || !configMap.contains("stn") ||
         !configMap.contains("wi") || !configMap.contains("url") ||
         !configMap.contains("oss_path") || !configMap.contains("log_file")) {
        qWarning() << "Missing required config values.";
        return false;
    }

    return true;
}

// Function to upload log to MES
bool uploadLogToMes(const QString &sn, const QString &time, const bool resultRet, const QString &data, const QMap<QString, QString> &configMap) {
    QJsonObject jsonData;
    jsonData["sn"] = sn;
    jsonData["time"] = time;
    jsonData["data"] = data;

    QUrl url(configMap.value("url"));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QUrlQuery postData;
    postData.addQueryItem("pass", QString::number(resultRet ? 1 : 0));
    postData.addQueryItem("barcode", sn);
    postData.addQueryItem("stn", configMap.value("stn"));
    postData.addQueryItem("wi", configMap.value("wi"));
    postData.addQueryItem("token", configMap.value("token"));
    postData.addQueryItem("param", QString(QJsonDocument(jsonData).toJson(QJsonDocument::Compact)));
    QNetworkAccessManager manager;
    QNetworkReply *reply = manager.post(request, postData.query(QUrl::FullyEncoded).toUtf8());

    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    QByteArray response = reply->readAll();
    reply->deleteLater();

    QJsonDocument responseDoc = QJsonDocument::fromJson(response);
    if (!responseDoc.isObject()) {
        qWarning() << "Invalid response from server:" << response;
        return false;
    }

    QJsonObject responseObj = responseDoc.object();
    if (responseObj.contains("code") && responseObj["code"].toInt() == 1) {
        return true;
    } else {
        qWarning() << "Upload failed with response:" << response;
        return false;
    }
}

#endif