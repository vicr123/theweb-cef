#include "client.h"

#define TOQSTR(cefstr) QString::fromStdString(cefstr.ToString())

extern void QuitApp(int exitCode = 0);extern CefBrowserSettings::struct_type browserSettings;


QString tr(const char* key, const char* disambiguation = nullptr, int n = -1) {
    return QApplication::translate("Client", key, disambiguation, n);
}

Client::Client()
{

}

void Client::registerTab(BrowserTab* tab) {
    tabs.append(tab);
    QObject::connect(tab, &BrowserTab::destroyed, [=] {
        tabs.removeOne(tab);
    });
}

BrowserTab* Client::getTab(CefRefPtr<CefBrowser> browser) {
    for (BrowserTab* tab : tabs) {
        if (tab->getBrowser() != nullptr) {
            if (tab->getBrowser().get()->IsSame(browser)) {
                return tab;
            }
        }
    }
    return nullptr;
}

void Client::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
    delete getTab(browser);//->deleteLater();
    numberOfBrowsers--;

    if (numberOfBrowsers == 0) {
        QuitApp();
    }
}

void Client::OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title) {
    getTab(browser)->setTitle(TOQSTR(title));
}

void Client::OnLoadingStateChange(CefRefPtr<CefBrowser> browser, bool isLoading, bool canGoBack, bool canGoForward) {
    getTab(browser)->loadStateChange(isLoading, canGoBack, canGoForward);
}

void Client::OnAddressChange(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString &url) {
    if (frame.get()->IsMain()) {
        getTab(browser)->newAddress(TOQSTR(url));
    }
}

bool Client::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect &rect) {
    BrowserTab* tab = getTab(browser);
    if (tab == nullptr) {
        return false;
    }

    Renderer* renderer = getTab(browser)->getRenderer();
    if (renderer == nullptr) {
        return false;
    }

    QRect r = renderer->RootScreenRect();
    rect.Set(r.x(), r.y(), r.width(), r.height());
    return true;
}

void Client::OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList &dirtyRects, const void *buffer, int width, int height) {
    QVector<CefRect> dirty = QVector<CefRect>::fromStdVector(dirtyRects);
    QList<QRect> dirtyQRects;
    for (CefRect rect : dirty) {
        QRect qr(rect.x, rect.y, rect.width, rect.height);
        dirtyQRects.append(qr);
    }

    getTab(browser)->getRenderer()->paint(type, dirtyQRects, buffer, width, height);

}

void Client::OnCursorChange(CefRefPtr<CefBrowser> browser, CefCursorHandle cursor, CursorType type, const CefCursorInfo &custom_cursor_info) {
    getTab(browser)->getRenderer()->setCursor(cursor, type, custom_cursor_info);
}

void Client::OnFullscreenModeChange(CefRefPtr<CefBrowser> browser, bool isFullScreen) {
    getTab(browser)->setFullScreen(isFullScreen);
}

bool Client::OnJSDialog(CefRefPtr<CefBrowser> browser, const CefString &origin_url, JSDialogType dialog_type, const CefString &message_text, const CefString &default_prompt_text, CefRefPtr<CefJSDialogCallback> callback, bool &suppress_message) {
    BrowserDialog* dialog = getTab(browser)->getDialog();
    switch (dialog_type) {
        case JSDIALOGTYPE_ALERT:
            dialog->alert(TOQSTR(message_text), callback);
            break;
        case JSDIALOGTYPE_CONFIRM:
            dialog->confirm(TOQSTR(message_text), callback);
            break;
        case JSDIALOGTYPE_PROMPT:
            dialog->prompt(TOQSTR(message_text), TOQSTR(default_prompt_text), callback);
            break;
    }
    return true;
}

void Client::OnResetDialogState(CefRefPtr<CefBrowser> browser) {
    if (getTab(browser) != nullptr) {
        getTab(browser)->getDialog()->reset();
    }
}

bool Client::OnBeforePopup(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString &target_url, const CefString &target_frame_name, CefLifeSpanHandler::WindowOpenDisposition target_disposition, bool user_gesture, const CefPopupFeatures &popupFeatures, CefWindowInfo &windowInfo, CefRefPtr<CefClient> &client, CefBrowserSettings &settings, bool *no_javascript_access) {
    if (target_disposition == WOD_NEW_POPUP) {
        newTabWindow = nullptr;
    } else {
        newTabWindow = getTab(browser)->getMainWindow();
    }
    windowInfo.SetAsWindowless(NULL);
    return false;
}

