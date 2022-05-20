#include "./../include/qprintercontext.h"

NSQRenderer::CQPrinterContext::CQPrinterContext(QPaintDevice *pDevice)
    : m_eMode{mode::simple}
    , m_pSimpleDevice{pDevice}
{
    if (m_pSimpleDevice) {
        m_bPainterActive = m_oPainter.begin(m_pSimpleDevice);
    }
}

NSQRenderer::CQPrinterContext::CQPrinterContext(QPagedPaintDevice *pDevice)
    : m_eMode{mode::paged}
    , m_pPagedDevice{pDevice}
{
    if (m_pPagedDevice) {
        m_bPainterActive = m_oPainter.begin(m_pPagedDevice);
    }
}

NSQRenderer::CQPrinterContext::~CQPrinterContext()
{
    if (m_bPainterActive) {
        m_oPainter.end();
    }
}

bool NSQRenderer::CQPrinterContext::newPage()
{
    switch (m_eMode) {
    case mode::paged:
    {
        if (m_pPagedDevice && m_pPagedDevice->newPage()) {
            return true;
        } else {
            return false;
        }
    }
    case mode::simple: return false;
    } // switch (m_eMode)
}

int NSQRenderer::CQPrinterContext::dpiX() const
{
    switch (m_eMode) {
    case mode::paged:
    {
        if (m_pPagedDevice) {
            return m_pPagedDevice->logicalDpiX();
        } else {
            return 0;
        }
    }
    case mode::simple:
    {
        if (m_pSimpleDevice) {
            return m_pSimpleDevice->logicalDpiX();
        } else {
            return 0;
        }
    }
    } // switch (m_eMode)
}

int NSQRenderer::CQPrinterContext::dpiY() const
{
    switch (m_eMode) {
    case mode::paged:
    {
        if (m_pPagedDevice) {
            return m_pPagedDevice->logicalDpiY();
        } else {
            return 0;
        }
    }
    case mode::simple:
    {
        if (m_pSimpleDevice) {
            return m_pSimpleDevice->logicalDpiY();
        } else {
            return 0;
        }
    }
    } // switch (m_eMode)
}

QSizeF NSQRenderer::CQPrinterContext::paperSize() const
{
    switch (m_eMode) {
    case mode::paged:
    {
        if (!m_pPagedDevice) {
            return {};
        }
        QRectF pageRect = m_pPagedDevice->pageLayout()
                .fullRect(QPageLayout::Unit::Millimeter);
        QMarginsF pageMargins = m_pPagedDevice->pageLayout()
                .margins(QPageLayout::Unit::Millimeter);
        return {
            pageMargins.left() + pageRect.width() + pageMargins.right()
          , pageMargins.bottom() + pageRect.height() + pageRect.top()
        };
    }
    case mode::simple:
    {
        if (!m_pSimpleDevice) {
            return {};
        }
        return {(qreal)m_pSimpleDevice->widthMM()
                    , (qreal)m_pSimpleDevice->heightMM()};
    }
    } // switch (m_eMode)
}

QPainter& NSQRenderer::CQPrinterContext::painter()
{
    return m_oPainter;
}

void NSQRenderer::CQPrinterContext::endPainting()
{
    m_oPainter.end();
    m_bPainterActive = false;
}
