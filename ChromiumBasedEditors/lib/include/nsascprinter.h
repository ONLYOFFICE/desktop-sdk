#ifndef NSASCPRINTER_H
#define NSASCPRINTER_H

#include "./applicationmanager.h"
#import <Cocoa/Cocoa.h>

class NSAscPrinterInfo
{
public:
    double  m_dDpiX;
    double  m_dDpiY;
    
    int     m_nMarginLeft;
    int     m_nMarginRight;
    int     m_nMarginTop;
    int     m_nMarginBottom;
    
    int     m_nPaperWidth;
    int     m_nPaperHeight;
    
    int     m_nPagesCount;
    
    int     m_nPaperWidthOrigin;
    int     m_nPaperHeightOrigin;
    
    NSEditorApi::CAscPrinterContextBase* m_pContext;
    
public:
    NSAscPrinterInfo()
    {
        m_dDpiX = 72.0;
        m_dDpiY = 72.0;
        
        m_nPaperWidth   = 1000;
        m_nPaperHeight  = 1000;
        
        m_nMarginLeft   = 0;
        m_nMarginRight  = 0;
        m_nMarginTop    = 0;
        m_nMarginBottom = 0;
        
        m_nPagesCount   = 0;
        m_pContext      = NULL;
        
        m_nPaperWidthOrigin = 0;
        m_nPaperHeightOrigin = 0;
    }
};

@interface NSAscPrintView : NSView
{
    CAscApplicationManager* m_pManager;
    CCefView*               m_pView;
    NSAscPrinterInfo*       m_pPrinterInfo;
    
    std::vector<bool>       m_arOrientation; // true - Horizontal
}

- (id) initWithParams:(CGRect)frame manager:(CAscApplicationManager*)_manager viewId:(int)_viewId info:(NSAscPrinterInfo*)_info;
- (void) fillInfo;
@end

@implementation NSAscPrintView

- (id) initWithParams:(CGRect)frame manager:(CAscApplicationManager*)_manager viewId:(int)_viewId info:(NSAscPrinterInfo*)_info;
{
    self = [super initWithFrame:frame];
    if (self)
    {
        m_pManager      = _manager;
        m_pView         = m_pManager->GetViewById(_viewId);
        m_pPrinterInfo  = _info;
    }
    return self;
}

- (void) drawRect:(NSRect)dirtyRect
{
    m_pPrinterInfo->m_pContext->AddRef();
    
    NSEditorApi::CAscPrintPage* pData = new NSEditorApi::CAscPrintPage();
    pData->put_Context(m_pPrinterInfo->m_pContext);
    
    int nPage = (int)[[NSPrintOperation currentOperation] currentPage];
    pData->put_Page((nPage - 1));
    
    NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent();
    pEvent->m_nType = ASC_MENU_EVENT_TYPE_CEF_PRINT_PAGE;
    pEvent->m_pData = pData;
    
    m_pView->Apply(pEvent);
}

- (BOOL)knowsPageRange:(NSRangePointer)range
{
    NSPrintInfo* pInfo = [[NSPrintOperation currentOperation] printInfo];
    [pInfo setHorizontallyCentered:NO];
    [pInfo setVerticallyCentered:NO];
    [pInfo setHorizontalPagination:NSAutoPagination];
    [pInfo setVerticalPagination:NSAutoPagination];
    [pInfo setLeftMargin:0];
    [pInfo setRightMargin:0];
    [pInfo setTopMargin:0];
    [pInfo setBottomMargin:0];
    
    // узнаем ориентацию
    [self fillInfo];
    m_arOrientation.clear();
    for (int nPage = 0; nPage < m_pPrinterInfo->m_nPagesCount; ++nPage)
    {
        NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_PRINT_PAGE_CHECK);
        NSEditorApi::CAscPrintPage* pData = new NSEditorApi::CAscPrintPage();
        pData->put_Context(m_pPrinterInfo->m_pContext);
        m_pPrinterInfo->m_pContext->AddRef();
    
        pData->put_Page(nPage);
    
        pEvent->m_pData = pData;
        m_pView->Apply(pEvent);
        
        m_arOrientation.push_back(pData->get_IsRotate());
        
        pEvent->Release();
    }
    
    range->location = 1;
    range->length   = m_pPrinterInfo->m_nPagesCount;
    
    if (0 < m_pPrinterInfo->m_nPagesCount)
    {
        [pInfo setOrientation: (m_arOrientation[0]) ? NSPaperOrientationLandscape : NSPaperOrientationPortrait];
    }
    
    return YES;
}

