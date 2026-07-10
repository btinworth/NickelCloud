#include <cstddef>

#include <QObject>

#include <NickelHook.h>

#include "nickelcloud.h"

// N3FSSyncManager is a QObject singleton inside libnickel. We can't PLT-hook its
// sync() (Nickel doesn't call it through the PLT), but — like NickelDBus does —
// we can resolve sharedInstance() with dlsym and connect to its signals. This
// spike just detects a sync via finished() and logs it.
typedef QObject N3FSSyncManager;
static N3FSSyncManager *(*N3FSSyncManager__sharedInstance)();

void NickelCloudWatcher::onSyncFinished() {
    nh_log("NickelCloud: N3FSSyncManager sync finished (detected!)");
    nh_dump_log(); // proof at /mnt/onboard/NickelCloud_*.log (drive root)
}

static int nickelcloud_init() {
    nh_log("NickelCloud: loaded");

    N3FSSyncManager *fss = N3FSSyncManager__sharedInstance
                         ? N3FSSyncManager__sharedInstance()
                         : NULL;
    if (!fss) {
        nh_log("NickelCloud: could not get N3FSSyncManager instance");
        nh_dump_log();
        return 0; // stay loaded so the log survives for diagnosis
    }

    // finished() is what NickelDBus connects to for its fssFinished signal.
    QObject::connect(fss, SIGNAL(finished()), new NickelCloudWatcher(),
                     SLOT(onSyncFinished()), Qt::UniqueConnection);

    nh_log("NickelCloud: watching N3FSSyncManager::finished()");
    nh_dump_log(); // proves init ran and the connection was made
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
