#ifndef FOOTER_SKIN_H_
#define FOOTER_SKIN_H_

#import <Foundation/NSString.h>

// color is specified in hex format, for example: 0xF1F1F1
using Color = unsigned;

struct CFooterStyle {
	Color bg_color;
};

struct CButtonStyle {
	NSString* icon_postfix;
	Color bg_color_regular;
	Color bg_color_hovered;
	Color bg_color_pressed;
	int border_radius;
};

struct CTimeLabelStyle {
	NSString* font_name;	// may be empty string (system font will be applied)
	int font_size;
	Color color;
};

class CFooterSkin {
public:
    enum class Type {
        kLight,
        kDark
    };

public:
	/*
	 * NOTE: DO NOT use the default constructor explicitly - instead get skin instance
	 * via `getSkin()` factory function.
	 *
	 * This constructor is public only because Objective-C class requires its members
	 * to have accessible default constructors.
	 */
	CFooterSkin() = default;

public:
	static CFooterSkin getSkin(Type type);

public:
	Type type;
	CFooterStyle footer;
	CButtonStyle button;
	CTimeLabelStyle time_label;

public:
	// some global constants (skin-independent)
	static constexpr int button_width = 30;
	static constexpr int button_y_offset = 5;
	static constexpr int button_space_between = 8;

	static constexpr int volume_control_width = 30;
	static constexpr int volume_control_height = 140;

	static constexpr int volume_slider_width = 20;
	static constexpr int volume_slider_height = 120;

	static constexpr int border_radius = 5;
};

#endif  // FOOTER_SKIN_H_
