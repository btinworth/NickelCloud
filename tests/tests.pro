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
    config_test.cc \
    nh_log_stub.cc \
    ../config.cc

HEADERS += \
    config_test.h \
    ../config.h
