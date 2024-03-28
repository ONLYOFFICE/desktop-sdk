#include "qvideoslider.h"

#include <QStyleOption>
#include <QPainter>

#include "../qwidgetutils.h"

QVideoSlider::QVideoSlider(QWidget *parent, HandleType handleType) : QSlider(parent)
{
	double dpi = QWidgetUtils::GetDPI(parent);
	QWidgetUtils::SetDPI(this, dpi);
	m_dDpi = dpi;

	m_bIsSeekOnClick = true;

	setHandleType(handleType);
}

void QVideoSlider::mousePressEvent(QMouseEvent *event)
{
	if (!m_bIsSeekOnClick)
		return QSlider::mousePressEvent(event);

	QStyleOptionSlider opt;
	initStyleOption(&opt);
	QRect sr = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);

	if (event->button() == Qt::LeftButton && !sr.contains(event->pos()))
	{
		int newVal;

		if (orientation() == Qt::Vertical)
		{
			double halfHandleHeight = (0.5 * sr.height()) + 0.5;
			int adaptedPosY = height() - event->y();
			if ( adaptedPosY < halfHandleHeight )
				adaptedPosY = halfHandleHeight;
			if ( adaptedPosY > height() - halfHandleHeight )
				adaptedPosY = height() - halfHandleHeight;

			double newHeight = (height() - halfHandleHeight) - halfHandleHeight;
			double normalizedPosition = (adaptedPosY - halfHandleHeight)  / newHeight ;

			newVal = minimum() + qRound((maximum()-minimum()) * normalizedPosition);
		}
		else
		{
			double halfHandleWidth = (0.5 * sr.width()) + 0.5;
			int adaptedPosX = event->x();
			if ( adaptedPosX < halfHandleWidth )
				adaptedPosX = halfHandleWidth;
			if ( adaptedPosX > width() - halfHandleWidth )
				adaptedPosX = width() - halfHandleWidth;

			double newWidth = (width() - halfHandleWidth) - halfHandleWidth;
			double normalizedPosition = (adaptedPosX - halfHandleWidth)  / newWidth ;

			newVal = minimum() + qRound((maximum()-minimum()) * normalizedPosition);
		}

		if (invertedAppearance())
			this->setValue( maximum() - newVal );
		else
			this->setValue(newVal);

		QSlider::mousePressEvent(event);
	}
	else
	{
		QSlider::mousePressEvent(event);
	}
}

void QVideoSlider::resizeEvent(QResizeEvent* e)
{
	QSlider::resizeEvent(e);

	double dDpi = QWidgetUtils::GetDPI(this);
	if (fabs(dDpi - m_dDpi) > 0.01)
	{
		m_dDpi = dDpi;
		updateStyle();
	}
}

void QVideoSlider::updateStyle()
{
	int nGrooveThickness = (int)(m_dDpi * 8);		// %1
	int nGrooveBorderRadius = (int)(m_dDpi * 4);	// %2

	QString sStyle;

	switch (m_handleType)
	{
	case htSimple:
	{
		sStyle = QString("\
			QSlider::groove:horizontal	 { height: %1px; background-color: transparent; margin: 0; }\
			\
			QSlider::groove:vertical     { width: %1px; background-color: transparent; margin: 0; }\
			\
			QSlider::sub-page:horizontal { height: %1px; background-color: " + m_oStyleOpt.m_sAddColor + "; border-top-left-radius: %2px; border-top-right-radius: 0; border-bottom-right-radius: 0; border-bottom-left-radius: %2px; }\
			QSlider::add-page:horizontal { height: %1px; background-color: " + m_oStyleOpt.m_sSubColor + "; border-top-left-radius: 0; border-top-right-radius: %2px; border-bottom-right-radius: %2px; border-bottom-left-radius: 0; }\
			\
			QSlider::sub-page:vertical   { width: %1px; background-color: " + m_oStyleOpt.m_sSubColor + "; border-top-left-radius: %2px; border-top-right-radius: %2px; border-bottom-right-radius: 0; border-bottom-left-radius: 0; }\
			QSlider::add-page:vertical   { width: %1px; background-color: " + m_oStyleOpt.m_sAddColor + "; border-top-left-radius: 0; border-top-right-radius: 0; border-bottom-right-radius: %2px; border-bottom-left-radius: %2px;}\
			\
			QSlider::handle:horizontal   { width: %1px; height: %1px; background-color: " + m_oStyleOpt.m_sAddColor + "; margin: 0 1px; border-radius: %2px; } \
			QSlider::handle:vertical     { width: %1px; height: %1px; background-color: " + m_oStyleOpt.m_sAddColor + "; margin: 1px 0; border-radius: %2px; } \
			\
			").arg(QString::number(nGrooveThickness), QString::number(nGrooveBorderRadius));
		break;
	}
	case htCircle:
	{
		int nHandleWidth = (int)(m_dDpi * 16);								// %3
		int nHandleBorderSize = (int)(m_dDpi * 2);							// %4

		int nHandleTotalSize = nHandleWidth + nHandleBorderSize * 2;		// %5
		int nGrooveMargin = (nHandleTotalSize - nGrooveThickness) / 2;		// %6
		int nHandleBorderRadius = nHandleTotalSize / 2;						// %7

		sStyle = QString("\
			QSlider::groove:horizontal   { height: %5px; background-color: transparent; margin: -%6px 0 -%6px 0; }\
			\
			QSlider::groove:vertical     { width: %5px; background-color: transparent; margin: 0 -%6px 0 -%6px; }\
			\
			QSlider::sub-page:horizontal { height: %1px; background-color: " + m_oStyleOpt.m_sAddColor + "; border-top-left-radius: %2px; border-top-right-radius: 0; border-bottom-right-radius: 0; border-bottom-left-radius: %2px; }\
			QSlider::add-page:horizontal { height: %1px; background-color: " + m_oStyleOpt.m_sSubColor + "; border-top-left-radius: 0; border-top-right-radius: %2px; border-bottom-right-radius: %2px; border-bottom-left-radius: 0; }\
			\
			QSlider::sub-page:vertical   { width: %1px; background-color: " + m_oStyleOpt.m_sSubColor + "; border-top-left-radius: %2px; border-top-right-radius: %2px; border-bottom-right-radius: 0; border-bottom-left-radius: 0; }\
			QSlider::add-page:vertical   { width: %1px; background-color: " + m_oStyleOpt.m_sAddColor + "; border-top-left-radius: 0; border-top-right-radius: 0; border-bottom-right-radius: %2px; border-bottom-left-radius: %2px;}\
			\
			QSlider::handle { width: %3px; height: %3px; background-color: " + m_oStyleOpt.m_sHandleColor + "; margin: 0; border: %4px solid " + m_oStyleOpt.m_sHandleBorderColor + "; border-radius: %7px; } \
			\
			").arg(QString::number(nGrooveThickness), QString::number(nGrooveBorderRadius), QString::number(nHandleWidth), QString::number(nHandleBorderSize), QString::number(nHandleTotalSize), QString::number(nGrooveMargin), QString::number(nHandleBorderRadius));
		break;
	}
	default:
		break;
	}

	setStyleSheet(sStyle);
}

void QVideoSlider::setSeekOnClick(bool bValue)
{
	m_bIsSeekOnClick = bValue;
}

void QVideoSlider::setHandleType(HandleType handleType)
{
	m_handleType = handleType;
	updateStyle();
}

void QVideoSlider::setStyleOptions(const CSliderStyleOptions& opt)
{
	m_oStyleOpt = opt;
	updateStyle();
}
