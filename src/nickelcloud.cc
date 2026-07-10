#include <cstddef>
#include <QObject>
#include <NickelHook.h>
#include "nickelcloud.h"

typedef QObject N3FSSyncManager;
static N3FSSyncManager *(*N3FSSyncManager__sharedInstance)();

void NickelCloudWatcher::onGotNumFiles(int num) {
    nh_log("NickelCloud: N3FSSyncManager sync started (%d files to process)", num);
}

void NickelCloudWatcher::onParseProgress(int n) {
    nh_log("NickelCloud: N3FSSyncManager parse progress: %d", n);
}

void NickelCloudWatcher::onSyncFinished() {
    nh_log("NickelCloud: N3FSSyncManager sync finished (detected!)");
    nh_dump_log();
}

static int nickelcloud_init() {
    nh_log("NickelCloud: loaded");

    N3FSSyncManager *fss = N3FSSyncManager__sharedInstance
                         ? N3FSSyncManager__sharedInstance()
                         : NULL;
    if (!fss) {
        nh_log("NickelCloud: could not get N3FSSyncManager instance");
        nh_dump_log();
        return 0;
    }

    NickelCloudWatcher *w = new NickelCloudWatcher();
    QObject::connect(fss, SIGNAL(gotNumFilesToProcess(int)), w, SLOT(onGotNumFiles(int)), Qt::UniqueConnection);
    QObject::connect(fss, SIGNAL(parseProgress(int)),        w, SLOT(onParseProgress(int)), Qt::UniqueConnection);
    QObject::connect(fss, SIGNAL(finished()),                w, SLOT(onSyncFinished()), Qt::UniqueConnection);

    nh_log("NickelCloud: watching N3FSSyncManager (gotNumFilesToProcess/parseProgress/finished)");
    nh_dump_log();
    return 0;
}

static struct nh_info NickelCloud = {
    .name           = "NickelCloud",
    .desc           = "Detect Nickel content sync (spike)",
    .uninstall_flag = "/mnt/onboard/.adds/nickelcloud/uninstall",
};

static struct nh_hook NickelCloudHook[] = {
    {0},
};

static struct nh_dlsym NickelCloudDlsym[] = {
    {
        .name = "_ZN15N3FSSyncManager14sharedInstanceEv",
        .out  = nh_symoutptr(N3FSSyncManager__sharedInstance),
        .desc = "N3FSSyncManager::sharedInstance",
    },
    {0},
};

NickelHook(
    .init  = nickelcloud_init,
    .info  = &NickelCloud,
    .hook  = NickelCloudHook,
    .dlsym = NickelCloudDlsym,
)