void Client::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
    numberOfBrowsers++;
    if (newTabWindow == nullptr) {
        MainWindow* w = new MainWindow(browser);
        w->show();
    } else {
        newTabWindow->createNewTab(browser);
    }
}

void Client::setNewTabWindow(MainWindow *newTabWindow) {
    this->newTabWindow = newTabWindow;
}

void Client::OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, ErrorCode errorCode, const CefString &errorText, const CefString &failedUrl) {
    if (frame.get()->IsMain()) {
        //getTab(browser)->getDialog()->loadError(errorCode, TOQSTR(failedUrl));
        //frame.get()->LoadString("booooo", failedUrl);

        QFile loadErrorFile(":/pages/loaderror.html");
        loadErrorFile.open(QFile::ReadOnly);
        QString stringSource = loadErrorFile.readAll();
        loadErrorFile.close();

        stringSource = stringSource.arg(TOQSTR(failedUrl));

        QString errText;
        QStringList helpPoints;
        QUrl request(TOQSTR(failedUrl));

        switch (errorCode) {
            case ERR_ABORTED:
                return; //Don't do anything
            case ERR_NAME_NOT_RESOLVED:
                errText = tr("The IP address of %1 could not be found.");
                helpPoints.append(tr("Try again later"));
                helpPoints.append(tr("Check your internet connection"));
                helpPoints.append(tr("Make sure theWeb is allowed to access the internet"));
                break;
            case ERR_INTERNET_DISCONNECTED:
                errText = tr("You're not connected to the internet.");
                helpPoints.append(tr("Check your internet connection"));
                break;
            case ERR_SSL_VERSION_OR_CIPHER_MISMATCH:
                errText = tr("%1 uses an unsupported security protocol.");
                break;
            case ERR_CONNECTION_REFUSED:
                errText = tr("%1 refused to connect.");
                helpPoints.append(tr("Check your internet connection"));
                helpPoints.append(tr("Make sure theWeb is allowed to access the internet"));
                helpPoints.append(tr("Check your proxy settings"));
                break;
            default:
                errText = tr("theWeb couldn't connect to the web page you were looking for.");
                break;
        }

        if (errText.contains("%1")) {
            errText = errText.arg("<b>" + request.host() + "</b>");
        }
        stringSource = stringSource.arg(errText);

        if (helpPoints.length() == 0) {
            stringSource = stringSource.arg("");
        } else {
            QString helpText = "Here are some things you can try:<ul>";
            for (QString point : helpPoints) {
                helpText += "<li>" + point + "</li>";
            }
            helpText += "</ul>";
            stringSource = stringSource.arg(helpText);
        }
        stringSource = stringSource.arg(TOQSTR(errorText));

        QPalette pal = QApplication::palette("MainWindow");
        stringSource = stringSource.arg(pal.color(QPalette::Window).name())
                                   .arg(pal.color(QPalette::WindowText).name())
                                   .arg(pal.color(QPalette::Button).name())
                                   .arg(pal.color(QPalette::ButtonText).name());

        frame.get()->LoadString(stringSource.toStdString(), failedUrl);
    }
}

bool Client::OnBeforeUnloadDialog(CefRefPtr<CefBrowser> browser, const CefString &message_text, bool is_reload, CefRefPtr<CefJSDialogCallback> callback) {
    getTab(browser)->getDialog()->beforeUnload(is_reload, callback);
    return true;
}

void Client::OnFaviconURLChange(CefRefPtr<CefBrowser> browser, const std::vector<CefString>& availableUrls) {
    QVector<CefString> available = QVector<CefString>::fromStdVector(availableUrls);
    QList<QString> availableQString;
    for (CefString url : available) {
        availableQString.append(TOQSTR(url));
    }

    getTab(browser)->getTabButton()->setSiteIcon(availableQString);

}

bool Client::GetAuthCredentials(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, bool isProxy, const CefString &host, int port, const CefString &realm, const CefString &scheme, CefRefPtr<CefAuthCallback> callback) {
    QString hostString = TOQSTR(host);
    hostString = hostString.append(":").append(QString::number(port));
    getTab(browser)->getDialog()->authenticate(hostString, TOQSTR(realm), callback);

    return true;
}

