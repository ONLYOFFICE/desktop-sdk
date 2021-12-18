#ifndef PAINTING_CONVERSIONS_H
#define PAINTING_CONVERSIONS_H

#include "./../../../../../../core/DesktopEditor/graphics/AggPlusEnums.h" // enum DashStyle
#include "./../../../../../../core/DesktopEditor/graphics/structures.h"
#include <QPen>
#include <QBrush>
#include <QTransform>

namespace NSConversions
{
const qreal default_dots_per_inch = 96.;

// units of measure
qreal mmToPixels(qreal mm);
qreal pixelsPerMM();
QTransform scaledMMToPx(const QTransform &transform);

// image
QImage getTexture(const std::wstring &path, LONG alpha, LONG mode, const QSize &bounds);

// pen
QPen penInternalToQt(const NSStructures::CPen &pen);

// brush
QBrush brushInternalToQt(const NSStructures::CBrush &brush, const QRectF &pathRect);

// color
struct CInternalColor
{
    LONG color;
    LONG alpha;
};
QColor colorInternalToQt(CInternalColor internalColor);
QColor colorInternalToQt(LONG internalColor);
CInternalColor colorQtToInternal(const QColor &color);

// font
QFont fontInternalToQt(const NSStructures::CFont &font);

class FontStyle
{
    unsigned char value{0};

    bool bitNumber(int number) const;
    void setBitNumber(int number, bool val);

    enum Numbers : int
    {
        nBold = 0
        , nItalic = 1
        , nUnderline = 2
        , nStrikeout = 3
    };

public:
    FontStyle() = default;
    FontStyle(LONG internalFontStyle);
    FontStyle(const QFont &font);

    LONG toInternalFontStyle() const;
    void assignToQFont(QFont &font) const;

    bool bold() const;
    void setBold(bool val);
    bool italic() const;
    void setItalic(bool val);
    bool underline() const;
    void setUnderline(bool val);
    bool strikeout() const;
    void setStrikeout(bool val);
};
}

#endif // PAINTING_CONVERSIONS_H
