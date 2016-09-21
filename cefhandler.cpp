#include "cefhandler.h"

extern SignalBroker* signalBroker;
extern void QuitApp(int exitCode = 0);
extern QVariantMap settingsData;

CefHandler::CefHandler(QObject* parent) : QObject(parent)
{

}

void CefHandler::OnAfterCreated(Browser browser) {
    numberOfBrowsers++;
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
    return true;
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
    //qDebug() << os_event->xkey.keycode;
    if (os_event) {
        if (os_event->xkey.keycode == 9) { //ESC key
            //Leave Full Screen (if the browser is in full screen)
            browser.get()->GetMainFrame().get()->ExecuteJavaScript("document.webkitExitFullscreen()", "", 0);
            return true;
        }
    }
    return false;
}

void CefHandler::OnBeforeDownload(Browser browser, CefRefPtr<CefDownloadItem> download_item, const CefString &suggested_name, CefRefPtr<CefBeforeDownloadCallback> callback) {

}
