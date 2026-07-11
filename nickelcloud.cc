#include "nickelcloud.h"
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
static const char* RCLONE_LOG = CONFIG_DIR "/rclone.log";
static const char* NICKELCLOUD_CONF = CONFIG_DIR "/nickelcloud.conf";
static const char* CACHE_DIR = CONFIG_DIR "/cache";
static const int SYNC_INTERVAL = 5 * 60 * 1000; // rescan frequency (5m)

static QQueue<SyncPair> SyncQueue; // remaining sources
static bool AnyTransferred = false; // true if anything was downloaded
static bool SyncInProgress = false; // true whilst syncing

static void TriggerRescan()
{
    auto* fss = N3FSSyncManagerInstance();
    if (fss == nullptr)
    {
        return;
    }

    QStringList path(ONBOARD_DIR);
    N3FSSyncManagerSync(fss, &path);
}

static void ReadConfig()
{
    // config file is formatted as source=destination pairs, one per line
    // skip blank lines and lines starting with '#'

    SyncQueue.clear();

    QFile file(NICKELCLOUD_CONF);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return;
    }

    while (!file.atEnd())
    {
        auto line = QString::fromUtf8(file.readLine()).trimmed();
        if (line.isEmpty() || line.startsWith('#'))
        {
            continue;
        }

        auto equals = line.indexOf('=');
        if (equals < 0)
        {
            nh_log("NickelCloud: ignoring line without '=': %s", qPrintable(line));
            continue;
        }

        auto source = line.left(equals).trimmed();
        auto destination = line.mid(equals + 1).trimmed();
        if (source.isEmpty() || destination.isEmpty())
        {
            nh_log("NickelCloud: ignoring malformed line: %s", qPrintable(line));
            continue;
        }

        // prefix all paths with /mnt/onboard, prevent writing outside this directory
        destination = QDir::cleanPath(QString(ONBOARD_DIR "/") + destination);
        if (destination != ONBOARD_DIR && !destination.startsWith(ONBOARD_DIR "/"))
        {
            nh_log("NickelCloud: ignoring out-of-bounds destination: %s", qPrintable(destination));
            continue;
        }

        SyncQueue.enqueue({source, destination});
    }
}

NickelCloudWatcher::NickelCloudWatcher()
{
    SyncTimer.setInterval(SYNC_INTERVAL);
    QObject::connect(&SyncTimer, SIGNAL(timeout()), this, SLOT(Sync()));
}

void NickelCloudWatcher::StartSync(const QString& source, const QString& dest)
{
    QDir().mkpath(dest);

    nh_log("NickelCloud: syncing %s -> %s", qPrintable(source), qPrintable(dest));

    auto* rclone = new QProcess(this);
    QObject::connect(rclone, SIGNAL(finished(int, QProcess::ExitStatus)), this,
        SLOT(OnSyncFinished(int, QProcess::ExitStatus)));
    QObject::connect(rclone, SIGNAL(error(QProcess::ProcessError)), this,
        SLOT(OnSyncError(QProcess::ProcessError)));
    QObject::connect(rclone, SIGNAL(finished(int, QProcess::ExitStatus)), rclone,
        SLOT(deleteLater()));

    QStringList args;
    args << "copy"
         << source << dest
         << "--config" << RCLONE_CONF
         << "--ca-cert" << CA_CERT
         << "--cache-dir" << CACHE_DIR
         << "--transfers" << "1"
         << "--stats" << "0"
         << "--error-on-no-transfer"
         << "--log-file" << RCLONE_LOG
         << "--log-level" << "INFO";
    rclone->start(RCLONE_BIN, args);
}

// start the next queued sync, or finish the cycle if the queue is empty
void NickelCloudWatcher::SyncNext()
{
    if (SyncQueue.isEmpty())
    {
        SyncInProgress = false;
        nh_dump_log();
        if (AnyTransferred)
        {
            TriggerRescan();
        }

        return;
    }

    const auto& next = SyncQueue.head();
    StartSync(next.source, next.dest);
}

void NickelCloudWatcher::Sync()
{
    if (SyncInProgress)
    {
        return;
    }

    ReadConfig();
    if (SyncQueue.isEmpty())
    {
        nh_log("NickelCloud: no sources configured");
        return;
    }

    SyncInProgress = true;
    AnyTransferred = false;

    nh_log("NickelCloud: pulling %d source(s) from cloud", SyncQueue.size());
    SyncNext();
}

void NickelCloudWatcher::OnNetworkConnected()
{
    SyncTimer.start();
    Sync();
}

void NickelCloudWatcher::OnNetworkDisconnected()
{
    SyncTimer.stop();
}

void NickelCloudWatcher::OnSyncFinished(int exitCode, QProcess::ExitStatus status)
{
    auto source = SyncQueue.head().source;

    if (status != QProcess::NormalExit)
    {
        nh_log("NickelCloud: rclone crashed for %s", qPrintable(source));
    }
    else if (exitCode == 0)
    {
        AnyTransferred = true;
        nh_log("NickelCloud: downloaded new files for %s", qPrintable(source));
    }
    else if (exitCode == 9)
    {
        nh_log("NickelCloud: nothing new for %s", qPrintable(source));
    }
    else
    {
        nh_log("NickelCloud: rclone failed for %s (exit %d)", qPrintable(source), exitCode);
    }

    SyncQueue.dequeue();
    SyncNext();
}

void NickelCloudWatcher::OnSyncError(QProcess::ProcessError error)
{
    // only FailedToStart skips finished(); other errors are handled by OnSyncFinished
    if (error != QProcess::FailedToStart)
    {
        return;
    }

    nh_log("NickelCloud: rclone failed to start for %s", qPrintable(SyncQueue.head().source));

    auto* rclone = qobject_cast<QProcess*>(sender());
    if (rclone != nullptr)
    {
        rclone->deleteLater();
    }

    SyncQueue.dequeue();
    SyncNext();
}

static void InitConfig()
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
}

static int NickelCloudInit()
{
    InitConfig();

    auto* wm = WirelessManagerInstance != nullptr ? WirelessManagerInstance() : nullptr;
    if (wm == nullptr)
    {
        nh_log("NickelCloud: could not get WirelessManager instance");
        return 0;
    }

    auto* watcher = new NickelCloudWatcher();
    QObject::connect(wm, SIGNAL(networkConnected()), watcher, SLOT(OnNetworkConnected()), Qt::UniqueConnection);
    QObject::connect(wm, SIGNAL(networkDisconnected()), watcher, SLOT(OnNetworkDisconnected()), Qt::UniqueConnection);
    return 0;
}

static struct nh_info NickelCloud = {
    .name = "NickelCloud",
    .desc = "Sync books from the cloud",
    .uninstall_flag = "/mnt/onboard/.adds/nickelcloud/uninstall",
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
