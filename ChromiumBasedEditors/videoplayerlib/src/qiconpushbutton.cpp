#include "qiconpushbutton.h"

#include <math.h>

#include "../qwidgetutils.h"

QIconPushButton::QIconPushButton(QWidget *parent, bool bIsSvgSupport, QString sIconR, QString sIconH, QString sIconP) : QPushButton(parent)
{
	m_bIsSvg = false;
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

bool QIconPushButton::event(QEvent *e)
{
	if (!m_bIsSvg)
		return QPushButton::event(e);

	QPushButton::event(e);
	if (e->type() == QEvent::Enter)
	{
		setIconFileName(":/icons/" + m_sIconH + ".svg");
	}
	if (e->type() == QEvent::Leave)
	{
		setIconFileName(":/icons/" + m_sIconR + ".svg");
	}
	return true;
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
	if (fabs(0.0 - m_dDpi) > 0.05 && fabs(1.0 - m_dDpi) > 0.05 && fabs(2.0 - m_dDpi) > 0.05)
	{
		if (m_bIsSvgSupport)
		{
			m_bIsSvg = true;
			setStyleSheet("QPushButton { background-color: transparent; border: none; margin:0; padding:0; outline:none; }");
			setIconFileName(":/icons/" + m_sIconR + ".svg");
			update();
			return;
		}
	}
	m_bIsSvg = false;

	QString sPostfix = ".png";
	if (fabs(2.0 - m_dDpi) < 0.1)
		sPostfix = "-2x.png";

	setIcon(QIcon());
	QString sStyle = "QPushButton {border-image:url(:/icons/" + m_sIconR + sPostfix + ") 0 0 0 0 stretch stretch;margin:0;padding:0;outline:none;}";
	sStyle += "QPushButton:hover {border-image:url(:/icons/" + m_sIconH + sPostfix + ") 0 0 0 0 stretch stretch;margin:0;padding:0;outline:none;}";
	sStyle += "QPushButton:pressed {border-image:url(:/icons/" + m_sIconP + sPostfix + ") 0 0 0 0 stretch stretch;margin:0;padding:0;outline:none;}";

	setStyleSheet(sStyle);
}

void QIconPushButton::setIconFileName(QString s)
{
	if (m_bIsSvg)
	{
		QIcon icon;
		QSize size = this->size();
		icon.addFile(s, size);
		this->setIcon(icon);
		this->setIconSize(size);
	}
}
