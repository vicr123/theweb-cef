#include "mainwindow.h"
#include <QApplication>
#include <QTimer>
#include "cef/app.h"
#include <include/cef_app.h>
#include <QFontDatabase>
#include "cef/client.h"
#include <QDir>
#include <QTranslator>
#include <QLibraryInfo>
#include <QDesktopWidget>

#ifdef Q_OS_MAC
    #include <CoreFoundation/CFBundle.h>
#endif

Client* cefClient;
CefBrowserSettings::struct_type browserSettings;
QTimer CefEventLoopTimer;

int main(int argc, char *argv[])
{
    CefMainArgs main_args(argc, argv);

    if (CefExecuteProcess(main_args, NULL, NULL) >= 0) {
        //CEF handles this process
        //Return here
        return 0;
    }

    CefSettings cSettings;
    CefString(&cSettings.cache_path) = QDir::homePath().append("/.theweb/cache").toStdString();
    CefString(&cSettings.product_version) = "theWeb/16.00 Chrome/62";

    CefRefPtr<App> app(new App);
    CefInitialize(main_args, cSettings, app.get(), NULL);

    QApplication a(argc, argv);

    a.setOrganizationName("theSuite");
    a.setOrganizationDomain("");
    a.setApplicationName("theWeb");

    a.setQuitOnLastWindowClosed(true);

    QTranslator qtTranslator;
    qtTranslator.load("qt_" + QLocale::system().name(),
            QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    a.installTranslator(&qtTranslator);

    QTranslator localTranslator;

#ifdef Q_OS_MAC
    a.setAttribute(Qt::AA_DontShowIconsInMenus, true);

    QIcon::setThemeName("contemporary");

    CFURLRef appUrlRef = CFBundleCopyBundleURL(CFBundleGetMainBundle());
    CFStringRef macPath = CFURLCopyFileSystemPath(appUrlRef,
                                           kCFURLPOSIXPathStyle);
    const char *pathPtr = CFStringGetCStringPtr(macPath,
                                           CFStringGetSystemEncoding());

    localTranslator.load(QLocale::system().name(), QString::fromLocal8Bit(pathPtr) + "/Contents/translations/");

    CFRelease(appUrlRef);
    CFRelease(macPath);
#elif defined(Q_OS_LINUX)
    localTranslator.load(QLocale::system().name(), "/usr/share/theweb/translations");
#endif

    a.installTranslator(&localTranslator);

    cefClient = new Client();

    /*MainWindow w;
    w.show();*/

    CefWindowInfo windowInfo;
    windowInfo.SetAsWindowless(NULL);
    browserSettings.background_color = 0xFFFFFFFF;
    browserSettings.windowless_frame_rate = 60;
    CefString(&browserSettings.sans_serif_font_family) = QFontDatabase::systemFont(QFontDatabase::GeneralFont).family().toStdString();
    CefString(&browserSettings.fixed_font_family) = QFontDatabase::systemFont(QFontDatabase::FixedFont).family().toStdString();
    CefBrowserHost::CreateBrowser(windowInfo, cefClient, "http://www.google.com/", browserSettings, CefRefPtr<CefRequestContext>());

    CefEventLoopTimer.setInterval(10);
    QObject::connect(&CefEventLoopTimer, &QTimer::timeout, [=] {
        CefDoMessageLoopWork();
    });
    CefEventLoopTimer.start();

    int exec = a.exec();

    CefCookieManager::GetGlobalManager(nullptr).get()->FlushStore(nullptr);
    CefShutdown();

    return exec;
}

void QuitApp(int exitCode = 0) {
    CefEventLoopTimer.stop();
    QApplication::exit(exitCode);
}

float getDPIScaling() {
    float currentDPI = QApplication::desktop()->logicalDpiX();
    return currentDPI / (float) 96;
}
