#ifndef STYLE_OPTIONS_H
#define STYLE_OPTIONS_H

#include <QString>
#include <QFont>

/* NOTES:
 * 1. Colors are written in the same format as in CSS (i.e. using color names, hex, rgb() or rgba()).
 * 2. Names for icons should be in format "<button_name>[-<skin_postfix>](.png|-2x.png|.svg)"
 */

struct CButtonStyleOptions
{
	// icon skin postfix
	QString m_sSkinPostfix;
	// background colors
	QString m_sBgColorR;
	QString m_sBgColorH;
	QString m_sBgColorP;
};

struct CSliderStyleOptions
{
	// groove colors
	QString m_sAddColor;			// the color of area between the start of a slider and the handle
	QString m_sSubColor;			// the color of area between the handle and the end of a slider
	// handle colors (only applied for sliders with QVideoSlider::HandleStyle = hsCircle)
	QString m_sHandleColor;
	QString m_sHandleBorderColor;
};

struct CTimeLabelOptions
{
	QFont m_oFont;
	QString m_sColor;
};

struct CFooterStyleOptions
{
	QString m_sBgColor;
	QString m_sVolumeControlBgColor;
};

#endif	// STYLE_OPTIONS_H
