#include "./../include/qrenderer.h"
#include "./../src/painting_conversions.h" // internal things to qt
#include "./../../include/qascprinter.h"

// DEBUG THINGS
#include <algorithm>
#include <iostream>

//#define ENABLE_LOGS

#ifdef ENABLE_LOGS
#include <QDebug>
#define TELL qDebug() << __func__
#endif

namespace
{
    QImage getQImage(const Aggplus::CImage &internal)
    {
        int width = (int)internal.GetWidth();
        int height = (int)internal.GetHeight();

        QImage result{width, height, QImage::Format::Format_ARGB32};

        DWORD size = internal.GetWidth()*internal.GetHeight();
        const BYTE *data = internal.GetData();
        data += 4*(size - 1);

        for (int lineNo = 0; lineNo < height; ++lineNo)
        {
            uchar *line = result.scanLine(lineNo);
            for (int pxNo = 0; pxNo < width; ++pxNo)
            {
                QRgb &px = *(QRgb*)line;
                quint32 bgra = *(quint32*)data;
                px = bgra;
                line += 4;
                data -= 4;
            }
        }

        return result;
    }

    template<typename T>
    QString toBin(T int_value)
    {
        QString result;
        for (std::size_t i = 0; i < sizeof(T) * 8; ++i)
        {
            if (i && 0 == i % 8)
                result += ' ';

            result += ((int_value >> i) & (T)1) ? '1' : '0';
        }
        std::reverse(result.begin(), result.end());
        return result;
    }

    // constants
    //// pen -----------------------------------------------------------
    //const long c_ag_LineCapFlat = 0;
    //const long c_ag_LineCapSquare = 1;
    //const long c_ag_LineCapTriangle = 3;
    //const long c_ag_LineCapNoAnchor = 16;
    //const long c_ag_LineCapSquareAnchor = 17;
    //const long c_ag_LineCapRoundAnchor = 18;
    //const long c_ag_LineCapDiamondAnchor = 19;
    //const long c_ag_LineCapArrowAnchor = 20;
    //const long c_ag_LineCapAnchorMask = 240;
    //const long c_ag_LineCapCustom = 255;
    QString getLineCap(long val)
    {
        switch (val)
        {
        case 0: return "c_ag_LineCapFlat";
        case 1: return "c_ag_LineCapSquare";
        case 3: return "c_ag_LineCapTriangle";
        case 16: return "c_ag_LineCapNoAnchor";
        case 17: return "c_ag_LineCapSquareAnchor";
        case 18: return "c_ag_LineCapRoundAnchor";
        case 19: return "c_ag_LineCapDiamondAnchor";
        case 20: return "c_ag_LineCapArrowAnchor";
        case 240: return "c_ag_LineCapAnchorMask";
        case 255: return "c_ag_LineCapCustom";
        default: return "unknown line cap";
        }
    }

    //const long c_ag_DashCapFlat = 0;
    //const long c_ag_DashCapRound = 2;
    //const long c_ag_DashCapTriangle = 3;
    QString getDashCap(long val) // UNUSED
    {
        switch (val)
        {
        case 0: return "c_ag_DashCapFlat";
        case 2: return "c_ag_DashCapRound";
        case 3: return "c_ag_DashCapTriangle";
        default: return "unknown dash cap";
        }
    }

    //const long c_ag_LineJoinMiter = 0;
    //const long c_ag_LineJoinBevel = 1;
    //const long c_ag_LineJoinRound = 2;
    //const long c_ag_LineJoinMiterClipped = 3;
    QString getLineJoin(long val)
    {
        switch (val)
        {
        case 0: return "c_ag_LineJoinMiter";
        case 1: return "c_ag_LineJoinBevel";
        case 2: return "c_ag_LineJoinRound";
        case 3: return "c_ag_LineJoinMiterClipped";
        default: return "unknown line join";
        }
    }

    //const long c_ag_PenAlignmentCenter = 0;
    //const long c_ag_PenAlignmentInset = 1;
    //const long c_ag_PenAlignmentOutset = 2;
    //const long c_ag_PenAlignmentLeft = 3;
    //const long c_ag_PenAlignmentRight = 4;
    QString getPenAlignment(long val) // UNUSED
    {
        switch (val)
        {
        case 0: return "c_ag_PenAlignmentCenter";
        case 1: return "c_ag_PenAlignmentInset";
        case 2: return "c_ag_PenAlignmentOutset";
        case 3: return "c_ag_PenAlignmentLeft";
        case 4: return "c_ag_PenAlignmentRight";
        default: return "unknown alignment";
        }
    }
    //// --------------------------------------------------------------
    //// brush --------------------------------------------------------
    //// old constants for brush type
    //const long c_BrushTypeSolid_ = 0;
    //const long c_BrushTypeHorizontal_ = 1;
    //const long c_BrushTypeVertical_ = 2;
    //const long c_BrushTypeDiagonal1_ = 3;
    //const long c_BrushTypeDiagonal2_ = 4;
    //const long c_BrushTypeCenter_ = 5;
    //const long c_BrushTypePathGradient1_ = 6;
    //const long c_BrushTypePathGradient2_ = 7;
    //const long c_BrushTypeTexture_ = 8;
    //const long c_BrushTypeHatch1_ = 9;
    //const long c_BrushTypeHatch53_ = 61;
    //const long c_BrushTypeGradient1_ = 62;
    //const long c_BrushTypeGradient6_ = 70;

