#include "Log.h"
#include "Constants.h"
#include <NickelHook.h>
#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <cstdarg>
#include <cstdio>

namespace
{
constexpr qint64 MAX_LOG_BYTES = 256 * 1024;
bool LogEnabled = false;

void RotateLog()
{
    if (QFileInfo(NICKELCLOUD_LOG).size() < MAX_LOG_BYTES)
    {
        return;
    }

    QFile::remove(NICKELCLOUD_LOG_OLD);
    QFile::rename(NICKELCLOUD_LOG, NICKELCLOUD_LOG_OLD);
}
}

void SetLogEnabled(bool enabled)
{
    LogEnabled = enabled;
}

void Log(const char* fmt, ...)
{
    char message[512];

    va_list args;
    va_start(args, fmt);
    vsnprintf(message, sizeof(message), fmt, args);
    va_end(args);

    nh_log("%s", message);

    if (!LogEnabled)
    {
        return;
    }

    RotateLog();

    // reopened per message so an open handle can't block onboard being unmounted for USB
    QFile file(NICKELCLOUD_LOG);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
    {
        return;
    }

    QTextStream out(&file);
    out << QDateTime::currentDateTime().toString(Qt::ISODate) << " " << message << "\n";
}
