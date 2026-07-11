#pragma once

#include <QObject>

class UtilsTest : public QObject
{
    Q_OBJECT

private slots:
    void resolvesSimpleRelativePath();
    void resolvesNestedRelativePath();
    void allowsRootItself();
    void collapsesInternalTraversalThatStaysWithinRoot();
    void rejectsParentTraversal();
    void rejectsExcessiveParentTraversal();
    void doesNotEscapeViaEmbeddedAbsolutePath();
};