    //const long c_BrushTypeSolid = 1000;
    //const long c_BrushTypeHorizontal = 2001;
    //const long c_BrushTypeVertical = 2002;
    //const long c_BrushTypeDiagonal1 = 2003;
    //const long c_BrushTypeDiagonal2 = 2004;
    //const long c_BrushTypeCenter = 2005;
    //const long c_BrushTypePathGradient1 = 2006; // left for comparability
    //const long c_BrushTypePathGradient2 = 2007; // left for comparability
    //const long c_BrushTypeCylinderHor = 2008;
    //const long c_BrushTypeCylinderVer = 2009;
    //const long c_BrushTypeTexture = 3008;
    //const long c_BrushTypePattern = 3009;
    //const long c_BrushTypeHatch1 = 4009;
    //const long c_BrushTypeHatch53 = 4061;
    //const long c_BrushTypeNoFill = 5000;
    //const long c_BrushTypeNotSet = 5001;

    //const long c_BrushTypeMyTestGradient = 6000;
    //const long c_BrushTypePathRadialGradient = 6001;
    //const long c_BrushTypePathConicalGradient = 6002;
    //const long c_BrushTypePathDiamondGradient = 6003;
    //const long c_BrushTypePathNewLinearGradient = 6004;
    //const long c_BrushTypeTriagnleMeshGradient = 6005;
    //const long c_BrushTypeCurveGradient     = 6006;
    //const long c_BrushTypeTensorCurveGradient = 6007;
    QString getBrushType(long val)
    {
        switch (val)
        {
        case 0: return "c_BrushTypeSolid_ (old)";
        case 1: return "c_BrushTypeHorizontal_ (old)";
        case 2: return "c_BrushTypeVertical_ (old)";
        case 3: return "c_BrushTypeDiagonal1_ (old)";
        case 4: return "c_BrushTypeDiagonal2_ (old)";
        case 5: return "c_BrushTypeCenter_ (old)";
        case 6: return "c_BrushTypePathGradient1_ (old)";
        case 7: return "c_BrushTypePathGradient2_ (old)";
        case 8: return "c_BrushTypeTexture_ (old)";
        case 9: return "c_BrushTypeHatch1_ (old)";
        case 61: return "c_BrushTypeHatch53_ (old)";
        case 62: return "c_BrushTypeGradient1_ (old)";
        case 70: return "c_BrushTypeGradient6_ (old)";

        case 1000: return "c_BrushTypeSolid";
        case 2001: return "c_BrushTypeHorizontal";
        case 2002: return "c_BrushTypeVertical";
        case 2003: return "c_BrushTypeDiagonal1";
        case 2004: return "c_BrushTypeDiagonal2";
        case 2005: return "c_BrushTypeCenter";
        case 2006: return "c_BrushTypePathGradient1";
        case 2007: return "c_BrushTypePathGradient2";
        case 2008: return "c_BrushTypeCylinderHor";
        case 2009: return "c_BrushTypeCylinderVer";

        case 3008: return "c_BrushTypeTexture";
        case 3009: return "c_BrushTypePattern";
        case 4009: return "c_BrushTypeHatch1";
        case 4061: return "c_BrushTypeHatch53";
        case 5000: return "c_BrushTypeNoFill";
        case 5001: return "c_BrushTypeNotSet";

        case 6000: return "c_BrushTypeMyTestGradient";
        case 6001: return "c_BrushTypePathRadialGradient";
        case 6002: return "c_BrushTypePathConicalGradient";
        case 6003: return "c_BrushTypePathDiamondGradient";
        case 6004: return "c_BrushTypePathNewLinearGradient";
        case 6005: return "c_BrushTypeTriagnleMeshGradient";
        case 6006: return "c_BrushTypeCurveGradient";
        case 6007: return "c_BrushTypeTensorCurveGradient";
        default: return "unknown type";
        }
    }


    //const long c_BrushTextureModeStretch = 0;
    //const long c_BrushTextureModeTile = 1;
    //const long c_BrushTextureModeTileCenter = 2;
    QString getBrushTextureMode(long val)
    {
        switch (val)
        {
        case 0: return "c_BrushTextureModeStretch";
        case 1: return "c_BrushTextureModeTile";
        case 2: return "c_BrushTextureModeTileCenter";
        default: return "unknown mode";
        }
    }
} // anonymous namespace
// DEBUG THINGS

NSQRenderer::CQRenderer::CQRenderer(QAscPrinterContext* pContext)
{
    m_pContext = pContext;
}

NSQRenderer::CQRenderer::~CQRenderer()
{
    RELEASEINTERFACE(m_pFontManager);
    RELEASEINTERFACE(m_pAppFonts);
}

void NSQRenderer::CQRenderer::InitDefaults()
{
    m_pAppFonts = NULL;
    m_pFontManager = NULL;
    m_bIsUseTextAsPath = true;

    m_lCurrentCommand = c_nNone;
    m_oSimpleGraphicsConverter.SetRenderer(this);

    m_oInstalledFont.Size = -1;
}

void NSQRenderer::CQRenderer::SetUseTextAsPath(const bool &bIsUseTextAsPath)
{
    m_bIsUseTextAsPath = bIsUseTextAsPath;
}

void NSQRenderer::CQRenderer::SetFont()
{
    double dDpiX = 0; get_DpiX(&dDpiX);
    double dDpiY = 0; get_DpiY(&dDpiY);
    double dPix = m_oFont.CharSpace * dDpiX / 25.4;

    if (m_oInstalledFont.IsEqual(&m_oFont))
    {
        m_pFontManager->SetCharSpacing(dPix);
        return;
    }

    m_pFontManager->SetStringGID(m_oFont.StringGID);
    m_pFontManager->SetCharSpacing(dPix);

    if (m_oFont.Path.empty())
    {
        m_pFontManager->LoadFontByName(m_oFont.Name, (float)m_oFont.Size, m_oFont.GetStyle(), dDpiX, dDpiY);
    }
    else
    {
        m_pFontManager->LoadFontFromFile(m_oFont.Path, m_oFont.FaceIndex, (float)m_oFont.Size, dDpiX, dDpiY);
    }

    m_oInstalledFont = m_oFont;
}

