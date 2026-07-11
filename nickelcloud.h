#pragma once

#include <QObject>
#include <QProcess>
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
    void StartSync(const QString& source, const QString& dest);
    void SyncNext();

    QTimer SyncTimer;
};
