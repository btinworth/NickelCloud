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
    void Stop();

signals:
    void Finished(bool success, bool transferred);

private slots:
    void OnOutput();
    void OnFinished(int exitCode, QProcess::ExitStatus status);
    void OnError(QProcess::ProcessError error);
    void EmitFinished();

private:
    void OnComplete(bool success);

    void HandleOutput(bool flush);
    void HandleOutputLine(const QString& line);

    QProcess Process;
    QByteArray PendingOutput;
    QString Source;
    bool Success = false;
    bool Transferred = false;
    bool FinishedEmitted = false;
};
