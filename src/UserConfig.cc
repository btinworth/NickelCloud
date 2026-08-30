#include "UserConfig.h"
#include "Constants.h"
#include "Log.h"
#include <QDir>
#include <QFile>

void UserConfig::Load(const QString& path)
{
    General.clear();
    Sources.clear();

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        Log("could not open config '%s': %s", qPrintable(path), qPrintable(file.errorString()));
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
                Log("ignoring unknown config section: %s", qPrintable(line));
                section = Section::None;
            }
            continue;
        }

        if (section == Section::None)
        {
            Log("ignoring line outside of a section: %s", qPrintable(line));
            continue;
        }

        auto equals = line.indexOf('=');
        if (equals < 0)
        {
            Log("ignoring line without '=': %s", qPrintable(line));
            continue;
        }

        auto key = line.left(equals).trimmed();
        auto value = line.mid(equals + 1).trimmed();
        if (key.isEmpty())
        {
            Log("ignoring malformed line: %s", qPrintable(line));
            continue;
        }

        if (section == Section::General)
        {
            General.insert(key, value);
        }
        else if (section == Section::Sources)
        {
            auto dest = ResolvePath(ONBOARD_DIR, value);
            if (!IsValidSource(key))
            {
                Log("ignoring invalid source: %s", qPrintable(key));
            }
            else if (value.isEmpty() || dest.isEmpty())
            {
                Log("ignoring invalid destination: %s", qPrintable(value));
            }
            else if (Sources.contains({key, dest}))
            {
                Log("ignoring duplicate source: %s = %s", qPrintable(key), qPrintable(value));
            }
            else
            {
                Sources.enqueue({key, dest});
            }
        }
    }
}

QString UserConfig::GetMode() const
{
    static const QString DEFAULT_MODE = "copy";

    auto mode = GetString("mode", DEFAULT_MODE);
    if (mode == "copy" || mode == "sync")
    {
        return mode;
    }

    Log("ignoring invalid mode '%s', defaulting to '%s'", qPrintable(mode), qPrintable(DEFAULT_MODE));
    return DEFAULT_MODE;
}

int UserConfig::GetInterval() const
{
    static const int DEFAULT_INTERVAL = 5 * 60; // seconds (5m)

    auto interval = GetInt("interval", DEFAULT_INTERVAL);
    if (interval < 0)
    {
        Log("ignoring negative interval '%d', defaulting to %d", interval, DEFAULT_INTERVAL);
        return DEFAULT_INTERVAL;
    }

    return interval;
}

int UserConfig::GetTransfers() const
{
    static const int DEFAULT_TRANSFERS = 1;

    auto transfers = GetInt("transfers", DEFAULT_TRANSFERS);
    if (transfers > 0)
    {
        return transfers;
    }

    Log("ignoring invalid transfers value '%d', defaulting to %d", transfers, DEFAULT_TRANSFERS);
    return DEFAULT_TRANSFERS;
}

int UserConfig::GetCheckers() const
{
    static const int DEFAULT_CHECKERS = 1;

    auto checkers = GetInt("checkers", DEFAULT_CHECKERS);
    if (checkers > 0)
    {
        return checkers;
    }

    Log("ignoring invalid checkers value '%d', defaulting to %d", checkers, DEFAULT_CHECKERS);
    return DEFAULT_CHECKERS;
}

// per-transfer read buffer, in MiB
int UserConfig::GetBufferSize() const
{
    static const int DEFAULT_BUFFER_SIZE = 1;

    auto bufferSize = GetInt("buffer_size", DEFAULT_BUFFER_SIZE);
    if (bufferSize >= 0)
    {
        return bufferSize;
    }

    Log("ignoring negative buffer_size '%d', defaulting to %d", bufferSize, DEFAULT_BUFFER_SIZE);
    return DEFAULT_BUFFER_SIZE;
}

bool UserConfig::GetLogEnabled() const
{
    return GetBool("log", false);
}

const QQueue<SyncPair>& UserConfig::GetSources() const
{
    return Sources;
}

QString UserConfig::GetString(const QString& key, const QString& defaultValue) const
{
    return General.value(key, defaultValue);
}

int UserConfig::GetInt(const QString& key, int defaultValue) const
{
    bool success = false;
    auto value = General.value(key).toInt(&success);
    return success ? value : defaultValue;
}

bool UserConfig::GetBool(const QString& key, bool defaultValue) const
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

bool UserConfig::IsValidSource(const QString& source)
{
    return !source.startsWith('-') && source.indexOf(':') > 0;
}

QString UserConfig::StripComment(const QString& line)
{
    auto comment = line.indexOf('#');
    return (comment < 0 ? line : line.left(comment)).trimmed();
}

QString UserConfig::ResolvePath(const QString& root, const QString& relative)
{
    auto resolved = QDir::cleanPath(root + "/" + relative);
    if (resolved != root && !resolved.startsWith(root + "/"))
    {
        return QString();
    }

    return resolved;
}
