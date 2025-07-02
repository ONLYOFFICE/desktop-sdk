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

			skin.video_slider.track.color = 0xE2E2E2;
			skin.video_slider.track.fill_color = 0x848484;
			skin.video_slider.track.thickness = 8;
			skin.video_slider.track.border_radius = 4;
			skin.video_slider.is_knob_visible = false;

			skin.volume_slider.track.color = 0xE2E2E2;
			skin.volume_slider.track.fill_color = 0xC0C0C0;
			skin.volume_slider.track.thickness = 8;
			skin.volume_slider.track.border_radius = 4;
			skin.volume_slider.is_knob_visible = true;
			skin.volume_slider.knob.color = 0xFFFFFF;
			skin.volume_slider.knob.border_color = 0x444444;
			skin.volume_slider.knob.thickness = 16;
			skin.volume_slider.knob.border_width = 2;
			skin.volume_slider.knob.border_radius = 8;

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

			skin.video_slider.track.color = 0x545454;
			skin.video_slider.track.fill_color = 0x9B9B9B;
			skin.video_slider.track.thickness = 8;
			skin.video_slider.track.border_radius = 4;
			skin.video_slider.is_knob_visible = false;

			skin.volume_slider.track.color = 0x545454;
			skin.volume_slider.track.fill_color = 0x808080;
			skin.volume_slider.track.thickness = 8;
			skin.volume_slider.track.border_radius = 4;
			skin.volume_slider.is_knob_visible = true;
			skin.volume_slider.knob.color = 0xFFFFFF;
			skin.volume_slider.knob.border_color = 0x222222;
			skin.volume_slider.knob.thickness = 16;
			skin.volume_slider.knob.border_width = 2;
			skin.volume_slider.knob.border_radius = 8;

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
