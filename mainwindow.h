#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWindow>
#include <QCloseEvent>
#include "cefheaders.h"
#include "cefhandler.h"
#include "signalbroker.h"
#include "thewebsettingsaccessor.h"
#include "downloadimagecallback.h"
#include "oblivionrequestcontext.h"
#include "hovertabbar.h"
#include <QToolButton>
#include <QMessageBox>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonArray>
#include <QSslSocket>
#include <QSslCertificate>
#include <QSslCertificateExtension>
#include <QMap>
#include <QTabBar>
#include <QMovie>
#include <QX11Info>
#include <QToolTip>
#include <QNetworkProxy>
#include "downloadframe.h"
#include <unistd.h>
#include <sys/types.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(Browser browser = NULL, bool isOblivion = false, QWidget *parent = 0);
    ~MainWindow();

    enum warningType {
        none,
        fullscreen,
        notification,
        warning
    };

public slots:
    void RenderProcessTerminated(Browser browser, CefRequestHandler::TerminationStatus status);
    void TitleChanged(Browser browser, const CefString& title);
    void AddressChange(Browser browser, CefRefPtr<CefFrame> frame, const CefString& url);
    void FullscreenModeChange(Browser browser, bool fullscreen);
    void JSDialog(CefRefPtr<CefBrowser> browser, const CefString &origin_url, CefHandler::JSDialogType dialog_type, const CefString &message_text, const CefString &default_prompt_text, CefRefPtr<CefJSDialogCallback> callback, bool &suppress_message);
    void LoadingStateChange(Browser browser, bool isLoading, bool canGoBack, bool canGoForward);
    void LoadError(Browser browser, CefRefPtr<CefFrame> frame, CefHandler::ErrorCode errorCode, const CefString& errorText, const CefString& failedUrl);
    void BeforeClose(Browser browser);
    void AuthCredentials(Browser browser, CefRefPtr<CefFrame> frame, bool isProxy, const CefString &host, int port, const CefString &realm, const CefString &scheme, CefRefPtr<CefAuthCallback> callback);
    void BeforeUnloadDialog(Browser browser, const CefString &message_text, bool is_reload, CefRefPtr<CefJSDialogCallback> callback);
    void BeforePopup(Browser browser, CefRefPtr<CefFrame> frame, const CefString &target_url, const CefString &target_frame_name, CefLifeSpanHandler::WindowOpenDisposition target_disposition, bool user_gesture, const CefPopupFeatures &popupFeatures, CefWindowInfo* windowInfo, CefBrowserSettings settings, bool *no_javascript_access);
    void CertificateError(Browser browser, cef_errorcode_t cert_error, const CefString &request_url, CefRefPtr<CefSSLInfo> ssl_info, CefRefPtr<CefRequestCallback> callback);
    void FaviconURLChange(Browser browser, std::vector<CefString> urls);
    void KeyEvent(CefRefPtr<CefBrowser> browser, const CefKeyEvent &event, XEvent *os_event);
    void ContextMenu(Browser browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefContextMenuParams> params, CefRefPtr<CefMenuModel> model, CefRefPtr<CefRunContextMenuCallback> callback);
    void ContextMenuCommand(Browser browser, int command_id, CefRefPtr<CefContextMenuParams> params);
    void ProtocolExecution(Browser browser, const CefString& url, bool& allow_os_execution);
    void Tooltip(Browser browser, CefString& text);
    void ShowBrowser(Browser browser);
    void AskForNotification(Browser browser, CefString host);
    void MprisStateChanged(Browser browser, bool isOn);
    void MprisPlayingStateChanged(Browser browser, bool isPlaying);
    void BeforeDownload(Browser browser, CefRefPtr<CefDownloadItem> download_item, const CefString& suggested_name, CefRefPtr<CefBeforeDownloadCallback> callback);
    void NewDownload(Browser browser, CefRefPtr<CefDownloadItem> download_item);
    void OpenURLFromTab(Browser browser, CefRefPtr<CefFrame> frame, const CefString &target_url, CefLifeSpanHandler::WindowOpenDisposition target_disposition, bool user_gesture);
    void ReloadSettings();

    void setPopup(CefPopupFeatures features = CefPopupFeatures());

    void createNewTab(Browser newBrowser = NULL, bool openInBackground = false);

private slots:
    void on_actionExit_triggered();

    void on_actionGo_Back_triggered();

    void on_actionGo_Forward_triggered();

    void on_reloadErrorButton_clicked();

    void on_actionReload_triggered();

    void on_spaceSearch_returnPressed();

    void on_warningOk_clicked();

    void on_warningCancel_clicked();

    void on_actionNew_Window_triggered();

    void on_JsDialogOk_clicked();

    void on_JsDialogCancel_clicked();

    void on_JsDialogPrompt_returnPressed();

    void on_AuthOk_clicked();

    void on_AuthCancel_clicked();

    void on_AuthUsername_returnPressed();

    void on_AuthPassword_returnPressed();

    void on_actionLimit_to_60_fps_triggered();

    void on_actionLimit_to_30_fps_triggered();

    void on_actionLimit_to_15_fps_triggered();

    void on_actionLimit_to_1_fps_triggered();

    void on_actionDon_t_Limit_triggered();

    void on_actionAbout_theWeb_triggered();

    void on_actionSettings_triggered();

    void on_fraudIgnore_clicked();

    void on_fraudBack_clicked();

    void on_securityFrame_clicked();

    void on_certIgnore_clicked();

    void on_certificateBack_clicked();

    void on_actionNew_Tab_triggered();

    void updateCurrentBrowserDisplay();

    void on_spaceSearch_GotFocus();

    void on_actionClose_Tab_triggered();

    void on_actionNew_Oblivion_Window_triggered();

    void on_findText_returnPressed();

    void on_actionFind_triggered();

    void on_findNext_clicked();

    void on_findClose_clicked();

    void on_findBack_clicked();

    void on_findText_textChanged(const QString &arg1);

    void on_actionHistory_triggered();

private:
    Ui::MainWindow *ui;

    Browser browser();

    //Browser browser;
    QList<Browser> browserList;
    QList<QVariantMap> browserMetadata;
    QList<QIcon> browserIcons;
    void insertIntoMetadata(Browser browser, QString key, QVariant value);
    void removeFromMetadata(Browser browser, QString key);

    HoverTabBar* tabBar;
    bool IsCorrectBrowser(Browser browser);
    Browser getBrowserFor(Browser browser);
    int indexOfBrowser(Browser browser);
    bool isOblivion = false;
    QToolButton* menuButton;
    QAction* menuButtonAction;

    QMovie* tabLoading;

    warningType currentWarning = none;

    CefRefPtr<CefJSDialogCallback> JsDialogCallback;

    QSettings settings;

    void closeEvent(QCloseEvent *event);
    void resizeEvent(QResizeEvent *event);
};

#endif // MAINWINDOW_H
