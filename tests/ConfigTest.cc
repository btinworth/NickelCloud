#include "ConfigTest.h"
#include "Constants.h"
#include <QFile>
#include <QTemporaryFile>
#include <QtTest>

UserConfig ConfigTest::LoadConfig(const QString& contents)
{
    QTemporaryFile file;
    if (!file.open())
    {
        qFatal("failed to create temp file for test config");
    }

    file.write(contents.toUtf8());
    file.close();

    UserConfig config;
    QFile configFile(file.fileName());
    config.Load(configFile.fileName());
    return config;
}

void ConfigTest::sources_parsesBasicPairs()
{
    auto config = LoadConfig(
        "[sources]\n"
        "OneDrive:eBooks = Books\n"
        "Dropbox:Novels = Fiction/Novels\n");

    auto sources = config.GetSources();
    QCOMPARE(sources.size(), 2);
    QCOMPARE(sources.at(0).source, QString("OneDrive:eBooks"));
    QCOMPARE(sources.at(0).dest, QString(ONBOARD_DIR) + "/Books");
    QCOMPARE(sources.at(1).source, QString("Dropbox:Novels"));
    QCOMPARE(sources.at(1).dest, QString(ONBOARD_DIR) + "/Fiction/Novels");
}

void ConfigTest::sources_ignoresEmptyDestination()
{
    auto config = LoadConfig(
        "[sources]\n"
        "OneDrive:eBooks =\n");

    QCOMPARE(config.GetSources().size(), 0);
}

void ConfigTest::sources_ignoresEmptySource()
{
    auto config = LoadConfig(
        "[sources]\n"
        " = Books\n");

    QCOMPARE(config.GetSources().size(), 0);
}

void ConfigTest::sources_ignoredOutsideSection()
{
    // no [sources] header at all, so this should never be parsed as a source
    auto config = LoadConfig("OneDrive:eBooks = Books\n");

    QCOMPARE(config.GetSources().size(), 0);
}

void ConfigTest::comments_stripFullLine()
{
    auto config = LoadConfig(
        "[sources]\n"
        "# OneDrive:eBooks = Books\n");

    QCOMPARE(config.GetSources().size(), 0);
}

void ConfigTest::comments_stripTrailing()
{
    auto config = LoadConfig(
        "[sources]\n"
        "OneDrive:eBooks = Books # inline comment\n");

    auto sources = config.GetSources();
    QCOMPARE(sources.size(), 1);
    QCOMPARE(sources.at(0).dest, QString(ONBOARD_DIR) + "/Books");
}

void ConfigTest::sources_collapsesInternalTraversalThatStaysWithinRoot()
{
    auto config = LoadConfig(
        "[sources]\n"
        "OneDrive:eBooks = Books/../Fiction\n");

    auto sources = config.GetSources();
    QCOMPARE(sources.size(), 1);
    QCOMPARE(sources.at(0).dest, QString(ONBOARD_DIR) + "/Fiction");
}

void ConfigTest::sources_allowsRootItselfViaDot()
{
    auto config = LoadConfig(
        "[sources]\n"
        "OneDrive:eBooks = .\n");

    auto sources = config.GetSources();
    QCOMPARE(sources.size(), 1);
    QCOMPARE(sources.at(0).dest, QString(ONBOARD_DIR));
}

void ConfigTest::sources_normalizesTrailingSlash()
{
    auto config = LoadConfig(
        "[sources]\n"
        "OneDrive:eBooks = Books/\n");

    auto sources = config.GetSources();
    QCOMPARE(sources.size(), 1);
    QCOMPARE(sources.at(0).dest, QString(ONBOARD_DIR) + "/Books");
}

void ConfigTest::sources_rejectsParentTraversal()
{
    auto config = LoadConfig(
        "[sources]\n"
        "OneDrive:eBooks = ../etc\n");

    QCOMPARE(config.GetSources().size(), 0);
}

void ConfigTest::sources_rejectsExcessiveParentTraversal()
{
    auto config = LoadConfig(
        "[sources]\n"
        "OneDrive:eBooks = ../../../../etc/passwd\n");

    QCOMPARE(config.GetSources().size(), 0);
}

void ConfigTest::sources_doesNotEscapeViaEmbeddedAbsolutePath()
{
    auto config = LoadConfig(
        "[sources]\n"
        "OneDrive:eBooks = /etc/passwd\n");

    auto sources = config.GetSources();
    QCOMPARE(sources.size(), 1);
    QCOMPARE(sources.at(0).dest, QString(ONBOARD_DIR) + "/etc/passwd");
}

void ConfigTest::sources_preservesDuplicateSourceOrder()
{
    auto config = LoadConfig(
        "[sources]\n"
        "OneDrive:eBooks = Books\n"
        "OneDrive:eBooks = Fiction\n");

    auto sources = config.GetSources();
    QCOMPARE(sources.size(), 2);
    QCOMPARE(sources.at(0).source, QString("OneDrive:eBooks"));
    QCOMPARE(sources.at(0).dest, QString(ONBOARD_DIR) + "/Books");
    QCOMPARE(sources.at(1).source, QString("OneDrive:eBooks"));
    QCOMPARE(sources.at(1).dest, QString(ONBOARD_DIR) + "/Fiction");
}

