#ifndef CEFHANDLER_H
#define CEFHANDLER_H

#include <QObject>
#include <QList>
#include <QVector>
#include "cefheaders.h"
#include "v8interceptor.h"
#include <QDebug>
#include <QApplication>
#include "cefengine.h"
#include <X11/Xlib.h>
#include <QX11Info>
#include <X11/keysym.h>
#include <QFileDialog>
#include <QClipboard>
#include <QMenu>
#include <QDBusInterface>
#include <QTimer>

#undef Bool
#undef None

class SignalBroker;
class MainWindow;

//MainDBus is defined here because of qdbuscpp2xml.
class MainDBus : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.thesuite.theweb")

    Q_PROPERTY(uint processID READ processID)
public:
    explicit MainDBus(QObject *parent = 0);

    uint processID();
signals:

public slots:
    Q_SCRIPTABLE void newWindow();
    Q_SCRIPTABLE void newWindow(QString url);
};

//org.mpris.MediaPlayer2 Interface
class MprisDBusMain : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.mpris.MediaPlayer2")

    Q_PROPERTY(bool CanQuit READ CanQuit)
    Q_PROPERTY(bool CanRaise READ CanRaise)
    Q_PROPERTY(bool HasTrackList READ HasTrackList)
    Q_PROPERTY(QString Identity READ Identity)
    Q_PROPERTY(QString DesktopEntry READ DesktopEntry)
    Q_PROPERTY(QStringList SupportedMimeTypes READ SupportedMimeTypes)
    Q_PROPERTY(QStringList SupportedUriSchemes READ SupportedUriSchemes)
public:
    explicit MprisDBusMain(QObject* parent);

    virtual bool CanQuit() {}
    virtual bool CanRaise() {}
    virtual bool HasTrackList() {}
    virtual QString Identity() {}
    virtual QString DesktopEntry() {}
    virtual QStringList SupportedMimeTypes() {}
    virtual QStringList SupportedUriSchemes() {}
public Q_SLOTS:
    Q_SCRIPTABLE virtual void Raise() {}
    Q_SCRIPTABLE virtual void Quit() {}

};

//org.mpris.MediaPlayer2.Player interface
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
        public CefRefCount
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.mpris.MediaPlayer2.Player")

    Q_PROPERTY(QVariantMap Metadata READ Metadata)
    Q_PROPERTY(QString PlaybackStatus READ PlaybackStatus)
    Q_PROPERTY(bool CanControl READ CanControl)
    Q_PROPERTY(bool CanPlay READ CanPlay)
    Q_PROPERTY(bool CanPause READ CanPause)
    Q_PROPERTY(bool CanGoPrevious READ CanGoPrevious)
    Q_PROPERTY(bool CanGoNext READ CanGoNext)
    Q_PROPERTY(bool CanSeek READ CanSeek)
public:
    explicit CefHandler(QObject* parent = 0);

    enum MenuItemIds {
        LinkSubmenu = MENU_ID_USER_FIRST, CopyLink, OpenLinkInNewTab, OpenLinkInNewWindow, OpenLinkInNewOblivion,
        MisspelledWordSubmenu,
        TextSubmenu,
        EditableSubmenu,
        Generic, DevTools
    };

    void AddRef() const;
    bool Release() const;
    bool HasOneRef() const;

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
    void OnProtocolExecution(Browser browser, const CefString& url, bool& allow_os_execution) override;
    void OnDownloadUpdated(Browser browser, CefRefPtr<CefDownloadItem> download_item, CefRefPtr<CefDownloadItemCallback> callback) override;
    void OnStatusMessage(Browser browser, const CefString& value) override;

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
    bool OnTooltip(Browser browser, CefString& text) override;
    bool OnOpenURLFromTab(Browser browser, CefRefPtr<CefFrame> frame, const CefString &target_url, CefLifeSpanHandler::WindowOpenDisposition target_disposition, bool user_gesture) override;
    bool OnResourceResponse(Browser browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, CefRefPtr<CefResponse> response) override;

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


    MainWindow* newBrowserTabWindow = NULL;
    MprisDBusMain* mainMprisDbus;
    bool newWindowIsDevToolsWindow = false;

public Q_SLOTS:
    //org.mpris.MediaPlayer2.Player methods
    Q_SCRIPTABLE void PlayPause();
    Q_SCRIPTABLE void Play();
    Q_SCRIPTABLE void Pause();
    Q_SCRIPTABLE void Stop();
    Q_SCRIPTABLE void Previous();

    //org.mpris.MediaPlayer2 methods
    void Raise();
    void Quit() {}
    bool CanQuit() { return false; }
    bool CanRaise() { return true; }
    bool HasTrackList() { return false; }
    QString Identity() { return "theWeb"; }
    QString DesktopEntry() { return "theweb.desktop"; }
    QStringList SupportedMimeTypes() { return QStringList(); }
    QStringList SupportedUriSchemes() { return QStringList(); }
private:
    //DBus Properties
    QVariantMap Metadata();
    QString PlaybackStatus();
    bool CanControl() { return true; }
    bool CanPlay() { return true; }
    bool CanPause() { return true; }
    bool CanGoPrevious() { return true; }
    bool CanGoNext() { return false; }
    bool CanSeek() { return false; }

    //Private Variables
    int numberOfBrowsers = 0;

    bool newWindowIsPopupWindow = false;
    CefPopupFeatures newWindowPopupFeatures;

    Browser currentMprisBrowser = NULL;
    QList<Browser> currentBrowsers;
    QMap<Browser, bool> mprisAvailableBrowsers;
    QList<QTimer*> mprisStopTimers;
    bool mprisIsPlaying;
    QString mprisTitle, mprisArtist, mprisAlbum;
    QTimer mprisStopTimer;
    QTimer browserMediaStopTimer;
    QList<Browser> browsersToStopMedia;

    QSettings settings;
};

#include "signalbroker.h"
#include "mainwindow.h"

#endif // CEFHANDLER_H
