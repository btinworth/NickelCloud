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
    void OnSyncFinished();
    void OnPullFinished(int exitCode, QProcess::ExitStatus status);

private:
    void StartSync(const QString& source, const QString& dest);
    void SyncNext();
};