void ConfigTest::sources_trimsWhitespaceAroundKeyAndValue()
{
    auto config = LoadConfig(
        "[sources]\n"
        "  OneDrive:eBooks   =   Books/Fiction   \n");

    auto sources = config.GetSources();
    QCOMPARE(sources.size(), 1);
    QCOMPARE(sources.at(0).source, QString("OneDrive:eBooks"));
    QCOMPARE(sources.at(0).dest, QString(ONBOARD_DIR) + "/Books/Fiction");
}

void ConfigTest::sections_areCaseInsensitive()
{
    auto config = LoadConfig(
        "[SOURCES]\n"
        "OneDrive:eBooks = Books\n");

    QCOMPARE(config.GetSources().size(), 1);
}

void ConfigTest::sections_unknownIsIgnored()
{
    auto config = LoadConfig(
        "[bogus]\n"
        "mode = sync\n");

    // key was never inserted, since the line fell outside any known section
    QCOMPARE(config.GetMode(), QString("copy"));
}

void ConfigTest::general_allowsEmptyValue()
{
    auto config = LoadConfig(
        "[general]\n"
        "extra_args = # nothing set\n");

    QVERIFY(config.GetExtraArgs().isEmpty());
}

void ConfigTest::mode_defaultsToCopy()
{
    auto config = LoadConfig("[general]\n");
    QCOMPARE(config.GetMode(), QString("copy"));
}

void ConfigTest::mode_acceptsSync()
{
    auto config = LoadConfig("[general]\nmode = sync\n");
    QCOMPARE(config.GetMode(), QString("sync"));
}

void ConfigTest::mode_rejectsInvalidValue()
{
    auto config = LoadConfig("[general]\nmode = delete\n");
    QCOMPARE(config.GetMode(), QString("copy"));
}

void ConfigTest::interval_defaultsToFiveMinutes()
{
    auto config = LoadConfig("[general]\n");
    QCOMPARE(config.GetInterval(), 300);
}

void ConfigTest::interval_negativeFallsBackToDefault()
{
    auto config = LoadConfig("[general]\ninterval = -5\n");
    QCOMPARE(config.GetInterval(), 300);
}

void ConfigTest::interval_zeroIsPreserved()
{
    // 0 is a deliberate sentinel meaning "disabled", not an invalid value
    auto config = LoadConfig("[general]\ninterval = 0\n");
    QCOMPARE(config.GetInterval(), 0);
}

void ConfigTest::transfers_defaultsToOne()
{
    auto config = LoadConfig("[general]\n");
    QCOMPARE(config.GetTransfers(), 1);
}

void ConfigTest::transfers_zeroFallsBackToDefault()
{
    auto config = LoadConfig("[general]\ntransfers = 0\n");
    QCOMPARE(config.GetTransfers(), 1);
}

void ConfigTest::transfers_negativeFallsBackToDefault()
{
    auto config = LoadConfig("[general]\ntransfers = -3\n");
    QCOMPARE(config.GetTransfers(), 1);
}

void ConfigTest::extraArgs_emptyByDefault()
{
    auto config = LoadConfig("[general]\n");
    QVERIFY(config.GetExtraArgs().isEmpty());
}

void ConfigTest::extraArgs_splitsOnSpaces()
{
    auto config = LoadConfig("[general]\nextra_args = --bwlimit 1M --dry-run\n");
    QCOMPARE(config.GetExtraArgs(), QStringList({"--bwlimit", "1M", "--dry-run"}));
}

void ConfigTest::logEnabled_defaultsToFalse()
{
    auto config = LoadConfig("[general]\n");
    QVERIFY(!config.GetLogEnabled());
}

void ConfigTest::logEnabled_acceptsTruthyValues()
{
    QVERIFY(LoadConfig("[general]\nlog = true\n").GetLogEnabled());
    QVERIFY(LoadConfig("[general]\nlog = 1\n").GetLogEnabled());
    QVERIFY(LoadConfig("[general]\nlog = yes\n").GetLogEnabled());
}

void ConfigTest::logEnabled_acceptsFalsyValues()
{
    QVERIFY(!LoadConfig("[general]\nlog = false\n").GetLogEnabled());
    QVERIFY(!LoadConfig("[general]\nlog = 0\n").GetLogEnabled());
    QVERIFY(!LoadConfig("[general]\nlog = no\n").GetLogEnabled());
}

void ConfigTest::logEnabled_unrecognizedValueFallsBackToDefault()
{
    auto config = LoadConfig("[general]\nlog = maybe\n");
    QVERIFY(!config.GetLogEnabled());
}
