#include "cefhandler.h"
#include "theweb_adaptor.h"

extern SignalBroker* signalBroker;
extern void QuitApp(int exitCode = 0);
extern QVariantMap settingsData;

CefHandler::CefHandler(QObject* parent) : QObject(parent)
{
    new PlayerAdaptor(this);
    new MediaPlayer2Adaptor(this);
    QDBusConnection::sessionBus().registerObject("/org/mpris/MediaPlayer2", this, QDBusConnection::ExportAdaptors | QDBusConnection::ExportChildObjects);

    QTimer* mprisDetectTimer = new QTimer;
    mprisDetectTimer->setInterval(1000);
    connect(mprisDetectTimer, &QTimer::timeout, [=]() {
        for (Browser browser : currentBrowsers) {
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("mprisCheck");
            browser.get()->SendProcessMessage(PID_RENDERER, message);
        }
    });
    mprisDetectTimer->start();
}

void CefHandler::AddRef() const {
    CefRefCount::AddRef();
}

bool CefHandler::Release() const {
    return CefRefCount::Release();
}

bool CefHandler::HasOneRef() const {
    return CefRefCount::HasOneRef();
}

void CefHandler::OnAfterCreated(Browser browser) {
    numberOfBrowsers++;
    currentBrowsers.append(browser);
    if (newBrowserTabWindow != NULL) {
        newBrowserTabWindow->createNewTab(browser);
    }
    this->newBrowserTabWindow = NULL;
}

void CefHandler::OnRenderProcessTerminated(Browser browser, CefRequestHandler::TerminationStatus status) {
    emit signalBroker->RenderProcessTerminated(browser, status);
}

void CefHandler::OnTitleChange(Browser browser, const CefString& title) {
    emit signalBroker->TitleChanged(browser, title);
}

void CefHandler::OnAddressChange(Browser browser, CefRefPtr<CefFrame> frame, const CefString &url) {
    emit signalBroker->AddressChange(browser, frame, url);
}

bool CefHandler::OnConsoleMessage(CefRefPtr<CefBrowser> browser, const CefString &message, const CefString &source, int line) {
    qDebug() << QString::fromStdString(message.ToString());
    return false;
}

void CefHandler::OnFullscreenModeChange(Browser browser, bool fullscreen) {
    emit signalBroker->FullscreenModeChange(browser, fullscreen);
}

bool CefHandler::OnJSDialog(CefRefPtr<CefBrowser> browser, const CefString &origin_url, JSDialogType dialog_type, const CefString &message_text, const CefString &default_prompt_text, CefRefPtr<CefJSDialogCallback> callback, bool &suppress_message) {
    emit signalBroker->JSDialog(browser, origin_url, dialog_type, message_text, default_prompt_text, callback, suppress_message);
    return true;
}

void CefHandler::OnLoadingStateChange(Browser browser, bool isLoading, bool canGoBack, bool canGoForward) {
    emit signalBroker->LoadingStateChange(browser, isLoading, canGoBack, canGoForward);
}

void CefHandler::OnLoadError(Browser browser, CefRefPtr<CefFrame> frame, ErrorCode errorCode, const CefString &errorText, const CefString &failedUrl) {
    emit signalBroker->LoadError(browser, frame, errorCode, errorText, failedUrl);
}

bool CefHandler::DoClose(Browser browser) {
    closingBrowsers.append(browser.get()->GetIdentifier());
    numberOfBrowsers--;
    if (numberOfBrowsers == 0) {
        QuitApp();
    }

    return false;
}

void CefHandler::OnBeforeClose(Browser browser) {
    emit signalBroker->BeforeClose(browser);
    currentBrowsers.removeAll(browser);
    browser = NULL;
}

bool CefHandler::OnSetFocus(Browser browser, FocusSource source) {
    return true;
}

void CefHandler::OnGotFocus(Browser browser) {

}

