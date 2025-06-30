#include "footerskin.h"

CFooterSkin CFooterSkin::getSkin(Type type) {
	CFooterSkin skin;
	skin.type = type;
	switch (type) {
		case Type::kLight: {
			skin.footer.bg_color = 0xF1F1F1;

			skin.button.icon_postfix = @"-light";
			skin.button.bg_color_regular = 0xF1F1F1;
			skin.button.bg_color_hovered = 0xCBCBCB;
			skin.button.bg_color_pressed = 0xCBCBCB;
			skin.button.border_radius = 3;

			// skin.m_oSliderStyleOpt1.m_sAddColor = "#848484";
			// skin.m_oSliderStyleOpt1.m_sSubColor = "#E2E2E2";
			// skin.m_oSliderStyleOpt1.m_sHandleColor = "#FFFFFF";
			// skin.m_oSliderStyleOpt1.m_sHandleBorderColor = "#444444";

			// skin.m_oSliderStyleOpt2.m_sAddColor = "#C0C0C0";
			// skin.m_oSliderStyleOpt2.m_sSubColor = "#E2E2E2";
			// skin.m_oSliderStyleOpt2.m_sHandleColor = "#FFFFFF";
			// skin.m_oSliderStyleOpt2.m_sHandleBorderColor = "#444444";

			skin.time_label.font_name = @"";
			skin.time_label.font_size = 14;
			skin.time_label.color = 0x444444;
			break;
		}
		case Type::kDark: {
			skin.footer.bg_color = 0x313437;

			skin.button.icon_postfix = @"-dark";
			skin.button.bg_color_regular = 0x313437;
			skin.button.bg_color_hovered = 0x46494B;
			skin.button.bg_color_pressed = 0x46494B;
			skin.button.border_radius = 3;

			// skin.m_oSliderStyleOpt1.m_sAddColor = "#9B9B9B";
			// skin.m_oSliderStyleOpt1.m_sSubColor = "#545454";
			// skin.m_oSliderStyleOpt1.m_sHandleColor = "#FFFFFF";
			// skin.m_oSliderStyleOpt1.m_sHandleBorderColor = "#222222";

			// skin.m_oSliderStyleOpt2.m_sAddColor = "#808080";
			// skin.m_oSliderStyleOpt2.m_sSubColor = "#545454";
			// skin.m_oSliderStyleOpt2.m_sHandleColor = "#FFFFFF";
			// skin.m_oSliderStyleOpt2.m_sHandleBorderColor = "#222222";

			skin.time_label.font_name = @"";
			skin.time_label.font_size = 14;
			skin.time_label.color = 0xD5D6D6;
			break;
		}
		default:
			// should never happen
			break;
	}
	return skin;
}
