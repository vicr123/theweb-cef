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
        void OnContextInitialized() override;

    private:
        IMPLEMENT_REFCOUNTING(App)
};

#endif // APP_H
