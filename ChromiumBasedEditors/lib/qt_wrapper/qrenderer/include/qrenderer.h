#ifndef Q_RENDERER_H
#define Q_RENDERER_H

#include "./../../../../../../core/DesktopEditor/graphics/IRenderer.h" // IRenderer
#include "./../../../../../../core/DesktopEditor/graphics/structures.h" // CPen и CBrush
#include "./../../../include/base.h" // DESKTOP_DECL
#include "qprintercontext.h" // CQPrinterContext
#include <QPainter> // QPainter
#include <QPainterPath> // QPainterPath
#include <QPaintDevice> // QPagedPaintDevice

namespace NSQRenderer
{
    class DESKTOP_DECL CQRenderer : public IRenderer
    {
    public:
        // own functions
        CQRenderer(QPaintDevice *pPaintDevice);
        CQRenderer(QPagedPaintDevice *pPaintDevice);

        void endPainting();

        // тип рендерера-----------------------------------------------------------------------------
        virtual HRESULT get_Type(LONG* lType) override;// используется
        //-------- Функции для работы со страницей --------------------------------------------------
        virtual HRESULT NewPage() override;// используется
        virtual HRESULT get_Height(double* dHeight) override;// используется
        virtual HRESULT put_Height(const double& dHeight) override;// используется
        virtual HRESULT get_Width(double* dWidth) override;// используется
        virtual HRESULT put_Width(const double& dWidth) override;// используется
        virtual HRESULT get_DpiX(double* dDpiX) override;// используется при другом рендерере
        virtual HRESULT get_DpiY(double* dDpiY) override;// NOT USED

        // pen --------------------------------------------------------------------------------------
        virtual HRESULT get_PenColor(LONG* lColor) override;// используется
        virtual HRESULT put_PenColor(const LONG& lColor) override;// используется
        virtual HRESULT get_PenAlpha(LONG* lAlpha) override;// используется
        virtual HRESULT put_PenAlpha(const LONG& lAlpha) override;// используется
        virtual HRESULT get_PenSize(double* dSize) override;// используется
        virtual HRESULT put_PenSize(const double& dSize) override;// используется
        virtual HRESULT get_PenDashStyle(BYTE* val) override;// используется
        virtual HRESULT put_PenDashStyle(const BYTE& val) override;// используется
        virtual HRESULT get_PenLineStartCap(BYTE* val) override;// используется с Aggplus::LineCapFlat
        virtual HRESULT put_PenLineStartCap(const BYTE& val) override;// используется с Aggplus::LineCapFlat
        virtual HRESULT get_PenLineEndCap(BYTE* val) override;// используется с Aggplus::LineCapFlat
        virtual HRESULT put_PenLineEndCap(const BYTE& val) override;// используется с Aggplus::LineCapFlat
        virtual HRESULT get_PenLineJoin(BYTE* val) override;// используется
        virtual HRESULT put_PenLineJoin(const BYTE& val) override;// используется
        virtual HRESULT get_PenDashOffset(double* dOffset) override;// NOT USED
        virtual HRESULT put_PenDashOffset(const double& dOffset) override;// NOT USED
        virtual HRESULT get_PenAlign(LONG* lAlign) override;// NOT USED
        virtual HRESULT put_PenAlign(const LONG& lAlign) override;// NOT USED
        virtual HRESULT get_PenMiterLimit(double* dOffset) override;// NOT USED
        virtual HRESULT put_PenMiterLimit(const double& dOffset) override;// NOT USED
        virtual HRESULT PenDashPattern(double* pPattern, LONG lCount) override;// используется

        // brush ------------------------------------------------------------------------------------
        virtual HRESULT get_BrushType(LONG* lType) override;// используется
        virtual HRESULT put_BrushType(const LONG& lType) override;// используется
        virtual HRESULT get_BrushColor1(LONG* lColor) override;// используется
        virtual HRESULT put_BrushColor1(const LONG& lColor) override;// используется
        virtual HRESULT get_BrushAlpha1(LONG* lAlpha) override;// используется
        virtual HRESULT put_BrushAlpha1(const LONG& lAlpha) override;// используется

        /////////////////////////////////////////////////////////////////
        // здесь только градиент, если таковой имеется. сеточку пока не делаем
        virtual HRESULT get_BrushColor2(LONG* lColor) override;// используется
        virtual HRESULT put_BrushColor2(const LONG& lColor) override;// используется
        virtual HRESULT get_BrushAlpha2(LONG* lAlpha) override;// используется
        virtual HRESULT put_BrushAlpha2(const LONG& lAlpha) override;// используется
        /////////////////////////////////////////////////////////////////


