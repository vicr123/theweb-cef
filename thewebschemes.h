#ifndef THEWEBSCHEMES_H
#define THEWEBSCHEMES_H

#include "cefheaders.h"
#include "cefengine.h"
#include <QUrl>
#include <QFile>
#include <QMimeDatabase>

class theWebSchemeResourceHandler : public CefResourceHandler, public CefEngine
{
    enum pageServed {
        None,
        Invalid,
        About,
        Settings
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
    QFile resourceFile;
    QMimeDatabase mimeDb;
};

class theWebSchemeHandler : public CefSchemeHandlerFactory, public CefEngine
{
    public:
     CefRefPtr<CefResourceHandler> Create(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& scheme_name, CefRefPtr<CefRequest> request) override {
       return new theWebSchemeResourceHandler();
     }
};


#endif // THEWEBSCHEMES_H