void NSQRenderer::CQRenderer::InitFonts(NSFonts::IApplicationFonts *pFonts)
{
#ifdef ENABLE_LOGS
    TELL;
#endif

    RELEASEINTERFACE(m_pAppFonts);
    m_pAppFonts = pFonts;
    ADDREFINTERFACE(m_pAppFonts);

    RELEASEINTERFACE(m_pFontManager);
    m_pFontManager = m_pAppFonts->GenerateFontManager();
}

HRESULT NSQRenderer::CQRenderer::get_Type(LONG *lType)
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    if (lType)
    {
        *lType = c_nQRenderer;
        return S_OK;
    }
    return S_FALSE;
}

HRESULT NSQRenderer::CQRenderer::NewPage()
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    m_pContext->NewPage();
    return S_OK;
}

HRESULT NSQRenderer::CQRenderer::get_Height(double *dHeight)
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    if (dHeight)
    {
        *dHeight = m_dLogicalPageHeight;
        return S_OK;
    }
    return S_FALSE;
}

HRESULT NSQRenderer::CQRenderer::put_Height(const double &dHeight)
{
#ifdef ENABLE_LOGS
    TELL << dHeight;
#endif
    m_dLogicalPageHeight = dHeight;
    double realHeight = (double)paperSize().height();
    double scale_Y = realHeight / m_dLogicalPageHeight;
    scaleTransformSetY(scale_Y);
    return S_OK;
}

HRESULT NSQRenderer::CQRenderer::get_Width(double *dWidth)
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    if (dWidth)
    {
        *dWidth = m_dLogicalPageWidth;
        return S_OK;
    }
    return S_FALSE;
}

HRESULT NSQRenderer::CQRenderer::put_Width(const double &dWidth)
{
#ifdef ENABLE_LOGS
    TELL << dWidth;
#endif
    m_dLogicalPageWidth = dWidth;
    double realWidth = (double)paperSize().width();
    double scale_X = realWidth / m_dLogicalPageWidth;
    scaleTransformSetX(scale_X);
    return S_OK;
}

HRESULT NSQRenderer::CQRenderer::get_DpiX(double *dDpiX)
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    if (!dDpiX)
        return S_FALSE;

    int nDpiX = 0;
    int nDpiY = 0;
    m_pContext->GetLogicalDPI(nDpiX, nDpiY);
    *dDpiX = (double)nDpiX;
    return S_OK;
}

HRESULT NSQRenderer::CQRenderer::get_DpiY(double *dDpiY)
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    if (!dDpiY)
        return S_FALSE;

    int nDpiX = 0;
    int nDpiY = 0;
    m_pContext->GetLogicalDPI(nDpiX, nDpiY);
    *dDpiY = (double)nDpiY;
    return S_OK;
}

HRESULT NSQRenderer::CQRenderer::get_PenColor(LONG *lColor)
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    if (lColor)
    {
        *lColor = m_oPen.Color;
        return S_OK;
    }
    return S_FALSE;
}

HRESULT NSQRenderer::CQRenderer::put_PenColor(const LONG &lColor)
{
#ifdef ENABLE_LOGS
    TELL << toBin(lColor);
#endif
    m_bPenChanged = true;
    m_oPen.Color = lColor;
    return S_OK;
}

HRESULT NSQRenderer::CQRenderer::get_PenAlpha(LONG *lAlpha)
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    if (lAlpha)
    {
        *lAlpha = m_oPen.Alpha;
        return S_OK;
    }
    return S_FALSE;
}

HRESULT NSQRenderer::CQRenderer::put_PenAlpha(const LONG &lAlpha)
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    m_bPenChanged = true;
    m_oPen.Alpha = lAlpha;
    return S_OK;
}

HRESULT NSQRenderer::CQRenderer::get_PenSize(double *dSize)
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    if (dSize)
    {
        *dSize = m_oPen.Size;
        return S_OK;
    }
    return S_FALSE;
}

HRESULT NSQRenderer::CQRenderer::put_PenSize(const double &dSize)
{
#ifdef ENABLE_LOGS
    TELL << dSize;
#endif
    m_bPenChanged = true;
    m_oPen.Size = dSize;
    return S_OK;
}

HRESULT NSQRenderer::CQRenderer::get_PenDashStyle(BYTE *val)
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    if (val)
    {
        *val = m_oPen.DashStyle;
        return S_OK;
    }
    return S_FALSE;
}

HRESULT NSQRenderer::CQRenderer::put_PenDashStyle(const BYTE &val)
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    m_bPenChanged = true;
    m_oPen.DashStyle = val;
    return S_OK;
}


HRESULT NSQRenderer::CQRenderer::get_PenLineStartCap(BYTE *val)
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    if (val)
    {
        *val = m_oPen.LineStartCap;
        return S_OK;
    }
    return S_FALSE;
}

HRESULT NSQRenderer::CQRenderer::put_PenLineStartCap(const BYTE &val)
{
#ifdef ENABLE_LOGS
    TELL << getLineCap(val);
#endif
    m_bPenChanged = true;
    m_oPen.LineStartCap = val;
    return S_OK;
}

HRESULT NSQRenderer::CQRenderer::get_PenLineEndCap(BYTE *val)
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    if (val)
    {
        *val = m_oPen.LineEndCap;
        return S_OK;
    }
    return S_FALSE;
}