        virtual HRESULT get_BrushTexturePath(std::wstring* bsPath) override;// используется
        virtual HRESULT put_BrushTexturePath(const std::wstring& bsPath) override;// используется
        virtual HRESULT get_BrushTextureMode(LONG* lMode) override;// используется
        virtual HRESULT put_BrushTextureMode(const LONG& lMode) override;// используется
        virtual HRESULT get_BrushTextureAlpha(LONG* lTxAlpha) override;// используется
        virtual HRESULT put_BrushTextureAlpha(const LONG& lTxAlpha) override;// используется
        virtual HRESULT get_BrushLinearAngle(double* dAngle) override;// NOT USED
        virtual HRESULT put_BrushLinearAngle(const double& dAngle) override;// NOT USED
        virtual HRESULT BrushRect(const INT& val
                                  , const double& left
                                  , const double& top
                                  , const double& width
                                  , const double& height) override;// используется
        virtual HRESULT BrushBounds(const double& left
                                    , const double& top
                                    , const double& width
                                    , const double& height) override;// NOT USED

        virtual HRESULT put_BrushGradientColors(LONG* lColors // ПРОВЕРЯТЬ НА NULL
                                                , double* pPositions // ПРОВЕРЯТЬ НА NULL
                                                , LONG nCount) override;// используется
            // есть дефолтная реализация
//            virtual void put_BrushGradInfo(const NSStructures::GradientInfo &_ginfo) override;

        // font -------------------------------------------------------------------------------------
        virtual HRESULT get_FontName(std::wstring* bsName) override;// используется
        virtual HRESULT put_FontName(const std::wstring& bsName) override;// используется
        virtual HRESULT get_FontPath(std::wstring* bsName) override;// NOT USED
        virtual HRESULT put_FontPath(const std::wstring& bsName) override;// NOT USED
        virtual HRESULT get_FontSize(double* dSize) override;// используется
        virtual HRESULT put_FontSize(const double& dSize) override;// используется
        virtual HRESULT get_FontStyle(LONG* lStyle) override;// используется
        virtual HRESULT put_FontStyle(const LONG& lStyle) override;// используется
        virtual HRESULT get_FontStringGID(INT* bGID) override;// NOT USED
        virtual HRESULT put_FontStringGID(const INT& bGID) override;// NOT USED
        virtual HRESULT get_FontCharSpace(double* dSpace) override;// NOT USED
        virtual HRESULT put_FontCharSpace(const double& dSpace) override;// NOT USED
        virtual HRESULT get_FontFaceIndex(int* lFaceIndex) override;// NOT USED
        virtual HRESULT put_FontFaceIndex(const int& lFaceIndex) override;// NOT USED

        //-------- Функции для вывода текста --------------------------------------------------------
        virtual HRESULT CommandDrawTextCHAR(const LONG& c
                                            , const double& x
                                            , const double& y
                                            , const double& w
                                            , const double& h) override;// NOT USED
        virtual HRESULT CommandDrawText(const std::wstring& bsText
                                        , const double& x
                                        , const double& y
                                        , const double& w
                                        , const double& h) override;// используется

        virtual HRESULT CommandDrawTextExCHAR(const LONG& c
                                              , const LONG& gid
                                              , const double& x
                                              , const double& y
                                              , const double& w
                                              , const double& h) override;// NOT USED
        virtual HRESULT CommandDrawTextEx(const std::wstring& bsUnicodeText
                                          , const unsigned int* pGids
                                          , const unsigned int nGidsCount
                                          , const double& x
                                          , const double& y
                                          , const double& w
                                          , const double& h) override;// NOT USED

        //-------- Маркеры для команд ---------------------------------------------------------------
        virtual HRESULT BeginCommand(const DWORD& lType) override;// используется
        virtual HRESULT EndCommand(const DWORD& lType) override;// используется

        //-------- Функции для работы с Graphics Path -----------------------------------------------
        virtual HRESULT PathCommandMoveTo(const double& x, const double& y) override;// используется
        virtual HRESULT PathCommandLineTo(const double& x, const double& y) override;// используется
        virtual HRESULT PathCommandLinesTo(double* points, const int& count) override;// NOT USED
        virtual HRESULT PathCommandCurveTo(const double& x1
                                           , const double& y1
                                           , const double& x2
                                           , const double& y2
                                           , const double& x3
                                           , const double& y3) override;// используется
        virtual HRESULT PathCommandCurvesTo(double* points, const int& count) override;// NOT USED
        virtual HRESULT PathCommandArcTo(const double& x
                                         , const double& y
                                         , const double& w
                                         , const double& h
                                         , const double& startAngle
                                         , const double& sweepAngle) override;// NOT USED
        virtual HRESULT PathCommandClose() override;// используется
        virtual HRESULT PathCommandEnd() override;// используется
        virtual HRESULT DrawPath(const LONG& nType) override;// используется
        virtual HRESULT PathCommandStart() override;// используется
        virtual HRESULT PathCommandGetCurrentPoint(double* x, double* y) override;// NOT USED

