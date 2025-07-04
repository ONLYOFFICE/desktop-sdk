#import "footerpanel.h"

#import "utils.h"

// Helper functions for maintaining auto layout
/*
 * Sets `view` positioning as follows:
 *  - to the left of `right_anchor` with `right_offset` pixels offset
 *  - under `top_anchor` with `top_offset` pixels offset
 *  - with width and size specified by `size`
 */
void setRightConstraintsToView(NSView* view, NSLayoutYAxisAnchor* top_anchor, NSLayoutXAxisAnchor* right_anchor,
							   CGFloat top_offset, CGFloat right_offset, NSSize size) {
	view.translatesAutoresizingMaskIntoConstraints = NO;
	[view.topAnchor constraintEqualToAnchor:top_anchor constant:top_offset].active = YES;
	[view.rightAnchor constraintEqualToAnchor:right_anchor constant:-right_offset].active = YES;
	[view.widthAnchor constraintEqualToConstant:size.width].active = YES;
	[view.heightAnchor constraintEqualToConstant:size.height].active = YES;
}

@interface NSFooterPanel ()
{
	// skin
	CFooterSkin m_skin;
	// volume control rect
	NSSubPanel* m_panel_volume;
	// constraints
	NSLayoutConstraint* m_time_label_width_constraint;
	NSLayoutConstraint* m_time_label_height_constraint;
}
@end

@implementation NSFooterPanel

- (instancetype)initWithFrame:(NSRect)frame_rect superview:(NSView*)parent {
	self = [super initWithFrame:frame_rect];
	if (self) {
		// set superview
		[parent addSubview:self positioned:NSWindowAbove relativeTo:nil];
		// set appearance
		[self setWantsLayer:YES];
		// apply light skin by default
		m_skin = CFooterSkin::getSkin(CFooterSkin::Type::kLight);
		[self updateStyle];
		// set border radius
		self.layer.cornerRadius = CFooterSkin::border_radius;
		self.layer.masksToBounds = YES;

		// place elements
		const int button_width = CFooterSkin::button_width;
		const int button_y_offset = CFooterSkin::button_y_offset;
		const int button_space_between = CFooterSkin::button_space_between;
		const NSSize button_size = NSMakeSize(button_width, button_width);

		// play button
		m_btn_play = [[NSIconPushButton alloc] initWithIconName:@"btn-play" size:button_size style:&m_skin.button];
		[self addSubview:m_btn_play];
		// constraints: place on the left side of the footer panel
		m_btn_play.translatesAutoresizingMaskIntoConstraints = NO;
		[m_btn_play.topAnchor constraintEqualToAnchor:self.topAnchor constant:button_y_offset].active = YES;
		[m_btn_play.leftAnchor constraintEqualToAnchor:self.leftAnchor constant:button_space_between].active = YES;
		[m_btn_play.widthAnchor constraintEqualToConstant:button_width].active = YES;
		[m_btn_play.heightAnchor constraintEqualToConstant:button_width].active = YES;
#if !__has_feature(objc_arc)
		[m_btn_play release];
#endif

		// volume button
		m_btn_volume = [[NSIconPushButton alloc] initWithIconName:@"btn-volume-2" size:button_size style:&m_skin.button];
		[self addSubview:m_btn_volume];
		// constraints: place on the right side of the footer panel
		setRightConstraintsToView(m_btn_volume, self.topAnchor, self.rightAnchor, button_y_offset, button_space_between, button_size);
#if !__has_feature(objc_arc)
		[m_btn_volume release];
#endif

		// time label
		m_time_label = [[NSTimeLabel alloc] initWithStyle:&m_skin.time_label];
		[self addSubview:m_time_label];
		NSSize text_bounds = [m_time_label getBoundingBoxSize];
		// constraints: place to the left of volume button
		m_time_label.translatesAutoresizingMaskIntoConstraints = NO;
		[m_time_label.centerYAnchor constraintEqualToAnchor:self.centerYAnchor].active = YES;
		[m_time_label.rightAnchor constraintEqualToAnchor:m_btn_volume.leftAnchor constant:-button_space_between].active = YES;
		// we need to save width and height constraints because changing skin may affect text width and height
		m_time_label_width_constraint = [m_time_label.widthAnchor constraintEqualToConstant:text_bounds.width];
		m_time_label_width_constraint.active = YES;
		m_time_label_height_constraint = [m_time_label.heightAnchor constraintEqualToConstant:text_bounds.height];
		m_time_label_height_constraint.active = YES;
#if !__has_feature(objc_arc)
		[m_time_label release];
#endif

		// rewind forward button
		m_btn_rewind_forward = [[NSIconPushButton alloc] initWithIconName:@"btn-rewind-forward" size:button_size style:&m_skin.button];
		[self addSubview:m_btn_rewind_forward];
		// constraints: place to the left of time label
		setRightConstraintsToView(m_btn_rewind_forward, self.topAnchor, m_time_label.leftAnchor, button_y_offset, button_space_between, button_size);
#if !__has_feature(objc_arc)
		[m_btn_rewind_forward release];
#endif

		// rewind back button
		m_btn_rewind_back = [[NSIconPushButton alloc] initWithIconName:@"btn-rewind-back" size:button_size style:&m_skin.button];
		[self addSubview:m_btn_rewind_back];
		// constraints: place to the left of rewind forward button
		setRightConstraintsToView(m_btn_rewind_back, self.topAnchor, m_btn_rewind_forward.leftAnchor, button_y_offset, button_space_between, button_size);
#if !__has_feature(objc_arc)
		[m_btn_rewind_back release];
#endif

		// video slider
		m_slider_video = [[NSStyledSlider alloc] initWithStyle:&m_skin.video_slider vertical:NO];
		[self addSubview:m_slider_video];
		// constraints: stretch between play button and rewind back button
		m_slider_video.translatesAutoresizingMaskIntoConstraints = NO;
		[m_slider_video.centerYAnchor constraintEqualToAnchor:self.centerYAnchor].active = YES;
		[m_slider_video.leftAnchor constraintEqualToAnchor:m_btn_play.rightAnchor constant:button_space_between].active = YES;
		[m_slider_video.rightAnchor constraintEqualToAnchor:m_btn_rewind_back.leftAnchor constant:-button_space_between].active = YES;
		// TODO: the whole slider area on panel becomes clickable because of this
		[m_slider_video.heightAnchor constraintEqualToConstant:frame_rect.size.height].active = YES;
#if !__has_feature(objc_arc)
		[m_slider_video release];
#endif

		// volume control rect
		m_panel_volume = [[NSSubPanel alloc] initWithStyle:&m_skin.footer];
		[m_panel_volume setHidden:YES];
		// add to parent instead of self, because volume panel should be rendered outside of the footer panel rect
		[parent addSubview:m_panel_volume positioned:NSWindowAbove relativeTo:nil];
		// constraints: place above the volume button
		m_panel_volume.translatesAutoresizingMaskIntoConstraints = NO;
		[m_panel_volume.bottomAnchor constraintEqualToAnchor:self.topAnchor constant:-button_space_between].active = YES;
		[m_panel_volume.centerXAnchor constraintEqualToAnchor:m_btn_volume.centerXAnchor].active = YES;
		[m_panel_volume.widthAnchor constraintEqualToConstant:CFooterSkin::volume_control_width].active = YES;
		[m_panel_volume.heightAnchor constraintEqualToConstant:CFooterSkin::volume_control_height].active = YES;
#if !__has_feature(objc_arc)
		[m_panel_volume release];
#endif

		// volume slider
		m_slider_volume = [[NSStyledSlider alloc] initWithStyle:&m_skin.volume_slider vertical:YES];
		[m_panel_volume addSubview:m_slider_volume];
		// constraints: place in the center of the subpanel
		m_slider_volume.translatesAutoresizingMaskIntoConstraints = NO;
		[m_slider_volume.centerYAnchor constraintEqualToAnchor:m_panel_volume.centerYAnchor].active = YES;
		[m_slider_volume.centerXAnchor constraintEqualToAnchor:m_panel_volume.centerXAnchor].active = YES;
		[m_slider_volume.widthAnchor constraintEqualToConstant:CFooterSkin::volume_slider_width].active = YES;
		[m_slider_volume.heightAnchor constraintEqualToConstant:CFooterSkin::volume_slider_height].active = YES;
#if !__has_feature(objc_arc)
		[m_slider_volume release];
#endif
	}
	return self;
}