bool CefHandler::GetAuthCredentials(Browser browser, CefRefPtr<CefFrame> frame, bool isProxy, const CefString &host, int port, const CefString &realm, const CefString &scheme, CefRefPtr<CefAuthCallback> callback) {
    emit signalBroker->AuthCredentials(browser, frame, isProxy, host, port, realm, scheme, callback);
    return true;
}

bool CefHandler::OnBeforeUnloadDialog(Browser browser, const CefString &message_text, bool is_reload, CefRefPtr<CefJSDialogCallback> callback) {
    emit signalBroker->BeforeUnloadDialog(browser, message_text, is_reload, callback);
    return true;
}

bool CefHandler::OnBeforePopup(Browser browser, CefRefPtr<CefFrame> frame, const CefString &target_url, const CefString &target_frame_name, CefLifeSpanHandler::WindowOpenDisposition target_disposition, bool user_gesture, const CefPopupFeatures &popupFeatures, CefWindowInfo &windowInfo, CefRefPtr<CefClient> &client, CefBrowserSettings &settings, bool *no_javascript_access) {

    emit signalBroker->BeforePopup(browser, frame, target_url, target_frame_name, target_disposition, user_gesture, popupFeatures, &windowInfo, settings, no_javascript_access);
    return false;
    //return true;
}

bool CefHandler::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId source_process, CefRefPtr<CefProcessMessage> message) {
    if (message.get()->GetName() == "theWebSettings") {
        CefRefPtr<CefListValue> args = message.get()->GetArgumentList();
        QString key = QString::fromStdString(args.get()->GetString(0).ToString());
        if (args.get()->GetString(1) == "bool") {
            settings.setValue(key, args.get()->GetBool(2));
        } else if (args.get()->GetString(1) == "string") {
            settings.setValue(key, QString::fromStdString(args.get()->GetString(2).ToString()));
        }
    } else if (message.get()->GetName() == "theWebSettings_get") {
        CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("theWebSettings_reply");
        CefRefPtr<CefListValue> args = message.get()->GetArgumentList();
        CefRefPtr<CefDictionaryValue> settingsDictionary = CefDictionaryValue::Create();

        for (QString key : settings.allKeys()) {
            if (settings.value(key).canConvert<QString>()) {
                QString string = settings.value(key).toString();
                if (string == "true" || string == "false") {
                    settingsDictionary.get()->SetBool(key.toStdString(), settings.value(key).toBool());
                } else {
                    settingsDictionary.get()->SetString(key.toStdString(), settings.value(key).toString().toStdString());
                }
            }
        }

        args.get()->SetDictionary(0, settingsDictionary);
        browser.get()->SendProcessMessage(PID_RENDERER, message);
    } else if (message.get()->GetName() == "ReloadSettings") {
        emit signalBroker->ReloadSettings();
    } else if (message.get()->GetName() == "showProxy") {
        QString desktop = qgetenv("XDG_CURRENT_DESKTOP");

        if (desktop == "KDE") { //or theShell. :)
            QProcess::startDetached("kcmshell5 proxy");
        } else if (desktop.contains("GNOME")) {
            QProcess::startDetached("gnome-control-center network");
        } else if (desktop == "Unity") {
            QProcess::startDetached("unity-control-center network");
        } else if (desktop == "XFCE") {

        } else if (desktop == "Cinnamon" || desktop == "X-Cimmamon") {
            QProcess::startDetached("cinnamon-control-center network");
        } else if (desktop == "ENLIGHTENMENT") {

        } else if (desktop == "LXDE") {

        } else if (desktop == "MATE") {

        }
    } else if (message.get()->GetName() == "mprisStart") {
        if (currentMprisBrowser.get() == NULL) {
            QDBusConnection::sessionBus().registerService("org.mpris.MediaPlayer2.theweb");
            currentMprisBrowser = browser;

            XGrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XF86XK_AudioPlay), AnyModifier, RootWindow(QX11Info::display(), 0), true, GrabModeAsync, GrabModeAsync);
            XGrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XF86XK_AudioStop), AnyModifier, RootWindow(QX11Info::display(), 0), true, GrabModeAsync, GrabModeAsync);
        }
    } else if (message.get()->GetName() == "mprisStop") {
        if (currentMprisBrowser.get() != NULL && browser.get()->IsSame(currentMprisBrowser)) {
            QDBusConnection::sessionBus().unregisterService("org.mpris.MediaPlayer2.theweb");
            currentMprisBrowser = NULL;

            XUngrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XF86XK_AudioPlay), AnyModifier, QX11Info::appRootWindow());
            XUngrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XF86XK_AudioStop), AnyModifier, QX11Info::appRootWindow());
        }
    } else if (message.get()->GetName() == "mprisData") {
        this->mprisIsPlaying = message.get()->GetArgumentList().get()->GetBool(0);
        this->mprisTitle = QString::fromStdString(message.get()->GetArgumentList().get()->GetString(1).ToString());
        this->mprisArtist = QString::fromStdString(message.get()->GetArgumentList().get()->GetString(2));
        this->mprisAlbum = QString::fromStdString(message.get()->GetArgumentList().get()->GetString(3));
    }
    return true;
}

