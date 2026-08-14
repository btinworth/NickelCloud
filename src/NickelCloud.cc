#include "NickelCloud.h"
#include "Constants.h"
#include "RcloneInterface.h"
#include <NickelHook.h>
#include <QDir>
#include <QFile>
#include <QObject>
#include <QQueue>
#include <QString>
#include <QStringList>
#include <QTimer>

QObject* (*WirelessManagerInstance)() = nullptr;
QObject* (*N3FSSyncManagerInstance)() = nullptr;
void (*N3FSSyncManagerSync)(QObject*, QStringList*) = nullptr;

NickelCloud::NickelCloud()
{
    CreateConfig(RCLONE_CONF, RCLONE_TMPL);
    CreateConfig(NICKELCLOUD_CONF, NICKELCLOUD_TMPL);

    Config.Load(NICKELCLOUD_CONF);

    UpdateSyncTimer();
    QObject::connect(&SyncTimer, &QTimer::timeout, this, &NickelCloud::Sync);
    QObject::connect(&Rclone, &RcloneInterface::Finished, this, &NickelCloud::OnRcloneFinished);
}

void NickelCloud::OnNetworkConnected()
{
    Offline = false;
    Sync();
}

void NickelCloud::OnNetworkDisconnected()
{
    Offline = true;
    SyncTimer.stop();
    Rclone.Stop();
    SyncQueue.clear();
}

void NickelCloud::OnRcloneFinished(bool success, bool transferred)
{
    AnyFailed |= !success;
    AnyTransferred |= transferred;

    if (!SyncQueue.isEmpty())
    {
        SyncQueue.dequeue();
    }

    SyncNext();
}

void NickelCloud::Sync()
{
    if (!SyncQueue.isEmpty())
    {
        // sync cycle is still running, do nothing
        return;
    }

    ReadConfig();

    if (SyncQueue.isEmpty())
    {
        nh_log("no sources configured");
        ScheduleNextSync();
        return;
    }

    AnyTransferred = false;
    AnyFailed = false;

    nh_log("pulling %d source(s) from cloud", SyncQueue.size());
    SyncNext();
}

void NickelCloud::CreateConfig(const char* filePath, const char* tmplFilePath)
{
    if (!QDir().mkpath(CONFIG_DIR))
    {
        nh_log("failed to create config directory: %s", CONFIG_DIR);
        return;
    }

    if (QFile::exists(filePath))
    {
        return; // nothing to do
    }

    if (QFile::copy(tmplFilePath, filePath))
    {
        nh_log("created config from template: %s", filePath);
    }
    else
    {
        nh_log("failed to create config from template: %s -> %s", tmplFilePath, filePath);
    }
}

void NickelCloud::ReadConfig()
{
    Config.Load(NICKELCLOUD_CONF);
    SyncQueue = Config.GetSources();

    UpdateSyncTimer();
}

void NickelCloud::UpdateSyncTimer()
{
    auto interval = Config.GetInterval();
    if (interval <= 0)
    {
        // disable timer
        SyncTimer.stop();
        return;
    }

    SyncTimer.setSingleShot(true);
    SyncTimer.setInterval(interval * 1000);
}

void NickelCloud::ScheduleNextSync()
{
    if (Offline)
    {
        return;
    }

    if (Config.GetInterval() > 0)
    {
        SyncTimer.start();
    }
}

void NickelCloud::StartSync(const QString& source, const QString& dest)
{
    if (!QDir().mkpath(dest))
    {
        AnyFailed = true;
        nh_log("failed to create destination directory for %s: %s", qPrintable(source), qPrintable(dest));

        if (!SyncQueue.isEmpty())
        {
            SyncQueue.dequeue();
        }

        SyncNext();
        return;
    }

    nh_log("syncing %s -> %s", qPrintable(source), qPrintable(dest));

    QStringList args;
    args << Config.GetMode()
         << source << dest
         << "--config" << RCLONE_CONF
         << "--ca-cert" << CA_CERT
         << "--cache-dir" << CACHE_DIR
         << "--stats" << "0"
         << "--log-level" << "INFO"
         << "--use-json-log"
         << "--transfers" << QString::number(Config.GetTransfers())
         << Config.GetExtraArgs();

    Rclone.Start(args, source);
}

// start the next queued sync, or finish the cycle if the queue is empty
void NickelCloud::SyncNext()
{
    if (SyncQueue.isEmpty())
    {
        if (Config.GetLogEnabled() && (AnyTransferred || AnyFailed))
        {
            nh_dump_log();
        }

        if (AnyTransferred)
        {
            // files have been modified, trigger a library scan
            QStringList paths;
            for (const auto& pair : Config.GetSources())
            {
                paths.append(pair.dest);
            }

            if (N3FSSyncManagerInstance != nullptr && N3FSSyncManagerSync != nullptr)
            {
                auto* fss = N3FSSyncManagerInstance();
                if (fss != nullptr)
                {
                    N3FSSyncManagerSync(fss, &paths);
                }
            }
            else
            {
                nh_log("N3FSSyncManager unavailable, skipping library scan");
            }
        }

        ScheduleNextSync();
        return;
    }

    const auto& next = SyncQueue.head();
    StartSync(next.source, next.dest);
}
