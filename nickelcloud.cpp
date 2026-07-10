#include "nickelcloud.h"
#include <NickelHook.h>
#include <QDir>
#include <QFile>
#include <QObject>
#include <QProcess>
#include <QStringList>

static QObject* (*N3FSSyncManagerInstance)();
static void (*N3FSSyncManagerSync)(QObject*, QStringList*);

#define CONFIG_DIR "/mnt/onboard/.adds/nickelcloud"
#define INSTALL_DIR "/usr/local/nickelcloud"

static const char* RCLONE_BIN = INSTALL_DIR "/rclone";
static const char* CA_CERT = INSTALL_DIR "/cacert.pem";
static const char* RCLONE_TMPL = INSTALL_DIR "/rclone.conf.tmpl";
static const char* RCLONE_CONF = CONFIG_DIR "/rclone.conf";
static const char* RCLONE_LOG = CONFIG_DIR "/rclone.log";

static bool ReScanning = false; // true while our own rescan runs, so the finished() it emits doesn't loop
static bool Pulling = false; // true while rclone is running

static void TriggerRescan()
{
    auto* fss = N3FSSyncManagerInstance();
    if (!fss)
    {
        return;
    }

    ReScanning = true;

    QStringList path("/mnt/onboard");
    N3FSSyncManagerSync(fss, &path);
}

void NickelCloudWatcher::OnSyncFinished()
{
    if (ReScanning)
    {
        ReScanning = false;
        return;
    }
    if (Pulling)
    {
        return;
    }

    nh_log("NickelCloud: sync finished, pulling from cloud");
    Pulling = true;

    // TODO(config): source/dest hardcoded for now — replaced with config in step 3
    auto* rclone = new QProcess(this);
    QObject::connect(rclone, SIGNAL(finished(int, QProcess::ExitStatus)), this,
        SLOT(OnPullFinished(int, QProcess::ExitStatus)));
    QObject::connect(rclone, SIGNAL(finished(int, QProcess::ExitStatus)), rclone,
        SLOT(deleteLater()));

    QStringList args;
    args << "copy"
         << "OneDrive:eBooks"
         << "/mnt/onboard/OneDrive/eBooks"
         << "--config" << RCLONE_CONF
         << "--ca-cert" << CA_CERT
         << "--error-on-no-transfer"
         << "--log-file" << RCLONE_LOG
         << "--log-level" << "INFO";
    rclone->start(RCLONE_BIN, args);
    nh_dump_log();
}

void NickelCloudWatcher::OnPullFinished(int exitCode, QProcess::ExitStatus status)
{
    Pulling = false;

    if (status != QProcess::NormalExit)
    {
        nh_log("NickelCloud: rclone crashed");
        nh_dump_log();
        return;
    }
    if (exitCode == 0)
    {
        nh_log("NickelCloud: rclone downloaded files, triggering rescan");
        nh_dump_log();

        TriggerRescan();
        return;
    }
    if (exitCode == 9)
    {
        nh_log("NickelCloud: rclone found nothing new to download");
        nh_dump_log();
        return;
    }

    nh_log("NickelCloud: rclone failed with exit code: %d", exitCode);
    nh_dump_log();
    return;
}

static void InitConfig()
{
    QDir().mkpath(CONFIG_DIR);
    if (!QFile::exists(RCLONE_CONF))
    {
        QFile::copy(RCLONE_TMPL, RCLONE_CONF);
        nh_log("NickelCloud: created rclone.conf from template");
    }
}

static int NickelCloudInit()
{
    InitConfig();

    auto* fss = N3FSSyncManagerInstance ? N3FSSyncManagerInstance() : nullptr;
    if (!fss)
    {
        nh_log("NickelCloud: could not get N3FSSyncManager instance");
        return 0;
    }

    QObject::connect(fss, SIGNAL(finished()), new NickelCloudWatcher(),
        SLOT(OnSyncFinished()), Qt::UniqueConnection);
    nh_log("NickelCloud: watching N3FSSyncManager::finished()");
    return 0;
}

static struct nh_info NickelCloud = {
    .name = "NickelCloud",
    .desc = "Pull books from the cloud",
    .uninstall_flag = "/mnt/onboard/.adds/nickelcloud/uninstall",
};

static struct nh_hook NickelCloudHook[] = {
    {0},
};

static struct nh_dlsym NickelCloudDlsym[] = {
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
