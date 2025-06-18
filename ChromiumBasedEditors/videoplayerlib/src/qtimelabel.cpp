#include "qtimelabel.h"

#include <cmath>
#include <QFontMetrics>
#include <QtGlobal>

#include "../qwidgetutils.h"

QTimeLabel::QTimeLabel(QWidget* parent) : QLabel(parent)
{
	double dpi = QWidgetUtils::GetDPI(parent);
	QWidgetUtils::SetDPI(this, dpi);
	m_dDpi = dpi;

	setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
}

QTimeLabel::~QTimeLabel()
{
}

void QTimeLabel::setTime(qint64 time)
{
	qint64 nH = (qint64)(time / 3600000);
	time -= (nH * 3600000);
	qint64 nM = (qint64)(time / 60000);
	time -= (nM * 60000);
	qint64 nS = (qint64)(time / 1000);

	QString sTime = "";
	if (nH < 10)
		sTime += "0";
	sTime += QString::number((uint)nH);
	sTime += ":";
	if (nM < 10)
		sTime += "0";
	sTime += QString::number((uint)nM);
	sTime += ":";
	if (nS < 10)
		sTime += "0";
	sTime += QString::number((uint)nS);

	setText(sTime);
}

int QTimeLabel::getMaxWidth()
{
	return m_nMaxWidth;
}

void QTimeLabel::updateStyle()
{
	QFont font = m_oStyleOpt.m_oFont;
	int nPixelSize = 0;

	if (font.pointSize() > 0)
	{
		nPixelSize = QWidgetUtils::ScaleDPI(font.pointSize(), m_dDpi);
	}
	else
	{
		nPixelSize = QWidgetUtils::ScaleDPI(font.pixelSize(), m_dDpi);
	}

	font.setPixelSize(nPixelSize);
	setFont(font);

	QString sStyle = "QLabel { color: " + m_oStyleOpt.m_sColor + "; }";
	setStyleSheet(sStyle);

	// update label's max width
	QFontMetrics fontMetrics(font);
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
	m_nMaxWidth = fontMetrics.horizontalAdvance("00:00:00");
#else
	m_nMaxWidth = fontMetrics.width("00:00:00");
#endif
}

void QTimeLabel::setStyleOptions(const CTimeLabelOptions& opt)
{
	m_oStyleOpt = opt;
	updateStyle();
}

void QTimeLabel::resizeEvent(QResizeEvent* e)
{
	QLabel::resizeEvent(e);

	double dDpi = QWidgetUtils::GetDPI(this);
	if (std::abs(dDpi - m_dDpi) > 0.01)
	{
		m_dDpi = dDpi;
		updateStyle();
	}
}
