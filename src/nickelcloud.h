#pragma once

#include <QObject>

// Watches N3FSSyncManager for a content sync. A plain QObject with a slot so we
// can use the string-based connect against Nickel's signal (we don't have the
// N3FSSyncManager class declaration, only its QObject instance via dlsym).
class NickelCloudWatcher : public QObject {
    Q_OBJECT
public slots:
    void onGotNumFiles(int num);   // sync started, N files to process
    void onParseProgress(int n);   // progress (chatty)
    void onSyncFinished();         // sync done
};
