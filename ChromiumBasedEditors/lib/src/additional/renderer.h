#ifndef RENDERER_ADDITIONAL_H
#define RENDERER_ADDITIONAL_H

#include "../cefwrapper/client_renderer.h"

class CApplicationRendererAdditionalBase
{
public:
    virtual bool OnProcessMessageReceived(
        CefRefPtr<client::ClientAppRenderer> app,
        CefRefPtr<CefBrowser> browser,
        CefProcessId source_process,
        CefRefPtr<CefProcessMessage> message)
    {
        return false;
    }
};

CApplicationRendererAdditionalBase* Create_ApplicationRendererAdditional();

#endif // RENDERER_ADDITIONAL_H
