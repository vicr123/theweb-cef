#include "cefheaders.h"
#include "mainwindow.h"
#include "cefengine.h"
#include "cefhandler.h"
#include "signalbroker.h"
#include "completioncallback.h"
#include <QApplication>
#include <QTimer>
#include <QDir>

CefHandler* handler;
SignalBroker* signalBroker;
QTimer cefEventLoopTimer;
QVariantMap settingsData;
QStringList certErrorUrls;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setOrganizationName("theSuite");
    a.setOrganizationDomain("");
    a.setApplicationName("theWeb");

    qRegisterMetaType<Browser>();
    qRegisterMetaType<CefRefPtr<CefFrame>>();
    qRegisterMetaType<CefString>();
    qRegisterMetaType<CefRefPtr<CefAuthCallback>>();
    qRegisterMetaType<CefLifeSpanHandler::WindowOpenDisposition>();
    qRegisterMetaType<CefPopupFeatures>();
    qRegisterMetaType<CefWindowInfo>();
    qRegisterMetaType<CefBrowserSettings>();

    signalBroker = new SignalBroker();

    //Initialize CEF
    CefSettings settings;
    //cef_string_t newUserAgent;
    //newUserAgent.str = (char16*) QString::fromLocal8Bit((char*) settings.user_agent.str, settings.user_agent.length).append(" theWeb/15.00").toStdString().data();
    //settings.user_agent = newUserAgent;
    /*QString userAgent = "Mozilla/5.0";
    cef_string_t* userAgentString = new cef_string_t();
    cef_string_utf16_set((const char16*) userAgent.toStdString().data(), userAgent.length(), userAgentString, 1);
    settings.user_agent = *userAgentString;*/

    CefString(&settings.user_agent) = "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) theWeb/15.00 Chrome/51.0.2704.103 Safari/537.36";
    CefString(&settings.cache_path) = QDir::homePath().toStdString().append("/.theweb/cache").data();
    settings.remote_debugging_port = 26154;
    //settings.single_process = true;

    CefRefPtr<CefEngine> app(new CefEngine);
    CefMainArgs cefArgs(argc, argv);

    CefInitialize(cefArgs, settings, app.get(), NULL);

    handler = new CefHandler();

    //Initialize Qt
    MainWindow w;
    w.show();

    cefEventLoopTimer.setInterval(0);
    QObject::connect(&cefEventLoopTimer, &QTimer::timeout, [=]() {
        CefDoMessageLoopWork();
    });
    cefEventLoopTimer.start();

    int ret = a.exec();

    CefShutdown();
    return ret;
}

void QuitApp(int exitCode = 0) {
    cefEventLoopTimer.stop();
    QApplication::exit(exitCode);
}
