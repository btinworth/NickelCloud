#include "utils.h"
#include <QDir>

QString Utils::ResolvePath(const QString& root, const QString& relative)
{
    auto resolved = QDir::cleanPath(root + "/" + relative);
    if (resolved != root && !resolved.startsWith(root + "/"))
    {
        return QString();
    }

    return resolved;
}
