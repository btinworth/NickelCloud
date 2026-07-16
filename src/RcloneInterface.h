#pragma once

#include <QByteArray>
#include <QObject>
#include <QProcess>
#include <QString>
#include <QStringList>

class RcloneInterface : public QObject
{
    Q_OBJECT

public:
    explicit RcloneInterface(QObject* parent = nullptr);

    void Start(const QStringList& args, const QString& source);

signals:
    void Finished(bool success, bool transferred);

private slots:
    void OnOutput();
    void OnFinished(int exitCode, QProcess::ExitStatus status);
    void OnError(QProcess::ProcessError error);

private:
    void ReadOutput();
    void FlushOutput();
    void HandleOutputLine(const QString& line);

    QProcess Process;
    QByteArray PendingOutput;
    QString Source;
    bool Transferred = false;
    bool FailedToStart = false;
};
