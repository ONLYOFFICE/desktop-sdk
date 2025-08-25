#ifndef FOOTER_PANEL_H_
#define FOOTER_PANEL_H_

#import <Cocoa/Cocoa.h>

#import "footerskin.h"
#import "iconpushbutton.h"
#import "timelabel.h"
#import "slider.h"
#import "subpanel.h"

@interface NSFooterPanel : NSView<StyledView>
{
@public
	// buttons
	NSIconPushButton* m_btn_play;
	NSIconPushButton* m_btn_volume;
	NSIconPushButton* m_btn_rewind_forward;
	NSIconPushButton* m_btn_rewind_back;
	// text labels
	NSTimeLabel* m_time_label;
	// sliders
	NSStyledSlider* m_slider_video;
	NSStyledSlider* m_slider_volume;
}
- (instancetype)initWithFrame:(NSRect)frame_rect superview:(NSView*)parent;
// appearance
- (void)applySkin:(CFooterSkin::Type)type;
- (void)updatePlayPauseButton:(float)player_rate;
- (void)toggleVolumeControls;
- (void)updateVolumeButton:(double)volume;
@end

#endif	// FOOTER_PANEL_H_
