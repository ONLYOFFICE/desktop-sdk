#ifndef Q_ICON_PUSH_BUTTON_H
#define Q_ICON_PUSH_BUTTON_H

#include <QWidget>
#include <QPushButton>
#include <QResizeEvent>

#include "style/style_options.h"

class QIconPushButton : public QPushButton
{
	Q_OBJECT

public:
	QIconPushButton(QWidget* parent, bool bSvgSupport, QString sIconR, QString sIconH = "", QString sIconP = "");
	virtual ~QIconPushButton();

public:
	void setIcons(QString sIconR, QString sIconH = "", QString sIconP = "");
	void updateStyle();
	void setStyleOptions(const CButtonStyleOptions& opt);

protected:
	virtual void resizeEvent(QResizeEvent* e);

private:
	QString getIconPostfix();

private:
	QString m_sIconR;
	QString m_sIconH;
	QString m_sIconP;

	CButtonStyleOptions m_oStyleOpt;

	bool m_bIsSvgSupport;
	double m_dDpi;
};

#endif // Q_ICON_PUSH_BUTTON_H
