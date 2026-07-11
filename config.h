#pragma once

#include <QHash>
#include <QQueue>
#include <QString>

struct SyncPair
{
    QString source;
    QString dest;
};

class NickelCloudConfig
{
public:
    void Load(const QString& path);

    QString GetMode() const;
    int GetInterval() const;

    const QQueue<SyncPair>& GetSources() const;

private:
    enum class Section
    {
        None,
        General,
        Sources,
    };

    static QString StripComment(const QString& line);

    QString GetString(const QString& key, const QString& defaultValue = QString()) const;
    int GetInt(const QString& key, int defaultValue = 0) const;
    bool GetBool(const QString& key, bool defaultValue = false) const;

    QHash<QString, QString> General;
    QQueue<SyncPair> Sources;
};
