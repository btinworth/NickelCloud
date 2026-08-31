#include "Toast.h"

void* (*MainWindowControllerInstance)() = nullptr;
void (*MainWindowControllerToast)(void*, const QString&, const QString&, int) = nullptr;

void ShowToast(const QString& primary, const QString& secondary, int milliseconds)
{
    if (MainWindowControllerInstance == nullptr || MainWindowControllerToast == nullptr)
    {
        return;
    }

    auto* mwc = MainWindowControllerInstance();
    if (mwc == nullptr)
    {
        return;
    }

    MainWindowControllerToast(mwc, primary, secondary, milliseconds);
}
