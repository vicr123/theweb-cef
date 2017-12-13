#-------------------------------------------------
#
# Project created by QtCreator 2017-12-12T17:09:52
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = theweb
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    browsertab.cpp \
    cef/app.cpp \
    cef/client.cpp \
    renderer.cpp \
    browserdialog.cpp \
    bar.cpp \
    tabbutton.cpp

HEADERS += \
        mainwindow.h \
    browsertab.h \
    cef/app.h \
    cef/client.h \
    renderer.h \
    browserdialog.h \
    bar.h \
    tabbutton.h

FORMS += \
        mainwindow.ui \
    browserdialog.ui

unix:!macx: {
    QT += thelib x11extras
    INCLUDEPATH += /opt/cef/
    LIBS += /opt/cef/Release/libcef.so /opt/cef/libcef_dll_wrapper/libcef_dll_wrapper.a -lX11
}

RESOURCES += \
    pages.qrc
