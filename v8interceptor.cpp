#include "v8interceptor.h"

V8Interceptor::V8Interceptor()
{

}

void V8Interceptor::AddRef() const {
    CefRefCount::AddRef();
}

bool V8Interceptor::Release() const {
    return CefRefCount::Release();
}

bool V8Interceptor::HasOneRef() const {
    return CefRefCount::HasOneRef();
}

bool V8Interceptor::Set(const CefString &name, const CefRefPtr<CefV8Value> object, const CefRefPtr<CefV8Value> value, CefString &exception) {
    return false;
}

bool V8Interceptor::Set(int index, const CefRefPtr<CefV8Value> object, const CefRefPtr<CefV8Value> value, CefString &exception) {
    return false;
}

bool V8Interceptor::Get(const CefString &name, const CefRefPtr<CefV8Value> object, CefRefPtr<CefV8Value> &retval, CefString &exception) {
    return false;
}

bool V8Interceptor::Get(int index, const CefRefPtr<CefV8Value> object, CefRefPtr<CefV8Value> &retval, CefString &exception) {
    return false;
}
