#include "downloadimagecallback.h"

DownloadImageCallback::DownloadImageCallback(std::function<void (CefRefPtr<CefImage>)> func)
{
    this->callbackFunction = func;
}

void DownloadImageCallback::OnDownloadImageFinished(const CefString &image_url, int http_status_code, CefRefPtr<CefImage> image) {
    if (http_status_code == 200) {
        this->callbackFunction(image);
    }
}

void DownloadImageCallback::AddRef() const {
    CefRefCount::AddRef();
}

bool DownloadImageCallback::Release() const {
    return CefRefCount::Release();
}

bool DownloadImageCallback::HasOneRef() const {
    return CefRefCount::HasOneRef();
}
