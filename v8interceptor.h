#ifndef V8INTERCEPTOR_H
#define V8INTERCEPTOR_H

#include "cefheaders.h"

class V8Interceptor : public CefV8Interceptor, public CefRefCount
{
public:
    V8Interceptor();

    void AddRef() const;
    bool Release() const;
    bool HasOneRef() const;

    bool Set(const CefString &name, const CefRefPtr<CefV8Value> object, const CefRefPtr<CefV8Value> value, CefString &exception) override;
    bool Get(const CefString &name, const CefRefPtr<CefV8Value> object, CefRefPtr<CefV8Value> &retval, CefString &exception) override;
    bool Get(int index, const CefRefPtr<CefV8Value> object, CefRefPtr<CefV8Value>& retval, CefString& exception) override;
    bool Set(int index, const CefRefPtr<CefV8Value> object, const CefRefPtr<CefV8Value> value, CefString& exception) override;
};

#endif // V8INTERCEPTOR_H
