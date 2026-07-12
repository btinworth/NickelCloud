QT += core testlib
QT -= gui

CONFIG += testcase console c++11
CONFIG -= app_bundle

TEMPLATE = app
TARGET = tests

INCLUDEPATH += .. ../src ../NickelHook

QMAKE_CXXFLAGS += -Wno-deprecated-declarations

SOURCES += \
    main.cc \
    ConfigTest.cc \
    TestStubs.cc \
    ../src/NickelCloudConfig.cc \
    ../src/Constants.cc

HEADERS += \
    ConfigTest.h \
    ../src/NickelCloudConfig.h
