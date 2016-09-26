#ifndef CEFENGINE_H
#define CEFENGINE_H

#include <QObject>
#include <QDebug>
#include <QSettings>
#include <QMessageBox>
#include <X11/Xlib.h>
#include <QX11Info>
#include <X11/keysym.h>
#include <X11/XF86keysym.h>
#include "cefheaders.h"
#include <QProcess>

#undef Unsorted
#undef None

class CefEngine : public CefApp, public CefRefCount, public CefBrowserProcessHandler, public CefRenderProcessHandler
{
public:
    explicit CefEngine();

    void AddRef() const;
    bool Release() const;
    bool HasOneRef() const;

    virtual CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override {
        return this;
    }
    virtual CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() override {
        return this;
    }

    void OnContextInitialized() override;
    void OnContextCreated(Browser browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context) override;
    void OnContextReleased(Browser browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context) override;
    bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId source_process, CefRefPtr<CefProcessMessage> message) override;

private:
    QSettings settings;

    CefRefPtr<CefV8Value> videoElement;
};

#include "thewebschemes.h"
#include "thewebsettingsaccessor.h"

#endif // CEFENGINE_H
