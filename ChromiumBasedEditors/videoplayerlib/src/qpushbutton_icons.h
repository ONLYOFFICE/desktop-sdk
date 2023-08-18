#ifndef QASCICONS_ENGINE_H
#define QASCICONS_ENGINE_H

#include <QWidget>
#include <QPushButton>
#include <QResizeEvent>
#include <QPalette>
#include <math.h>

static QString getButtonBackground(QString src, QString srcH, QString srcP, double dDpi = 0.0)
{
#if 0
	QString sRet = "QPushButton {background-image:url(:/icons/" + src + ".png);border:none;margin:0;padding:0;outline:none;}";
	sRet += "QPushButton::hover {background-image:url(:/icons/" + srcH + ".png);border:none;margin:0;padding:0;outline:none;}";
	sRet += "QPushButton::pressed {background-image:url(:/icons/" + srcP + ".png);border:none;margin:0;padding:0;outline:none;}";
#else
	QString sPostfix = ".svg";
	if (fabs(1.0 - dDpi) < 0.1 || fabs(0.0 - dDpi) < 0.1)
		sPostfix = ".png";
	else if (fabs(2.0 - dDpi) < 0.1)
		sPostfix = "-2x.png";

	QString sRet = "QPushButton {border-image:url(:/icons/" + src + sPostfix + ") 0 0 0 0 stretch stretch;margin:0;padding:0;outline:none;}";
	sRet += "QPushButton::hover {border-image:url(:/icons/" + srcH + sPostfix + ") 0 0 0 0 stretch stretch;margin:0;padding:0;outline:none;}";
	sRet += "QPushButton::pressed {border-image:url(:/icons/" + srcP + sPostfix + ") 0 0 0 0 stretch stretch;margin:0;padding:0;outline:none;}";
#endif
	return sRet;
}
static QString getButtonBackground(QString src, double dDpi = 0.0)
{
	return getButtonBackground(src, src + "-hover", src + "-active", dDpi);
}
static QString getButtonBackground2(QString src, double dDpi = 0.0)
{
	return getButtonBackground(src, src + "-active", src + "-active", dDpi);
}
static QString getButtonBackgroundSimple(QString src, double dDpi = 0.0)
{
	return getButtonBackground(src, src, src, dDpi);
}

class QIconPushButtonPrivate;
class QIconPushButton : public QPushButton
{
	Q_OBJECT

public:
	QIconPushButton(QWidget *parent, bool bSvgSupport, QString sIconR, QString sIconH = "", QString sIconP = "");
	void changeIcons(QString sIconR, QString sIconH = "", QString sIconP = "");
	virtual ~QIconPushButton();

protected:
	virtual bool event(QEvent *e);
	virtual void resizeEvent(QResizeEvent* e);

private:
	QIconPushButtonPrivate* m_internal;
};

static void QWidget_SetProperty(QObject* w, const QVariant& p)
{
	w->setProperty("native_dpi", p);
	QObjectList childs = w->children();
	for (int i = childs.count() - 1; i >= 0; --i)
	{
		QWidget_SetProperty(childs.at(i), p);
	}
}

static void QWidget_SetDPI(QWidget* w, const double& v)
{
	QVariant p(v);
	QWidget_SetProperty(w, p);
}
static double QWidget_GetDPI(QWidget* w)
{
	QVariant p = w->property("native_dpi");
	if (p.isValid())
		return p.toDouble();
	return 1.0;
}
static int QWidget_ScaleDPI(const int& v, const double& s)
{
	return (int)(v * s + 0.001);
}

#include <QPalette>
static void QWidget_setBackground(QWidget* widget, int r, int g, int b)
{
#if 1
	QString s1 = QString::number(r, 16); if (s1.length() == 0) s1 = "0" + s1;
	QString s2 = QString::number(g, 16); if (s2.length() == 0) s2 = "0" + s2;
	QString s3 = QString::number(b, 16); if (s3.length() == 0) s3 = "0" + s3;
	QString sBackColor = "background-color:#" + s1 + s2 + s3 + ";";
	widget->setStyleSheet(sBackColor);
#endif

	QPalette pal;
	pal.setColor(QPalette::Window, QColor(r, g, b));
	widget->setAutoFillBackground(true);
	widget->setPalette(pal);
}


#endif // QASCICONS_ENGINE_H
