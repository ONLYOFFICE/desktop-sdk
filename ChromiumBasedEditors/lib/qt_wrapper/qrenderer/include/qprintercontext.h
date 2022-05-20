#ifndef QPRINTERCONTEXT_H
#define QPRINTERCONTEXT_H

#include "./../../../include/base.h" // DESKTOP_DECL
#include <QPagedPaintDevice>
#include <QPainter>

namespace NSQRenderer
{
    class DESKTOP_DECL CQPrinterContext
    {
        enum class mode : bool
        {
            simple
            , paged
        } m_eMode;
        QPaintDevice *m_pSimpleDevice{nullptr};
        QPagedPaintDevice *m_pPagedDevice{nullptr};
        QPainter m_oPainter{};
        bool m_bPainterActive{false};

    public:
        CQPrinterContext(QPaintDevice *pDevice);
        CQPrinterContext(QPagedPaintDevice *pDevice);
        ~CQPrinterContext();

        bool newPage();
        int dpiX() const;
        int dpiY() const;
        QSizeF paperSize() const;
        QPainter& painter();
        void endPainting();
    };
}

#endif
