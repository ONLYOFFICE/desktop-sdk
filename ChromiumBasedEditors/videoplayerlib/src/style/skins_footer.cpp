#include "skins_footer.h"

CFooterSkin CFooterSkin::getSkin(Type type)
{
	CFooterSkin skin;
	switch (type)
	{
	case tLight:
	{
		break;
	}
	case tDark:
	{
		skin.m_oFooterStyleOpt.m_sBgColor = "#313437";
		skin.m_oFooterStyleOpt.m_sVolumeControlBgColor = "#313437";

		skin.m_oButtonStyleOpt.m_sSkinPostfix = "";
		skin.m_oButtonStyleOpt.m_sBgColorR = "transparent";
		skin.m_oButtonStyleOpt.m_sBgColorH = "transparent";
		skin.m_oButtonStyleOpt.m_sBgColorP = "transparent";

		skin.m_oSliderStyleOpt.m_sAddColor = "#9B9B9B";
		skin.m_oSliderStyleOpt.m_sSubColor = "#545454";
		skin.m_oSliderStyleOpt.m_sHandleColor = "#FFFFFF";
		skin.m_oSliderStyleOpt.m_sHandleBorderColor = "#222222";
		break;
	}
	default:
		// should never happen
		break;
	}

	return skin;
}
