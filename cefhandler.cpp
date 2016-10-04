#include "cefhandler.h"
#include "theweb_adaptor.h"

extern SignalBroker* signalBroker;
extern void QuitApp(int exitCode = 0);
extern QVariantMap settingsData;
extern QFile historyFile;

CefHandler::CefHandler(QObject* parent) : QObject(parent)
{
    new PlayerAdaptor(this);
    new MediaPlayer2Adaptor(this);
    QDBusConnection::sessionBus().registerObject("/org/mpris/MediaPlayer2", this, QDBusConnection::ExportAdaptors | QDBusConnection::ExportChildObjects);

    QTimer* mprisDetectTimer = new QTimer;
    mprisDetectTimer->setInterval(1000);
    connect(mprisDetectTimer, &QTimer::timeout, [=]() {
        //Send all browsers the mprisCheck signal
        for (Browser browser : currentBrowsers) {
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("mprisCheck");
            browser.get()->SendProcessMessage(PID_RENDERER, message);
        }

        //Send the PropertiesChanged signal.
        QDBusMessage signal = QDBusMessage::createSignal("/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties", "PropertiesChanged");

        QList<QVariant> args;
        args.append("org.mpris.MediaPlayer2.Player");

        QVariantMap changedProperties;
        changedProperties.insert("Metadata", this->Metadata());
        changedProperties.insert("PlaybackStatus", this->PlaybackStatus());
        args.append(changedProperties);

        QStringList invalidatedProperties;
        invalidatedProperties.append("Metadata");
        invalidatedProperties.append("PlaybackStatus");
        args.append(invalidatedProperties);

        signal.setArguments(args);

        QDBusConnection::sessionBus().send(signal);

        //Update browser tabs' media controls
        for (Browser browser : mprisAvailableBrowsers.keys()) {
            emit signalBroker->MprisPlayingStateChanged(browser, mprisAvailableBrowsers.value(browser));
        }
    });
    mprisDetectTimer->start();

    mprisStopTimer.setInterval(3000);
    mprisStopTimer.setSingleShot(true);
    connect(&mprisStopTimer, &QTimer::timeout, [=]() {
        //Stop MPRIS interfaces
        QDBusConnection::sessionBus().unregisterService("org.mpris.MediaPlayer2.theWeb");
        currentMprisBrowser = NULL;

        XUngrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XF86XK_AudioPlay), AnyModifier, QX11Info::appRootWindow());
        XUngrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XF86XK_AudioPrev), AnyModifier, QX11Info::appRootWindow());
        XUngrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XF86XK_AudioStop), AnyModifier, QX11Info::appRootWindow());
    });
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
    if (currentMprisBrowser != NULL) {
        if (currentMprisBrowser.get()->GetIdentifier() == browser.get()->GetIdentifier()) {
            //Stop MPRIS interfaces
            QDBusConnection::sessionBus().unregisterService("org.mpris.MediaPlayer2.theWeb");
            currentMprisBrowser = NULL;

            XUngrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XF86XK_AudioPlay), AnyModifier, QX11Info::appRootWindow());
            XUngrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XF86XK_AudioPrev), AnyModifier, QX11Info::appRootWindow());
            XUngrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XF86XK_AudioStop), AnyModifier, QX11Info::appRootWindow());

            //Send signal to window to hide player controls
            bool isFound = false;
            for (Browser checkBrowser : mprisAvailableBrowsers.keys()) {
                if (checkBrowser.get()->GetIdentifier() == browser.get()->GetIdentifier()) {
                    isFound = true;
                }
            }
            if (!isFound) {
                emit signalBroker->MprisStateChanged(browser, false);
            }
        }
    }
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

        historyFile.seek(0);
        QString historyData = historyFile.readAll();
        QStringList historyDataItems = historyData.split("\n");
        std::reverse(historyDataItems.begin(), historyDataItems.end());
        args.get()->SetString(2, historyDataItems.join("\n").toStdString());

        settings.beginGroup("notifications");
        CefRefPtr<CefDictionaryValue> notificationsDictionary = CefDictionaryValue::Create();

        for (QString key : settings.allKeys()) {
            notificationsDictionary.get()->SetString(key.toStdString(), settings.value(key).toString().toStdString());
        }
        settings.endGroup();

        args.get()->SetDictionary(1, notificationsDictionary);
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
            qDebug() << "New!";
            QDBusConnection::sessionBus().registerService("org.mpris.MediaPlayer2.theWeb");
            currentMprisBrowser = browser;

            XGrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XF86XK_AudioPlay), AnyModifier, RootWindow(QX11Info::display(), 0), true, GrabModeAsync, GrabModeAsync);
            XGrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XF86XK_AudioPrev), AnyModifier, RootWindow(QX11Info::display(), 0), true, GrabModeAsync, GrabModeAsync);
            XGrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XF86XK_AudioStop), AnyModifier, RootWindow(QX11Info::display(), 0), true, GrabModeAsync, GrabModeAsync);

        } else if (currentMprisBrowser.get()->GetIdentifier() == browser.get()->GetIdentifier()) {
            //Cancel MPRIS stopping
            mprisStopTimer.stop();
            for (QTimer* timer : mprisStopTimers) {
                timer->stop();
                timer->deleteLater();
            }
            mprisStopTimers.clear();
        }

        //Send signal to window to show player controls
        bool isFound = false;
        for (Browser checkBrowser : mprisAvailableBrowsers.keys()) {
            if (checkBrowser.get()->GetIdentifier() == browser.get()->GetIdentifier()) {
                isFound = true;
            }
        }
        if (!isFound) {
            emit signalBroker->MprisStateChanged(browser, true);
            mprisAvailableBrowsers.insert(browser, false);
        }

    } else if (message.get()->GetName() == "mprisStop") {
        if (mprisAvailableBrowsers.keys().contains(browser)) {
            QTimer* timer = new QTimer;
            int indexOfTimer = mprisStopTimers.count();
            timer->setInterval(3000);
            timer->setSingleShot(true);
            connect(timer, &QTimer::timeout, [=]() {
                mprisStopTimers.removeAt(indexOfTimer);
                timer->deleteLater();
            });
            timer->start();
            mprisStopTimers.append(timer);
        }

        if (currentMprisBrowser.get() != NULL) {
            if (currentMprisBrowser.get()->GetIdentifier() == browser.get()->GetIdentifier()) {
                //Stop MPRIS after a delay. If the Start signal is sent, cancel stopping.
                if (!mprisStopTimer.isActive()) {
                    mprisStopTimer.start();
                }
            }
        }
    } else if (message.get()->GetName() == "mprisDoStop") {
        if (currentMprisBrowser.get() != NULL) {
            if (currentMprisBrowser.get()->GetIdentifier() == browser.get()->GetIdentifier()) {
                //Stop MPRIS interfaces
                QDBusConnection::sessionBus().unregisterService("org.mpris.MediaPlayer2.theWeb");
                currentMprisBrowser = NULL;

                XUngrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XF86XK_AudioPlay), AnyModifier, QX11Info::appRootWindow());
                XUngrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XF86XK_AudioPrev), AnyModifier, QX11Info::appRootWindow());
                XUngrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XF86XK_AudioStop), AnyModifier, QX11Info::appRootWindow());
            }
        }

        //Send signal to window to hide player controls
        bool isFound = false;
        for (Browser checkBrowser : mprisAvailableBrowsers.keys()) {
            if (checkBrowser.get()->GetIdentifier() == browser.get()->GetIdentifier()) {
                isFound = true;
            }
        }
        if (!isFound) {
            emit signalBroker->MprisStateChanged(browser, false);
        }
    } else if (message.get()->GetName() == "mprisData") {
        if (browser.get()->GetIdentifier() == currentMprisBrowser.get()->GetIdentifier()) {
            this->mprisIsPlaying = message.get()->GetArgumentList().get()->GetBool(0);
            this->mprisTitle = QString::fromStdString(message.get()->GetArgumentList().get()->GetString(1).ToString());
            this->mprisArtist = QString::fromStdString(message.get()->GetArgumentList().get()->GetString(2));
            this->mprisAlbum = QString::fromStdString(message.get()->GetArgumentList().get()->GetString(3));

            for (Browser checkBrowser : mprisAvailableBrowsers.keys()) {
                if (checkBrowser.get()->GetIdentifier() == browser.get()->GetIdentifier()) {
                    mprisAvailableBrowsers.insert(checkBrowser, message.get()->GetArgumentList().get()->GetBool(0));
                }
            }
        }
    } else if (message.get()->GetName() == "jsNotificationRequest") {
        emit signalBroker->AskForNotification(browser, message.get()->GetArgumentList().get()->GetString(0));
    } else if (message.get()->GetName() == "jsNotifications_set") {
        settings.beginGroup("notifications");
        settings.setValue(QString::fromStdString(message.get()->GetArgumentList().get()->GetString(0).ToString()), QString::fromStdString(message.get()->GetArgumentList().get()->GetString(1).ToString()));
        settings.endGroup();
    } else if (message.get()->GetName() == "jsNotifications_post") {
        //Create DBus Message
        QDBusMessage dbusMessage = QDBusMessage::createMethodCall("org.freedesktop.Notifications", "/org/freedesktop/Notifications", "org.freedesktop.Notifications", "Notify");

        //Create message arguments
        QVariantList args;
        args.append("theWeb"); //app_name
        args.append((uint) 0); //replaces_id
        args.append(""); //app_icon
        args.append(QString::fromStdString(message.get()->GetArgumentList().get()->GetString(0).ToString())); //summary
        args.append(QString::fromStdString(message.get()->GetArgumentList().get()->GetString(1).ToString())); //body
        args.append(QStringList() << "1" << "Activate"); //actions
        args.append(QVariantMap()); //hints
        args.append(-1); //expire_timeout
        dbusMessage.setArguments(args);

        //Send the message to the notification server
        QDBusConnection::sessionBus().call(dbusMessage, QDBus::NoBlock);
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
    emit signalBroker->BeforeDownload(browser, download_item, suggested_name, callback);
}

