#ifndef Q_RENDERER_H
#define Q_RENDERER_H

#include "./../../../../../../core/DesktopEditor/graphics/IRenderer.h"
#include "./../../../../../../core/DesktopEditor/graphics/structures.h"
#include "./../../../../../../core/DesktopEditor/graphics/GraphicsPath.h"
#include "./../../../../../../core/DesktopEditor/graphics/Image.h"
#include "./../../../../../../core/DesktopEditor/graphics/MetafileToRenderer.h"
#include "./../../../include/base.h"
#include <QPainter>
#include <QPainterPath>
#include <QPaintDevice>

class QAscPrinterContext;
namespace NSQRenderer
{
    class DESKTOP_DECL CQRenderer : public IRenderer
    {
    public:
        // own functions
        CQRenderer(QAscPrinterContext* pContext);

        virtual ~CQRenderer();

        void InitFonts(NSFonts::IApplicationFonts* pFonts);
        void SetFontsManager(NSFonts::IFontManager* pFontsManager);

        // тип рендерера-----------------------------------------------------------------------------
        virtual HRESULT get_Type(LONG* lType) override;
        //-------- Функции для работы со страницей --------------------------------------------------
        virtual HRESULT NewPage() override;
        virtual HRESULT get_Height(double* dHeight) override;
        virtual HRESULT put_Height(const double& dHeight) override;
        virtual HRESULT get_Width(double* dWidth) override;
        virtual HRESULT put_Width(const double& dWidth) override;
        virtual HRESULT get_DpiX(double* dDpiX) override;
        virtual HRESULT get_DpiY(double* dDpiY) override;

        // pen --------------------------------------------------------------------------------------
        virtual HRESULT get_PenColor(LONG* lColor) override;
        virtual HRESULT put_PenColor(const LONG& lColor) override;
        virtual HRESULT get_PenAlpha(LONG* lAlpha) override;
        virtual HRESULT put_PenAlpha(const LONG& lAlpha) override;
        virtual HRESULT get_PenSize(double* dSize) override;
        virtual HRESULT put_PenSize(const double& dSize) override;
        virtual HRESULT get_PenDashStyle(BYTE* val) override;
        virtual HRESULT put_PenDashStyle(const BYTE& val) override;
        virtual HRESULT get_PenLineStartCap(BYTE* val) override; // Aggplus::LineCapFlat
        virtual HRESULT put_PenLineStartCap(const BYTE& val) override; // Aggplus::LineCapFlat
        virtual HRESULT get_PenLineEndCap(BYTE* val) override; // Aggplus::LineCapFlat
        virtual HRESULT put_PenLineEndCap(const BYTE& val) override; // Aggplus::LineCapFlat
        virtual HRESULT get_PenLineJoin(BYTE* val) override;
        virtual HRESULT put_PenLineJoin(const BYTE& val) override;
        virtual HRESULT get_PenDashOffset(double* dOffset) override;
        virtual HRESULT put_PenDashOffset(const double& dOffset) override;
        virtual HRESULT get_PenAlign(LONG* lAlign) override;
        virtual HRESULT put_PenAlign(const LONG& lAlign) override;
        virtual HRESULT get_PenMiterLimit(double* dOffset) override;
        virtual HRESULT put_PenMiterLimit(const double& dOffset) override;
        virtual HRESULT PenDashPattern(double* pPattern, LONG lCount) override;

        // brush ------------------------------------------------------------------------------------
        virtual HRESULT get_BrushType(LONG* lType) override;
        virtual HRESULT put_BrushType(const LONG& lType) override;
        virtual HRESULT get_BrushColor1(LONG* lColor) override;
        virtual HRESULT put_BrushColor1(const LONG& lColor) override;
        virtual HRESULT get_BrushAlpha1(LONG* lAlpha) override;
        virtual HRESULT put_BrushAlpha1(const LONG& lAlpha) override;

        /////////////////////////////////////////////////////////////////
        // здесь только градиент, если таковой имеется. сеточку пока не делаем
        virtual HRESULT get_BrushColor2(LONG* lColor) override;
        virtual HRESULT put_BrushColor2(const LONG& lColor) override;
        virtual HRESULT get_BrushAlpha2(LONG* lAlpha) override;
        virtual HRESULT put_BrushAlpha2(const LONG& lAlpha) override;
        /////////////////////////////////////////////////////////////////

