#pragma once

#include <QObject>
#include <QProcess>

class NickelCloudWatcher : public QObject
{
    Q_OBJECT

public slots:
    void OnSyncFinished();
    void OnPullFinished(int exitCode, QProcess::ExitStatus status);
};
