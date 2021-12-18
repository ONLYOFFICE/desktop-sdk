#include "./../src/painting_conversions.h"
#include <QFont> // font
#include <QPainter> // painting on QImage to set alpha
#include <QScreen> // converting mm to px
#include <QApplication> // converting mm to px
#include <QDebug>

qreal NSConversions::mmToPixels(qreal mm)
{
    static constexpr qreal mm_per_inch = 25.4;
    qreal inch = (qreal)mm / mm_per_inch;

    qreal dots_per_inch = default_dots_per_inch;

    QScreen *screen = QApplication::primaryScreen();
    if (screen) {
        dots_per_inch = screen->logicalDotsPerInch();
    }

    return inch * dots_per_inch;
}
qreal NSConversions::pixelsPerMM()
{
    return mmToPixels(1.);
}
QTransform NSConversions::scaledMMToPx(const QTransform &transform)
{
    QTransform result = transform;
    qreal pxPerMM = pixelsPerMM();
    result.scale(pxPerMM, pxPerMM);
    return result;
}

QImage NSConversions::getTexture(const std::wstring &path
                                 , LONG alpha
                                 , LONG mode
                                 , const QSize &bounds)
{
    // texture path
    QString qtPath = QString::fromStdWString(path);
    qDebug() << __func__;
    qDebug() << "qtPath" << qtPath;

    if (qtPath.isEmpty()) {
        return QImage{};
    }

    QImage textureDraft_1{qtPath};
    qDebug() << "texture size =" << textureDraft_1.size();
    qDebug() << "path bounds size =" << bounds;

    if (textureDraft_1.widthMM() == 0 || textureDraft_1.heightMM() == 0) {
        return QImage{};
    }

    // texture alpha
    int here;
    constexpr qreal maxAlpha = (qreal)
            // думаю что правильно 100
//            (std::numeric_limits<decltype(alpha)>::max)()
            100
            ;
    qreal dAlpha = (qreal)alpha / maxAlpha;

    QImage textureDraft_2{textureDraft_1.size(), QImage::Format::Format_ARGB32_Premultiplied};
    textureDraft_2.fill(Qt::GlobalColor::transparent);
    QPainter painter{&textureDraft_2};
    painter.setOpacity(dAlpha);
    painter.drawImage(0, 0, textureDraft_1);
    painter.end();

    // texture mode
    switch (mode) {
    // подгон по размеру path
    case c_BrushTextureModeStretch: {
        if (bounds.isNull() || 0 == bounds.width() || 0 == bounds.height()) {
            return textureDraft_2;
        }
        int here1;
        QImage scaledTexture = textureDraft_2.scaled(
                    bounds
                    // расширяется, чтобы полностью заполнять path,
                    // может вылезать за его пределы
                    // можно сделать KeepAspectRatio -
                    // чтобы расширялось, не вылезая, но заполняя не полностью
                    // https://doc.qt.io/qt-5/qpixmap.html#scaled
                    , Qt::AspectRatioMode::KeepAspectRatioByExpanding
                    // ресайз медленно, но со сглаживанием
                    // https://doc.qt.io/qt-5/qt.html#TransformationMode-enum
                    , Qt::TransformationMode::SmoothTransformation
                    );
        return scaledTexture;
    }
    case c_BrushTextureModeTile: // обычное выравнивание по левому верхнему углу
    case c_BrushTextureModeTileCenter: // пока не делаем
    default: {
        return textureDraft_2;
    }
    } // switch (mode)
}

//class CPen

//    long Color;
//    long Alpha;
//    double Size;

//    unsigned char DashStyle;
//    unsigned char LineStartCap;
//    unsigned char LineEndCap;
//    unsigned char LineJoin;

//    double *DashPattern;
//    long Count;

//    double DashOffset;

//    long Align;
//    double MiterLimit;

//    DefaultParams()

//        Color = 0;
//        Alpha = 255;
//        Size = 1;

//        DashStyle = 0;
//        LineStartCap = 0;
//        LineEndCap = 0;
//        LineJoin = 0;

