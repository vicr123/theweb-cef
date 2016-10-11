#include "cefhandler.h"
#include "theweb_adaptor.h"

extern CefHandler* handler;

MainDBus::MainDBus(QObject *parent) : QObject(parent)
{
    new ThewebAdaptor(this);
    QDBusConnection::sessionBus().registerService("org.thesuite.theweb");
    QDBusConnection::sessionBus().registerObject("/org/thesuite/theweb", this);
}

void MainDBus::newWindow() {
    MainWindow* window = new MainWindow();
    window->show();
}

void MainDBus::newWindow(QString url) {
    Browser browser = CefBrowserHost::CreateBrowserSync(CefWindowInfo(), handler, url.toStdString(), CefBrowserSettings(), CefRefPtr<CefRequestContext>());
    MainWindow* window = new MainWindow(browser);
    window->show();
}

uint MainDBus::processID() {
    return QApplication::applicationPid();
}
