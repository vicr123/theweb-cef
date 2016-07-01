#include "cefheaders.h"
#include "mainwindow.h"
#include "cefengine.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //Initialize CEF
    CefSettings settings;
    CefRefPtr<CefEngine> app(new CefEngine);
    CefMainArgs cefArgs;

    CefInitialize(cefArgs, settings, app, NULL);

    MainWindow w;
    w.show();

    int ret = a.exec();

    CefShutdown();
    return ret;
}
