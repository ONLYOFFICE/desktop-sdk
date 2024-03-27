#pragma once

#include <QWidget>
#include <QPaintEvent>
#include <QSlider>
#include <QMouseEvent>

class QVideoSlider : public QSlider
{
	Q_OBJECT

public:
	enum HandleStyle
	{
		hsSimple,
		hsCircle
	};

public:
	explicit QVideoSlider(QWidget* parent, HandleStyle handleStyle = hsSimple);

	virtual void mousePressEvent(QMouseEvent* e);
	virtual void resizeEvent(QResizeEvent* e);

public:
	void updateStyle();
	void setSeekOnClick(bool bValue);
	void setHandleStyle(HandleStyle handleStyle);

private:
	double m_dDpi;
	bool m_bIsSeekOnClick;

	HandleStyle m_handleStyle;
};