CefHandler::ReturnValue CefHandler::OnBeforeResourceLoad(Browser browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, CefRefPtr<CefRequestCallback> callback) {
    if (settings.value("data/dnt", false).toBool()) {
        std::multimap<CefString, CefString> headers;
        request.get()->GetHeaderMap(headers);
        headers.insert(std::pair<CefString, CefString>("DNT", "1"));
        request.get()->SetHeaderMap(headers);
    }
    return RV_CONTINUE;
}

bool CefHandler::OnCertificateError(Browser browser, cef_errorcode_t cert_error, const CefString &request_url, CefRefPtr<CefSSLInfo> ssl_info, CefRefPtr<CefRequestCallback> callback) {
    emit signalBroker->CertificateError(browser, cert_error, request_url, ssl_info, callback);
    return true;
}

void CefHandler::OnFaviconURLChange(Browser browser, const std::vector<CefString> &urls) {
    emit signalBroker->FaviconURLChange(browser, urls);
}

bool CefHandler::OnPreKeyEvent(CefRefPtr<CefBrowser> browser, const CefKeyEvent &event, XEvent *os_event, bool *is_keyboard_shortcut) {

    if (os_event) {
        if (os_event->xkey.keycode == XKeysymToKeycode(QX11Info::display(), XK_Escape)) { //ESC key
            //Leave Full Screen (if the browser is in full screen)
            browser.get()->GetMainFrame().get()->ExecuteJavaScript("document.webkitExitFullscreen()", "", 0);
            return true;
        }
    }
    return false;
}

void CefHandler::OnBeforeDownload(Browser browser, CefRefPtr<CefDownloadItem> download_item, const CefString &suggested_name, CefRefPtr<CefBeforeDownloadCallback> callback) {
    browser.get()->GetMainFrame().get()->ExecuteJavaScript("alert(\"Downloads aren't supported in theWeb yet. Sorry for the inconvenience... :(\")", "", 0);
}

bool CefHandler::OnKeyEvent(CefRefPtr<CefBrowser> browser, const CefKeyEvent &event, XEvent *os_event) {
    emit signalBroker->KeyEvent(browser, event, os_event);
    return false;
}

