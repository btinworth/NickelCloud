#include <cstddef>
#include <NickelHook.h>

static void (*N3FSSyncManager__sync)(void *_this, void *paths);

extern "C" __attribute__((visibility("default")))
void nickelcloud_fss_sync(void *_this, void *paths) {
    nh_log("NickelCloud: N3FSSyncManager::sync intercepted");
    nh_dump_log(); // persist proof to /mnt/onboard/.kobo/NickelCloud_*.log
    N3FSSyncManager__sync(_this, paths); // call through to Nickel's real sync
}

static int nickelcloud_init() {
    nh_log("NickelCloud: loaded");
    return 0;
}

static struct nh_info NickelCloud = {
    .name           = "NickelCloud",
    .desc           = "Pull books from the cloud during Nickel sync",
    .uninstall_flag = "/mnt/onboard/.adds/nickelcloud/uninstall",
};

static struct nh_hook NickelCloudHook[] = {
    {
        .sym     = "_ZN15N3FSSyncManager4syncERK11QStringList",
        .sym_new = "nickelcloud_fss_sync",
        .lib     = "libnickel.so.1.0.0",
        .out     = nh_symoutptr(N3FSSyncManager__sync),
        .desc    = "intercept filesystem content sync",
    },
    {0},
};

static struct nh_dlsym NickelCloudDlsym[] = {
    {0},
};

NickelHook(
    .init  = nickelcloud_init,
    .info  = &NickelCloud,
    .hook  = NickelCloudHook,
    .dlsym = NickelCloudDlsym,
)
