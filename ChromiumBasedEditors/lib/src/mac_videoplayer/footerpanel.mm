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


@implementation NSFooterPanel

- (instancetype)initWithFrame:(NSRect)frame_rect {
	self = [super initWithFrame:frame_rect];
	if (self) {
		[self setWantsLayer:YES];
		// TODO: rework with skins
		// bg color
		CGColorRef bg_color = [NSColorFromHex(0x313437) CGColor];
		[self.layer setBackgroundColor:bg_color];
		// border radius
		self.layer.cornerRadius = 5.0;
		self.layer.masksToBounds = YES;

		// place elements
		const NSSize button_size = NSMakeSize(30, 30);
		// play button
		NSIconPushButton* btn_play = [[NSIconPushButton alloc] initWithIconName:@"btn-play" size:button_size];
		[self addSubview:btn_play];
		setLeftConstraintsToView(btn_play, self.topAnchor, self.leftAnchor, 5, 8, button_size);
		[btn_play release];
		// volume button
		NSIconPushButton* btn_volume = [[NSIconPushButton alloc] initWithIconName:@"btn-volume-2" size:button_size];
		[self addSubview:btn_volume];
		setRightConstraintsToView(btn_volume, self.topAnchor, self.rightAnchor, 5, 8, button_size);
		[btn_volume release];
		// rewind forward button
		NSIconPushButton* btn_rewind_forward = [[NSIconPushButton alloc] initWithIconName:@"btn-rewind-forward" size:button_size];
		[self addSubview:btn_rewind_forward];
		setRightConstraintsToView(btn_rewind_forward, self.topAnchor, btn_volume.leftAnchor, 5, 8, button_size);
		[btn_rewind_forward release];
	}
	return self;
}

- (void)dealloc {
	NSLog(@"debug: footer panel deallocated");
#if !__has_feature(objc_arc)
	[super dealloc];
#endif
}

@end