- (void)updateStyle {
	CGColorRef bg_color = [NSColorFromHex(m_skin.footer.bg_color) CGColor];
	[self.layer setBackgroundColor:bg_color];
}

- (void)applySkin:(CFooterSkin::Type)type {
	if (type == m_skin.type)
		return;
	m_skin = CFooterSkin::getSkin(type);
	// update self
	[self updateStyle];
	// update buttons
	[m_btn_play updateStyle];
	[m_btn_volume updateStyle];
	[m_btn_rewind_forward updateStyle];
	[m_btn_rewind_back updateStyle];
	// update time label and its constraints
	[m_time_label updateStyle];
	NSSize text_bounds = [m_time_label getBoundingBoxSize];
	m_time_label_width_constraint.constant = text_bounds.width;
	m_time_label_height_constraint.constant = text_bounds.height;
	// update sliders
	[m_slider_video updateStyle];
	[m_slider_volume updateStyle];
	// update subpanels
	[m_panel_volume updateStyle];
}

- (void)updatePlayPauseButton:(float)player_rate {
	if (player_rate > 0.0) {
		[m_btn_play setIcon:@"btn-pause"];
	} else {
		[m_btn_play setIcon:@"btn-play"];
	}
}

- (void)toggleVolumeControls {
	[m_panel_volume setHidden:(!m_panel_volume.hidden)];
}

- (void)updateVolumeButton:(double)volume {
	if (volume > 75) {
		[m_btn_volume setIcon:@"btn-volume-3"];
	} else if (volume > 25) {
		[m_btn_volume setIcon:@"btn-volume-2"];
	} else if (volume > 0) {
		[m_btn_volume setIcon:@"btn-volume-1"];
	} else {
		[m_btn_volume setIcon:@"btn-volume-mute"];
	}
}

- (void)dealloc {
	NSLog(@"debug: footer panel deallocated");
	[m_panel_volume removeFromSuperview];
#if !__has_feature(objc_arc)
	[super dealloc];
#endif
}

@end
