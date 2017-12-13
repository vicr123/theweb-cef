# This project only needs to be compiled on macOS

TEMPLATE = app
CONFIG += console c++11
CONFIG -= qt

SOURCES += main.cpp

INCLUDEPATH += ../libcef/
LIBS += -F$$PWD/../libcef/Release/ -framework "Chromium Embedded Framework" $$PWD/../libcef/libcef_dll_wrapper/libcef_dll_wrapper.a
TARGET = "theWeb Helper"
QMAKE_RPATHDIR += @executable_path/../../../../
