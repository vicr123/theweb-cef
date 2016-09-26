#ifndef CEFHANDLER_H
#define CEFHANDLER_H

#include <QObject>
#include <QList>
#include <QVector>
#include "cefheaders.h"
#include <QDebug>
#include <QApplication>
#include "cefengine.h"
#include <X11/Xlib.h>
#include <QX11Info>
#include <X11/keysym.h>
#include <QFileDialog>
#include <QClipboard>
#include <QMenu>

#undef Bool
#undef None

class SignalBroker;
class MainWindow;

class CefHandler : public QObject,
        public CefClient,
        public CefDisplayHandler,
        public CefLifeSpanHandler,
        public CefLoadHandler,
        public CefRequestHandler,
        public CefJSDialogHandler,
        public CefFocusHandler,
        public CefKeyboardHandler,
        public CefDownloadHandler,
        public CefDialogHandler,
        public CefContextMenuHandler,
        public CefEngine
{
    Q_OBJECT
public:
    explicit CefHandler(QObject* parent = 0);

    enum MenuItemIds {
        LinkSubmenu = MENU_ID_USER_FIRST, CopyLink, OpenLinkInNewTab, OpenLinkInNewWindow, OpenLinkInNewOblivion,
        MisspelledWordSubmenu,
        Generic
    };

    virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override {
        return this;
    }
    virtual CefRefPtr<CefLoadHandler> GetLoadHandler() override {
        return this;
    }
    virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler() override {
        return this;
    }
    virtual CefRefPtr<CefJSDialogHandler> GetJSDialogHandler() override {
        return this;
    }
    virtual CefRefPtr<CefFocusHandler> GetFocusHandler() override {
        return this;
    }
    virtual CefRefPtr<CefRequestHandler> GetRequestHandler() override {
        return this;
    }
    virtual CefRefPtr<CefKeyboardHandler> GetKeyboardHandler() override {
        return this;
    }
    virtual CefRefPtr<CefDownloadHandler> GetDownloadHandler() override {
        return this;
    }
    virtual CefRefPtr<CefDialogHandler> GetDialogHandler() override {
        return this;
    }
    virtual CefRefPtr<CefContextMenuHandler> GetContextMenuHandler() override {
        return this;
    }

    void OnAfterCreated(Browser browser) override;
    void OnRenderProcessTerminated(Browser browser, CefRequestHandler::TerminationStatus status) override;
    void OnTitleChange(Browser browser, const CefString& title) override;
    void OnAddressChange(Browser browser, CefRefPtr<CefFrame> frame, const CefString& url) override;
    void OnFullscreenModeChange(Browser browser, bool fullscreen) override;
    void OnLoadingStateChange(Browser browser, bool isLoading, bool canGoBack, bool canGoForward) override;
    void OnLoadError(Browser browser, CefRefPtr<CefFrame> frame, ErrorCode errorCode, const CefString& errorText, const CefString& failedUrl) override;
    void OnBeforeClose(Browser browser) override;
    void OnGotFocus(Browser browser) override;
    void OnFaviconURLChange(Browser browser, const std::vector<CefString> &urls) override;
    void OnBeforeDownload(Browser browser, CefRefPtr<CefDownloadItem> download_item, const CefString& suggested_name, CefRefPtr<CefBeforeDownloadCallback> callback) override;
    void OnBeforeContextMenu(Browser browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefContextMenuParams> params, CefRefPtr<CefMenuModel> model) override;
    void OnProtocolExecution(Browser browser, const CefString& url, bool& allow_os_execution);

    ReturnValue OnBeforeResourceLoad(Browser browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, CefRefPtr<CefRequestCallback> callback) override;

    bool OnBeforeUnloadDialog(Browser browser, const CefString &message_text, bool is_reload, CefRefPtr<CefJSDialogCallback> callback) override;
    bool GetAuthCredentials(Browser browser, CefRefPtr<CefFrame> frame, bool isProxy, const CefString &host, int port, const CefString &realm, const CefString &scheme, CefRefPtr<CefAuthCallback> callback) override;
    bool OnBeforePopup(Browser browser, CefRefPtr<CefFrame> frame, const CefString &target_url, const CefString &target_frame_name, CefLifeSpanHandler::WindowOpenDisposition target_disposition, bool user_gesture, const CefPopupFeatures &popupFeatures, CefWindowInfo &windowInfo, CefRefPtr<CefClient> &client, CefBrowserSettings &settings, bool *no_javascript_access) override;
    bool OnSetFocus(Browser browser, FocusSource source) override;
    bool OnJSDialog(CefRefPtr<CefBrowser> browser, const CefString &origin_url, JSDialogType dialog_type, const CefString &message_text, const CefString &default_prompt_text, CefRefPtr<CefJSDialogCallback> callback, bool &suppress_message) override;
    bool OnConsoleMessage(CefRefPtr<CefBrowser> browser, const CefString &message, const CefString &source, int line) override;
    bool DoClose(Browser browser) override;
    bool OnCertificateError(Browser browser, cef_errorcode_t cert_error, const CefString &request_url, CefRefPtr<CefSSLInfo> ssl_info, CefRefPtr<CefRequestCallback> callback) override;
    bool OnPreKeyEvent(Browser browser, const CefKeyEvent &event, XEvent *os_event, bool *is_keyboard_shortcut) override;
    bool OnKeyEvent(Browser browser, const CefKeyEvent &event, XEvent *os_event) override;
    bool OnFileDialog(Browser browser, FileDialogMode mode, const CefString &title, const CefString &default_file_path, const std::vector<CefString> &accept_filters, int selected_accept_filter, CefRefPtr<CefFileDialogCallback> callback) override;
    bool RunContextMenu(Browser browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefContextMenuParams> params, CefRefPtr<CefMenuModel> model, CefRefPtr<CefRunContextMenuCallback> callback) override;
    bool OnContextMenuCommand(Browser browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefContextMenuParams> params, int command_id, EventFlags event_flags) override;

    bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId source_process, CefRefPtr<CefProcessMessage> message) override;

    void setNewBrowserTabWindow(MainWindow* newBrowserTabWindow) {
        this->newBrowserTabWindow = newBrowserTabWindow;
    }

    QList<int> closingBrowsers;
    bool canClose(Browser browser) {
        if (closingBrowsers.contains(browser.get()->GetIdentifier())) {
            return true;
        } else {
            return false;
        }
    }

    int numberOfBrowsers = 0;

    QSettings settings;

    MainWindow* newBrowserTabWindow = NULL;
};

#include "signalbroker.h"
#include "mainwindow.h"

#endif // CEFHANDLER_H
