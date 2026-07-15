#pragma once

#include <QHash>
#include <QQueue>
#include <QString>
#include <QStringList>

struct SyncPair
{
    QString source;
    QString dest;
};

class UserConfig
{
public:
    void Load(const QString& path);

    QString GetMode() const;
    int GetInterval() const;
    int GetTransfers() const;
    QStringList GetExtraArgs() const;
    bool GetLogEnabled() const;

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

    static QString ResolvePath(const QString& root, const QString& relative);

    QHash<QString, QString> General;
    QQueue<SyncPair> Sources;
};
