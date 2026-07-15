QT += core testlib
QT -= gui

CONFIG += testcase console c++11
CONFIG -= app_bundle

TEMPLATE = app
TARGET = tests

INCLUDEPATH += .. ../src ../NickelHook

QMAKE_CXXFLAGS += -Wno-deprecated-declarations

SOURCES += $$files(*.cc) $$files(../src/*.cc)
SOURCES -= ../src/Main.cc

HEADERS += $$files(*.h) $$files(../src/*.h)
