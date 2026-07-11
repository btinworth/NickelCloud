#include "utils_test.h"
#include "../utils.h"
#include <QtTest>

void UtilsTest::resolvesSimpleRelativePath()
{
    QCOMPARE(Utils::ResolvePath("/mnt/onboard", "Books"), QString("/mnt/onboard/Books"));
}

void UtilsTest::resolvesNestedRelativePath()
{
    QCOMPARE(Utils::ResolvePath("/mnt/onboard", "Books/Fiction"), QString("/mnt/onboard/Books/Fiction"));
}

void UtilsTest::allowsRootItself()
{
    QCOMPARE(Utils::ResolvePath("/mnt/onboard", ""), QString("/mnt/onboard"));
}

void UtilsTest::collapsesInternalTraversalThatStaysWithinRoot()
{
    QCOMPARE(Utils::ResolvePath("/mnt/onboard", "Books/../Fiction"), QString("/mnt/onboard/Fiction"));
}

void UtilsTest::rejectsParentTraversal()
{
    QVERIFY(Utils::ResolvePath("/mnt/onboard", "../etc").isEmpty());
}

void UtilsTest::rejectsExcessiveParentTraversal()
{
    QVERIFY(Utils::ResolvePath("/mnt/onboard", "../../../../etc/passwd").isEmpty());
}

void UtilsTest::doesNotEscapeViaEmbeddedAbsolutePath()
{
    // a leading '/' in the config value must not be treated as replacing the root
    QCOMPARE(Utils::ResolvePath("/mnt/onboard", "/etc/passwd"), QString("/mnt/onboard/etc/passwd"));
}
