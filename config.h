#pragma once

#include <QQueue>
#include <QString>

struct SyncPair
{
    QString source;
    QString dest;
};

// Parses the NickelCloud config file:
//
//   [sources]
//   remote:folder = /local/destination   # inline comments are stripped
//
class NickelCloudConfig
{
public:
    void Load(const QString& path);

    const QQueue<SyncPair>& Sources() const { return SourceList; }

private:
    static QString StripComment(const QString& line);

    QQueue<SyncPair> SourceList;
};