//        DashPattern = NULL;
//        Count = 0;

//        DashOffset = 0;
//        Align = Aggplus::PenAlignmentCenter;
//        MiterLimit = 0.5;
QPen NSConversions::penInternalToQt(const NSStructures::CPen &pen)
{
    if (0. == pen.Size) {
        return QPen{Qt::PenStyle::NoPen};
    }
    // цвет и альфа
    // размер в миллиметрах
    // line cap
    // line join
    // dash offset
    // miter limit
    QPen result;

    // цвет и альфа
    result.setColor(colorInternalToQt({pen.Color, pen.Alpha}));

    // размер
    result.setWidthF(pen.Size);

    // dash style
    //    DashStyleSolid,          // solid
    //    DashStyleDash,           // dash 3 space 1
    //    DashStyleDot,            // dash 1 space 1
    //    DashStyleDashDot,        // dash 3 space 1 dash 1 space 1
    //    DashStyleDashDotDot,     // dash 3 space 1 dash 1 space 1 dash 1 space 1
    //    DashStyleCustom          // из массива
    switch (pen.DashStyle) {
    case (BYTE)Aggplus::DashStyle::DashStyleCustom: {
        int patternCount = (int)pen.Count;
        if (patternCount < 1) {
            break;
        }
        QVector<qreal> pattern(patternCount);
        for (int i = 0; i < patternCount; ++i) {
            pattern[i] = (qreal)pen.DashPattern[i];
        }
        result.setDashPattern(pattern);
        break;
    }
    case (BYTE)Aggplus::DashStyle::DashStyleDash: {
        result.setDashPattern({3, 1});
        break;
    }
    case (BYTE)Aggplus::DashStyle::DashStyleDashDot: {
        result.setDashPattern({3, 1, 1, 1});
        break;
    }
    case (BYTE)Aggplus::DashStyle::DashStyleDashDotDot: {
        result.setDashPattern({3, 1, 1, 1, 1, 1});
        break;
    }
    case (BYTE)Aggplus::DashStyle::DashStyleDot: {
        result.setDashPattern({1, 1});
        break;
    }
    case (BYTE)Aggplus::DashStyle::DashStyleSolid: {
        result.setStyle(Qt::PenStyle::SolidLine);
        break;
    }
    default:
        break;
    } //switch (pen.DashStyle)

    // line cap
    // Graphics.cpp
    //        agg::line_cap_e LineCap = agg::round_cap;
    //        switch(pPen->LineStartCap)
    //        {
    //        case LineCapFlat         : LineCap = agg::butt_cap; break;
    //        case LineCapRound        : LineCap = agg::round_cap; break;
    //        case LineCapSquare       : LineCap = agg::square_cap; break;
    //        default:	break;
    //        }

    switch (pen.LineStartCap) {
    case (BYTE)Aggplus::LineCap::LineCapFlat: {
        result.setCapStyle(Qt::PenCapStyle::FlatCap);
        break;
    }
    case (BYTE)Aggplus::LineCap::LineCapRound: {
        result.setCapStyle(Qt::PenCapStyle::RoundCap);
        break;
    }
    case (BYTE)Aggplus::LineCap::LineCapSquare: {
        result.setCapStyle(Qt::PenCapStyle::SquareCap);
        break;
    }
    default: { // в Graphics.cpp по дефолту round_cap
        result.setCapStyle(Qt::PenCapStyle::RoundCap);
        break;
    }
    } // switch (pen.LineStartCap)

    // line join
    // Graphics.cpp
    //        agg::line_join_e LineJoin = agg::round_join;
    //        switch(pPen->LineJoin)
    //        {
    //        case LineJoinMiter			: LineJoin = agg::miter_join_revert; break;
    //        case LineJoinBevel			: LineJoin = agg::bevel_join; break;
    //        case LineJoinRound			: LineJoin = agg::round_join; break;
    //        case LineJoinMiterClipped	: LineJoin = agg::miter_join_revert; break;
    //        default:	break;
    //        }

    switch (pen.LineJoin) {
    case (BYTE)Aggplus::LineJoin::LineJoinMiter: // в Graphics.cpp MiterClipped то же самое, что Miter
    case (BYTE)Aggplus::LineJoin::LineJoinMiterClipped: {
        result.setJoinStyle(Qt::PenJoinStyle::MiterJoin);
        break;
    }
    case (BYTE)Aggplus::LineJoin::LineJoinBevel: {
        result.setJoinStyle(Qt::PenJoinStyle::BevelJoin);
        break;
    }
    case (BYTE)Aggplus::LineJoin::LineJoinRound: {
        result.setJoinStyle(Qt::PenJoinStyle::RoundJoin);
        break;
    }
    default: { // в Graphics.cpp по дефолту round_join
        result.setJoinStyle(Qt::PenJoinStyle::RoundJoin);
        break;
    }
    } // switch (pen.LineJoin)

    // dash offset
    result.setDashOffset(pen.DashOffset);

    // miter limit
    result.setMiterLimit(pen.MiterLimit);

    return result;
}