HRESULT NSQRenderer::CQRenderer::put_PenLineEndCap(const BYTE &val)
{
#ifdef ENABLE_LOGS
    TELL << getLineCap(val);
#endif
    m_bPenChanged = true;
    m_oPen.LineEndCap = val;
    return S_OK;
}

HRESULT NSQRenderer::CQRenderer::get_PenLineJoin(BYTE *val)
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    if (val)
    {
        *val = m_oPen.LineJoin;
        return S_OK;
    }
    return S_FALSE;
}

HRESULT NSQRenderer::CQRenderer::put_PenLineJoin(const BYTE &val)
{
#ifdef ENABLE_LOGS
    TELL << getLineJoin(val);
#endif
    m_bPenChanged = true;
    m_oPen.LineJoin = val;
    return S_OK;
}

HRESULT NSQRenderer::CQRenderer::get_PenDashOffset(double *dOffset)
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    if (dOffset)
    {
        *dOffset = m_oPen.DashOffset;
        return S_OK;
    }
    return S_FALSE;
}

HRESULT NSQRenderer::CQRenderer::put_PenDashOffset(const double &dOffset)
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    m_bPenChanged = true;
    m_oPen.DashOffset = dOffset;
    return S_OK;
}

HRESULT NSQRenderer::CQRenderer::get_PenAlign(LONG *lAlign)
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    if (lAlign)
    {
        *lAlign = m_oPen.Align;
        return S_OK;
    }
    return S_FALSE;
}

HRESULT NSQRenderer::CQRenderer::put_PenAlign(const LONG &lAlign)
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    m_bPenChanged = true;
    m_oPen.Align = lAlign;
    return S_OK;
}

HRESULT NSQRenderer::CQRenderer::get_PenMiterLimit(double *dOffset)
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    if (dOffset)
    {
        *dOffset = m_oPen.MiterLimit;
        return S_OK;
    }
    return S_FALSE;
}

HRESULT NSQRenderer::CQRenderer::put_PenMiterLimit(const double &dOffset)
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    m_bPenChanged = true;
    m_oPen.MiterLimit = dOffset;
    return S_OK;
}

HRESULT NSQRenderer::CQRenderer::PenDashPattern(double *pPattern, LONG lCount)
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    m_bPenChanged = true;
    m_oPen.DashPattern = pPattern;
    m_oPen.Count = lCount;
    return S_OK;
}

HRESULT NSQRenderer::CQRenderer::get_BrushType(LONG *lType)
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    if (lType)
    {
        *lType = m_oBrush.Type;
        return S_OK;
    }
    return S_FALSE;
}

HRESULT NSQRenderer::CQRenderer::put_BrushType(const LONG &lType)
{
#ifdef ENABLE_LOGS
    TELL << getBrushType(lType);
#endif
    m_bBrushChanged = true;
    m_oBrush.Type = lType;
    return S_OK;
}

HRESULT NSQRenderer::CQRenderer::get_BrushColor1(LONG *lColor)
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    if (lColor)
    {
        *lColor = m_oBrush.Color1;
        return S_OK;
    }
    return S_FALSE;
}

HRESULT NSQRenderer::CQRenderer::put_BrushColor1(const LONG &lColor)
{
#ifdef ENABLE_LOGS
    TELL << toBin(lColor);
#endif
    m_bBrushChanged = true;
    m_oBrush.Color1 = lColor;
    return S_OK;
}

HRESULT NSQRenderer::CQRenderer::get_BrushAlpha1(LONG *lAlpha)
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    if (lAlpha)
    {
        *lAlpha = m_oBrush.Alpha1;
        return S_OK;
    }
    return S_FALSE;
}

HRESULT NSQRenderer::CQRenderer::put_BrushAlpha1(const LONG &lAlpha)
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    m_bBrushChanged = true;
    m_oBrush.Alpha1 = lAlpha;
    return S_OK;
}

HRESULT NSQRenderer::CQRenderer::get_BrushColor2(LONG *lColor)
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    if (lColor)
    {
        *lColor = m_oBrush.Color2;
        return S_OK;
    }
    return S_FALSE;
}

HRESULT NSQRenderer::CQRenderer::put_BrushColor2(const LONG &lColor)
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    m_bBrushChanged = true;
    m_oBrush.Color2 = lColor;
    return S_OK;
}

HRESULT NSQRenderer::CQRenderer::get_BrushAlpha2(LONG *lAlpha)
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    if (lAlpha)
    {
        *lAlpha = m_oBrush.Alpha2;
        return S_OK;
    }
    return S_FALSE;
}

HRESULT NSQRenderer::CQRenderer::put_BrushAlpha2(const LONG &lAlpha)
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    m_bBrushChanged = true;
    m_oBrush.Alpha2 = lAlpha;
    return S_OK;
}

HRESULT NSQRenderer::CQRenderer::get_BrushTexturePath(std::wstring *bsPath)
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    if (bsPath)
    {
        *bsPath = m_oBrush.TexturePath;
        return S_OK;
    }
    return S_FALSE;
}

HRESULT NSQRenderer::CQRenderer::put_BrushTexturePath(const std::wstring &bsPath)
{
#ifdef ENABLE_LOGS
    TELL << "path size = " << bsPath.size();
#endif
    m_bBrushChanged = true;
    m_oBrush.TexturePath = bsPath;
    return S_OK;
}

HRESULT NSQRenderer::CQRenderer::get_BrushTextureMode(LONG *lMode)
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    if (lMode)
    {
        *lMode = m_oBrush.TextureMode;
        return S_OK;
    }
    return S_FALSE;
}

