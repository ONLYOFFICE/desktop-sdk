#ifndef Q_WIDGET_UTILS_H
#define Q_WIDGET_UTILS_H

#include "src/videoplayerlib_export.h"

#include <QWidget>
#include <QVariant>

namespace QWidgetUtils
{
	VIDEO_LIB_EXPORT void SetProperty(QObject* widget, const char* property, const QVariant& value);
	VIDEO_LIB_EXPORT void SetDPI(QWidget* widget, const double& dpi);
	VIDEO_LIB_EXPORT double GetDPI(QWidget* widget);
	VIDEO_LIB_EXPORT int ScaleDPI(const int& value, const double& scale);
	VIDEO_LIB_EXPORT int ScaleDPI(QWidget* widget, const int& value);
	VIDEO_LIB_EXPORT void SetBackground(QWidget* widget, int r, int g, int b);
}

#endif	// Q_WIDGET_UTILS_H
