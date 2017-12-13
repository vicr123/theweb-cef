#include "client.h"

#define TOQSTR(cefstr) QString::fromStdString(cefstr.ToString())

extern void QuitApp(int exitCode = 0);

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

        switch (errorCode) {
            case ERR_ABORTED:
                return; //Don't do anything
            case ERR_NAME_NOT_RESOLVED:
                errText = tr("The IP address of the server could not be found.");
                break;
            case ERR_INTERNET_DISCONNECTED:
                errText = tr("You're not connected to the internet.");
                break;
            default:
                errText = tr("theWeb couldn't connect to the web page you were looking for.");
                break;
        }

        stringSource = stringSource.arg(errText);

        stringSource = stringSource.arg("");
        stringSource = stringSource.arg(TOQSTR(errorText));

        frame.get()->LoadString(stringSource.toStdString(), failedUrl);
    }
}

bool Client::OnBeforeUnloadDialog(CefRefPtr<CefBrowser> browser, const CefString &message_text, bool is_reload, CefRefPtr<CefJSDialogCallback> callback) {
    getTab(browser)->getDialog()->beforeUnload(is_reload, callback);
    return true;
}
