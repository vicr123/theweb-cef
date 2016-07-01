#include "cefengine.h"

CefEngine::CefEngine(QObject *parent) : QObject(parent)
{

}

void CefEngine::AddRef() const {

}

bool CefEngine::Release() const {
    return true;
}

bool CefEngine::HasOneRef() const {
    return true;
}
