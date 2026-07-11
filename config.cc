#include "config.h"
#include <NickelHook.h>
#include <QFile>

void NickelCloudConfig::Load(const QString& path)
{
    General.clear();
    SourceList.clear();

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return;
    }

    auto section = Section::None;

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
            if (name == "general")
            {
                section = Section::General;
            }
            else if (name == "sources")
            {
                section = Section::Sources;
            }
            else
            {
                nh_log("NickelCloud: ignoring unknown config section: %s", qPrintable(line));
                section = Section::None;
            }
            continue;
        }

        if (section == Section::None)
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

        auto key = line.left(equals).trimmed();
        auto value = line.mid(equals + 1).trimmed();
        if (key.isEmpty() || value.isEmpty())
        {
            nh_log("NickelCloud: ignoring malformed line: %s", qPrintable(line));
            continue;
        }

        if (section == Section::General)
        {
            General.insert(key, value);
        }
        else
        {
            SourceList.enqueue({key, value});
        }
    }
}

QString NickelCloudConfig::GetString(const QString& key, const QString& defaultValue) const
{
    return General.value(key, defaultValue);
}

int NickelCloudConfig::GetInt(const QString& key, int defaultValue) const
{
    bool success = false;
    auto value = General.value(key).toInt(&success);
    return success ? value : defaultValue;
}

bool NickelCloudConfig::GetBool(const QString& key, bool defaultValue) const
{
    if (!General.contains(key))
    {
        return defaultValue;
    }

    auto value = General.value(key).trimmed().toLower();
    if (value == "true" || value == "1" || value == "yes")
    {
        return true;
    }
    if (value == "false" || value == "0" || value == "no")
    {
        return false;
    }

    return defaultValue;
}

QString NickelCloudConfig::StripComment(const QString& line)
{
    auto comment = line.indexOf('#');
    return (comment < 0 ? line : line.left(comment)).trimmed();
}
