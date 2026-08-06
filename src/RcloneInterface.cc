#include "RcloneInterface.h"
#include "Constants.h"
#include <NickelHook.h>
#include <QJsonDocument>
#include <QJsonObject>

RcloneInterface::RcloneInterface(QObject* parent)
    : QObject(parent)
{
    Process.setProcessChannelMode(QProcess::MergedChannels);

    QObject::connect(&Process, &QProcess::readyReadStandardOutput, this, &RcloneInterface::OnOutput);
    QObject::connect(&Process, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &RcloneInterface::OnFinished);
    QObject::connect(&Process, static_cast<void (QProcess::*)(QProcess::ProcessError)>(&QProcess::error), this, &RcloneInterface::OnError);
}

void RcloneInterface::Start(const QStringList& args, const QString& source)
{
    Source = source;
    Success = false;
    Transferred = false;
    FinishedEmitted = false;
    PendingOutput.clear();
    Process.start(RCLONE_BIN, args);
}

void RcloneInterface::Stop()
{
    if (Process.state() == QProcess::NotRunning)
    {
        return;
    }

    nh_log("stopping rclone for %s", qPrintable(Source));
    Process.terminate();
}

void RcloneInterface::OnOutput()
{
    HandleOutput(false);
}

void RcloneInterface::OnFinished(int exitCode, QProcess::ExitStatus status)
{
    if (FinishedEmitted)
    {
        return;
    }

    bool success = false;
    if (status != QProcess::NormalExit)
    {
        nh_log("rclone crashed for %s", qPrintable(Source));
    }
    else if (exitCode == 0)
    {
        success = true;
        nh_log("rclone completed successfully for %s", qPrintable(Source));
    }
    else
    {
        nh_log("rclone failed for %s (exit %d)", qPrintable(Source), exitCode);
    }

    OnComplete(success);
}

void RcloneInterface::OnError(QProcess::ProcessError error)
{
    // only FailedToStart skips finished(); other errors are handled by OnFinished
    if (error != QProcess::FailedToStart)
    {
        return;
    }

    nh_log("rclone failed to start for %s", qPrintable(Source));
    OnComplete(false);
}

void RcloneInterface::OnComplete(bool success)
{
    if (FinishedEmitted)
    {
        return;
    }

    FinishedEmitted = true;
    Success = success;

    HandleOutput(true);

    // deferred so a handler can safely start the next run without re-entering this one
    QMetaObject::invokeMethod(this, "EmitFinished", Qt::QueuedConnection);
}

void RcloneInterface::EmitFinished()
{
    emit Finished(Success, Transferred);
}

void RcloneInterface::HandleOutput(bool flush)
{
    PendingOutput += Process.readAllStandardOutput();

    // treat unterminated trailing output as a final line
    if (flush && !PendingOutput.isEmpty() && !PendingOutput.endsWith('\n'))
    {
        PendingOutput += '\n';
    }

    int newline;
    while ((newline = PendingOutput.indexOf('\n')) >= 0)
    {
        auto line = QString::fromUtf8(PendingOutput.left(newline)).trimmed();
        PendingOutput.remove(0, newline + 1);
        HandleOutputLine(line);
    }
}

void RcloneInterface::HandleOutputLine(const QString& line)
{
    if (line.isEmpty())
    {
        return;
    }

    auto msg = QJsonDocument::fromJson(line.toUtf8()).object().value("msg").toString();
    if (msg.startsWith("Copied") || msg.startsWith("Moved") || msg.startsWith("Renamed") || msg.startsWith("Deleted"))
    {
        // a file has changed, flag for library scan
        Transferred = true;
    }

    nh_log("%s", qPrintable(line));
}
