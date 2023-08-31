#include "qvideoslider.h"
#include <QStyleOption>
#include <QPainter>
#include "qpushbutton_icons.h"

QVideoSlider::QVideoSlider(QWidget *parent) : QSlider(parent)
{
	QString sStyle = "\
	QSlider::groove:horizontal { background-color: transparent; border: 1px solid transparent; border-radius: 3px; background: transparent; height: 16px; \
	margin-top: -5px; margin-bottom: -5px; margin-left: 0px; margin-right:0px; border-radius: 3px; }\
	\
	QSlider::groove:vertical { background-color: transparent; border: 1px solid transparent; border-radius: 3px; background: transparent; width: 16px; \
	margin-left: -5px; margin-right: -5px; margin-top: 0px; margin-bottom:0px; border-radius: 3px; }\
	\
	QSlider::sub-page:horizontal { background-color: #9B9B9B; border: 1px solid #9B9B9B; border-radius: 3px; }\
	QSlider::add-page:horizontal { background-color: #545454; border: 1px solid #545454; border-radius: 3px; }\
	\
	QSlider::sub-page:vertical { background-color: #545454; border: 1px solid #545454; border-radius: 3px; }\
	QSlider::add-page:vertical { background-color: #9B9B9B; border: 1px solid #9B9B9B; border-radius: 3px; }\
	\
	QSlider::handle { background-color: #FFFFFF; border: 1px solid #FFFFFF; border-radius: 7px;\
	width: 14px; height: 14px; margin-top: 0px; margin-bottom: 0px; }\
	\
	QSlider::sub-page:disabled { background: #545454; border-color: #545454; }\
	\
	QSlider::add-page:disabled { background: #545454; border-color: #545454; }\
	\
	QSlider::handle:disabled { background: #545454; border: 1px solid #545454; border-radius: 8px;\
}\
			";
			m_dDpi = 1;
	this->setStyleSheet(sStyle);
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
	double dDpi = QWidget_GetDPI(this);
	if (fabs(dDpi - m_dDpi) > 0.01)
	{
		m_dDpi = dDpi;
		int n8 = (int)(m_dDpi * 7);
		int n14 = (int)(m_dDpi * 14);
		int n3 = (int)(m_dDpi * 3);
		int n5 = (int)(m_dDpi * 5);
		int n16 = (int)(m_dDpi * 16);
		int n1 = (int)(m_dDpi * 1);

		QString sStyle = QString("\
		QSlider::groove:horizontal { background-color: transparent; border: %6px solid transparent; border-radius: %3px; background: transparent; height: %5px; \
								  margin-top: -%4px; margin-bottom: -%4px; margin-left: 0px; margin-right:0px; border-radius: %3px; }\
		\
		QSlider::groove:vertical { background-color: transparent; border: %6px solid transparent; border-radius: %3px; background: transparent; width: %5px; \
								margin-left: -%4px; margin-right: -%4px; margin-top: 0px; margin-bottom:0px; border-radius: %3px; }\
		\
		QSlider::sub-page:horizontal { background-color: #9B9B9B; border: %6px solid #9B9B9B; border-radius: %3px; }\
		QSlider::add-page:horizontal { background-color: #545454; border: %6px solid #545454; border-radius: %3px; }\
		\
		QSlider::sub-page:vertical { background-color: #545454; border: %6px solid #545454; border-radius: %3px; }\
		QSlider::add-page:vertical { background-color: #9B9B9B; border: %6px solid #9B9B9B; border-radius: %3px; }\
		\
		QSlider::handle { background-color: #FFFFFF; border: %6px solid #FFFFFF; border-radius: %1px;\
					   width: %2px; height: %2px; margin-top: 0px; margin-bottom: 0px; }\
		\
		QSlider::sub-page:disabled { background: #545454; border-color: #545454; }\
		\
		QSlider::add-page:disabled { background: #545454; border-color: #545454; }\
		\
		QSlider::handle:disabled { background: #545454; border: %6px solid #545454; border-radius: %1px;\
		}\
		").arg(QString::number(n8), QString::number(n14), QString::number(n3), QString::number(n5), QString::number(n16), QString::number(n1));
								 this->setStyleSheet(sStyle);
	}
}

void QVideoSlider::SetSeekOnClick(bool bValue)
{
	m_bIsSeekOnClick = bValue;
}

