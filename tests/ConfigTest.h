#pragma once

#include "UserConfig.h"
#include <QObject>

class ConfigTest : public QObject
{
    Q_OBJECT

private slots:
    void sources_parsesBasicPairs();
    void sources_ignoresEmptyDestination();
    void sources_ignoresEmptySource();
    void sources_ignoredOutsideSection();

    void comments_stripFullLine();
    void comments_stripTrailing();

    void sources_collapsesInternalTraversalThatStaysWithinRoot();
    void sources_allowsRootItselfViaDot();
    void sources_normalizesTrailingSlash();
    void sources_rejectsParentTraversal();
    void sources_rejectsExcessiveParentTraversal();
    void sources_doesNotEscapeViaEmbeddedAbsolutePath();
    void sources_preservesDuplicateSourceOrder();
    void sources_trimsWhitespaceAroundKeyAndValue();

    void sections_areCaseInsensitive();
    void sections_unknownIsIgnored();

    void general_allowsEmptyValue();

    void mode_defaultsToCopy();
    void mode_acceptsSync();
    void mode_rejectsInvalidValue();

    void interval_defaultsToFiveMinutes();
    void interval_negativeFallsBackToDefault();
    void interval_zeroIsPreserved();

    void transfers_defaultsToOne();
    void transfers_zeroFallsBackToDefault();
    void transfers_negativeFallsBackToDefault();

    void extraArgs_emptyByDefault();
    void extraArgs_splitsOnSpaces();

    void logEnabled_defaultsToFalse();
    void logEnabled_acceptsTruthyValues();
    void logEnabled_acceptsFalsyValues();
    void logEnabled_unrecognizedValueFallsBackToDefault();

private:
    static UserConfig LoadConfig(const QString& contents);
};
