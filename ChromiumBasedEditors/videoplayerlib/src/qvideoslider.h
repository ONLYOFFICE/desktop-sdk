#pragma once

#include <QWidget>
#include <QPaintEvent>
#include <QSlider>
#include <QMouseEvent>

#include "style/style_options.h"

class QVideoSlider : public QSlider
{
	Q_OBJECT

public:
	enum HandleType
	{
		htSimple,
		htCircle
	};

public:
	explicit QVideoSlider(QWidget* parent, HandleType handleType = htSimple);

	virtual void mousePressEvent(QMouseEvent* e);
	virtual void resizeEvent(QResizeEvent* e);

public:
	void updateStyle();
	void setSeekOnClick(bool bValue);
	void setHandleType(HandleType handleType);
	void setStyleOptions(const CSliderStyleOptions& opt);

private:
	double m_dDpi;
	bool m_bIsSeekOnClick;

	CSliderStyleOptions m_oStyleOpt;

	HandleType m_handleType;
};