HRESULT NSQRenderer::CQRenderer::put_BrushTextureMode(const LONG &lMode)
{
#ifdef ENABLE_LOGS
    TELL << getBrushTextureMode(lMode);
#endif
    m_bBrushChanged = true;
    m_oBrush.TextureMode = lMode;
    return S_OK;
}

HRESULT NSQRenderer::CQRenderer::get_BrushTextureAlpha(LONG *lTxAlpha)
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    if (lTxAlpha)
    {
        *lTxAlpha = m_oBrush.TextureAlpha;
        return S_OK;
    }
    return S_FALSE;
}

HRESULT NSQRenderer::CQRenderer::put_BrushTextureAlpha(const LONG &lTxAlpha)
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    m_bBrushChanged = true;
    m_oBrush.TextureAlpha = lTxAlpha;
    return S_OK;
}

HRESULT NSQRenderer::CQRenderer::get_BrushLinearAngle(double *dAngle)
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    if (dAngle)
    {
        *dAngle = m_oBrush.LinearAngle;
        return S_OK;
    }
    return S_FALSE;
}

HRESULT NSQRenderer::CQRenderer::put_BrushLinearAngle(const double &dAngle)
{
#ifdef ENABLE_LOGS
    TELL << dAngle;
#endif
    m_bBrushChanged = true;
    m_oBrush.LinearAngle = dAngle;
    return S_OK;
}

HRESULT NSQRenderer::CQRenderer::BrushRect(const INT &val
                                                 , const double &left
                                                 , const double &top
                                                 , const double &width
                                                 , const double &height)
{
#ifdef ENABLE_LOGS
    TELL << QRectF{left, top, width, height};
#endif
    m_oBrush.Rect = {(float)left, (float)top, (float)width, (float)height};
    m_bBrushChanged = true;
    return S_OK;
}

HRESULT NSQRenderer::CQRenderer::BrushBounds(const double &left
                                                 , const double &top
                                                 , const double &width
                                                 , const double &height)
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    m_bBrushChanged = true;
    double right = left + width;
    double bottom = top + height;
    m_oBrush.Bounds = {left, top, right, bottom};
    return S_OK;
}

HRESULT NSQRenderer::CQRenderer::put_BrushGradientColors(LONG *lColors
                                                               , double *pPositions
                                                               , LONG nCount)
{
#ifdef ENABLE_LOGS
    TELL << nCount;
#endif
    if (0 == nCount || nullptr == lColors || nullptr == pPositions)
        return S_FALSE;

    m_bBrushChanged = true;
    m_oBrush.m_arrSubColors.clear();
    m_oBrush.m_arrSubColors.reserve((std::size_t)nCount);
    for (LONG i = 0; i < nCount; ++i) {
        NSStructures::CBrush::TSubColor color;
        color.color = lColors[i];
        color.position = (long)(pPositions[i] * 65536);

#ifdef ENABLE_LOGS
        qDebug() << toBin(lColors[i]) << "at" << pPositions[i];
#endif

        m_oBrush.m_arrSubColors.push_back(color);
    }
    return S_OK;
}

HRESULT NSQRenderer::CQRenderer::get_FontName(std::wstring *bsName)
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    if (bsName) {
        *bsName = m_oFont.Name;
        return S_OK;
    } else {
        return S_FALSE;
    }
}

HRESULT NSQRenderer::CQRenderer::put_FontName(const std::wstring &bsName)
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    m_oFont.Name = bsName;
    return S_OK;
}

HRESULT NSQRenderer::CQRenderer::get_FontPath(std::wstring *bsName)
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    if (bsName)
    {
        *bsName = m_oFont.Path;
        return S_OK;
    }
    return S_FALSE;
}

HRESULT NSQRenderer::CQRenderer::put_FontPath(const std::wstring &bsName)
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    m_oFont.Path = bsName;
    return S_OK;
}

HRESULT NSQRenderer::CQRenderer::get_FontSize(double *dSize)
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    if (dSize)
    {
        *dSize = m_oFont.Size;
        return S_OK;
    }
    return S_FALSE;
}

HRESULT NSQRenderer::CQRenderer::put_FontSize(const double &dSize)
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    m_oFont.Size = dSize;
    return S_OK;
}

HRESULT NSQRenderer::CQRenderer::get_FontStyle(LONG *lStyle)
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    if (lStyle)
    {
        *lStyle = m_oFont.GetStyle();
        return S_OK;
    }
    return S_FALSE;
}

HRESULT NSQRenderer::CQRenderer::put_FontStyle(const LONG &lStyle)
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    m_oFont.SetStyle(lStyle);
    return S_OK;
}

HRESULT NSQRenderer::CQRenderer::get_FontStringGID(INT *bGID)
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    if (bGID)
    {
        *bGID = m_oFont.StringGID;
        return S_OK;
    }
    return S_FALSE;
}

HRESULT NSQRenderer::CQRenderer::put_FontStringGID(const INT &bGID)
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    m_oFont.StringGID = bGID;
    return S_OK;
}

HRESULT NSQRenderer::CQRenderer::get_FontCharSpace(double *dSpace)
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    if (dSpace)
    {
        *dSpace = m_oFont.CharSpace;
        return S_OK;
    }
    return S_FALSE;
}

HRESULT NSQRenderer::CQRenderer::put_FontCharSpace(const double &dSpace)
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    m_oFont.CharSpace = dSpace;
    return S_OK;
}

HRESULT NSQRenderer::CQRenderer::get_FontFaceIndex(int *lFaceIndex)
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    if (lFaceIndex)
    {
        *lFaceIndex = m_oFont.FaceIndex;
        return S_OK;
    }
    return S_FALSE;
}

