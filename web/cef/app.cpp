#include "app.h"
#include "schemehandler.h"

App::App()
{

}

CefRefPtr<CefBrowserProcessHandler> App::GetBrowserProcessHandler() {
    return this;
}

void App::OnContextInitialized() {
    CefRegisterSchemeHandlerFactory("theweb", "theweb", new theWebSchemeHandler());
}
