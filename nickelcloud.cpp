#include "nickelcloud.h"
#include <NickelHook.h>
#include <QObject>

using N3FSSyncManager = QObject;
static N3FSSyncManager* (*N3FSSyncManager__sharedInstance)();

void NickelCloudWatcher::onSyncFinished()
{
    nh_log("NickelCloud: sync finished");
    nh_dump_log();
}

static int nickelcloud_init()
{
    auto* fss = N3FSSyncManager__sharedInstance
        ? N3FSSyncManager__sharedInstance()
        : nullptr;
    if (!fss)
    {
        nh_log("NickelCloud: could not get N3FSSyncManager instance");
        return 0;
    }

    QObject::connect(fss, SIGNAL(finished()), new NickelCloudWatcher(),
        SLOT(onSyncFinished()), Qt::UniqueConnection);
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
        .out = nh_symoutptr(N3FSSyncManager__sharedInstance),
        .desc = "N3FSSyncManager::sharedInstance",
    },
    {0},
};

NickelHook(
    .init = nickelcloud_init,
    .info = &NickelCloud,
    .hook = NickelCloudHook,
    .dlsym = NickelCloudDlsym,
)
