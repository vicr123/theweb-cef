#ifndef DOWNLOADIMAGECALLBACK_H
#define DOWNLOADIMAGECALLBACK_H

#include <cefheaders.h>
#include <functional>

class DownloadImageCallback : public CefDownloadImageCallback, public CefRefCount
{
public:
    DownloadImageCallback(std::function<void(CefRefPtr<CefImage>)> func);

    void AddRef() const;
    bool Release() const;
    bool HasOneRef() const;

    void OnDownloadImageFinished(const CefString& image_url, int http_status_code, CefRefPtr<CefImage> image);

private:
    std::function<void(CefRefPtr<CefImage>)> callbackFunction;
};

#endif // DOWNLOADIMAGECALLBACK_H
