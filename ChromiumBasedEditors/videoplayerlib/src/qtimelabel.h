#ifndef Q_TIME_LABEL_H
#define Q_TIME_LABEL_H

#include <QLabel>
#include <QWidget>
#include <QFont>

#include "style/style_options.h"

class QTimeLabel : public QLabel
{
	Q_OBJECT

public:
	QTimeLabel(QWidget* parent);
	~QTimeLabel();

public:
	void setTime(qint64 time);
	int getMaxWidth();

	void updateStyle();
	void setStyleOptions(const CTimeLabelOptions& opt);

public:
	virtual void resizeEvent(QResizeEvent* e);

private:
	CTimeLabelOptions m_oStyleOpt;

	double m_dDpi;
	int m_nMaxWidth;
};

#endif	// Q_TIME_LABEL_H
