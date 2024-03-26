#ifndef Q_ICON_PUSH_BUTTON_H
#define Q_ICON_PUSH_BUTTON_H

#include <QWidget>
#include <QColor>
#include <QPushButton>
#include <QResizeEvent>

class QIconPushButton : public QPushButton
{
	Q_OBJECT

public:
	QIconPushButton(QWidget* parent, bool bSvgSupport, QString sIconR, QString sIconH = "", QString sIconP = "");
	virtual ~QIconPushButton();

public:
	void setIcons(QString sIconR, QString sIconH = "", QString sIconP = "");
	void setBackgrounds(const QColor& oBgR, const QColor& oBgH = QColor(), const QColor& oBgP = QColor());
	void updateStyle();

protected:
	virtual void resizeEvent(QResizeEvent* e);

private:
	QString getIconPostfix();
	QString getColorAsString(const QColor& color);

private:
	QString m_sIconR;
	QString m_sIconH;
	QString m_sIconP;

	QColor m_oBackgroundR;
	QColor m_oBackgroundH;
	QColor m_oBackgroundP;

	bool m_bIsSvgSupport;
	double m_dDpi;
};

#endif // Q_ICON_PUSH_BUTTON_H