//class CBrush

//    struct TSubColor
//    {
//        long color;
//        long position; // [0..65536]
//    };

//    long Type;

//    long Color1;
//    long Color2;
//    long Alpha1;
//    long Alpha2;

//    std::wstring TexturePath;
//    long TextureAlpha;
//    long TextureMode;

//    int Rectable;
//    Aggplus::RectF Rect; float X, Y, WIDTH, HEIGHT
//    Aggplus::CDoubleRect Bounds; double top, left, right, bottom

//    double LinearAngle;
//    std::vector<TSubColor> m_arrSubColors;
//    NSStructures::GradientInfo m_oGradientInfo;

//    GradientInfo:
//    Point p0, p1;
//    float r0, r1;

//    float littleRadius, largeRadius;
//    float centerX, centerY;          // used in radial, diamond and conical gradient - offset relative to figure center
//    float angle;                     // used in linear and conical gradient (rad)
//    float discrete_step;             // used to make discrete gradient. <= 0 to make continuous
//    float xsize, ysize;              // stretch image; can be negative to reflect relative to other axis; cannot be zero
//    bool reflected;                  // 1234567 ->  1357531 works kind of like this
//    bool periodic;
//    float periods;    // number of periods (best with to colours, works as saw function in color space)
//    float linstretch; // stretch linear gradient, can be negative (eq angle = 180) can not be zero
//    float linoffset;  // offset relative to image size
//    float continue_shading_b, continue_shading_f;
//    ShadingInfo shading;

//    ShadingInfo:
//        ShadingInfo() : shading_type(Parametric), f_type(UseNew), inv_map(6){}
//        enum ShadingType
//        {
//            FunctionOnly,
//            Parametric,
//            TriangleInterpolation,
//            CurveInterpolation,
//            TensorCurveInterpolation
//        } shading_type;

//        // if UseOld old function is called, look for IGraphicsRender.put_BrushGradientColors;
//        enum ColorFunctionType
//        {
//            UseOld, UseNew
//        } f_type;
//        ColorFunction<agg::rgba8> function;
//        ColorFunction:
//        size_t RESOLUTION;
//        std::vector<std::vector<ColorT>> values;
//        float x_domain_min, x_domain_max;
//        float y_domain_min, y_domain_max;

//        // Обратное преобразование из картинки в цветовую функцию
//        std::vector<float> mapping;
//        std::vector<float> inv_map;

//        // Линейный градиент задается в pdf 2 точками
//        bool set_two_points;
//        Point point1, point2;


//        // triangle shading
//        std::vector<Point> triangle;
//        std::vector<agg::rgba8> triangle_colors;
//        std::vector<float> triangle_parameters;

