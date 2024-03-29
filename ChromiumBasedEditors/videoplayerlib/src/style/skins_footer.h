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
	CFooterStyleOptions m_oFooterStyleOpt;
	CButtonStyleOptions m_oButtonStyleOpt;
	CSliderStyleOptions m_oSliderStyleOpt1;
	CSliderStyleOptions m_oSliderStyleOpt2;
};

#endif	// SKINS_FOOTER_PANEL_H
