#include "ConfigTest.h"
#include <QtTest>

int main(int argc, char** argv)
{
    int status = 0;

    ConfigTest configTest;
    status |= QTest::qExec(&configTest, argc, argv);

    return status;
}