bool CefHandler::OnFileDialog(Browser browser, FileDialogMode mode, const CefString &title, const CefString &default_file_path, const std::vector<CefString> &accept_filters, int selected_accept_filter, CefRefPtr<CefFileDialogCallback> callback) {
    switch (mode >> 25) {
    case FILE_DIALOG_OPEN:
    case FILE_DIALOG_OPEN_MULTIPLE:
    {
        QFileDialog* dialog = new QFileDialog();
        dialog->setAcceptMode(QFileDialog::AcceptOpen);
        if (title.ToString() == "") {
            dialog->setWindowTitle("Open File");
        } else {
            dialog->setWindowTitle(QString::fromStdString(title.ToString()));
        }

        QStringList filters;
        /*for (CefString filter : accept_filters) {
            filters.append(QString::fromStdString(filter.ToString()));
        }
        dialog->setNameFilters(filters);*/

        connect(dialog, &QFileDialog::accepted, [=]() {
            QList<CefString> selectedFiles;
            for (QString file : dialog->selectedFiles()) {
                selectedFiles.append(file.toStdString());
            }
            std::vector<CefString> selectedFilesVector = selectedFiles.toVector().toStdVector();
            callback.get()->Continue(0, selectedFilesVector);
            dialog->deleteLater();
        });
        connect(dialog, &QFileDialog::rejected, [=]() {
            callback.get()->Cancel();
            dialog->deleteLater();
        });
        dialog->show();
        return true;
    }
        break;

    }
    return false;
}

bool CefHandler::RunContextMenu(Browser browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefContextMenuParams> params, CefRefPtr<CefMenuModel> model, CefRefPtr<CefRunContextMenuCallback> callback) {
    emit signalBroker->ContextMenu(browser, frame, params, model, callback);
    return true;
}

void CefHandler::OnBeforeContextMenu(Browser browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefContextMenuParams> params, CefRefPtr<CefMenuModel> model) {
    if (!params.get()->IsPepperMenu()) {
        model.get()->Clear();

        if (params.get()->GetMisspelledWord() != "") {
            model.get()->AddSubMenu(MisspelledWordSubmenu, "Correct misspelled word \"" + params.get()->GetMisspelledWord().ToString() + "\" to");
            std::vector<CefString> suggestionsVector;
            params.get()->GetDictionarySuggestions(suggestionsVector);

            QVector<CefString> suggestionsList = QVector<CefString>::fromStdVector(suggestionsVector);

            if (suggestionsList.count() == 0) {
                model.get()->AddItem(MENU_ID_NO_SPELLING_SUGGESTIONS, "Not sure what you're trying to type");
                model.get()->SetEnabled(MENU_ID_NO_SPELLING_SUGGESTIONS, false);
            } else {
                switch (suggestionsList.count()) {
                case 5:
                    model.get()->AddItem(MENU_ID_SPELLCHECK_SUGGESTION_4, suggestionsList.at(4));
                case 4:
                    model.get()->AddItem(MENU_ID_SPELLCHECK_SUGGESTION_3, suggestionsList.at(3));
                case 3:
                    model.get()->AddItem(MENU_ID_SPELLCHECK_SUGGESTION_2, suggestionsList.at(2));
                case 2:
                    model.get()->AddItem(MENU_ID_SPELLCHECK_SUGGESTION_1, suggestionsList.at(1));
                case 1:
                    model.get()->AddItem(MENU_ID_SPELLCHECK_SUGGESTION_0, suggestionsList.at(0));
                }
            }
        }

        if (params.get()->GetLinkUrl() != "") {
            QMenu* menu = new QMenu;

            model.get()->AddSubMenu(LinkSubmenu, "For link \"" + menu->fontMetrics().elidedText(QString::fromStdString(params.get()->GetLinkUrl().ToString()), Qt::ElideMiddle, 400).toStdString() + "\"");
            model.get()->AddItem(CopyLink, "Copy Link");
            model.get()->AddItem(OpenLinkInNewTab, "Open Link in new tab");
            model.get()->AddItem(OpenLinkInNewWindow, "Open Link in new Window");
            model.get()->AddItem(OpenLinkInNewOblivion, "Open Link in new Oblivion Window");

            menu->deleteLater();
        }

        model.get()->AddSubMenu(Generic, "For Webpage");
        model.get()->AddItem(MENU_ID_BACK, "Go Back");
        model.get()->AddItem(MENU_ID_FORWARD, "Go Forward");
        model.get()->AddItem(MENU_ID_RELOAD_NOCACHE, "Reload");
        model.get()->AddItem(MENU_ID_VIEW_SOURCE, "View Page Source");
    }
}

