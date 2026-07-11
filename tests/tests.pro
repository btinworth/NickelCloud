QT += core testlib
QT -= gui

CONFIG += testcase console c++11
CONFIG -= app_bundle

TEMPLATE = app
TARGET = config_test

INCLUDEPATH += .. ../NickelHook

QMAKE_CXXFLAGS += -Wno-deprecated-declarations

SOURCES += \
    config_test.cc \
    nh_log_stub.cc \
    ../config.cc

HEADERS += \
    ../config.h
