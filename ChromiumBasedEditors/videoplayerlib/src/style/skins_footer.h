#ifndef SKINS_FOOTER_PANEL_H
#define SKINS_FOOTER_PANEL_H

#include "style_options.h"

class CFooterSkin
{
public:
	enum Type
	{
		tLight,
		tDark
	};

private:
	CFooterSkin() = default;

public:
	static CFooterSkin getSkin(Type type);

public:
	CButtonStyleOptions m_oButtonStyleOpt;
	CSliderStyleOptions m_oSliderStyleOpt;
	CFooterStyleOptions m_oFooterStyleOpt;
};

#endif	// SKINS_FOOTER_PANEL_H
