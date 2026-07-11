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

    QString GetString(const QString& key, const QString& defaultValue = QString()) const;
    int GetInt(const QString& key, int defaultValue = 0) const;
    bool GetBool(const QString& key, bool defaultValue = false) const;

    const QQueue<SyncPair>& Sources() const { return SourceList; }

private:
    enum class Section
    {
        None,
        General,
        Sources,
    };

    static QString StripComment(const QString& line);

    QHash<QString, QString> General;
    QQueue<SyncPair> SourceList;
};
