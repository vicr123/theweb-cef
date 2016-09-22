#include "maindbus.h"
#include "theweb_adaptor.h"

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

uint MainDBus::processID() {
    return QApplication::applicationPid();
}
