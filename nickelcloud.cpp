#include "nickelcloud.h"
#include <NickelHook.h>
#include <QObject>
#include <QStringList>

static QObject* (*N3FSSyncManagerInstance)();
static void (*N3FSSyncManagerSync)(QObject*, QStringList*);

static bool ReScanning = false;

void NickelCloudWatcher::OnSyncFinished()
{
    if (ReScanning)
    {
        ReScanning = false;

        nh_log("NickelCloud: Rescan finished");
        nh_dump_log();
        return;
    }

    auto* fss = N3FSSyncManagerInstance();
    if (!fss)
    {
        return;
    }

    nh_log("NickelCloud: sync finished, triggering rescan");

    ReScanning = true;

    QStringList path("/mnt/onboard");
    N3FSSyncManagerSync(fss, &path);
    nh_dump_log();
}

static int NickelCloudInit()
{
    auto* fss = N3FSSyncManagerInstance
        ? N3FSSyncManagerInstance()
        : nullptr;
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
