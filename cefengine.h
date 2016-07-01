#ifndef CEFENGINE_H
#define CEFENGINE_H

#include <QObject>
#include "cefheaders.h"

class CefEngine : public QObject, public CefApp, public CefBrowserProcessHandler
{
    Q_OBJECT
public:
    explicit CefEngine(QObject *parent = 0);

    void AddRef() const;
    bool Release() const;
    bool HasOneRef() const;

signals:

public slots:
};

#endif // CEFENGINE_H
