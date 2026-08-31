#pragma once

#include <QHash>
#include <QQueue>
#include <QString>

struct SyncPair
{
    QString source;
    QString dest;
};

inline bool operator==(const SyncPair& lhs, const SyncPair& rhs)
{
    return lhs.source == rhs.source && lhs.dest == rhs.dest;
}

class UserConfig
{
public:
    void Load(const QString& path);

    QString GetMode() const;
    int GetInterval() const;
    int GetTransfers() const;
    int GetCheckers() const;
    int GetBufferSize() const;
    bool GetLogEnabled() const;
    bool GetNotifyEnabled() const;

    const QQueue<SyncPair>& GetSources() const;

private:
    enum class Section
    {
        None,
        General,
        Sources,
    };

    static QString StripComment(const QString& line);
    static bool IsValidSource(const QString& source);

    QString GetString(const QString& key, const QString& defaultValue = QString()) const;
    int GetInt(const QString& key, int defaultValue = 0) const;
    bool GetBool(const QString& key, bool defaultValue = false) const;

    static QString ResolvePath(const QString& root, const QString& relative);

    QHash<QString, QString> General;
    QQueue<SyncPair> Sources;
};
