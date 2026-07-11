#pragma once

#include "config.h"
#include <QObject>
#include <QProcess>
#include <QQueue>
#include <QString>
#include <QTimer>

class NickelCloudWatcher : public QObject
{
    Q_OBJECT

public:
    NickelCloudWatcher();

public slots:
    void OnNetworkConnected();
    void OnNetworkDisconnected();
    void OnSyncFinished(int exitCode, QProcess::ExitStatus status);
    void OnSyncError(QProcess::ProcessError error);

private slots:
    void Sync();
    void OnSyncOutput();

private:
    void ReadConfig();
    void UpdateSyncTimer();
    void StartSync(const QString& source, const QString& dest);
    void SyncNext();

    NickelCloudConfig Config;
    QQueue<SyncPair> SyncQueue;
    bool AnyTransferred = false;
    QTimer SyncTimer;
};