void CefHandler::OnDownloadUpdated(Browser browser, CefRefPtr<CefDownloadItem> download_item, CefRefPtr<CefDownloadItemCallback> callback) {
    emit signalBroker->DownloadUpdated(browser, download_item, callback);
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
        QMenu* menu = new QMenu;
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
                model.get()->AddItem(MENU_ID_ADD_TO_DICTIONARY, "Add to Dictionary");
            }
        }

        if (params.get()->GetLinkUrl() != "") {

            model.get()->AddSubMenu(LinkSubmenu, "For link \"" + menu->fontMetrics().elidedText(QString::fromStdString(params.get()->GetLinkUrl().ToString()), Qt::ElideMiddle, 400).toStdString() + "\"");
            model.get()->AddItem(CopyLink, "Copy Link");
            model.get()->AddItem(OpenLinkInNewTab, "Open Link in new tab");
            model.get()->AddItem(OpenLinkInNewWindow, "Open Link in new Window");
            model.get()->AddItem(OpenLinkInNewOblivion, "Open Link in new Oblivion Window");

            menu->deleteLater();
        }

        if (params.get()->GetSelectionText() != "") {
            model.get()->AddSubMenu(TextSubmenu, "For text \"" + menu->fontMetrics().elidedText(QString::fromStdString(params.get()->GetSelectionText().ToString()), Qt::ElideMiddle, 400).toStdString() + "\"");
            model.get()->AddItem(MENU_ID_COPY, "Copy");

            if ((params.get()->GetEditStateFlags() & CM_EDITFLAG_CAN_COPY) == 0) {
                model.get()->SetEnabled(MENU_ID_COPY, false);
            }

            if (params.get()->IsEditable()) {
                model.get()->AddItem(MENU_ID_CUT, "Cut");
                model.get()->AddItem(MENU_ID_DELETE, "Delete");

                if ((params.get()->GetEditStateFlags() & CM_EDITFLAG_CAN_CUT) == 0) {
                    model.get()->SetEnabled(MENU_ID_CUT, false);
                }

                if ((params.get()->GetEditStateFlags() & CM_EDITFLAG_CAN_DELETE) == 0) {
                    model.get()->SetEnabled(MENU_ID_DELETE, false);
                }
            }
        }

        if (params.get()->IsEditable()) {
            model.get()->AddSubMenu(EditableSubmenu, "For editable box");
            model.get()->AddItem(MENU_ID_PASTE, "Paste");
            model.get()->AddItem(MENU_ID_SELECT_ALL, "Select All");
            model.get()->AddItem(MENU_ID_UNDO, "Undo");
            model.get()->AddItem(MENU_ID_REDO, "Redo");


            if ((params.get()->GetEditStateFlags() & CM_EDITFLAG_CAN_PASTE) == 0) {
                model.get()->SetEnabled(MENU_ID_PASTE, false);
            }

            if ((params.get()->GetEditStateFlags() & CM_EDITFLAG_CAN_SELECT_ALL) == 0) {
                model.get()->SetEnabled(MENU_ID_SELECT_ALL, false);
            }

            if ((params.get()->GetEditStateFlags() & CM_EDITFLAG_CAN_UNDO) == 0) {
                model.get()->SetEnabled(MENU_ID_UNDO, false);
            }

            if ((params.get()->GetEditStateFlags() & CM_EDITFLAG_CAN_REDO) == 0) {
                model.get()->SetEnabled(MENU_ID_REDO, false);
            }
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

void CefHandler::Play() {
    CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("mprisPlay");
    currentMprisBrowser.get()->SendProcessMessage(PID_RENDERER, message);
}

void CefHandler::Pause() {
    CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("mprisPause");
    currentMprisBrowser.get()->SendProcessMessage(PID_RENDERER, message);
}

void CefHandler::Stop() {
    Pause();
}

void CefHandler::Previous() {
    CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("mprisBack");
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
            if (browserUrl.host() != "") {
                retval.insert("xesam:title", browserUrl.host());
            }
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
