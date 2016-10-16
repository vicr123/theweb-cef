#include "cefheaders.h"
#include "mainwindow.h"
#include "cefengine.h"
#include "cefhandler.h"
#include "signalbroker.h"
#include "completioncallback.h"
#include <QApplication>
#include <QTimer>
#include <QDir>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QFontDatabase>
#include <QNetworkProxyFactory>
#include <nativeeventfilter.h>
#include <unistd.h>
#include <sys/types.h>

CefHandler* handler;
SignalBroker* signalBroker;
QTimer cefEventLoopTimer;
QVariantMap settingsData;
QVariantMap notificationsData;
QStringList certErrorUrls;
QTimer batteryCheckTimer;
QFile historyFile(QDir::homePath() + "/.theweb/history"); //File used for reading/writing
CefString historyData; //Used in settings accessor
CefBrowserSettings defaultBrowserSettings;

int main(int argc, char *argv[])
{
    //Create space for extra arguments (if need be)
    if (geteuid() == 0) {
        char **newArgv = (char**) malloc((argc + 1) * sizeof(*newArgv));
        memcpy(newArgv, argv, sizeof(*newArgv) * argc);
        newArgv[argc] = "--no-sandbox";
        argc++;
        argv = newArgv;
    }


    QApplication a(argc, argv);

    //Set Application Information
    a.setOrganizationName("theSuite");
    a.setOrganizationDomain("");
    a.setApplicationName("theWeb");

    QSettings appSettings;

    NativeEventFilter* nativeFilter = new NativeEventFilter;
    a.installNativeEventFilter(nativeFilter);

    qRegisterMetaType<Browser>();
    qRegisterMetaType<CefRefPtr<CefFrame>>();
    qRegisterMetaType<CefString>();
    qRegisterMetaType<CefRefPtr<CefAuthCallback>>();
    qRegisterMetaType<CefLifeSpanHandler::WindowOpenDisposition>();
    qRegisterMetaType<CefPopupFeatures>();
    qRegisterMetaType<CefWindowInfo>();
    qRegisterMetaType<CefBrowserSettings>();

    //Use the system proxy settings in Qt
    QNetworkProxyFactory::setUseSystemConfiguration(true);

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

    CefString(&settings.user_agent) = "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) theWeb/15.00 Chrome/53.0.2785.101 Safari/537.36";
    /*QString userAgent = QString::fromStdString(CefString(&settings.user_agent).ToString());
    userAgent.insert(userAgent.indexOf("Chrome/"), "theWeb/15.00 ");
    CefString(&settings.user_agent) = userAgent.toStdString();*/
    CefString(&settings.cache_path) = QDir::homePath().append("/.theweb/cache").toStdString();
    CefString(&settings.locale) = "en-US";
    //settings.remote_debugging_port = 26154;
    //settings.single_process = true;

    CefRefPtr<CefEngine> app(new CefEngine);
    CefMainArgs cefArgs(argc, argv);

    //Initialize CEF
    CefInitialize(cefArgs, settings, app.get(), NULL);

    //Set default browser settings
    CefString(&defaultBrowserSettings.sans_serif_font_family) = appSettings.value("fonts/sansSerif", QFontDatabase::systemFont(QFontDatabase::GeneralFont).family()).toString().toStdString();

    //Create a handler
    handler = new CefHandler();
    //Connect required signals
    QObject::connect(nativeFilter, SIGNAL(PlayPause()), handler, SLOT(PlayPause()));
    QObject::connect(nativeFilter, SIGNAL(Previous()), handler, SLOT(Previous()));

    //Read command line arguments
    QStringList urlsToOpen;
    for (QString arg : a.arguments().first().split(" ")) {
        if (arg != a.applicationFilePath() && arg != "" && !arg.startsWith("-")) {
            urlsToOpen.append(arg);
        }
    }

    //Check if theWeb is already running. We do this after CEF initializes because CEF can block things.
    bool isRunning;
    isRunning = QDBusConnection::sessionBus().interface()->registeredServiceNames().value().contains("org.thesuite.theweb");
    if (isRunning) {
        //theWeb is already running. Check the PID
        QDBusInterface interface("org.thesuite.theweb", "/org/thesuite/theweb", "org.thesuite.theweb");
        if (interface.property("processID") != a.applicationPid()) {
            //PID is different. Open a new window in parent process and exit this one.
            qDebug() << "theWeb is already running. Opening new window in existing browser process (via DBus).";

            //Send message via DBus to other theWeb process
            QDBusMessage message = QDBusMessage::createMethodCall("org.thesuite.theweb", "/org/thesuite/theweb", "org.thesuite.theweb", "newWindow");
            if (urlsToOpen.count() > 0) {
                QVariantList arguments;
                arguments.append(urlsToOpen.first());
                message.setArguments(arguments);
            }
            QDBusConnection::sessionBus().call(message, QDBus::NoBlock);

            //Exit theWeb
            CefShutdown();
            return 0;
        }
    } else {
        //We'll get here only if theWeb is not running.
        new MainDBus(); //Initialize DBus service
    }

    //Initialize Windows
    historyFile.open(QFile::ReadWrite | QFile::Append);

    bool windowOpened = false;
    for (QString url : urlsToOpen) {

        windowOpened = true;

        CefWindowInfo windowInfo;
        CefBrowserSettings settings;
        MainWindow* w = new MainWindow(CefBrowserHost::CreateBrowserSync(windowInfo, CefRefPtr<CefHandler>(handler), url.toStdString(), settings, CefRefPtr<CefRequestContext>()));
        w->show();
    }

    if (!windowOpened) {
        MainWindow* w = new MainWindow();
        w->show();
    }

    cefEventLoopTimer.setInterval(0);
    QObject::connect(&cefEventLoopTimer, &QTimer::timeout, [=]() {
        CefDoMessageLoopWork();

    });
    cefEventLoopTimer.start();

    batteryCheckTimer.setInterval(1000);
    QObject::connect(&batteryCheckTimer, &QTimer::timeout, [=]() {
        QDBusInterface upower("org.freedesktop.UPower", "/org/freedesktop/UPower", "org.freedesktop.UPower", QDBusConnection::systemBus());
        if (upower.isValid()) {
            if (upower.property("OnBattery").toBool()) {
                if (cefEventLoopTimer.interval() == 0) {
                    cefEventLoopTimer.setInterval(1000 / 60);
                    emit signalBroker->ReloadSettings();
                }
            } else {
                if (cefEventLoopTimer.interval() == (1000 / 60)) {
                    cefEventLoopTimer.setInterval(0);
                    emit signalBroker->ReloadSettings();
                }
            }
        }
    });
    batteryCheckTimer.start();

    int ret = a.exec();

    CefCookieManager::GetGlobalManager(NULL).get()->FlushStore(NULL);
    CefShutdown();
    appSettings.remove("browser/lastPages");
    return ret;
}

void QuitApp(int exitCode = 0) {
    cefEventLoopTimer.stop();
    QApplication::exit(exitCode);
}

QString calculateSize(quint64 size) {
    QString ret;
    if (size > 1073741824) {
        ret = QString::number(((float) size / 1024 / 1024 / 1024), 'f', 2).append(" GiB");
    } else if (size > 1048576) {
        ret = QString::number(((float) size / 1024 / 1024), 'f', 2).append(" MiB");
    } else if (size > 1024) {
        ret = QString::number(((float) size / 1024), 'f', 2).append(" KiB");
    } else {
        ret = QString::number((float) size, 'f', 2).append(" B");
    }

    return ret;
}
