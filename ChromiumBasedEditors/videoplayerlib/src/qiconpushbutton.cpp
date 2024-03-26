#include "qiconpushbutton.h"

#include <math.h>

#include "../qwidgetutils.h"

QIconPushButton::QIconPushButton(QWidget *parent, bool bIsSvgSupport, QString sIconR, QString sIconH, QString sIconP) : QPushButton(parent)
{
	m_bIsSvgSupport = bIsSvgSupport;

	m_dDpi = 0.0;

	changeIcons(sIconR, sIconH, sIconP);
}

QIconPushButton::~QIconPushButton()
{
}

void QIconPushButton::changeIcons(QString sIconR, QString sIconH, QString sIconP)
{
	m_sIconR = sIconR;

	if (sIconH.isEmpty() && sIconP.isEmpty())
	{
		m_sIconH = m_sIconR + "-hover";
		m_sIconP = m_sIconR + "-active";
	}
	else if (sIconP.isEmpty())
	{
		m_sIconH = sIconH;
		m_sIconP = m_sIconH;
	}
	else
	{
		m_sIconH = sIconH;
		m_sIconP = sIconP;
	}

	this->checkDpi(m_dDpi, true);
}

void QIconPushButton::resizeEvent(QResizeEvent* e)
{
	QPushButton::resizeEvent(e);
	this->checkDpi(QWidgetUtils::GetDPI(this));
}

void QIconPushButton::checkDpi(const double dDpi, const bool isAttack)
{
	if (!isAttack && fabs(dDpi - m_dDpi) < 0.05)
		return;

	m_dDpi = dDpi;

	QString sPostfix = getIconPostfix();

	QString sStyle = "QPushButton {border-image:url(:/icons/" + m_sIconR + sPostfix + ") 0 0 0 0 stretch stretch;margin:0;padding:0;outline:none;}";
	sStyle += "QPushButton:hover {border-image:url(:/icons/" + m_sIconH + sPostfix + ") 0 0 0 0 stretch stretch;margin:0;padding:0;outline:none;}";
	sStyle += "QPushButton:pressed {border-image:url(:/icons/" + m_sIconP + sPostfix + ") 0 0 0 0 stretch stretch;margin:0;padding:0;outline:none;}";

	setStyleSheet(sStyle);
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
