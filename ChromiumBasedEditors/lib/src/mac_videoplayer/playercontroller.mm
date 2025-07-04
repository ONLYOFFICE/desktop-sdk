#import "playercontroller.h"

@interface NSPlayerControllerBridge ()
{
	AVPlayer* m_player;
	NSFooterPanel* m_footer;
}
// button callbacks
- (void)onBtnPlayPausePressed:(NSIconPushButton*)sender;
- (void)onBtnVolumePressed:(NSIconPushButton*)sender;
- (void)onBtnRewindBackPressed:(NSIconPushButton*)sender;
- (void)onBtnRewindForwardPressed:(NSIconPushButton*)sender;
// slider callbacks
- (void)onSliderVolumeChanged:(NSIconPushButton*)sender;
@end

@implementation NSPlayerControllerBridge

- (instancetype)initWithPlayer:(AVPlayer*)player footer:(NSFooterPanel*)footer {
	self = [super init];
	if (self) {
		m_player = player;
		m_footer = footer;

		// PLAY & PAUSE
		// UI button action
		[m_footer->m_btn_play setTarget:self];
		[m_footer->m_btn_play setAction:@selector(onBtnPlayPausePressed:)];
		// player rate observer
		[m_player addObserver:self forKeyPath:@"rate" options:NSKeyValueObservingOptionNew context:nil];

		// VOLUME
		// UI button action
		[m_footer->m_btn_volume setTarget:self];
		[m_footer->m_btn_volume setAction:@selector(onBtnVolumePressed:)];
		// UI slider action
		[m_footer->m_slider_volume setTarget:self];
		[m_footer->m_slider_volume setAction:@selector(onSliderVolumeChanged:)];
		// player volume observer
		[m_player addObserver:self forKeyPath:@"volume" options:NSKeyValueObservingOptionNew context:nil];

		// TODO: change
		[m_footer->m_btn_rewind_forward setTarget:self];
		[m_footer->m_btn_rewind_forward setAction:@selector(onBtnRewindForwardPressed:)];
	}
	return self;
}

- (void)onBtnPlayPausePressed:(NSIconPushButton*)sender {
	[self togglePause];
}

- (void)onBtnVolumePressed:(NSIconPushButton*)sender {
	[m_footer toggleVolumeControls];
}

- (void)onBtnRewindBackPressed:(NSIconPushButton*)sender {

}

- (void)onBtnRewindForwardPressed:(NSIconPushButton*)sender {
	[self toggleMute];
}

- (void)onSliderVolumeChanged:(NSIconPushButton*)sender {
	[self setVolume:(sender.doubleValue / 100.0)];
}

- (void)observeValueForKeyPath:(NSString*)key_path ofObject:(id)object change:(NSDictionary<NSString*, id>*)change context:(void*)context {
	if ([key_path isEqualToString:@"rate"]) {
		float rate = [change[NSKeyValueChangeNewKey] floatValue];
		[m_footer updatePlayPauseButton:rate];
	} else if ([key_path isEqualToString:@"volume"]) {
		double volume = [change[NSKeyValueChangeNewKey] floatValue] * 100.0;
		[m_footer->m_slider_volume setDoubleValue:volume];
		[m_footer updateVolumeButton:volume];
	}
}

- (void)removeObservers {
	[m_player removeObserver:self forKeyPath:@"rate"];
	[m_player removeObserver:self forKeyPath:@"volume"];
}

- (void)play {
	[m_player play];
}

- (void)pause {
	[m_player pause];
}

- (void)togglePause {
	if (m_player.rate > 0.0) {
		[self pause];
	} else {
		[self play];
	}
}

- (void)setVolume:(CGFloat)value {
	m_player.volume = value;
}

- (void)toggleMute {
	// TODO: restore previous volume on second call
	[self setVolume:0];
}

- (void)stop {
	// TODO
}

- (void)dealloc {
	NSLog(@"debug: player controller deallocated");
#if !__has_feature(objc_arc)
	[super dealloc];
#endif
}
@end