        virtual HRESULT PathCommandTextCHAR(const LONG& c
                                            , const double& x
                                            , const double& y
                                            , const double& w
                                            , const double& h) override;// NOT USED
        virtual HRESULT PathCommandText(const std::wstring& bsText
                                        , const double& x
                                        , const double& y
                                        , const double& w
                                        , const double& h) override;// NOT USED

        virtual HRESULT PathCommandTextExCHAR(const LONG& c
                                              , const LONG& gid
                                              , const double& x
                                              , const double& y
                                              , const double& w
                                              , const double& h) override;// NOT USED
        virtual HRESULT PathCommandTextEx(const std::wstring& sText
                                          , const unsigned int* pGids
                                          , const unsigned int nGidsCount
                                          , const double& x
                                          , const double& y
                                          , const double& w
                                          , const double& h) override;// NOT USED

        //-------- Функции для вывода изображений ---------------------------------------------------
        virtual HRESULT DrawImage(IGrObject* pImage
                                  , const double& x
                                  , const double& y
                                  , const double& w
                                  , const double& h) override;// NOT USED
        virtual HRESULT DrawImageFromFile(const std::wstring&
                                          , const double& x
                                          , const double& y
                                          , const double& w
                                          , const double& h
                                          , const BYTE& lAlpha = 255) override;// используется

        // transform --------------------------------------------------------------------------------
        // есть дефолтная реализация
//            virtual HRESULT GetCommandParams(double* dAngle
//                                             , double* dLeft
//                                             , double* dTop
//                                             , double* dWidth
//                                             , double* dHeight
//                                             , DWORD* lFlags) override;
        virtual HRESULT SetCommandParams(double dAngle
                                         , double dLeft
                                         , double dTop
                                         , double dWidth
                                         , double dHeight
                                         , DWORD lFlags) override {return S_OK;};// tmp

        virtual HRESULT SetTransform(const double& m11
                                     , const double& m12
                                     , const double& m21
                                     , const double& m22
                                     , const double& dx
                                     , const double& dy) override;// используется
        virtual HRESULT GetTransform(double *m11
                                     , double *m12
                                     , double *m21
                                     , double *m22
                                     , double *dx
                                     , double *dy)	override;// используется
        virtual HRESULT ResetTransform() override;// NOT USED

        // -----------------------------------------------------------------------------------------
        virtual HRESULT get_ClipMode(LONG* plMode) override;// NOT USED
        virtual HRESULT put_ClipMode(const LONG& lMode) override;// NOT USED

        // additiaonal params ----------------------------------------------------------------------
        virtual HRESULT CommandLong(const LONG& lType, const LONG& lCommand) override;// NOT USED
        virtual HRESULT CommandDouble(const LONG& lType, const double& dCommand) override;// NOT USED
        virtual HRESULT CommandString(const LONG& lType, const std::wstring& sCommand) override;// NOT USED

        // есть дефолтная реализация
//            virtual HRESULT StartConvertCoordsToIdentity() override;
//            virtual HRESULT EndConvertCoordsToIdentity() override;

            // используется, но есть дефолтная реализация
//            virtual HRESULT AddHyperlink(const double& dX
//                                         , const double& dY
//                                         , const double& dW
//                                         , const double& dH
//                                         , const std::wstring& wsUrl
//                                         , const std::wstring& wsTooltip) override;
//            virtual HRESULT AddLink(const double& dX
//                                    , const double& dY
//                                    , const double& dW
//                                    , const double& dH
//                                    , const double& dDestX
//                                    , const double& dDestY
//                                    , const int& nPage) override;
//            virtual HRESULT AddFormField(const CFormFieldInfo& oInfo) override;

    public:
        void SetBaseTransform(double m11
                                     , double m12
                                     , double m21
                                     , double m22
                                     , double dx
                                     , double dy);
        void GetBaseTransform(double &m11
                                     , double &m12
                                     , double &m21
                                     , double &m22
                                     , double &dx
                                     , double &dy);
        void ResetBaseTransform();

    private:
        void applyTransform();

    private:
        static constexpr double defaultDouble = -1.;

        CQPrinterContext m_oPrinterContext;
        QPainterPath m_oUntransformedPainterPath{};

        QTransform m_oScaleTransform{};
        QTransform m_oBaseTransform{};
        QTransform m_oCurrentTransform{};

        NSStructures::CPen m_oPen{};
        bool m_bPenChanged{true};
        NSStructures::CBrush m_oBrush{};
        bool m_bBrushChanged{true};
        NSStructures::CFont m_oFont{};
        bool m_bFontChanged{true};

        double m_dLogicalPageWidth{0.};
        double m_dLogicalPageHeight{0.};

        long m_oCurrentCommand{c_nNone};


    private:
        //
        void scaleTransformSetX(double scale);
        void scaleTransformSetY(double scale);

        //
        QRectF pathRect() const;

        //
        QSizeF paperSize() const;

        //
        QPen pen() const;
        QBrush brush() const;
        QFont font() const;

        //
        void setPaintingThings();
        void fillPath();
    };
}

#endif // Q_RENDERER_H
