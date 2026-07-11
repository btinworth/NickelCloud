#pragma once

#include <QObject>
#include <QProcess>
#include <QQueue>
#include <QString>
#include <QTimer>

struct SyncPair
{
    QString source;
    QString dest;
};

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

private:
    void ReadConfig();
    void StartSync(const QString& source, const QString& dest);
    void SyncNext();
    static void TriggerRescan();
    static void InitConfig();

    QQueue<SyncPair> SyncQueue;
    bool AnyTransferred = false;
    bool SyncInProgress = false;
    QTimer SyncTimer;
};
