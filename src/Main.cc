#include "Constants.h"
#include "NickelCloud.h"
#include <NickelHook.h>

static int NickelCloudInit()
{
    auto* wm = WirelessManagerInstance();
    if (wm == nullptr)
    {
        nh_log("NickelCloud: could not get WirelessManager instance");
        return 0;
    }

    static NickelCloud nickelCloud;
    QObject::connect(wm, SIGNAL(networkConnected()), &nickelCloud, SLOT(OnNetworkConnected()), Qt::UniqueConnection);
    QObject::connect(wm, SIGNAL(networkDisconnected()), &nickelCloud, SLOT(OnNetworkDisconnected()), Qt::UniqueConnection);
    return 0;
}

static struct nh_info NickelCloud = {
    .name = "NickelCloud",
    .desc = "Pull books from cloud storage into your library using rclone",
    .uninstall_flag = UNINSTALL_FLAG,
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
