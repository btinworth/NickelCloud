QT += core testlib
QT -= gui

CONFIG += testcase console c++11
CONFIG -= app_bundle

TEMPLATE = app
TARGET = tests

INCLUDEPATH += .. ../NickelHook

QMAKE_CXXFLAGS += -Wno-deprecated-declarations

SOURCES += \
    main.cc \
    ConfigTest.cc \
    TestStubs.cc \
    ../NickelCloudConfig.cc

HEADERS += \
    ConfigTest.h \
    ../NickelCloudConfig.h