HRESULT NSQRenderer::CQRenderer::put_FontFaceIndex(const int &lFaceIndex)
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    m_oFont.FaceIndex = lFaceIndex;
    return S_OK;
}

HRESULT NSQRenderer::CQRenderer::CommandDrawTextCHAR(const LONG &c
                                                         , const double &x
                                                         , const double &y
                                                         , const double &w
                                                         , const double &h)
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    if (m_bIsUseTextAsPath)
    {
        PathCommandEnd();
        PathCommandTextCHAR(c, x, y, w, h);
        DrawPath(c_nWindingFillMode);
        PathCommandEnd();
        return S_OK;
    }

    return S_OK;
}

HRESULT NSQRenderer::CQRenderer::CommandDrawText(const std::wstring &bsText
                                                       , const double &x
                                                       , const double &y
                                                       , const double &w
                                                       , const double &h)
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    if (m_bIsUseTextAsPath)
    {
        PathCommandEnd();
        PathCommandText(bsText, x, y, w, h);
        DrawPath(c_nWindingFillMode);
        PathCommandEnd();
        return S_OK;
    }

    return S_OK;
}

HRESULT NSQRenderer::CQRenderer::CommandDrawTextExCHAR(const LONG &c
                                                           , const LONG &gid
                                                           , const double &x
                                                           , const double &y
                                                           , const double &w
                                                           , const double &h)
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    if (m_bIsUseTextAsPath)
    {
        PathCommandEnd();
        PathCommandTextExCHAR(c, gid, x, y, w, h);
        DrawPath(c_nWindingFillMode);
        PathCommandEnd();
        return S_OK;
    }

    return S_OK;
}

HRESULT NSQRenderer::CQRenderer::CommandDrawTextEx(const std::wstring &bsUnicodeText
                                                       , const unsigned int *pGids
                                                       , const unsigned int nGidsCount
                                                       , const double &x
                                                       , const double &y
                                                       , const double &w
                                                       , const double &h)
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    if (m_bIsUseTextAsPath)
    {
        PathCommandEnd();
        PathCommandTextEx(bsUnicodeText, pGids, nGidsCount, x, y, w, h);
        DrawPath(c_nWindingFillMode);
        PathCommandEnd();
        return S_OK;
    }

    return S_OK;
}

HRESULT NSQRenderer::CQRenderer::BeginCommand(const DWORD &lType)
{
#ifdef ENABLE_LOGS
    TELL << lType;
#endif
    m_lCurrentCommand = (long)lType;
    return S_OK;
}

HRESULT NSQRenderer::CQRenderer::EndCommand(const DWORD &lType)
{
#ifdef ENABLE_LOGS
    TELL << lType;
#endif
    m_lCurrentCommand = c_nNone;
    return S_FALSE;
}

HRESULT NSQRenderer::CQRenderer::PathCommandMoveTo(const double &x, const double &y)
{
#ifdef ENABLE_LOGS
    TELL << QPointF{x, y};
#endif
    if (c_nSimpleGraphicType == m_lCurrentCommand)
    {
        m_oUntransformedPainterPath.moveTo((qreal)x, (qreal)y);
    }
    else
    {
        m_oSimpleGraphicsConverter.PathCommandMoveTo(x, y);
    }
    return S_OK;
}

HRESULT NSQRenderer::CQRenderer::PathCommandLineTo(const double &x, const double &y)
{
#ifdef ENABLE_LOGS
    TELL << QPointF{x, y};
#endif
    if (c_nSimpleGraphicType == m_lCurrentCommand)
    {
        m_oUntransformedPainterPath.lineTo((qreal)x, (qreal)y);
    }
    else
    {
        m_oSimpleGraphicsConverter.PathCommandLineTo(x, y);
    }
    return S_OK;
}

HRESULT NSQRenderer::CQRenderer::PathCommandLinesTo(double *points, const int &count)
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    m_oSimpleGraphicsConverter.PathCommandLinesTo(points, (LONG)count);
    return S_OK;
}

HRESULT NSQRenderer::CQRenderer::PathCommandCurveTo(const double &x1
                                                          , const double &y1
                                                          , const double &x2
                                                          , const double &y2
                                                          , const double &x3
                                                          , const double &y3)
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    if (c_nSimpleGraphicType == m_lCurrentCommand)
    {
        m_oUntransformedPainterPath.cubicTo({(qreal)x1, (qreal)y1}
                            , {(qreal)x2, (qreal)y2}
                            , {(qreal)x3, (qreal)y3});
    }
    else
    {
        m_oSimpleGraphicsConverter.PathCommandCurveTo(x1, y1, x2, y2, x3, y3);
    }
    return S_OK;
}

HRESULT NSQRenderer::CQRenderer::PathCommandCurvesTo(double *points, const int &count)
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    m_oSimpleGraphicsConverter.PathCommandCurvesTo(points, (LONG)count);
    return S_OK;
}

HRESULT NSQRenderer::CQRenderer::PathCommandArcTo(const double &x
                                                      , const double &y
                                                      , const double &w
                                                      , const double &h
                                                      , const double &startAngle
                                                      , const double &sweepAngle)
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    m_oSimpleGraphicsConverter.PathCommandArcTo(x, y, w, h, startAngle, sweepAngle);
    return S_OK;
}

HRESULT NSQRenderer::CQRenderer::PathCommandClose()
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    if (c_nSimpleGraphicType == m_lCurrentCommand)
    {
        m_oUntransformedPainterPath.closeSubpath();
    }
    else
    {
        m_oSimpleGraphicsConverter.PathCommandClose();
    }
    return S_OK;
}

