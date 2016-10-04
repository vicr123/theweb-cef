#include "thewebsettingsaccessor.h"

extern QVariantMap settingsData;

theWebSettingsAccessor::theWebSettingsAccessor(Browser browser)
{
    this->associatedBrowser = browser;

    //Update browser settings
    CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("theWebSettings_get");
    browser.get()->SendProcessMessage(PID_BROWSER, message);
}

theWebSettingsAccessor::~theWebSettingsAccessor() {


}

void theWebSettingsAccessor::sync() {

}

bool theWebSettingsAccessor::Set(const CefString &name, const CefRefPtr<CefV8Value> object, const CefRefPtr<CefV8Value> value, CefString &exception) {
    if (name == "dnt") {
        if (value.get()->IsBool()) {
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("theWebSettings");
            CefRefPtr<CefListValue> args = message.get()->GetArgumentList();
            args.get()->SetString(0, "data/dnt");
            args.get()->SetString(1, "bool");
            args.get()->SetBool(2, value.get()->GetBoolValue());
            this->associatedBrowser.get()->SendProcessMessage(PID_BROWSER, message);

            settingsData.insert("data/dnt", value.get()->GetBoolValue());
        } else {
            exception = "Invalid Value Type";
        }
        return true;
    } else if (name == "home") {
        if (value.get()->IsString()) {
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("theWebSettings");
            CefRefPtr<CefListValue> args = message.get()->GetArgumentList();
            args.get()->SetString(0, "browser/home");
            args.get()->SetString(1, "string");
            args.get()->SetString(2, value.get()->GetStringValue());
            this->associatedBrowser.get()->SendProcessMessage(PID_BROWSER, message);

            settingsData.insert("browser/home", QString::fromStdString(value.get()->GetStringValue().ToString()));
        } else {
            exception = "Invalid Value Type";
        }
        return true;
    } else if (name == "toolbar") {
        if (value.get()->IsBool()) {
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("theWebSettings");
            CefRefPtr<CefListValue> args = message.get()->GetArgumentList();
            args.get()->SetString(0, "browser/toolbarOnBottom");
            args.get()->SetString(1, "bool");
            args.get()->SetBool(2, value.get()->GetBoolValue());
            this->associatedBrowser.get()->SendProcessMessage(PID_BROWSER, message);

            settingsData.insert("browser/toolbarOnBottom", value.get()->GetBoolValue());

            SendReloadSettings();
        } else {
            exception = "Invalid Value Type";
        }
        return true;
    } else if (name == "tabText") {
        if (value.get()->IsBool()) {
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("theWebSettings");
            CefRefPtr<CefListValue> args = message.get()->GetArgumentList();
            args.get()->SetString(0, "browser/tabText");
            args.get()->SetString(1, "bool");
            args.get()->SetBool(2, value.get()->GetBoolValue());
            this->associatedBrowser.get()->SendProcessMessage(PID_BROWSER, message);

            settingsData.insert("browser/tabText", value.get()->GetBoolValue());

            SendReloadSettings();
        } else {
            exception = "Invalid Value Type";
        }
        return true;
    } else if (name == "malwareProtect") {
        if (value.get()->IsBool()) {
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("theWebSettings");
            CefRefPtr<CefListValue> args = message.get()->GetArgumentList();
            args.get()->SetString(0, "data/malwareProtect");
            args.get()->SetString(1, "bool");
            args.get()->SetBool(2, value.get()->GetBoolValue());
            this->associatedBrowser.get()->SendProcessMessage(PID_BROWSER, message);

            settingsData.insert("data/malwareProtect", value.get()->GetBoolValue());

            SendReloadSettings();
        } else {
            exception = "Invalid Value Type";
        }
        return true;
    } else if (name == "tabPreview") {
        if (value.get()->IsBool()) {
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("theWebSettings");
            CefRefPtr<CefListValue> args = message.get()->GetArgumentList();
            args.get()->SetString(0, "behaviour/tabPreview");
            args.get()->SetString(1, "bool");
            args.get()->SetBool(2, value.get()->GetBoolValue());
            this->associatedBrowser.get()->SendProcessMessage(PID_BROWSER, message);

            settingsData.insert("behaviour/tabPreview", value.get()->GetBoolValue());

            SendReloadSettings();
        } else {
            exception = "Invalid Value Type";
        }
        return true;
    }
    return false;
}

bool theWebSettingsAccessor::Get(const CefString &name, const CefRefPtr<CefV8Value> object, CefRefPtr<CefV8Value> &retval, CefString &exception) {
    if (name == "dnt") {
        retval = CefV8Value::CreateBool(settingsData.value("data/dnt", false).toBool());
        return true;
    } else if (name == "home") {
        retval = CefV8Value::CreateString(settingsData.value("browser/home", "theweb://newtab").toString().toStdString());
        return true;
    } else if (name == "toolbar") {
        retval = CefV8Value::CreateBool(settingsData.value("browser/toolbarOnBottom", false).toBool());
        return true;
    } else if (name == "tabText") {
        retval = CefV8Value::CreateBool(settingsData.value("browser/tabText", false).toBool());
        return true;
    } else if (name == "malwareProtect") {
        retval = CefV8Value::CreateBool(settingsData.value("data/malwareProtect", true).toBool());
        return true;
    } else if (name == "tabPreview") {
        retval = CefV8Value::CreateBool(settingsData.value("behaviour/tabPreview", false).toBool());
        return true;
    }
    return false;
}

void theWebSettingsAccessor::SendReloadSettings() {
    CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("ReloadSettings");
    this->associatedBrowser.get()->SendProcessMessage(PID_BROWSER, message);
}

void theWebSettingsAccessor::AddRef() const {
    CefRefCount::AddRef();
}

bool theWebSettingsAccessor::Release() const {
    return CefRefCount::Release();
}

bool theWebSettingsAccessor::HasOneRef() const {
    return CefRefCount::HasOneRef();
}

V8Function::V8Function(std::function<void (CefV8ValueList &)> function) {
    this->function = function;
    hasArguments = true;
    returnsValue = false;
}

V8Function::V8Function(std::function<void()> function) {
    this->noArgFunction = function;
    hasArguments = false;
    returnsValue = false;
}

V8Function::V8Function(std::function<CefRefPtr<CefV8Value>()> function) {
    this->returnFunction = function;
    hasArguments = false;
    returnsValue = true;
}

bool V8Function::Execute(const CefString &name, CefRefPtr<CefV8Value> object, const CefV8ValueList &arguments, CefRefPtr<CefV8Value> &retval, CefString &exception) {
    try {
        if (returnsValue) {
            if (hasArguments) {
                //TODO: Create handler for function that returns a value with arguments
            } else {
                retval = returnFunction();
            }
        } else {
            if (hasArguments) {
                function(const_cast<CefV8ValueList&>(arguments));
            } else {
                noArgFunction();
            }
        }
        return true;
    } catch (...) {
        exception = "Error";
        return true;
    }
}
