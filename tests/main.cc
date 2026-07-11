#include "config_test.h"
#include "utils_test.h"
#include <QtTest>

int main(int argc, char** argv)
{
    int status = 0;

    ConfigTest configTest;
    status |= QTest::qExec(&configTest, argc, argv);

    UtilsTest utilsTest;
    status |= QTest::qExec(&utilsTest, argc, argv);

    return status;
}
