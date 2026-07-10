#pragma once

#include <QObject>
#include <QProcess>
#include <QString>

struct SyncPair
{
    QString source;
    QString dest;
};

class NickelCloudWatcher : public QObject
{
    Q_OBJECT

public slots:
    void OnNetworkConnected();
    void OnSyncFinished(int exitCode, QProcess::ExitStatus status);
    void OnSyncError(QProcess::ProcessError error);

private:
    void StartSync(const QString& source, const QString& dest);
    void SyncNext();
};
