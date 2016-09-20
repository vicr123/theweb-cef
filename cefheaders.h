#ifndef CEFHEADERS_H
#define CEFHEADERS_H

#include <cef/include/cef_app.h>
#include <cef/include/cef_base.h>
#include <cef/include/cef_client.h>
#include <cef/include/cef_browser.h>

#include <QObject>

typedef CefRefPtr<CefBrowser> Browser;

Q_DECLARE_METATYPE(Browser)
Q_DECLARE_METATYPE(CefRefPtr<CefFrame>)
Q_DECLARE_METATYPE(CefString)
Q_DECLARE_METATYPE(CefRefPtr<CefAuthCallback>)
Q_DECLARE_METATYPE(CefLifeSpanHandler::WindowOpenDisposition)
Q_DECLARE_METATYPE(CefPopupFeatures)
Q_DECLARE_METATYPE(CefWindowInfo)
Q_DECLARE_METATYPE(CefBrowserSettings)
Q_DECLARE_METATYPE(CefRefPtr<CefJSDialogCallback>)
Q_DECLARE_METATYPE(CefRefPtr<CefRequestCallback>)

#endif // CEFHEADERS_H