- (void) fillInfo
{
    NSPrintInfo* pInfo = [[NSPrintOperation currentOperation] printInfo];
    
    PMPrintSession _session     = (PMPrintSession)[pInfo PMPrintSession];
    PMPrintSettings _settings   = (PMPrintSettings)[pInfo PMPrintSettings];
    
    PMPrinter _printer;
    OSStatus err = PMSessionGetCurrentPrinter(_session, &_printer);
    
    PMResolution _resolution;
    err = PMPrinterGetOutputResolution(_printer, _settings, &_resolution);
    if (err == 0)
    {
        m_pPrinterInfo->m_dDpiX = _resolution.hRes;
        m_pPrinterInfo->m_dDpiY = _resolution.vRes;
    }
    else
    {
        UInt32 _res_count = 0;
        PMPrinterGetPrinterResolutionCount(_printer, &_res_count);
        
        PMResolution* pResolutions = (PMResolution*)malloc(_res_count * sizeof(PMResolution));
        
        for (UInt32 i = 1; i <= _res_count; ++i)
        {
            PMPrinterGetIndexedPrinterResolution(_printer, i, pResolutions + i - 1);
        }
        
        m_pPrinterInfo->m_dDpiX = pResolutions[0].hRes;
        m_pPrinterInfo->m_dDpiY = pResolutions[0].vRes;
        
        free(pResolutions);
    }
    
    double dKoefX = m_pPrinterInfo->m_dDpiX / 72.0;
    double dKoefY = m_pPrinterInfo->m_dDpiX / 72.0;
    
    m_pPrinterInfo->m_nPaperWidthOrigin = (int)pInfo.paperSize.width;
    m_pPrinterInfo->m_nPaperHeightOrigin = (int)pInfo.paperSize.height;
    
    m_pPrinterInfo->m_nPaperWidth = (int)(dKoefX * pInfo.paperSize.width);
    m_pPrinterInfo->m_nPaperHeight = (int)(dKoefY * pInfo.paperSize.height);
    
    NSRect _rectMargins = pInfo.imageablePageBounds;
    
    m_pPrinterInfo->m_nMarginLeft = (int)(dKoefX * _rectMargins.origin.x);
    m_pPrinterInfo->m_nMarginRight = (int)(dKoefX * (m_pPrinterInfo->m_nPaperWidthOrigin - (_rectMargins.origin.x + _rectMargins.size.width)));
    m_pPrinterInfo->m_nMarginTop = (int)(dKoefY * (m_pPrinterInfo->m_nPaperHeightOrigin - (_rectMargins.origin.y + _rectMargins.size.height)));
    m_pPrinterInfo->m_nMarginBottom = (int)(dKoefY * _rectMargins.origin.y);
}

- (NSRect)rectForPage:(NSInteger)page
{
    [self fillInfo];
    
    int nPage = (int)(page - 1);
    if (nPage < m_pPrinterInfo->m_nPagesCount)
    {
        NSPrintInfo* pInfo = [[NSPrintOperation currentOperation] printInfo];
        [pInfo setOrientation: (m_arOrientation[nPage]) ? NSPaperOrientationLandscape : NSPaperOrientationPortrait];
    }
    
    NSRect R = NSMakeRect( 0, 0, m_pPrinterInfo->m_nPaperWidth, m_pPrinterInfo->m_nPaperHeight );
    [self setFrame: R];
    return R;
}

@end


class NSAscPrinterContext : public NSEditorApi::CAscPrinterContextBase
{
    CAscApplicationManager* m_pManager;
    NSAscPrinterInfo m_oInfo;
    NSView* m_pView;
    CCefView* m_pParent;

public:
    NSAscPrinterContext(CAscApplicationManager* pManager) : NSEditorApi::CAscPrinterContextBase()
    {
        m_pManager = pManager;
        m_pView = nil;
        m_pParent = NULL;
    }

    void BeginPaint(int nViewId, int nPagesCount, id sender, SEL didRunSelector)
    {
        m_oInfo.m_nPagesCount = nPagesCount;
        m_oInfo.m_pContext = this;
        m_pView = [[NSAscPrintView alloc] initWithParams:NSMakeRect(0, 0, 100, 100) manager:m_pManager viewId:nViewId info:&m_oInfo];
        m_pParent = m_pManager->GetViewById(nViewId);
        
        NSView* pViewParent = (__bridge NSView*)m_pParent->GetWidgetImpl()->parent_wid();
        
        // start print dialog
        NSPrintInfo* pPrintInfo;
        NSString* sKeyPrint = @"prihord"; // Ключ в котором расположен объект NSPrintInfo
        
        // Пробуем извлечь
        NSUserDefaults* pUserDef = [[NSUserDefaults alloc] init];
        NSData* pDataInfo = [pUserDef dataForKey:sKeyPrint];
        if (pDataInfo)
        {
            // Если удачно извлекли
            pPrintInfo = [NSUnarchiver unarchiveObjectWithData:pDataInfo];
        }
        else
        {
#if 0
            // Иначе выводим панель настройки печати
            NSPageLayout* pPageLayout = [[NSPageLayout alloc] init];
            if ([pPageLayout runModal] == NSModalResponseOK)
            {
                pPrintInfo = pPageLayout.printInfo;
                
                // сохраним введенные настройки в User Defaults
                pDataInfo = [NSArchiver archivedDataWithRootObject:pPrintInfo];
                [pUserDef setObject:pDataInfo forKey:sKeyPrint];
            }
#endif
        }
        
        NSPrintPanelOptions options = NSPrintPanelShowsCopies;
        options |= NSPrintPanelShowsPageRange;
        options |= NSPrintPanelShowsPaperSize;
        options |= NSPrintPanelShowsPrintSelection;
        options |= NSPrintPanelShowsPreview;
        
        NSPrintOperation* pro;
        if (pPrintInfo)
            pro = [NSPrintOperation printOperationWithView:m_pView printInfo:pPrintInfo];
        else
            pro = [NSPrintOperation printOperationWithView:m_pView];
        
        [pro setShowsPrintPanel:YES]; // Выводим на печать или предпросмотр
        [[pro printPanel] setOptions:options];
        //[pro runOperation];
        [pro runOperationModalForWindow:pViewParent.window delegate:sender didRunSelector:didRunSelector contextInfo:nil];
    }
    void EndPaint()
    {
        NSEditorApi::CAscMenuEvent* pEventEnd = new NSEditorApi::CAscMenuEvent();
        pEventEnd->m_nType = ASC_MENU_EVENT_TYPE_CEF_PRINT_END;

        m_pParent->Apply(pEventEnd);

        m_pView = nil;
        this->Release();
    }
    