//        /**
//         *  Матрица 4 на 4 заполняется как в документации к пдф 7 тип
//         * Если выбран тип 6 то значения (1,2) (2,1) (1,1) (2,2)
//         * В массиве игнормруется и заполняются автоматически, следите за переданным типом градинта
//         * (Нумерация от нуля)
//         *
//         * Наверное напишу адаптор который переводит порядок точек из 6 типа в общий.
//         */
//        std::vector<std::vector<Point>> patch;
//        std::vector<std::vector<agg::rgba8>> patch_colors; // 2 на 2 цвета по углам
//        std::vector<std::vector<float>> patch_parameters; // 2 на 2 параметра

QBrush NSConversions::brushInternalToQt(const NSStructures::CBrush &brush
                                        , const QRectF &pathRect)
{
    qDebug() << __func__;
    // type
    // color 1
    // color 2 - только для градиента
    // texture path
    // texture mode
    // texture alpha
    // gradient colors

    // type
    switch (brush.Type) {
    case c_BrushTypeSolid: {
        QBrush result;
        result.setStyle(Qt::BrushStyle::SolidPattern);

        // color 1
        QColor color1 = colorInternalToQt({brush.Color1, brush.Alpha1});
        result.setColor(color1);

        return result;
    }
    case c_BrushTypeTexture: {
        QBrush result;
//        result.setStyle(Qt::BrushStyle::TexturePattern);

        QImage texture = getTexture(brush.TexturePath
                                    , brush.TextureAlpha
                                    , brush.TextureMode
                                    , pathRect.size().toSize());

        if (texture.isNull()) {
            return result;
        }

        result.setTextureImage(texture);
        return result;
    }
    case c_BrushTypePathGradient1: // linear
    {
        QLinearGradient gradient;

        // color 1 and 2
        gradient.setColorAt(0., colorInternalToQt({brush.Color1, brush.Alpha1}));
        gradient.setColorAt(1., colorInternalToQt({brush.Color2, brush.Alpha2}));

        // sub colors
        for (const auto &subColor: brush.m_arrSubColors) {
            qreal pos = (qreal)subColor.position / 65536.;
            QColor col = colorInternalToQt(subColor.color);
            gradient.setColorAt(pos, col);
        }

        // start & stop
        QPointF startPoint = (pathRect.topLeft() + pathRect.bottomLeft()) / 2;
        QPointF stopPoint = (pathRect.topRight() + pathRect.bottomRight()) / 2;


        gradient.setStart(startPoint);
        gradient.setFinalStop(stopPoint);



        return {gradient};
    }
    case c_BrushTypePathGradient2: // radial
    {
        QPointF center =
                (pathRect.topLeft() + pathRect.bottomRight()) / 2
                ;
        qreal radius =
                (qMax(pathRect.width(), pathRect.height())) / 2
                ;
        QRadialGradient gradient{center, radius};

        // color 1 and 2
        gradient.setColorAt(0., colorInternalToQt({brush.Color1, brush.Alpha1}));
        gradient.setColorAt(1., colorInternalToQt({brush.Color2, brush.Alpha2}));

        // sub colors
        for (const auto &subColor: brush.m_arrSubColors) {
            qreal pos = (qreal)subColor.position / 65536.;
            QColor col = colorInternalToQt(subColor.color);
            gradient.setColorAt(pos, col);
        }

//        gradient.setFocalPoint(center);
//        gradient.setFocalRadius(radius);
        qDebug() << "RADIAL GRADIENT";
        qDebug() << "center" << center;
        qDebug() << "radius" << radius;
        int todo;
        // todo: radial gradient specific things
        return {gradient};
    }
    default: {
        return QBrush{};
    }
    } // switch (brush.Type)
}

bool NSConversions::FontStyle::bitNumber(int number) const
{
    return (value >> number) & 0x01;
}

void NSConversions::FontStyle::setBitNumber(int number, bool val)
{
    unsigned char result = val ? 1 : 0;
    result <<= number; // все нули, байт number значащий

    unsigned char nuller = 1;
    nuller <<= number;
    nuller = ~nuller; // все единицы, кроме байта number

    value &= nuller; // зануляем байт number
    value |= result; // ставим новое значение
}

