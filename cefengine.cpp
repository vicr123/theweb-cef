#include "cefengine.h"

extern QVariantMap settingsData;

CefEngine::CefEngine() : CefApp()
{

}

void CefEngine::AddRef() const {
    CefRefCount::AddRef();
}

bool CefEngine::Release() const {
    return CefRefCount::Release();
}

bool CefEngine::HasOneRef() const {
    return CefRefCount::HasOneRef();
}

void CefEngine::OnContextInitialized() {
    CefRegisterSchemeHandlerFactory("theweb", "theweb", new theWebSchemeHandler());
}

void CefEngine::OnContextCreated(Browser browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context) {
    //Get browser settings
    CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("theWebSettings_get");
    browser.get()->SendProcessMessage(PID_BROWSER, message);

    if (QUrl(QString::fromStdString(frame.get()->GetURL().ToString())).scheme() == "theweb") {

        //Register the theWebSettingsObject JavaScript object
        CefRefPtr<theWebSettingsAccessor> accessor = new theWebSettingsAccessor(browser);
        CefRefPtr<CefV8Value> JsObject = CefV8Value::CreateObject(accessor);
        JsObject.get()->SetValue("dnt", V8_ACCESS_CONTROL_DEFAULT, V8_PROPERTY_ATTRIBUTE_NONE);
        JsObject.get()->SetValue("home", V8_ACCESS_CONTROL_DEFAULT, V8_PROPERTY_ATTRIBUTE_NONE);
        JsObject.get()->SetValue("toolbar", V8_ACCESS_CONTROL_DEFAULT, V8_PROPERTY_ATTRIBUTE_NONE);
        JsObject.get()->SetValue("tabText", V8_ACCESS_CONTROL_DEFAULT, V8_PROPERTY_ATTRIBUTE_NONE);
        JsObject.get()->SetValue("malwareProtect", V8_ACCESS_CONTROL_DEFAULT, V8_PROPERTY_ATTRIBUTE_NONE);
        JsObject.get()->SetValue("resetBrowser", CefV8Value::CreateFunction("resetBrowser", new V8Function([=]() {
            CefRefPtr<CefV8Value> returnVal;
            CefRefPtr<CefV8Exception> exception;
            context.get()->Eval("confirm(\"Reset theWeb?:Here\'s what we\'ll do.\\n"
                                "- Reset all settings back to defaults\\n"
                                "- Erase all history items\\n"
                                "- Clear the cache\\n"
                                "- Erase all cookies\\n"
                                "\\n"
                                "Are you sure that you want to reset theWeb?:Continue:Cancel\")", returnVal, exception);
            if (returnVal.get()->GetBoolValue()) {
                context.get()->Eval("confirm(\"Reset theWeb?:We're ready to reset theWeb. Once this is done, theWeb will exit. Is this OK?:Reset:Cancel\")", returnVal, exception);
            }
        })), V8_PROPERTY_ATTRIBUTE_NONE);
        JsObject.get()->SetValue("proxySettings", CefV8Value::CreateFunction("proxySettings", new V8Function([=]() {
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("showProxy");
            browser.get()->SendProcessMessage(PID_BROWSER, message);
        })), V8_PROPERTY_ATTRIBUTE_NONE);

        context.get()->GetGlobal()->SetValue("theWebSettingsObject", JsObject, V8_PROPERTY_ATTRIBUTE_NONE);
    }
}

void CefEngine::OnContextReleased(Browser browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context) {
    if (this->videoFrame.get() != NULL) {
        //Stop MPRIS because the browser just closed
        if (this->videoFrame.get()->GetIdentifier() == frame.get()->GetIdentifier()) {
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("mprisForceStop");
            browser.get()->SendProcessMessage(PID_BROWSER, message);
            this->videoFrame = NULL;
        }
    }
}

