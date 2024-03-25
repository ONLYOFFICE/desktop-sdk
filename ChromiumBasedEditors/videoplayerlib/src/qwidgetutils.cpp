#include "../qwidgetutils.h"

#include <QColor>
#include <QPalette>

namespace QWidgetUtils
{
	void SetProperty(QObject* widget, const char* property, const QVariant& value)
	{
		// set property for the widget
		widget->setProperty(property, value);
		// and for all of its children
		QObjectList childs = widget->children();
		for (int i = childs.count() - 1; i >= 0; i--)
		{
			SetProperty(childs.at(i), property, value);
		}
	}

	void SetDPI(QWidget* widget, const double& dpi)
	{
		QVariant dpiValue(dpi);
		SetProperty(widget, "native_dpi", dpiValue);
	}

	double GetDPI(QWidget* widget)
	{
		QVariant dpiValue = widget->property("native_dpi");
		if (dpiValue.isValid())
			return dpiValue.toDouble();
		return 1.0;
	}

	int ScaleDPI(const int& value, const double& scale)
	{
		return (int)(value * scale + 0.001);
	}

	int ScaleDPI(QWidget* widget, const int& value)
	{
		return ScaleDPI(value, GetDPI(widget));
	}

	void SetBackground(QWidget* widget, int r, int g, int b)
	{
		// set stylesheet
		QColor color(r, g, b);
		QString sBackColor = "background-color: " + color.name() + ";";
		widget->setStyleSheet(sBackColor);
		// set palette
		QPalette pal;
		pal.setColor(QPalette::Window, QColor(r, g, b));
		widget->setAutoFillBackground(true);
		widget->setPalette(pal);
	}
}
