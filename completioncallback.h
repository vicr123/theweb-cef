#ifndef COMPLETIONCALLBACK_H
#define COMPLETIONCALLBACK_H

#include "cefheaders.h"
#include "cefengine.h"

class CompletionCallback : public CefCompletionCallback, public CefEngine
{
public:
    CompletionCallback();

    void OnComplete();
};

#endif // COMPLETIONCALLBACK_H
