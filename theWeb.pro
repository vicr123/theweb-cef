#-------------------------------------------------
#
# Project created by QtCreator 2016-07-01T18:32:54
#
#-------------------------------------------------

QT       += core gui network x11extras dbus
CONFIG   += c++11
LIBS     += libcef.so libcef_dll_wrapper.a -lX11
INCLUDEPATH += "$$PWD/cef"
QMAKE_LFLAGS += -Wl,-R -Wl,$$OUT_PWD
QMAKE_CXXFLAGS += -Wno-unused-parameter

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

mainDBus.files = org.thesuite.theweb.xml
DBUS_ADAPTORS += mainDBus

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
    thewebsettingsaccessor.cpp \
    clickableframe.cpp \
    downloadimagecallback.cpp \
    oblivionrequestcontext.cpp \
    maindbus.cpp \
    animatedstackedwidget.cpp \
    nativeeventfilter.cpp \
    hovertabbar.cpp \
    downloadframe.cpp \
    filepicker.cpp

HEADERS  += \
    cefheaders.h \
    mainwindow.h \
    cefengine.h \
    cefhandler.h \
    signalbroker.h \
    spacesearchbox.h \
    completioncallback.h \
    thewebschemes.h \
    thewebsettingsaccessor.h \
    clickableframe.h \
    downloadimagecallback.h \
    oblivionrequestcontext.h \
    animatedstackedwidget.h \
    nativeeventfilter.h \
    hovertabbar.h \
    downloadframe.h \
    filepicker.h

FORMS    += \
    mainwindow.ui

RESOURCES += \
    resources.qrc

DISTFILES += \
    theweb-execscript
