#ifndef CEFENGINE_H
#define CEFENGINE_H

#include <QObject>
#include <QDebug>
#include <QSettings>
#include <QMessageBox>
#include <QException>
#include "cefheaders.h"
#include <QProcess>
#include <QVector>
#include <functional>
#include <QPrintDialog>
#include <QPrinter>
#include <QPainter>
#include <poppler-qt5.h>

#include <X11/Xlib.h>
#include <QX11Info>
#include <X11/keysym.h>
#include <X11/XF86keysym.h>
#undef Unsorted
#undef None
#undef Expose
#undef FocusIn
#undef FocusOut
#undef KeyPress
#undef KeyRelease
#undef FontChange
#undef GrayScale

#include "v8interceptor.h"

class CefEngine : public CefApp, public CefRefCount, public CefBrowserProcessHandler, public CefRenderProcessHandler, public CefPrintHandler
{
public:
    explicit CefEngine();

    void AddRef() const override;
    bool Release() const override;
    bool HasOneRef() const override;

    virtual CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override {
        return this;
    }
    virtual CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() override {
        return this;
    }
    virtual CefRefPtr<CefPrintHandler> GetPrintHandler() override {
        return this;
    }

    void OnContextInitialized() override;
    void OnContextCreated(Browser browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context) override;
    void OnContextReleased(Browser browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context) override;
    bool OnPrintDialog(bool has_selection, CefRefPtr<CefPrintDialogCallback> callback) override;
    void OnPrintStart(Browser browser) override;
    void OnPrintSettings(CefRefPtr<CefPrintSettings> settings, bool get_defaults) override;
    bool OnPrintJob(const CefString &document_name, const CefString &pdf_file_path, CefRefPtr<CefPrintJobCallback> callback) override;
    void OnPrintReset() override;

    bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId source_process, CefRefPtr<CefProcessMessage> message) override;

    bool videoPlaying;
    CefRefPtr<CefFrame> videoFrame;
    static CefRefPtr<CefPrintSettings> getCefPrinterSettings(QPrinter* printer, CefRefPtr<CefPrintSettings> currentSettings = CefPrintSettings::Create());
private:
    QSettings settings;

    CefRefPtr<CefV8Value> videoElement;
    CefRefPtr<CefV8Context> videoContext;
    QString mprisElementTagType;

    CefRefPtr<CefV8Value> notificationRequestPromise = NULL;
    CefRefPtr<CefV8Value> notificationRequestResolver = NULL;
    CefRefPtr<CefFrame> notificationRequestFrame = NULL;
    CefRefPtr<CefPrintSettings> printSettings;
    QPrinter* printer = NULL;

    Browser printingBrowser = NULL;
};

#include "thewebschemes.h"
#include "thewebsettingsaccessor.h"

#ifndef CEFHANDLER_H
#include "signalbroker.h"
#endif //CEFHANDLER_H

#endif // CEFENGINE_H