bool Client::OnCertificateError(CefRefPtr<CefBrowser> browser, cef_errorcode_t cert_error, const CefString &request_url, CefRefPtr<CefSSLInfo> ssl_info, CefRefPtr<CefRequestCallback> callback) {
    QString extra;
    QUrl request(TOQSTR(request_url));

    switch (cert_error) {
        case ERR_CERT_COMMON_NAME_INVALID:
            extra = tr("The security certificate was issued for the host %1, but you're trying to visit %2.")
                    .arg("<b>" + TOQSTR(ssl_info.get()->GetX509Certificate().get()->GetSubject().get()->GetCommonName()) + "</b>", "<b>" + request.host() + "</b>");
            break;
        case ERR_CERT_DATE_INVALID: {
            QDateTime start = QDateTime::fromTime_t(ssl_info.get()->GetX509Certificate().get()->GetValidStart().GetTimeT());
            QDateTime expiry = QDateTime::fromTime_t(ssl_info.get()->GetX509Certificate().get()->GetValidExpiry().GetTimeT());
            QDateTime now = QDateTime::currentDateTime();
            if (now.daysTo(start) > 0) { //Certificate is before start date
                extra = tr("The security certificate's validity date doesn't start for another %n day(s).", nullptr, now.daysTo(start));
            } else {
                extra = tr("The security certificate expired %n day(s) ago.", nullptr, expiry.daysTo(now));
            }
            extra += " " + tr("At the moment, according to your device, the date is %1. If that is incorrect, you'll need to change your device's date and time settings so they are correct.")
                    .arg(now.toString(QLocale::system().dateFormat()));
            break;
        }
        case ERR_CERT_AUTHORITY_INVALID:
            extra = tr("The security certificate is not trusted by your device.");
            break;
        case ERR_CERT_WEAK_SIGNATURE_ALGORITHM:
            extra = tr("The security certificate is signed using a weak signature algorithm.");
            break;
        case ERR_CERT_CONTAINS_ERRORS:
            extra = tr("The security certificate contains errors.");
            break;
        case ERR_CERT_REVOKED:
            extra = tr("The security certificate has been revoked by its issuer.");
            break;
        default:
            extra = tr("The website's security certificate is invalid.");
    }

    getTab(browser)->getDialog()->certificate(extra, false, callback);
    return true;
}

bool Client::OnBeforeBrowse(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, bool is_redirect) {

    return false;
}

bool Client::OnContextMenuCommand(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefContextMenuParams> params, int command_id, EventFlags event_flags) {
    switch (command_id) {
        case OpenLinkInNewTab:
            //emit signalBroker->ContextMenuCommand(browser, command_id, params);
            break;
        case DevTools:
            //newWindowIsDevToolsWindow = true;
            //emit signalBroker->ContextMenuCommand(browser, command_id, params);
            break;
        case CopyLink:
            QApplication::clipboard()->setText(QString::fromStdString(params.get()->GetLinkUrl().ToString()));
            break;
        case OpenLinkInNewWindow: {
            CefWindowInfo windowInfo;
            windowInfo.SetAsWindowless(NULL);

            newTabWindow = nullptr;
            CefBrowserHost::CreateBrowser(windowInfo, this, params.get()->GetLinkUrl(), browserSettings, CefRefPtr<CefRequestContext>());
            break;
        }
        case OpenLinkInNewOblivion: {
            /*CefBrowserSettings settings = defaultBrowserSettings;
            settings.application_cache = STATE_DISABLED;

            CefRequestContextSettings contextSettings;
            CefRefPtr<CefRequestContext> context = CefRequestContext::CreateContext(contextSettings, new OblivionRequestContextHandler);
            context.get()->RegisterSchemeHandlerFactory("theweb", "theweb", new theWebSchemeHandler());
            Browser newBrowser = browser.get()->GetHost().get()->CreateBrowserSync(CefWindowInfo(), this, params.get()->GetLinkUrl(), settings, context);

            MainWindow* window = new MainWindow(newBrowser, true);
            window->show();*/
            break;
        }
        default:
            return false;
    }
    return true;
}

bool Client::RunContextMenu(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefContextMenuParams> params, CefRefPtr<CefMenuModel> model, CefRefPtr<CefRunContextMenuCallback> callback) {
    getTab(browser)->contextMenu(params, model, callback);
    return true;
}

void Client::OnBeforeContextMenu(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefContextMenuParams> params, CefRefPtr<CefMenuModel> model) {
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
        model.get()->AddItem(DevTools, "Open Developer Tools Here");
    }
}

void Client::OnStatusMessage(CefRefPtr<CefBrowser> browser, const CefString& message) {
    getTab(browser)->statusMessage(TOQSTR(message));
}
