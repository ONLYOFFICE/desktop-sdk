#import "footerpanel.h"

#import "iconpushbutton.h"
#import "timelabel.h"
#import "utils.h"

// Helper functions for maintaining auto layout
/*
 * Sets `view` positioning as follows:
 *  - to the right of `left_anchor` with `left_offset` pixels offset
 *  - under `top_anchor` with `top_offset` pixels offset
 *  - with width and size specified by `size`
 */
void setLeftConstraintsToView(NSView* view, NSLayoutYAxisAnchor* top_anchor, NSLayoutXAxisAnchor* left_anchor,
							  CGFloat top_offset, CGFloat left_offset, NSSize size) {
	view.translatesAutoresizingMaskIntoConstraints = NO;
	[view.topAnchor constraintEqualToAnchor:top_anchor constant:top_offset].active = YES;
	[view.leftAnchor constraintEqualToAnchor:left_anchor constant:left_offset].active = YES;
	[view.widthAnchor constraintEqualToConstant:size.width].active = YES;
	[view.heightAnchor constraintEqualToConstant:size.height].active = YES;
}

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
	CFooterSkin m_skin;
	// buttons
	NSIconPushButton* m_btn_play;
	NSIconPushButton* m_btn_volume;
	NSIconPushButton* m_btn_rewind_forward;
	NSIconPushButton* m_btn_rewind_back;
	// text labels
	NSTimeLabel* m_time_label;
	// constraints
	NSLayoutConstraint* m_time_label_width_constraint;
	NSLayoutConstraint* m_time_label_height_constraint;
}
- (void)updateStyle;
@end

@implementation NSFooterPanel

- (instancetype)initWithFrame:(NSRect)frame_rect {
	self = [super initWithFrame:frame_rect];
	if (self) {
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
		m_btn_play = [[NSIconPushButton alloc] initWithIconName:@"btn-play" size:button_size skin:&m_skin];
		[self addSubview:m_btn_play];
		setLeftConstraintsToView(m_btn_play, self.topAnchor, self.leftAnchor, button_y_offset, button_space_between, button_size);
#if !__has_feature(objc_arc)
		[m_btn_play release];
#endif

		// volume button
		m_btn_volume = [[NSIconPushButton alloc] initWithIconName:@"btn-volume-2" size:button_size skin:&m_skin];
		[self addSubview:m_btn_volume];
		setRightConstraintsToView(m_btn_volume, self.topAnchor, self.rightAnchor, button_y_offset, button_space_between, button_size);
#if !__has_feature(objc_arc)
		[m_btn_volume release];
#endif

		// time label
		m_time_label = [[NSTimeLabel alloc] initWithSkin:&m_skin];
		[self addSubview:m_time_label];
		NSSize text_bounds = [m_time_label getBoundingBoxSize];
		// manually set all constraints
		m_time_label.translatesAutoresizingMaskIntoConstraints = NO;
		[m_time_label.centerYAnchor constraintEqualToAnchor:self.centerYAnchor constant:0].active = YES;
		[m_time_label.rightAnchor constraintEqualToAnchor:m_btn_volume.leftAnchor constant:-button_space_between].active = YES;
		// we need to save width and height constraints becuase changing skin may affect text width and height
		m_time_label_width_constraint = [m_time_label.widthAnchor constraintEqualToConstant:text_bounds.width];
		m_time_label_width_constraint.active = YES;
		m_time_label_height_constraint = [m_time_label.heightAnchor constraintEqualToConstant:text_bounds.height];
		m_time_label_height_constraint.active = YES;
#if !__has_feature(objc_arc)
		[m_time_label release];
#endif

		// rewind forward button
		m_btn_rewind_forward = [[NSIconPushButton alloc] initWithIconName:@"btn-rewind-forward" size:button_size skin:&m_skin];
		[self addSubview:m_btn_rewind_forward];
		setRightConstraintsToView(m_btn_rewind_forward, self.topAnchor, m_time_label.leftAnchor, button_y_offset, button_space_between, button_size);
#if !__has_feature(objc_arc)
		[m_btn_rewind_forward release];
#endif

		// rewind back button
		m_btn_rewind_back = [[NSIconPushButton alloc] initWithIconName:@"btn-rewind-back" size:button_size skin:&m_skin];
		[self addSubview:m_btn_rewind_back];
		setRightConstraintsToView(m_btn_rewind_back, self.topAnchor, m_btn_rewind_forward.leftAnchor, button_y_offset, button_space_between, button_size);
#if !__has_feature(objc_arc)
		[m_btn_rewind_back release];
#endif

		// video slider
		// TODO:
	}
	return self;
}

- (void)dealloc {
	NSLog(@"debug: footer panel deallocated");
#if !__has_feature(objc_arc)
	[super dealloc];
#endif
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
}

@end