NSConversions::FontStyle::FontStyle(LONG internalFontStyle)
{
    // from structures.h CFont::SetStyle
    // [int] Bold = (0x01 == (0x01 & lStyle));
    // [int] Italic = (0x02 == (0x02 & lStyle));
    // [unsigned char] Underline = (unsigned char)(0x7C & lStyle) >> 2;
    // [unsigned char] Strikeout = (unsigned char)(0x0180 & lStyle) >> 7;
    bool bBold = (0 != (
                0x01 // 0...01
                & internalFontStyle) // 0...0x
            );
    bool bItalic = (0 != (
                0x02 // 0...010
                & internalFontStyle) // 0...0x0
            );
    bool bUnderline = (0 != ((
                                0x7C // 01111100
                                & internalFontStyle) // 0...0xxxxx00
                            >> 2) // 0...000xxxxx
            );
    bool bStrikeout = (0 != ((
                                0x0180 // 0000000110000000
                                & internalFontStyle) // 0...0xx0000000
                            >> 7) // 0...0xx
            );
    setBold(bBold);
    setItalic(bItalic);
    setUnderline(bUnderline);
    setStrikeout(bStrikeout);
}

NSConversions::FontStyle::FontStyle(const QFont &font)
{
    setBold(font.bold());
    setItalic(font.italic());
    setUnderline(font.underline());
    setStrikeout(font.strikeOut());
}

LONG NSConversions::FontStyle::toInternalFontStyle() const
{
    BYTE iBold = bold() ? 1 : 0;
    BYTE iItalic = italic() ? 1 : 0;
    BYTE iUnderline = underline() ? 1 : 0;
    BYTE iStrikeout = strikeout() ? 1 : 0;

    //ssuuuuuib
    iBold <<= 0; // 1
    iItalic <<= 1; // 1
    iUnderline <<= 2; // 5
    iStrikeout <<= 7; // 2

    return LONG(iBold | iItalic | iUnderline | iStrikeout);
}

void NSConversions::FontStyle::assignToQFont(QFont &font) const
{
    font.setBold(bold());
    font.setItalic(italic());
    font.setUnderline(underline());
    font.setStrikeOut(strikeout());
}

bool NSConversions::FontStyle::bold() const
{
    return bitNumber(nBold);
}

void NSConversions::FontStyle::setBold(bool val)
{
    setBitNumber(nBold, val);
}

bool NSConversions::FontStyle::italic() const
{
    return bitNumber(nItalic);
}

void NSConversions::FontStyle::setItalic(bool val)
{
    setBitNumber(nItalic, val);
}

bool NSConversions::FontStyle::underline() const
{
    return bitNumber(nUnderline);
}

void NSConversions::FontStyle::setUnderline(bool val)
{
    setBitNumber(nUnderline, val);
}

bool NSConversions::FontStyle::strikeout() const
{
    return bitNumber(nStrikeout);
}

void NSConversions::FontStyle::setStrikeout(bool val)
{
    setBitNumber(nStrikeout, val);
}

QColor NSConversions::colorInternalToQt(CInternalColor internalColor)
{
    int red = (int)(internalColor.color & 0xFF);
    int green = (int)((internalColor.color >> 8) & 0xFF);
    int blue = (int)((internalColor.color >> 16) & 0xFF);
    QColor result{red, green, blue};
    result.setAlpha(internalColor.alpha);
    return result;
}

NSConversions::CInternalColor NSConversions::colorQtToInternal(const QColor &color)
{
    int r = color.red();
    int g = color.green();
    int b = color.blue();
    if (r < 0 || g < 0 || b < 0) {
        return {0, 0};
    }
    LONG ret = 0;
    ret |= b;
    ret <<= 8;
    ret |= g;
    ret <<= 8;
    ret |= r;
    return {ret, (LONG)color.alpha()};
}

QColor NSConversions::colorInternalToQt(LONG internalColor)
{
    return colorInternalToQt({internalColor, 255});
}

QFont NSConversions::fontInternalToQt(const NSStructures::CFont &font)
{
    int todo;
    return {};
}
