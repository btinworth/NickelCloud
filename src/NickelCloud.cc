#include "NickelCloud.h"
#include "Constants.h"
#include <NickelHook.h>
#include <QDir>
#include <QFile>
#include <QObject>
#include <QProcess>
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
}

void NickelCloud::OnNetworkConnected()
{
    Sync();
}

void NickelCloud::OnNetworkDisconnected()
{
    SyncTimer.stop();
}

void NickelCloud::OnSyncFinished(int exitCode, QProcess::ExitStatus status)
{
    auto source = SyncQueue.head().source;

    auto* rclone = qobject_cast<QProcess*>(sender());
    if (rclone != nullptr)
    {
        ReadSyncOutput(rclone);
    }

    FlushSyncOutput();

    if (status != QProcess::NormalExit)
    {
        AnyFailed = true;
        nh_log("NickelCloud: rclone crashed for %s", qPrintable(source));
    }
    else if (exitCode == 0)
    {
        nh_log("NickelCloud: rclone completed successfully for %s", qPrintable(source));
    }
    else
    {
        AnyFailed = true;
        nh_log("NickelCloud: rclone failed for %s (exit %d)", qPrintable(source), exitCode);
    }

    SyncQueue.dequeue();
    SyncNext();
}

void NickelCloud::OnSyncOutput()
{
    auto* rclone = qobject_cast<QProcess*>(sender());
    if (rclone == nullptr)
    {
        return;
    }

    ReadSyncOutput(rclone);
}

void NickelCloud::ReadSyncOutput(QProcess* rclone)
{
    PendingOutput += rclone->readAllStandardOutput();

    int newline;
    while ((newline = PendingOutput.indexOf('\n')) >= 0)
    {
        auto line = QString::fromUtf8(PendingOutput.left(newline)).trimmed();
        PendingOutput.remove(0, newline + 1);
        HandleSyncOutputLine(line);
    }
}

void NickelCloud::FlushSyncOutput()
{
    auto line = QString::fromUtf8(PendingOutput).trimmed();
    PendingOutput.clear();
    HandleSyncOutputLine(line);
}

void NickelCloud::HandleSyncOutputLine(const QString& line)
{
    if (line.isEmpty())
    {
        return;
    }

    if (line.contains(": Copied") || line.contains(": Deleted"))
    {
        // a file has changed, flag for library scan
        AnyTransferred = true;
    }

    nh_log("NickelCloud: %s", qPrintable(line));
}

void NickelCloud::OnSyncError(QProcess::ProcessError error)
{
    // only FailedToStart skips finished(); other errors are handled by OnSyncFinished
    if (error != QProcess::FailedToStart)
    {
        return;
    }

    AnyFailed = true;
    nh_log("NickelCloud: rclone failed to start for %s", qPrintable(SyncQueue.head().source));

    auto* rclone = qobject_cast<QProcess*>(sender());
    if (rclone != nullptr)
    {
        rclone->deleteLater();
    }

    FlushSyncOutput();

    SyncQueue.dequeue();
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
        nh_log("NickelCloud: no sources configured");
        ScheduleNextSync();
        return;
    }

    AnyTransferred = false;
    AnyFailed = false;

    nh_log("NickelCloud: pulling %d source(s) from cloud", SyncQueue.size());
    SyncNext();
}

void NickelCloud::CreateConfig(const char* filePath, const char* tmplFilePath)
{
    if (!QDir().mkpath(CONFIG_DIR))
    {
        nh_log("NickelCloud: failed to create config directory: %s", CONFIG_DIR);
        return;
    }

    if (QFile::exists(filePath))
    {
        return; // nothing to do
    }

    if (QFile::copy(tmplFilePath, filePath))
    {
        nh_log("NickelCloud: created config from template: %s", filePath);
    }
    else
    {
        nh_log("NickelCloud: failed to create config from template: %s -> %s", tmplFilePath, filePath);
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
        nh_log("NickelCloud: failed to create destination directory for %s: %s", qPrintable(source), qPrintable(dest));

        SyncQueue.dequeue();
        SyncNext();
        return;
    }

    nh_log("NickelCloud: syncing %s -> %s", qPrintable(source), qPrintable(dest));

    auto* rclone = new QProcess(this);
    rclone->setProcessChannelMode(QProcess::MergedChannels);
    QObject::connect(rclone, &QProcess::readyReadStandardOutput, this, &NickelCloud::OnSyncOutput);
    QObject::connect(rclone, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &NickelCloud::OnSyncFinished);
    QObject::connect(rclone, static_cast<void (QProcess::*)(QProcess::ProcessError)>(&QProcess::error), this, &NickelCloud::OnSyncError);
    QObject::connect(rclone, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), rclone, &QObject::deleteLater);

    QStringList args;
    args << Config.GetMode()
         << source << dest
         << "--config" << RCLONE_CONF
         << "--ca-cert" << CA_CERT
         << "--cache-dir" << CACHE_DIR
         << "--stats" << "0"
         << "--log-level" << "INFO"
         << "--transfers" << QString::number(Config.GetTransfers())
         << Config.GetExtraArgs();

    rclone->start(RCLONE_BIN, args);
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

            auto* fss = N3FSSyncManagerInstance();
            N3FSSyncManagerSync(fss, &paths);
        }

        ScheduleNextSync();
        return;
    }

    const auto& next = SyncQueue.head();
    StartSync(next.source, next.dest);
}
