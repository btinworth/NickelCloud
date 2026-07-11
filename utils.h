#pragma once

#include <QString>

class Utils
{
public:
    static QString ResolvePath(const QString& root, const QString& relative);
};
