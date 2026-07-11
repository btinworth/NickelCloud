#include "config.h"
#include <NickelHook.h>
#include <QFile>

void NickelCloudConfig::Load(const QString& path)
{
    SourceList.clear();

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return;
    }

    bool inSources = false;

    while (!file.atEnd())
    {
        auto line = StripComment(QString::fromUtf8(file.readLine()));
        if (line.isEmpty())
        {
            continue;
        }

        if (line.startsWith('[') && line.endsWith(']'))
        {
            auto name = line.mid(1, line.length() - 2).trimmed().toLower();
            inSources = (name == "sources");
            if (!inSources)
            {
                nh_log("NickelCloud: ignoring unknown config section: %s", qPrintable(line));
            }
            continue;
        }

        if (!inSources)
        {
            nh_log("NickelCloud: ignoring line outside of a section: %s", qPrintable(line));
            continue;
        }

        auto equals = line.indexOf('=');
        if (equals < 0)
        {
            nh_log("NickelCloud: ignoring line without '=': %s", qPrintable(line));
            continue;
        }

        auto source = line.left(equals).trimmed();
        auto destination = line.mid(equals + 1).trimmed();
        if (source.isEmpty() || destination.isEmpty())
        {
            nh_log("NickelCloud: ignoring malformed line: %s", qPrintable(line));
            continue;
        }

        SourceList.enqueue({source, destination});
    }
}

QString NickelCloudConfig::StripComment(const QString& line)
{
    auto comment = line.indexOf('#');
    return (comment < 0 ? line : line.left(comment)).trimmed();
}
