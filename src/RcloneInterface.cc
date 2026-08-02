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
    if (Process.state() != QProcess::NotRunning)
    {
        nh_log("NickelCloud: rclone is already running, ignoring start request for %s", qPrintable(source));
        return;
    }

    Source = source;
    Transferred = false;
    FailedToStart = false;
    FinishedEmitted = false;
    PendingOutput.clear();
    Process.start(RCLONE_BIN, args);
}

void RcloneInterface::OnOutput()
{
    HandleOutput(false);
}

void RcloneInterface::OnFinished(int exitCode, QProcess::ExitStatus status)
{
    if (FailedToStart)
    {
        return;
    }

    bool success = false;
    if (status != QProcess::NormalExit)
    {
        nh_log("NickelCloud: rclone crashed for %s", qPrintable(Source));
    }
    else if (exitCode == 0)
    {
        success = true;
        nh_log("NickelCloud: rclone completed successfully for %s", qPrintable(Source));
    }
    else
    {
        nh_log("NickelCloud: rclone failed for %s (exit %d)", qPrintable(Source), exitCode);
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

    FailedToStart = true;
    nh_log("NickelCloud: rclone failed to start for %s", qPrintable(Source));
    OnComplete(false);
}

void RcloneInterface::OnComplete(bool success)
{
    HandleOutput(true);

    if (FinishedEmitted)
    {
        return;
    }

    FinishedEmitted = true;
    emit Finished(success, Transferred);
}

void RcloneInterface::HandleOutput(bool handleRemainder)
{
    PendingOutput += Process.readAllStandardOutput();

    int newline;
    while ((newline = PendingOutput.indexOf('\n')) >= 0)
    {
        auto line = QString::fromUtf8(PendingOutput.left(newline)).trimmed();
        PendingOutput.remove(0, newline + 1);
        HandleOutputLine(line);
    }

    if (handleRemainder)
    {
        auto line = QString::fromUtf8(PendingOutput).trimmed();
        PendingOutput.clear();
        HandleOutputLine(line);
    }
}

void RcloneInterface::HandleOutputLine(const QString& line)
{
    if (line.isEmpty())
    {
        return;
    }

    auto doc = QJsonDocument::fromJson(line.toUtf8());
    if (!doc.isObject())
    {
        return;
    }

    auto msg = doc.object().value("msg").toString();

    if (msg.startsWith("Copied") || msg.startsWith("Deleted"))
    {
        // a file has changed, flag for library scan
        Transferred = true;
    }

    nh_log(qPrintable(msg));
}
