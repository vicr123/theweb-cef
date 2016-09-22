#ifndef OBLIVIONREQUESTCONTEXT_H
#define OBLIVIONREQUESTCONTEXT_H

#include "cefheaders.h"

class OblivionRequestContextHandler : public CefRequestContextHandler, public CefRefCount
{
public:
    OblivionRequestContextHandler();

    void AddRef() const override;
    bool Release() const override;
    bool HasOneRef() const override;

    CefRefPtr<CefCookieManager> GetCookieManager() override {
        return cookieManager;
    }

    CefCookieManager* cookieManager;
};

#endif // OBLIVIONREQUESTCONTEXT_H
