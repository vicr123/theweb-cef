#ifndef APP_H
#define APP_H

#include <include/cef_app.h>

class App : public CefApp, public CefBrowserProcessHandler
{
    public:
        App();

        // CefApp methods:
        virtual CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override;

        // CefBrowserProcessHandler methods:
        virtual void OnContextInitialized() OVERRIDE;

    private:
        IMPLEMENT_REFCOUNTING(App)
};

#endif // APP_H
