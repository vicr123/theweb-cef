#-------------------------------------------------
#
# Project created by QtCreator 2016-07-01T18:32:54
#
#-------------------------------------------------

QT       += core gui network
CONFIG   += c++11
LIBS     += libcef.so libcef_dll_wrapper.a
INCLUDEPATH += "$$PWD/cef"
QMAKE_LFLAGS += -Wl,-R -Wl,$$OUT_PWD
QMAKE_CXXFLAGS += -Wno-unused-parameter

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = theweb
TEMPLATE = app

SOURCES += main.cpp\
    mainwindow.cpp \
    cefengine.cpp \
    cefhandler.cpp \
    signalbroker.cpp \
    spacesearchbox.cpp \
    completioncallback.cpp \
    thewebschemes.cpp \
    thewebsettingsaccessor.cpp

HEADERS  += \
    cefheaders.h \
    mainwindow.h \
    cefengine.h \
    cefhandler.h \
    signalbroker.h \
    spacesearchbox.h \
    completioncallback.h \
    thewebschemes.h \
    thewebsettingsaccessor.h

FORMS    += \
    mainwindow.ui

RESOURCES += \
    resources.qrc
