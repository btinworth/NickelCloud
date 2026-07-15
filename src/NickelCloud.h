#pragma once

#include "UserConfig.h"
#include <QByteArray>
#include <QObject>
#include <QProcess>
#include <QQueue>
#include <QString>
#include <QTimer>

extern QObject* (*WirelessManagerInstance)();
extern QObject* (*N3FSSyncManagerInstance)();
extern void (*N3FSSyncManagerSync)(QObject*, QStringList*);

class NickelCloud : public QObject
{
    Q_OBJECT

public:
    NickelCloud();

public slots:
    void OnNetworkConnected();
    void OnNetworkDisconnected();
    void OnSyncFinished(int exitCode, QProcess::ExitStatus status);
    void OnSyncError(QProcess::ProcessError error);

private slots:
    void Sync();
    void OnSyncOutput();

private:
    static void CreateConfig(const char* filePath, const char* tmplFilePath);
    void ReadConfig();
    void UpdateSyncTimer();
    void ScheduleNextSync();
    void StartSync(const QString& source, const QString& dest);
    void SyncNext();
    void ReadSyncOutput(QProcess* rclone);
    void FlushSyncOutput();
    void HandleSyncOutputLine(const QString& line);

    UserConfig Config;
    QQueue<SyncPair> SyncQueue;
    QByteArray PendingOutput;
    bool AnyTransferred = false;
    bool AnyFailed = false;
    QTimer SyncTimer;
};