HRESULT NSQRenderer::CQRenderer::DrawPath(const LONG &nType)
{
    if (m_oUntransformedPainterPath.isEmpty())
    {
        return S_FALSE;
    }

    bool stroke = (0 != (nType & c_nStroke));
    bool windingFill = (0 != (nType & c_nWindingFillMode));
    bool evenOddFill = (0 != (nType & c_nEvenOddFillMode));
    bool fill = (windingFill || evenOddFill);

    QString mode;
    if (stroke) {
        mode += "stroke ";
    }
    if (windingFill) {
        mode += "winding fill ";
    }
    if (evenOddFill) {
        mode += "odd-even fill ";
    }

#ifdef ENABLE_LOGS
    TELL << mode;
#endif

    // ставим fill mode (odd even by default)
    if (windingFill)
        m_oUntransformedPainterPath.setFillRule(Qt::FillRule::WindingFill);
    if (evenOddFill)
        m_oUntransformedPainterPath.setFillRule(Qt::FillRule::OddEvenFill);

    if (stroke)
    {
        m_pContext->GetPainter()->strokePath(m_oUntransformedPainterPath, pen());
    }
    if (fill)
    {
        fillPath();
    }

    return S_OK;
}

HRESULT NSQRenderer::CQRenderer::PathCommandStart()
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    m_oUntransformedPainterPath = QPainterPath{};
    return S_OK;
}

HRESULT NSQRenderer::CQRenderer::PathCommandGetCurrentPoint(double *x, double *y)
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    m_oSimpleGraphicsConverter.PathCommandGetCurrentPoint(x, y);
    return S_OK;
}

HRESULT NSQRenderer::CQRenderer::PathCommandTextCHAR(const LONG& lUnicode, const double& dX, const double& dY, const double& dW, const double& dH)
{
    SetFont();
    int nCode = (int)lUnicode;
    m_oSimpleGraphicsConverter.PathCommandText2(&nCode, NULL, 1, m_pFontManager, dX, dY, dW, dH);
    return S_OK;
}
HRESULT NSQRenderer::CQRenderer::PathCommandTextExCHAR(const LONG& lUnicode, const LONG& lGid, const double& dX, const double& dY, const double& dW, const double& dH)
{
    SetFont();
    int nCode = (int)lUnicode;
    int nGid = (int)lGid;
    m_oSimpleGraphicsConverter.PathCommandText2(&nCode, &nGid, 1, m_pFontManager, dX, dY, dW, dH);
    return S_OK;
}
HRESULT NSQRenderer::CQRenderer::PathCommandText(const std::wstring& wsUnicodeText, const double& dX, const double& dY, const double& dW, const double& dH)
{
    SetFont();
    m_oSimpleGraphicsConverter.PathCommandText(wsUnicodeText, m_pFontManager, dX, dY, dW, dH, 0);
    return S_OK;
}
HRESULT NSQRenderer::CQRenderer::PathCommandTextEx(const std::wstring& wsUnicodeText, const unsigned int* pGids, const unsigned int nGidsCount, const double& dX, const double& dY, const double& dW, const double& dH)
{
    SetFont();
    m_oSimpleGraphicsConverter.PathCommandText2(wsUnicodeText, (const int*)pGids, nGidsCount, m_pFontManager, dX, dY, dW, dH);
    return S_OK;
}

HRESULT NSQRenderer::CQRenderer::DrawImage(IGrObject *pImage
                                               , const double &x
                                               , const double &y
                                               , const double &w
                                               , const double &h)
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    Aggplus::CImage* pPixels = (Aggplus::CImage*)pImage;
    if (Aggplus::Status::Ok != pPixels->GetLastStatus())
    {
        return S_FALSE;
    }

    QImage image = getQImage(*pPixels)
            .scaled(
                (int)w
                , (int)h
                , Qt::AspectRatioMode::IgnoreAspectRatio
                , Qt::TransformationMode::SmoothTransformation);
    QPointF point{x, y};

    m_pContext->GetPainter()->drawImage(point, image);
    return S_OK;
}

HRESULT NSQRenderer::CQRenderer::PathCommandEnd()
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    m_oUntransformedPainterPath = QPainterPath{};
    return S_OK;
}

HRESULT NSQRenderer::CQRenderer::DrawImageFromFile(const std::wstring &filePath
                                                         , const double &x
                                                         , const double &y
                                                         , const double &w
                                                         , const double &h
                                                         , const BYTE &lAlpha)
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    QRectF targetRect{x, y, w, h};

    QImage alphedImage = NSConversions::getTexture(filePath, lAlpha
                                                   // любое число, не являющееся c_BrushTextureMode
                                                   , 42, {});
    if (alphedImage.isNull()) {
        return S_FALSE;
    }
    QRectF imageRect{alphedImage.rect()};

    setPaintingThings();
    m_pContext->GetPainter()->drawImage(targetRect, alphedImage, imageRect);
    return S_OK;
}

HRESULT NSQRenderer::CQRenderer::SetTransform(const double &m11
                                                    , const double &m12
                                                    , const double &m21
                                                    , const double &m22
                                                    , const double &dx
                                                    , const double &dy)
{
    // combine with scale and base transforms which should be always set
    m_oCurrentTransform = QTransform{m11, m12, m21, m22, dx, dy};
#ifdef ENABLE_LOGS
    TELL << m_oCurrentTransform;
#endif
    applyTransform();
    return S_OK;
}

