#import "subpanel.h"

#import "utils.h"

@interface NSSubPanel ()
{
	CFooterStyle* m_style;
}
@end

@implementation NSSubPanel

- (instancetype)initWithStyle:(CFooterStyle*)style {
	self = [super init];
	if (self) {
		// set appearance
		[self setWantsLayer:YES];
		m_style = style;
		[self updateStyle];
		// set border radius
		self.layer.cornerRadius = CFooterSkin::border_radius;
		self.layer.masksToBounds = YES;
	}
	return self;
}

- (void)updateStyle {
	CGColorRef bg_color = [NSColorFromHex(m_style->bg_color) CGColor];
	[self.layer setBackgroundColor:bg_color];
}

- (void)dealloc {
	NSLog(@"debug: sub panel deallocated");
#if !__has_feature(objc_arc)
	[super dealloc];
#endif
}

@end