        virtual HRESULT get_BrushTexturePath(std::wstring* bsPath) override;
        virtual HRESULT put_BrushTexturePath(const std::wstring& bsPath) override;
        virtual HRESULT get_BrushTextureMode(LONG* lMode) override;
        virtual HRESULT put_BrushTextureMode(const LONG& lMode) override;
        virtual HRESULT get_BrushTextureAlpha(LONG* lTxAlpha) override;
        virtual HRESULT put_BrushTextureAlpha(const LONG& lTxAlpha) override;
        virtual HRESULT get_BrushLinearAngle(double* dAngle) override;
        virtual HRESULT put_BrushLinearAngle(const double& dAngle) override;
        virtual HRESULT BrushRect(const INT& val
                                  , const double& left
                                  , const double& top
                                  , const double& width
                                  , const double& height) override;
        virtual HRESULT BrushBounds(const double& left
                                    , const double& top
                                    , const double& width
                                    , const double& height) override;

        virtual HRESULT put_BrushGradientColors(LONG* lColors
                                                , double* pPositions
                                                , LONG nCount) override;
        // есть дефолтная реализация
        // virtual void put_BrushGradInfo(const NSStructures::GradientInfo &_ginfo) override;

        // font -------------------------------------------------------------------------------------
        virtual HRESULT get_FontName(std::wstring* bsName) override;
        virtual HRESULT put_FontName(const std::wstring& bsName) override;
        virtual HRESULT get_FontPath(std::wstring* bsName) override;
        virtual HRESULT put_FontPath(const std::wstring& bsName) override;
        virtual HRESULT get_FontSize(double* dSize) override;
        virtual HRESULT put_FontSize(const double& dSize) override;
        virtual HRESULT get_FontStyle(LONG* lStyle) override;
        virtual HRESULT put_FontStyle(const LONG& lStyle) override;
        virtual HRESULT get_FontStringGID(INT* bGID) override;
        virtual HRESULT put_FontStringGID(const INT& bGID) override;
        virtual HRESULT get_FontCharSpace(double* dSpace) override;
        virtual HRESULT put_FontCharSpace(const double& dSpace) override;
        virtual HRESULT get_FontFaceIndex(int* lFaceIndex) override;
        virtual HRESULT put_FontFaceIndex(const int& lFaceIndex) override;

        //-------- Функции для вывода текста --------------------------------------------------------
        virtual HRESULT CommandDrawTextCHAR(const LONG& c
                                            , const double& x
                                            , const double& y
                                            , const double& w
                                            , const double& h) override;
        virtual HRESULT CommandDrawText(const std::wstring& bsText
                                        , const double& x
                                        , const double& y
                                        , const double& w
                                        , const double& h) override;

        virtual HRESULT CommandDrawTextExCHAR(const LONG& c
                                              , const LONG& gid
                                              , const double& x
                                              , const double& y
                                              , const double& w
                                              , const double& h) override;
        virtual HRESULT CommandDrawTextEx(const std::wstring& bsUnicodeText
                                          , const unsigned int* pGids
                                          , const unsigned int nGidsCount
                                          , const double& x
                                          , const double& y
                                          , const double& w
                                          , const double& h) override;

        //-------- Маркеры для команд ---------------------------------------------------------------
        virtual HRESULT BeginCommand(const DWORD& lType) override;
        virtual HRESULT EndCommand(const DWORD& lType) override;

        //-------- Функции для работы с Graphics Path -----------------------------------------------
        virtual HRESULT PathCommandMoveTo(const double& x, const double& y) override;
        virtual HRESULT PathCommandLineTo(const double& x, const double& y) override;
        virtual HRESULT PathCommandLinesTo(double* points, const int& count) override;
        virtual HRESULT PathCommandCurveTo(const double& x1
                                           , const double& y1
                                           , const double& x2
                                           , const double& y2
                                           , const double& x3
                                           , const double& y3) override;
        virtual HRESULT PathCommandCurvesTo(double* points, const int& count) override;
        virtual HRESULT PathCommandArcTo(const double& x
                                         , const double& y
                                         , const double& w
                                         , const double& h
                                         , const double& startAngle
                                         , const double& sweepAngle) override;
        virtual HRESULT PathCommandClose() override;
        virtual HRESULT PathCommandEnd() override;
        virtual HRESULT DrawPath(const LONG& nType) override;
        virtual HRESULT PathCommandStart() override;
        virtual HRESULT PathCommandGetCurrentPoint(double* x, double* y) override;

