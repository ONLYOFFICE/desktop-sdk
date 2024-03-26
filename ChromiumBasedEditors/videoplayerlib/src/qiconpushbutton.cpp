#include "qiconpushbutton.h"

#include <math.h>

#include "../qwidgetutils.h"

QIconPushButton::QIconPushButton(QWidget *parent, bool bIsSvgSupport, QString sIconR, QString sIconH, QString sIconP) : QPushButton(parent)
{
	double dpi = QWidgetUtils::GetDPI(parent);
	QWidgetUtils::SetDPI(this, dpi);
	m_dDpi = dpi;

	m_bIsSvgSupport = bIsSvgSupport;

	setIcons(sIconR, sIconH, sIconP);
}

QIconPushButton::~QIconPushButton()
{
}

void QIconPushButton::setIcons(QString sIconR, QString sIconH, QString sIconP)
{
	m_sIconR = sIconR;
	m_sIconH = sIconH.isEmpty() ? m_sIconR : sIconH;
	m_sIconP = sIconP.isEmpty() ? m_sIconH : sIconP;

	updateStyle();
}

void QIconPushButton::setBackgrounds(const QColor& oBgR, const QColor& oBgH, const QColor& oBgP)
{
	m_oBackgroundR = oBgR;
	m_oBackgroundH = oBgH.isValid() ? oBgH : m_oBackgroundR;
	m_oBackgroundP = oBgP.isValid() ? oBgP : m_oBackgroundH;

	updateStyle();
}

void QIconPushButton::updateStyle()
{
	QString sPostfix = getIconPostfix();

	QString sStyle = "QPushButton {border-image:url(:/icons/" + m_sIconR + sPostfix + ") 0 0 0 0 stretch stretch; background-color: " + getColorAsString(m_oBackgroundR) + ";margin:0;padding:0;outline:none;}";
	sStyle += "QPushButton:hover {border-image:url(:/icons/" + m_sIconH + sPostfix + ") 0 0 0 0 stretch stretch; background-color: " + getColorAsString(m_oBackgroundH) + ";margin:0;padding:0;outline:none;}";
	sStyle += "QPushButton:pressed {border-image:url(:/icons/" + m_sIconP + sPostfix + ") 0 0 0 0 stretch stretch; background-color: " + getColorAsString(m_oBackgroundP) + ";margin:0;padding:0;outline:none;}";

	setStyleSheet(sStyle);
}

void QIconPushButton::resizeEvent(QResizeEvent* e)
{
	QPushButton::resizeEvent(e);

	double dpi = QWidgetUtils::GetDPI(this);
	if (fabs(dpi - m_dDpi) > 0.01)
	{
		m_dDpi = dpi;
		updateStyle();
	}
}

QString QIconPushButton::getIconPostfix()
{
	// use SVG icons if scale is fractional
	if (fabs(0.0 - m_dDpi) > 0.05 && fabs(1.0 - m_dDpi) > 0.05 && fabs(2.0 - m_dDpi) > 0.05)
	{
		if (m_bIsSvgSupport)
			return ".svg";
	}
	// use PNG icons otherwise
	if (fabs(2.0 - m_dDpi) < 0.1)
		return "-2x.png";
	return ".png";
}

QString QIconPushButton::getColorAsString(const QColor& color)
{
	if (!color.isValid())
		return "transparent";

	if (color.alpha() == 255)
		return color.name();

	return QString("rgba(%1, %2, %3, %4)").arg(QString::number(color.red()), QString::number(color.green()), QString::number(color.blue()), QString::number(color.alpha()));
}