bool CefEngine::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId source_process, CefRefPtr<CefProcessMessage> message) {
    if (message.get()->GetName() == "theWebSettings_reply") {
        CefRefPtr<CefListValue> args = message.get()->GetArgumentList();
        CefRefPtr<CefDictionaryValue> settingsDictionary = args.get()->GetDictionary(0);
        std::vector<CefString> keys;
        settingsDictionary.get()->GetKeys(keys);
        for (CefString key : keys) {
            //Convert CEF types into Qt types
            if (settingsDictionary.get()->GetType(key) == VTYPE_BOOL) {
                settingsData.insert(QString::fromStdString(key.ToString()), settingsDictionary.get()->GetBool(key));
            } else if (settingsDictionary.get()->GetType(key) == VTYPE_STRING) {
                settingsData.insert(QString::fromStdString(key.ToString()), QString::fromStdString(settingsDictionary.get()->GetString(key).ToString()));
            }
        }
    } else if (message.get()->GetName() == "mprisCheck") {
        //Iterate over all frames
        std::vector<int64> frameIdentifiers;
        browser.get()->GetFrameIdentifiers(frameIdentifiers);
        for (unsigned long i = 0; i < frameIdentifiers.size(); i++) {
            //Check if we should check the browser
            bool doCheck = false;
            if (this->videoFrame == NULL) { //If there is no browser playing media, check the browser
                doCheck = true;
            } else if (this->videoFrame.get()->GetIdentifier() == browser.get()->GetFrame(frameIdentifiers[i]).get()->GetIdentifier() &&
                       browser.get()->GetIdentifier() == this->videoFrame.get()->GetBrowser().get()->GetIdentifier()) {
                //If this is the same browser playing media, check the browser
                doCheck = true;
            }

            if (doCheck) {
                //Enter the V8 Context
                CefRefPtr<CefV8Context> context = browser.get()->GetFrame(frameIdentifiers[i]).get()->GetV8Context();
                context.get()->Enter();

                //Get all <video> elements
                CefRefPtr<CefV8Value> returnVal;
                CefRefPtr<CefV8Exception> exception;
                context.get()->Eval("document.getElementsByTagName('video').length;", returnVal, exception);

                if (returnVal.get() == NULL) {
                    mprisElementTagType = "";
                } else {
                    if (returnVal.get()->GetIntValue() > 0) {
                        //Video found.
                        mprisElementTagType = "video";
                    } else {
                        //No <video> elements. Get all <audio> elements
                        context.get()->Eval("document.getElementsByTagName('audio').length;", returnVal, exception);
                        if (returnVal.get()->GetIntValue() > 0) {
                            //Audio found.
                            mprisElementTagType = "audio";
                        } else {
                            mprisElementTagType = "";
                        }
                    }
                }

                if (mprisElementTagType != "") {
                    this->videoContext = context;
                    this->videoFrame = browser.get()->GetFrame(frameIdentifiers[i]);

                    CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("mprisStart");
                    browser.get()->SendProcessMessage(PID_BROWSER, message);

                    //Get if the video is paused
                    context.get()->Eval("document.getElementsByTagName('" + mprisElementTagType.toStdString() + "')[0].paused", returnVal, exception);
                    videoPlaying = !returnVal.get()->GetBoolValue();

                    CefString title, artist, album;

                    //Provide extra information for videos from youtube.com
                    if (QUrl(QString::fromStdString(videoFrame.get()->GetURL().ToString())).host().startsWith("www.youtube.com")) {
                        context.get()->Eval("document.getElementById('eow-title').innerHTML", returnVal, exception);
                        if (returnVal.get() != NULL) {
                            title = QString::fromStdString(returnVal.get()->GetStringValue().ToString()).trimmed().replace("&amp;", "&")
                                    .replace("&lt;", "<").replace("&gt;", ">").toStdString();
                        }

                        context.get()->Eval("document.getElementById('watch7-user-header').childNodes[3].childNodes[1].innerHTML", returnVal, exception);
                        if (returnVal.get() != NULL) {
                            artist = QString::fromStdString(returnVal.get()->GetStringValue().ToString()).replace("&amp;", "&")
                                    .replace("&lt;", "<").replace("&gt;", ">").toStdString();
                        }

                        album = QUrl(QString::fromStdString(videoFrame.get()->GetURL().ToString())).host().toStdString();
                    }

                    {
                        CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("mprisData");
                        message.get()->GetArgumentList().get()->SetBool(0, videoPlaying);
                        message.get()->GetArgumentList().get()->SetString(1, title);
                        message.get()->GetArgumentList().get()->SetString(2, artist);
                        message.get()->GetArgumentList().get()->SetString(3, album);
                        browser.get()->SendProcessMessage(PID_BROWSER, message);
                    }
                } else {
                    CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("mprisStop");
                    message.get()->GetArgumentList().get()->SetBool(0, browser.get()->GetFrame(frameIdentifiers[i]).get()->IsMain());
                    browser.get()->SendProcessMessage(PID_BROWSER, message);
                    this->videoFrame = NULL;
                }

                //Exit the V8 Context
                context.get()->Exit();
            }
        }
    } else if (message.get()->GetName() == "mprisPlayPause") {
        //Enter the V8 Context
        CefRefPtr<CefV8Context> context = videoFrame.get()->GetV8Context();
        context.get()->Enter();

        CefRefPtr<CefV8Value> returnVal;
        CefRefPtr<CefV8Exception> exception;
        if (videoPlaying) {
            //The media is playing, pause the media.
            context.get()->Eval("document.getElementsByTagName('" + mprisElementTagType.toStdString() + "')[0].pause()", returnVal, exception);
        } else {
            //The media is paused, play the media.
            context.get()->Eval("document.getElementsByTagName('" + mprisElementTagType.toStdString() + "')[0].play()", returnVal, exception);
        }

        //Exit the V8 Context
        context.get()->Exit();

        //Do MPRIS Check
        CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("mprisCheck");
        browser.get()->SendProcessMessage(PID_RENDERER, message);
    } else if (message.get()->GetName() == "mprisPlay") {
        if (!videoPlaying) {
            //Enter the V8 Context
            CefRefPtr<CefV8Context> context = videoFrame.get()->GetV8Context();
            context.get()->Enter();

            //The media is paused, play the media.
            CefRefPtr<CefV8Value> returnVal;
            CefRefPtr<CefV8Exception> exception;
            context.get()->Eval("document.getElementsByTagName('" + mprisElementTagType.toStdString() + "')[0].play()", returnVal, exception);

            //Exit the V8 Context
            context.get()->Exit();
        }

        //Do MPRIS Check
        CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("mprisCheck");
        browser.get()->SendProcessMessage(PID_RENDERER, message);
    } else if (message.get()->GetName() == "mprisPause") {
        if (videoPlaying) {
            //Enter the V8 Context
            CefRefPtr<CefV8Context> context = videoFrame.get()->GetV8Context();
            context.get()->Enter();

            //The media is playing, pause the media.
            CefRefPtr<CefV8Value> returnVal;
            CefRefPtr<CefV8Exception> exception;
            context.get()->Eval("document.getElementsByTagName('" + mprisElementTagType.toStdString() + "')[0].pause()", returnVal, exception);

            //Exit the V8 Context
            context.get()->Exit();
        }

        //Do MPRIS Check
        CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("mprisCheck");
        browser.get()->SendProcessMessage(PID_RENDERER, message);
    } else if (message.get()->GetName() == "mprisBack") {
        //Enter the V8 Context
        CefRefPtr<CefV8Context> context = videoFrame.get()->GetV8Context();
        context.get()->Enter();

        //Set the media to 0 seconds
        CefRefPtr<CefV8Value> returnVal;
        CefRefPtr<CefV8Exception> exception;
        context.get()->Eval("document.getElementsByTagName('" + mprisElementTagType.toStdString() + "')[0].currentTime = 0", returnVal, exception);

        //Exit the V8 Context
        context.get()->Exit();

        //Do MPRIS Check
        CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("mprisCheck");
        browser.get()->SendProcessMessage(PID_RENDERER, message);
    }
    return true;
}
