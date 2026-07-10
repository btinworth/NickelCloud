#pragma once

#include <QObject>

class NickelCloudWatcher : public QObject {
    Q_OBJECT

public slots:
    void onSyncFinished();
};