bool CefHandler::OnContextMenuCommand(Browser browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefContextMenuParams> params, int command_id, EventFlags event_flags) {
    switch (command_id) {
    case OpenLinkInNewTab:
        emit signalBroker->ContextMenuCommand(browser, command_id, params);
        break;
    case CopyLink:
        QApplication::clipboard()->setText(QString::fromStdString(params.get()->GetLinkUrl().ToString()));
        break;
    case OpenLinkInNewWindow:
    {
        Browser newBrowser = browser.get()->GetHost().get()->CreateBrowserSync(CefWindowInfo(), this, params.get()->GetLinkUrl(), CefBrowserSettings(), CefRefPtr<CefRequestContext>());

        MainWindow* window = new MainWindow(newBrowser);
        window->show();
    }
        break;
    case OpenLinkInNewOblivion:
    {
        CefBrowserSettings settings;
        settings.application_cache = STATE_DISABLED;

        CefRequestContextSettings contextSettings;
        CefRefPtr<CefRequestContext> context = CefRequestContext::CreateContext(contextSettings, new OblivionRequestContextHandler);
        context.get()->RegisterSchemeHandlerFactory("theweb", "theweb", new theWebSchemeHandler());
        Browser newBrowser = browser.get()->GetHost().get()->CreateBrowserSync(CefWindowInfo(), this, params.get()->GetLinkUrl(), settings, context);

        MainWindow* window = new MainWindow(newBrowser, true);
        window->show();
    }
        break;
    default:
        return false;
    }
    return true;
}

void CefHandler::OnProtocolExecution(Browser browser, const CefString &url, bool &allow_os_execution) {
    emit signalBroker->ProtocolExecution(browser, url, allow_os_execution);
    allow_os_execution = false;
}

bool CefHandler::OnTooltip(Browser browser, CefString &text) {
    emit signalBroker->Tooltip(browser, text);
    return true;
}

void CefHandler::PlayPause() {
    CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("mprisPlayPause");
    currentMprisBrowser.get()->SendProcessMessage(PID_RENDERER, message);
}

void CefHandler::Raise() {
    emit signalBroker->ShowBrowser(currentMprisBrowser);
}

QVariantMap CefHandler::Metadata() {
    if (currentMprisBrowser.get() != NULL) {
        QUrl browserUrl(QString::fromStdString(currentMprisBrowser.get()->GetMainFrame().get()->GetURL().ToString()));

        QVariantMap retval;
        retval.insert("mpris.length", -1000);
        retval.insert("mpris:trackid", QVariant::fromValue(QDBusObjectPath("/org/thesuite/theweb/webvideo")));
        retval.insert("xesam:url", QString::fromStdString(currentMprisBrowser.get()->GetMainFrame().get()->GetURL().ToString()));

        if (mprisTitle == "") {
            retval.insert("xesam:title", browserUrl.host());
        } else {
            retval.insert("xesam:title", mprisTitle);
        }

        if (mprisArtist != "") {
            retval.insert("xesam:artist", QStringList() << mprisArtist);
        }

        if (mprisAlbum != "") {
            retval.insert("xesam:album", mprisAlbum);
        }
        return retval;
    } else {
        return QVariantMap();
    }
}

QString CefHandler::PlaybackStatus() {
    if (currentMprisBrowser.get() != NULL) {
        if (mprisIsPlaying) {
            return "Playing";
        } else {
            return "Paused";
        }
    } else {
        return "";
    }
}

MprisDBusMain::MprisDBusMain(QObject *parent) : QObject(parent) {
    new MediaPlayer2Adaptor(this);
}
