#include "nickelcloud.h"
#include "utils.h"
#include <NickelHook.h>
#include <QDir>
#include <QFile>
#include <QObject>
#include <QProcess>
#include <QQueue>
#include <QString>
#include <QStringList>
#include <QTimer>

static QObject* (*WirelessManagerInstance)();
static QObject* (*N3FSSyncManagerInstance)();
static void (*N3FSSyncManagerSync)(QObject*, QStringList*);

#define ONBOARD_DIR "/mnt/onboard"
#define CONFIG_DIR "/mnt/onboard/.adds/nickelcloud"
#define INSTALL_DIR "/usr/local/nickelcloud"

static const char* RCLONE_BIN = INSTALL_DIR "/rclone";
static const char* CA_CERT = INSTALL_DIR "/cacert.pem";
static const char* RCLONE_TMPL = INSTALL_DIR "/rclone.conf.tmpl";
static const char* NICKELCLOUD_TMPL = INSTALL_DIR "/nickelcloud.conf.tmpl";
static const char* RCLONE_CONF = CONFIG_DIR "/rclone.conf";
static const char* NICKELCLOUD_CONF = CONFIG_DIR "/nickelcloud.conf";
static const char* CACHE_DIR = CONFIG_DIR "/cache";

NickelCloudWatcher::NickelCloudWatcher()
{
    QDir().mkpath(CONFIG_DIR);
    if (!QFile::exists(RCLONE_CONF))
    {
        QFile::copy(RCLONE_TMPL, RCLONE_CONF);
        nh_log("NickelCloud: created rclone.conf from template");
    }
    if (!QFile::exists(NICKELCLOUD_CONF))
    {
        QFile::copy(NICKELCLOUD_TMPL, NICKELCLOUD_CONF);
        nh_log("NickelCloud: created nickelcloud.conf from template");
    }

    Config.Load(NICKELCLOUD_CONF);

    UpdateSyncTimer();
    QObject::connect(&SyncTimer, &QTimer::timeout, this, &NickelCloudWatcher::Sync);
}

void NickelCloudWatcher::OnNetworkConnected()
{
    Sync();
}

void NickelCloudWatcher::OnNetworkDisconnected()
{
    SyncTimer.stop();
}

void NickelCloudWatcher::OnSyncFinished(int exitCode, QProcess::ExitStatus status)
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

void NickelCloudWatcher::OnSyncOutput()
{
    auto* rclone = qobject_cast<QProcess*>(sender());
    if (rclone == nullptr)
    {
        return;
    }

    ReadSyncOutput(rclone);
}

void NickelCloudWatcher::ReadSyncOutput(QProcess* rclone)
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

void NickelCloudWatcher::FlushSyncOutput()
{
    auto line = QString::fromUtf8(PendingOutput).trimmed();
    PendingOutput.clear();
    HandleSyncOutputLine(line);
}

void NickelCloudWatcher::HandleSyncOutputLine(const QString& line)
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

void NickelCloudWatcher::OnSyncError(QProcess::ProcessError error)
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

void NickelCloudWatcher::Sync()
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

void NickelCloudWatcher::ReadConfig()
{
    SyncQueue.clear();

    Config.Load(NICKELCLOUD_CONF);

    UpdateSyncTimer();

    for (const auto& pair : Config.GetSources())
    {
        auto destination = Utils::ResolvePath(ONBOARD_DIR, pair.dest);
        if (destination.isEmpty())
        {
            nh_log("NickelCloud: ignoring out-of-bounds destination: %s", qPrintable(pair.dest));
            continue;
        }

        SyncQueue.enqueue({pair.source, destination});
    }
}

void NickelCloudWatcher::UpdateSyncTimer()
{
    auto interval = Config.GetInterval();
    if (interval <= 0)
    {
        // an interval of 0 disables periodic re-checking entirely
        SyncTimer.stop();
        return;
    }

    SyncTimer.setSingleShot(true);
    SyncTimer.setInterval(interval * 1000);
}

void NickelCloudWatcher::ScheduleNextSync()
{
    if (Config.GetInterval() > 0)
    {
        SyncTimer.start();
    }
}

void NickelCloudWatcher::StartSync(const QString& source, const QString& dest)
{
    QDir().mkpath(dest);

    nh_log("NickelCloud: syncing %s -> %s", qPrintable(source), qPrintable(dest));

    auto* rclone = new QProcess(this);
    rclone->setProcessChannelMode(QProcess::MergedChannels);
    QObject::connect(rclone, &QProcess::readyReadStandardOutput, this, &NickelCloudWatcher::OnSyncOutput);
    QObject::connect(rclone, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &NickelCloudWatcher::OnSyncFinished);
    QObject::connect(rclone, static_cast<void (QProcess::*)(QProcess::ProcessError)>(&QProcess::error), this, &NickelCloudWatcher::OnSyncError);
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
void NickelCloudWatcher::SyncNext()
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
            auto* fss = N3FSSyncManagerInstance();
            if (fss != nullptr)
            {
                QStringList path(ONBOARD_DIR);
                N3FSSyncManagerSync(fss, &path);
            }
        }

        ScheduleNextSync();
        return;
    }

    const auto& next = SyncQueue.head();
    StartSync(next.source, next.dest);
}

static int NickelCloudInit()
{
    static NickelCloudWatcher watcher;

    auto* wm = WirelessManagerInstance != nullptr ? WirelessManagerInstance() : nullptr;
    if (wm == nullptr)
    {
        nh_log("NickelCloud: could not get WirelessManager instance");
        return 0;
    }

    QObject::connect(wm, SIGNAL(networkConnected()), &watcher, SLOT(OnNetworkConnected()), Qt::UniqueConnection);
    QObject::connect(wm, SIGNAL(networkDisconnected()), &watcher, SLOT(OnNetworkDisconnected()), Qt::UniqueConnection);
    return 0;
}

static struct nh_info NickelCloud = {
    .name = "NickelCloud",
    .desc = "Pull books from cloud storage into your library using rclone",
    .uninstall_flag = CONFIG_DIR "/uninstall",
};

static struct nh_hook NickelCloudHook[] = {
    {0},
};

static struct nh_dlsym NickelCloudDlsym[] = {
    {
        .name = "_ZN15WirelessManager14sharedInstanceEv",
        .out = nh_symoutptr(WirelessManagerInstance),
        .desc = "WirelessManager::sharedInstance",
    },
    {
        .name = "_ZN15N3FSSyncManager14sharedInstanceEv",
        .out = nh_symoutptr(N3FSSyncManagerInstance),
        .desc = "N3FSSyncManager::sharedInstance",
    },
    {
        .name = "_ZN15N3FSSyncManager4syncERK11QStringList",
        .out = nh_symoutptr(N3FSSyncManagerSync),
        .desc = "N3FSSyncManager::sync",
    },
    {0},
};

NickelHook(
    .init = NickelCloudInit,
    .info = &NickelCloud,
    .hook = NickelCloudHook,
    .dlsym = NickelCloudDlsym,
)
