#ifndef Q_ICON_PUSH_BUTTON_H
#define Q_ICON_PUSH_BUTTON_H

#include <QWidget>
#include <QPushButton>
#include <QResizeEvent>

class QIconPushButton : public QPushButton
{
	Q_OBJECT

public:
	QIconPushButton(QWidget* parent, bool bSvgSupport, QString sIconR, QString sIconH = "", QString sIconP = "");
	virtual ~QIconPushButton();

public:
	void changeIcons(QString sIconR, QString sIconH = "", QString sIconP = "");

protected:
	virtual void resizeEvent(QResizeEvent* e);

private:
	void checkDpi(const double dDpi, const bool isAttack = false);
	QString getIconPostfix();

private:
	QString m_sIconR;
	QString m_sIconH;
	QString m_sIconP;

	bool m_bIsSvgSupport;
	double m_dDpi;
};

#endif // Q_ICON_PUSH_BUTTON_H
