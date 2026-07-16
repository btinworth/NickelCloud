#pragma once

#include "RcloneInterface.h"
#include "UserConfig.h"
#include <QObject>
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
    void OnRcloneFinished(bool success, bool transferred);

private slots:
    void Sync();

private:
    static void CreateConfig(const char* filePath, const char* tmplFilePath);
    void ReadConfig();
    void UpdateSyncTimer();
    void ScheduleNextSync();
    void StartSync(const QString& source, const QString& dest);
    void SyncNext();

    UserConfig Config;
    QQueue<SyncPair> SyncQueue;
    RcloneInterface Rclone;
    bool AnyTransferred = false;
    bool AnyFailed = false;
    QTimer SyncTimer;
};
