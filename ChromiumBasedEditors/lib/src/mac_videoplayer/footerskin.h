#ifndef FOOTER_SKIN_H_
#define FOOTER_SKIN_H_

#import <Foundation/NSString.h>

// views with styling options should implement this protocol
@protocol StyledView
// applies style and updates view appearance
- (void)updateStyle;
@end

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

struct CSliderStyle {
	struct CTrackStyle {
		Color color;		// color of the track (right of knob)
		Color fill_color;	// color of the filled portion of the track (left of knob)
		int thickness;
		int border_radius;
	};

	struct CKnobStyle {
		Color color;
		Color border_color;
		int thickness;
		int border_width = 0;
		int border_radius = 0;
	};

	CTrackStyle track;
	bool is_knob_visible;
	CKnobStyle knob;
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
	CSliderStyle video_slider;
	CSliderStyle volume_slider;

public:
	// some global constants (skin-independent)
	// buttons
	static constexpr int button_width = 30;
	static constexpr int button_y_offset = 5;
	static constexpr int button_space_between = 8;
	// volume controls
	static constexpr int volume_control_width = 30;
	static constexpr int volume_control_height = 140;
	static constexpr int volume_slider_width = 20;
	static constexpr int volume_slider_height = 120;
	// general footer panel params
	static constexpr int border_radius = 5;
};

#endif  // FOOTER_SKIN_H_