    virtual void GetLogicalDPI(int& nDpiX, int& nDpiY)
    {
        nDpiX = (int)m_oInfo.m_dDpiX;
        nDpiY = (int)m_oInfo.m_dDpiY;
    }

    virtual void GetPhysicalRect(int& nX, int& nY, int& nW, int& nH)
    {
        nX = m_oInfo.m_nMarginLeft;
        nY = m_oInfo.m_nMarginTop;
        nW = m_oInfo.m_nPaperWidth;
        nH = m_oInfo.m_nPaperHeight;
    }

    virtual void GetPrintAreaSize(int& nW, int& nH)
    {
        nW = m_oInfo.m_nPaperWidth - m_oInfo.m_nMarginLeft - m_oInfo.m_nMarginRight;
        nH = m_oInfo.m_nPaperHeight - m_oInfo.m_nMarginTop - m_oInfo.m_nMarginBottom;
    }

    virtual void BitBlt(unsigned char* pBGRA, const int& nRasterX, const int& nRasterY, const int& nRasterW, const int& nRasterH,
                        const double& x, const double& y, const double& w, const double& h, const double& dAngle)
    {
        
        
        CGColorSpaceRef cs = CGColorSpaceCreateDeviceRGB();
        CGDataProviderRef pDataProvider = CGDataProviderCreateWithData(NULL, pBGRA, 4 * nRasterW * nRasterH, NULL);
        
        CGImageRef pCgImage = CGImageCreate(nRasterW, nRasterH, 8, 32, 4 * nRasterW, cs,
                                            kCGImageAlphaPremultipliedLast, pDataProvider,
                                            NULL, false, kCGRenderingIntentAbsoluteColorimetric);
        
        CGColorSpaceRelease(cs);
        CGDataProviderRelease(pDataProvider);
        
        // здесь никаких поворотов - разруливаем все раньше
        CGFloat fX = (CGFloat)((x + m_oInfo.m_nMarginLeft) * m_oInfo.m_nPaperWidthOrigin / m_oInfo.m_nPaperWidth);
        CGFloat fY = (CGFloat)((y + m_oInfo.m_nMarginTop) * m_oInfo.m_nPaperHeightOrigin / m_oInfo.m_nPaperHeight);
        CGFloat fW = (CGFloat)(nRasterW * m_oInfo.m_nPaperWidthOrigin / m_oInfo.m_nPaperWidth);
        CGFloat fH = (CGFloat)(nRasterH * m_oInfo.m_nPaperHeightOrigin / m_oInfo.m_nPaperHeight);
        fY = (m_oInfo.m_nPaperHeightOrigin - fY - fH);
        
        CGContextRef _context = (CGContextRef)[[NSGraphicsContext currentContext] graphicsPort];
        
        CGAffineTransform orig_cg_matrix = CGContextGetCTM(_context);
        CGAffineTransform orig_cg_matrix_inv = CGAffineTransformInvert(orig_cg_matrix);
        
        orig_cg_matrix.a = fabs(orig_cg_matrix.a);
        orig_cg_matrix.d = fabs(orig_cg_matrix.d);
        orig_cg_matrix.tx = 0;
        orig_cg_matrix.ty = 0;
            
        CGContextSaveGState(_context);
            
        CGContextConcatCTM(_context, orig_cg_matrix_inv);
        CGContextConcatCTM(_context, orig_cg_matrix);
        
        CGRect _rect = NSMakeRect(fX, fY, fW, fH);
        CGContextDrawImage(_context, _rect, pCgImage);
            
        CGContextRestoreGState(_context);
        
        CGImageRelease(pCgImage);
    }
};

#endif  // NSASCPRINTER_H
