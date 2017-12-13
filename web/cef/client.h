#ifndef CLIENT_H
#define CLIENT_H

#include <include/cef_client.h>
#include <QList>
#include <QApplication>
#include "browsertab.h"

class BrowserTab;
class MainWindow;

class Client : public CefClient,
        public CefDisplayHandler,
        public CefLifeSpanHandler,
        public CefRequestHandler,
        public CefLoadHandler,
        public CefRenderHandler,
        public CefJSDialogHandler
{
    public:
        Client();
        void registerTab(BrowserTab* tab);

        CefRefPtr<CefDisplayHandler> GetDisplayHandler() override {
            return this;
        }
        CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override {
            return this;
        }
        CefRefPtr<CefRequestHandler> GetRequestHandler() override {
            return this;
        }
        CefRefPtr<CefLoadHandler> GetLoadHandler() override {
            return this;
        }
        CefRefPtr<CefRenderHandler> GetRenderHandler() override {
            return this;
        }
        CefRefPtr<CefJSDialogHandler> GetJSDialogHandler() override {
            return this;
        }

        void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;
        void OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title) override;
        void OnLoadingStateChange(CefRefPtr<CefBrowser> browser, bool isLoading, bool canGoBack, bool canGoForward) override;
        void OnAddressChange(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& url) override;
        bool GetViewRect(CefRefPtr<CefBrowser> browser, CefRect &rect) override;
        void OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList& dirtyRects, const void* buffer, int width, int height) override;
        void OnCursorChange(CefRefPtr<CefBrowser> browser, CefCursorHandle cursor, CursorType type, const CefCursorInfo& custom_cursor_info) override;
        void OnFullscreenModeChange(CefRefPtr<CefBrowser> browser, bool isFullScreen) override;
        bool OnJSDialog(CefRefPtr<CefBrowser> browser, const CefString& origin_url, JSDialogType dialog_type, const CefString& message_text, const CefString& default_prompt_text, CefRefPtr<CefJSDialogCallback> callback, bool& suppress_message) override;
        void OnResetDialogState(CefRefPtr<CefBrowser> browser) override;
        bool OnBeforePopup(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString &target_url, const CefString &target_frame_name, CefLifeSpanHandler::WindowOpenDisposition target_disposition, bool user_gesture, const CefPopupFeatures &popupFeatures, CefWindowInfo &windowInfo, CefRefPtr<CefClient> &client, CefBrowserSettings &settings, bool *no_javascript_access) override;
        void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
        void OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, ErrorCode errorCode, const CefString& errorText, const CefString& failedUrl) override;
        bool OnBeforeUnloadDialog(CefRefPtr<CefBrowser> browser, const CefString &message_text, bool is_reload, CefRefPtr<CefJSDialogCallback> callback);

        void setNewTabWindow(MainWindow* newTabWindow);

    private:
        IMPLEMENT_REFCOUNTING(Client)
        QList<BrowserTab*> tabs;

        BrowserTab* getTab(CefRefPtr<CefBrowser> browser);
        MainWindow* newTabWindow = nullptr;
        uint numberOfBrowsers = 0;
};

#endif // CLIENT_H
