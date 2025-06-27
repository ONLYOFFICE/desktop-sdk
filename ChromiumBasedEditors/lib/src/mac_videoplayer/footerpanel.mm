#import "footerpanel.h"

#import "iconpushbutton.h"
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
		[m_btn_play release];
		// volume button
		m_btn_volume = [[NSIconPushButton alloc] initWithIconName:@"btn-volume-2" size:button_size skin:&m_skin];
		[self addSubview:m_btn_volume];
		setRightConstraintsToView(m_btn_volume, self.topAnchor, self.rightAnchor, button_y_offset, button_space_between, button_size);
		[m_btn_volume release];
		// rewind forward button
		m_btn_rewind_forward = [[NSIconPushButton alloc] initWithIconName:@"btn-rewind-forward" size:button_size skin:&m_skin];
		[self addSubview:m_btn_rewind_forward];
		setRightConstraintsToView(m_btn_rewind_forward, self.topAnchor, m_btn_volume.leftAnchor, button_y_offset, button_space_between, button_size);
		[m_btn_rewind_forward release];
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
	[self updateStyle];
	[m_btn_play updateStyle];
	[m_btn_volume updateStyle];
	[m_btn_rewind_forward updateStyle];
}

@end
