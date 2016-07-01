#-------------------------------------------------
#
# Project created by QtCreator 2016-07-01T18:32:54
#
#-------------------------------------------------

QT       += core gui
CONFIG   += c++11
LIBS     += -shared -o libcef.so
INCLUDEPATH += "$$PWD/cef"

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = theWeb
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    cefengine.cpp

HEADERS  += mainwindow.h \
    cefheaders.h \
    cefengine.h

FORMS    += mainwindow.ui

RESOURCES += \
    resources.qrc