HRESULT NSQRenderer::CQRenderer::GetTransform(double *m11
                                                    , double *m12
                                                    , double *m21
                                                    , double *m22
                                                    , double *dx
                                                    , double *dy)
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    if (!m11 || !m12 || !m21 || !m22 || !dx || !dy)
        return S_FALSE;

    *m11 = (double)m_oCurrentTransform.m11();
    *m12 = (double)m_oCurrentTransform.m12();
    *m21 = (double)m_oCurrentTransform.m21();
    *m22 = (double)m_oCurrentTransform.m22();
    *dx = (double)m_oCurrentTransform.dx();
    *dy = (double)m_oCurrentTransform.dy();
    return S_OK;
}

HRESULT NSQRenderer::CQRenderer::ResetTransform()
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    m_oCurrentTransform = {};
    applyTransform();
    return S_OK;
}

HRESULT NSQRenderer::CQRenderer::get_ClipMode(LONG *plMode)
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    // not used
    return S_OK;
}

HRESULT NSQRenderer::CQRenderer::put_ClipMode(const LONG &lMode)
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    // not used
    return S_OK;
}

HRESULT NSQRenderer::CQRenderer::CommandLong(const LONG &lType, const LONG &lCommand)
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    // not used
    return S_OK;
}

HRESULT NSQRenderer::CQRenderer::CommandDouble(const LONG &lType, const double &dCommand)
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    // not used
    return S_OK;
}

HRESULT NSQRenderer::CQRenderer::CommandString(const LONG &lType, const std::wstring &sCommand)
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    // not used
    return S_OK;
}

void NSQRenderer::CQRenderer::SetBaseTransform(double m11
                                               , double m12
                                               , double m21
                                               , double m22
                                               , double dx
                                               , double dy)
{
    m_oBaseTransform = QTransform{m11, m12, m21, m22, dx, dy};
#ifdef ENABLE_LOGS
    TELL << m_oBaseTransform;
#endif
    applyTransform();
}

void NSQRenderer::CQRenderer::GetBaseTransform(double &m11
                                               , double &m12
                                               , double &m21
                                               , double &m22
                                               , double &dx
                                               , double &dy)
{
    m11 = m_oBaseTransform.m11();
    m12 = m_oBaseTransform.m12();
    m21 = m_oBaseTransform.m21();
    m22 = m_oBaseTransform.m22();
    dx = m_oBaseTransform.dx();
    dy = m_oBaseTransform.dy();
}

void NSQRenderer::CQRenderer::ResetBaseTransform()
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    m_oBaseTransform = {};
    applyTransform();
}

void NSQRenderer::CQRenderer::applyTransform()
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    m_pContext->GetPainter()->setTransform(NSConversions::scaledMMToPx(m_oScaleTransform));
    m_pContext->GetPainter()->setTransform(m_oBaseTransform, true);
    m_pContext->GetPainter()->setTransform(m_oCurrentTransform, true);
}

void NSQRenderer::CQRenderer::scaleTransformSetX(double scale)
{
#ifdef ENABLE_LOGS
    TELL << scale;
#endif
    m_oScaleTransform = {scale, m_oScaleTransform.m12()
                       , m_oScaleTransform.m21(), m_oScaleTransform.m22()
                       , m_oScaleTransform.dx(), m_oScaleTransform.dy()};
    applyTransform();
}

void NSQRenderer::CQRenderer::scaleTransformSetY(double scale)
{
#ifdef ENABLE_LOGS
    TELL << scale;
#endif
    m_oScaleTransform = {m_oScaleTransform.m11(), m_oScaleTransform.m12()
                       , m_oScaleTransform.m21(), scale
                        , m_oScaleTransform.dx(), m_oScaleTransform.dy()};
    applyTransform();
}

QRectF NSQRenderer::CQRenderer::pathRect() const
{
    //return m_oUntransformedPainterPath.boundingRect(); // этот точнее
    return m_oUntransformedPainterPath.controlPointRect(); // этот быстрее
}

QSizeF NSQRenderer::CQRenderer::paperSize() const
{
#ifdef ENABLE_LOGS
    TELL;
#endif

    // tmp
    return m_pContext->paperSize();

    int x, y, w, h;
    m_pContext->GetPhysicalRect(x, y, w, h);

    return QSizeF(w, h);
}

void NSQRenderer::CQRenderer::setPaintingThings()
{
#ifdef ENABLE_LOGS
    TELL;
#endif
    if (m_bPenChanged)
    {
        m_bPenChanged = false;
        m_pContext->GetPainter()->setPen(pen());
    }
    if (m_bBrushChanged)
    {
        m_bBrushChanged = false;
        m_pContext->GetPainter()->setBrush(brush());
    }
}

void NSQRenderer::CQRenderer::fillPath()
{
    if (c_BrushTypeTexture == m_oBrush.Type)
    {
        // суть в том, что текстура должна быть в исходном виде
        // а так painter её скейлит
        // поэтому снимаем трансформ с painter'а
        // и отдельно трансформим path
        // получение brush'а при этом не меняется
        QTransform transform = m_pContext->GetPainter()->transform();
        m_pContext->GetPainter()->resetTransform();
        QPainterPath transformedPath = transform.map(m_oUntransformedPainterPath);
        m_pContext->GetPainter()->fillPath(transformedPath, brush());
        // вешаем трансформ обратно
        m_pContext->GetPainter()->setTransform(transform);
    }
    else
    {
        m_pContext->GetPainter()->fillPath(m_oUntransformedPainterPath, brush());
    }
}

QPen NSQRenderer::CQRenderer::pen() const
{
    return NSConversions::penInternalToQt(m_oPen);
}

QBrush NSQRenderer::CQRenderer::brush() const
{
    return NSConversions::brushInternalToQt(m_oBrush
                                            , pathRect()
                                            );
}

QFont NSQRenderer::CQRenderer::font() const
{
    return NSConversions::fontInternalToQt(m_oFont);
}
