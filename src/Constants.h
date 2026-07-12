#pragma once

#include <QDir>
#include <QFile>

static const QDir ONBOARD_DIR("/mnt/onboard");
static const QDir CONFIG_DIR(ONBOARD_DIR.filePath(".adds/nickelcloud"));
static const QDir INSTALL_DIR("/usr/local/nickelcloud");
static const QDir CACHE_DIR(CONFIG_DIR.filePath("cache"));

static const QFile RCLONE_BIN(INSTALL_DIR.filePath("rclone"));
static const QFile CA_CERT(INSTALL_DIR.filePath("cacert.pem"));
static const QFile RCLONE_TMPL(INSTALL_DIR.filePath("rclone.conf.tmpl"));
static const QFile NICKELCLOUD_TMPL(INSTALL_DIR.filePath("nickelcloud.conf.tmpl"));
static const QFile RCLONE_CONF(CONFIG_DIR.filePath("rclone.conf"));
static const QFile NICKELCLOUD_CONF(CONFIG_DIR.filePath("nickelcloud.conf"));

static const QByteArray UNINSTALL_FLAG = CONFIG_DIR.filePath("uninstall").toUtf8();
