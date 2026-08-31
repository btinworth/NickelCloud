#pragma once

#include <QString>

// resolved via dlsym in Main.cc; null (and ShowToast a no-op) if nickel doesn't expose them
extern void* (*MainWindowControllerInstance)();
extern void (*MainWindowControllerToast)(void*, const QString&, const QString&, int);

void ShowToast(const QString& primary, const QString& secondary = QString(), int milliseconds = 3000);
