#ifndef MAINDBUS_H
#define MAINDBUS_H

#include <QObject>
#include <QDBusConnection>
#include "mainwindow.h"

class MainDBus : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.thesuite.theweb")

    Q_PROPERTY(uint processID READ processID)
public:
    explicit MainDBus(QObject *parent = 0);

    uint processID();
signals:

public slots:
    Q_SCRIPTABLE void newWindow();
};

#endif // MAINDBUS_H
