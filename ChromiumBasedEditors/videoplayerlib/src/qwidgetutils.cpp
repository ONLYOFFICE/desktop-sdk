#include "../qwidgetutils.h"

#include <QColor>
#include <QPalette>

// for masking widget
// TODO: remove
#include <QPainter>
#include <QPixmap>
#include <QBitmap>

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

	void SetBackground(QWidget* widget, const QColor& color)
	{
		// set stylesheet
		QString sBackColor = "background-color: " + color.name() + ";";
		widget->setStyleSheet(sBackColor);
		// set palette
		// QPalette pal;
		// pal.setColor(QPalette::Window, color);
		// widget->setAutoFillBackground(true);
		// widget->setPalette(pal);
	}

	void SetRoundedRectMask(QWidget* widget, int nRadius)
	{
		const int width = widget->width();
		const int height = widget->height();

		QPixmap pixmap(width, height);
		pixmap.fill(QColor(Qt::transparent));
		QPainter painter(&pixmap);
		// mask is a bitmap, so there is no need for antialiasing
		// painter.setRenderHint(QPainter::Antialiasing);
		painter.setPen(Qt::NoPen);
		painter.setBrush(QColor(Qt::white));
		int radius = ScaleDPI(nRadius, GetDPI(widget));
		painter.drawRoundedRect(0, 0, width, height, radius, radius);

		widget->setMask(pixmap.mask());
	}

}
