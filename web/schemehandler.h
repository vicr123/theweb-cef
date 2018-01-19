#ifndef SCHEMEHANDLER_H
#define SCHEMEHANDLER_H

#include "cef/app.h"
#include <QUrl>
#include <QFile>
#include <QMimeDatabase>
#include <QIcon>
#include <QBuffer>

class theWebSchemeResourceHandler : public CefResourceHandler
{
    enum pageServed {
        None,
        Invalid,
        Crash,
        Kill,
        Icon
    };

public:
    explicit theWebSchemeResourceHandler();
    bool ProcessRequest(CefRefPtr<CefRequest> request, CefRefPtr<CefCallback> callback) override;
    void GetResponseHeaders(CefRefPtr<CefResponse> response, int64& response_length, CefString& redirectUrl) override;
    void Cancel() override;
    bool ReadResponse(void* data_out, int bytes_to_read, int& bytes_read, CefRefPtr<CefCallback> callback) override;

private:
    pageServed servingPage = None;
    QString resourcePath = "";
    QIODevice* resourceFile;
    QMimeDatabase mimeDb;

    IMPLEMENT_REFCOUNTING(theWebSchemeResourceHandler)
};

class theWebSchemeHandler : public CefSchemeHandlerFactory
{
    public:
        CefRefPtr<CefResourceHandler> Create(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& scheme_name, CefRefPtr<CefRequest> request) override {
            return new theWebSchemeResourceHandler();
        }

    private:
        IMPLEMENT_REFCOUNTING(theWebSchemeHandler)
};

#endif // SCHEMEHANDLER_H
