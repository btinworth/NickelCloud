#include "Constants.h"
#include "Log.h"
#include "NickelCloud.h"
#include <NickelHook.h>
#include <QDir>

static int NickelCloudInit()
{
    static NickelCloud nickelCloud;

    auto* wm = WirelessManagerInstance();
    if (wm != nullptr)
    {
        QObject::connect(wm, SIGNAL(networkConnected()), &nickelCloud, SLOT(OnNetworkConnected()), Qt::UniqueConnection);
        QObject::connect(wm, SIGNAL(networkDisconnected()), &nickelCloud, SLOT(OnNetworkDisconnected()), Qt::UniqueConnection);
    }
    else
    {
        Log("could not get WirelessManager instance, syncing will not respond to network changes");
        return 1;
    }

    auto* pwm = PlugWorkflowManagerInstance();
    if (pwm != nullptr)
    {
        QObject::connect(pwm, SIGNAL(aboutToConnect()), &nickelCloud, SLOT(OnUsbConnecting()), Qt::UniqueConnection);
        QObject::connect(pwm, SIGNAL(doneProcessing()), &nickelCloud, SLOT(OnUsbDoneProcessing()), Qt::UniqueConnection);
    }
    else
    {
        Log("could not get PlugWorkflowManager instance, syncing will not pause for USB");
    }

    return 0;
}

static bool NickelCloudUninstall()
{
    Log("removing NickelCloud config and program files");

    const char* const dirs[] = {CONFIG_DIR, INSTALL_DIR};

    auto deleted = true;
    for (const auto* path : dirs)
    {
        QDir dir(path);
        deleted &= !dir.exists() || dir.removeRecursively();
    }

    return deleted;
}

static struct nh_info NickelCloudInfo = {
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
        .name = "_ZN19PlugWorkflowManager14sharedInstanceEv",
        .out = nh_symoutptr(PlugWorkflowManagerInstance),
        .desc = "PlugWorkflowManager::sharedInstance",
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
    .info = &NickelCloudInfo,
    .hook = NickelCloudHook,
    .dlsym = NickelCloudDlsym,
    .uninstall = NickelCloudUninstall,
)
