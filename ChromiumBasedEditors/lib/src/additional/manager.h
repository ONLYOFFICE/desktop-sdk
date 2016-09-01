#ifndef APPLICATION_MANAGER_ADDITIONAL_H
#define APPLICATION_MANAGER_ADDITIONAL_H

#include "../../include/applicationmanager_events.h"
#include "./renderer.h"
#include "../../../../../core/DesktopEditor/graphics/GraphicsRenderer.h"
#include "./../../include/applicationmanager.h"

class CApplicationManagerAdditionalBase
{
public:
    std::vector<NSEditorApi::CAscMenuEvent*>* m_arApplyEvents;

public:
    CApplicationManagerAdditionalBase(CAscApplicationManager* pManager)
    {
        m_arApplyEvents = NULL;
    }
    virtual ~CApplicationManagerAdditionalBase()
    {
    }

public:
    virtual bool GetEditorPermission() { return true; }

    virtual void Apply(NSEditorApi::CAscMenuEvent* pEvent) {}
    virtual void ApplyView(CCefView* pView, NSEditorApi::CAscMenuEvent* pEvent, CefRefPtr<CefBrowser> browser) {}

    virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                          CefProcessId source_process,
                                          CefRefPtr<CefProcessMessage> message)
    {
        return false;
    }

    virtual void CheckSaveStart(std::wstring sDirectory, int nFileType) {}
    virtual void CheckSaveEnd() {}

    virtual void Print_Start() {}
    virtual void Print_End() {}
    virtual void Check_Print(CGraphicsRenderer* pRenderer, CFontManager* pFontManager, int nRasterW, int nRasterH, double fPageWidth, double fPageHeight) {}

    virtual void Local_Save_Start() {}
    virtual bool Local_Save_End(bool bIsNeedSaveDialog, int nId, CefRefPtr<CefBrowser> browser) { return false; }
};

CApplicationManagerAdditionalBase* Create_ApplicationManagerAdditional(CAscApplicationManager* pManager);

#endif // APPLICATION_MANAGER_ADDITIONAL_H
