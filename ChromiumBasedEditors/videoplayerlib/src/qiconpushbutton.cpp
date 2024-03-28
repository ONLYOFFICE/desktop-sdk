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

void QIconPushButton::updateStyle()
{
	QString sPostfix = getIconPostfix();

	QString sStyle = "QPushButton {border-image:url(:/icons/" + m_sIconR + sPostfix + ") 0 0 0 0 stretch stretch; background-color: " + m_oStyleOpt.m_sBgColorR + ";margin:0;padding:0;outline:none;}";
	sStyle += "QPushButton:hover {border-image:url(:/icons/" + m_sIconH + sPostfix + ") 0 0 0 0 stretch stretch; background-color: " + m_oStyleOpt.m_sBgColorH + ";margin:0;padding:0;outline:none;}";
	sStyle += "QPushButton:pressed {border-image:url(:/icons/" + m_sIconP + sPostfix + ") 0 0 0 0 stretch stretch; background-color: " + m_oStyleOpt.m_sBgColorP + ";margin:0;padding:0;outline:none;}";

	setStyleSheet(sStyle);
}

void QIconPushButton::setStyleOptions(const CButtonStyleOptions& opt)
{
	m_oStyleOpt = opt;
	updateStyle();
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
	QString sPostfix = m_oStyleOpt.m_sSkinPostfix;
	if (fabs(0.0 - m_dDpi) > 0.05 && fabs(1.0 - m_dDpi) > 0.05 && fabs(2.0 - m_dDpi) > 0.05 && m_bIsSvgSupport)
	{
		// use SVG icon if scale is fractional
		sPostfix += ".svg";
	}
	else if (fabs(2.0 - m_dDpi) < 0.1)
	{
		// use double-scaled PNG icon if scale is 2.0
		sPostfix += "-2x.png";
	}
	else
	{
		// use regular PNG icon otherwise
		sPostfix += ".png";
	}
	return sPostfix;
}
