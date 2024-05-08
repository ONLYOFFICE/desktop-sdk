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

public:
	virtual void resizeEvent(QResizeEvent* e);
	virtual bool event(QEvent* event);

private:
	QString getIconPostfix();
	void setIconSVG(const QString& sUrl);

private:
	QString m_sIconR;
	QString m_sIconH;
	QString m_sIconP;

	CButtonStyleOptions m_oStyleOpt;

	double m_dDpi;

	bool m_bIsSvgSupport;
	bool m_bIsUseSVG;
};

#endif // Q_ICON_PUSH_BUTTON_H