        virtual HRESULT PathCommandTextCHAR(const LONG& c
                                            , const double& x
                                            , const double& y
                                            , const double& w
                                            , const double& h) override;
        virtual HRESULT PathCommandText(const std::wstring& bsText
                                        , const double& x
                                        , const double& y
                                        , const double& w
                                        , const double& h) override;

        virtual HRESULT PathCommandTextExCHAR(const LONG& c
                                              , const LONG& gid
                                              , const double& x
                                              , const double& y
                                              , const double& w
                                              , const double& h) override;
        virtual HRESULT PathCommandTextEx(const std::wstring& sText
                                          , const unsigned int* pGids
                                          , const unsigned int nGidsCount
                                          , const double& x
                                          , const double& y
                                          , const double& w
                                          , const double& h) override;

        //-------- Функции для вывода изображений ---------------------------------------------------
        virtual HRESULT DrawImage(IGrObject* pImage
                                  , const double& x
                                  , const double& y
                                  , const double& w
                                  , const double& h) override;
        virtual HRESULT DrawImageFromFile(const std::wstring&
                                          , const double& x
                                          , const double& y
                                          , const double& w
                                          , const double& h
                                          , const BYTE& lAlpha = 255) override;

        // transform --------------------------------------------------------------------------------
        virtual HRESULT SetCommandParams(double dAngle
                                         , double dLeft
                                         , double dTop
                                         , double dWidth
                                         , double dHeight
                                         , DWORD lFlags) override {return S_OK;}

        virtual HRESULT SetTransform(const double& m11
                                     , const double& m12
                                     , const double& m21
                                     , const double& m22
                                     , const double& dx
                                     , const double& dy) override;
        virtual HRESULT GetTransform(double *m11
                                     , double *m12
                                     , double *m21
                                     , double *m22
                                     , double *dx
                                     , double *dy)	override;
        virtual HRESULT ResetTransform() override;

        // -----------------------------------------------------------------------------------------
        virtual HRESULT get_ClipMode(LONG* plMode) override;
        virtual HRESULT put_ClipMode(const LONG& lMode) override;

        // additiaonal params ----------------------------------------------------------------------
        virtual HRESULT CommandLong(const LONG& lType, const LONG& lCommand) override;
        virtual HRESULT CommandDouble(const LONG& lType, const double& dCommand) override;
        virtual HRESULT CommandString(const LONG& lType, const std::wstring& sCommand) override;

    public:
        virtual HRESULT SetBaseTransform(const double& m11, const double& m12, const double& m21, const double& m22, const double& dx, const double& dy) override;
        void GetBaseTransform(double &m11, double &m12, double &m21, double &m22, double &dx, double &dy);
        void ResetBaseTransform();

    public:
        static IMetafileToRenderter* GetChecker();

    private:
        void applyTransform();

    private:
        QAscPrinterContext* m_pContext;
        QPainterPath m_oPath;

        QTransform m_oCoordTransform;
        QTransform m_oBaseTransform;
        QTransform m_oCurrentTransform;
        QTransform m_oFullTransform;

        NSStructures::CPen m_oPen;
        NSStructures::CBrush m_oBrush;

        NSFonts::IApplicationFonts* m_pAppFonts;
        NSFonts::IFontManager* m_pFontManager;
        bool m_bIsUseTextAsPath;

        NSStructures::CFont	m_oFont;
        NSStructures::CFont	m_oInstalledFont;

        Aggplus::CGraphicsPathSimpleConverter m_oSimpleGraphicsConverter;

        double m_dLogicalPageWidth;
        double m_dLogicalPageHeight;

        long m_lCurrentCommand;
        long m_lCurrentClipMode;

    private:
        QSizeF paperSize() const;

    private:
        QPen pen() const;

        void fillPath(QPainterPath* pPath);

    public:
        void SetUseTextAsPath(const bool& bIsUseTextAsPath);

    private:
        void InitDefaults();
        void SetFont();
    };
}

#endif // Q_RENDERER_H
