#import "playercontroller.h"

#include <cmath>
#include <algorithm>

@interface NSPlayerControllerBridge ()
{
	AVPlayer* m_player;
	NSFooterPanel* m_footer;
	// observers
	id m_time_observer_token;
}
// button callbacks
- (void)onBtnPlayPausePressed:(NSIconPushButton*)sender;
- (void)onBtnVolumePressed:(NSIconPushButton*)sender;
- (void)onBtnRewindForwardPressed:(NSIconPushButton*)sender;
- (void)onBtnRewindBackPressed:(NSIconPushButton*)sender;
// slider callbacks
- (void)onSliderVideoChanged:(NSStyledSlider*)sender;
- (void)onSliderVolumeChanged:(NSStyledSlider*)sender;
// player callbacks
- (void)onPlayerTimeChanged:(CMTime)time;
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

		// TIME SLIDER
		// UI slider action
		[m_footer->m_slider_video setTarget:self];
		[m_footer->m_slider_video setAction:@selector(onSliderVideoChanged:)];
		// player
		double observe_interval_sec = 0.1;
		m_time_observer_token = [m_player addPeriodicTimeObserverForInterval:CMTimeMakeWithSeconds(observe_interval_sec, NSEC_PER_SEC)
										  queue:dispatch_get_main_queue()
										  usingBlock:^(CMTime time) {
			[self onPlayerTimeChanged:time];
		}];

		// TIME REWIND
		// UI button actions
		[m_footer->m_btn_rewind_forward setTarget:self];
		[m_footer->m_btn_rewind_forward setAction:@selector(onBtnRewindForwardPressed:)];
		[m_footer->m_btn_rewind_back setTarget:self];
		[m_footer->m_btn_rewind_back setAction:@selector(onBtnRewindBackPressed:)];
	}
	return self;
}

- (void)onBtnPlayPausePressed:(NSIconPushButton*)sender {
	[self togglePause];
}

- (void)onBtnVolumePressed:(NSIconPushButton*)sender {
	[m_footer toggleVolumeControls];
}

- (void)onBtnRewindForwardPressed:(NSIconPushButton*)sender {
	// TODO: cache duration value
	CMTime duration = m_player.currentItem.duration;
	double duration_sec = CMTimeGetSeconds(duration);
	if (std::isfinite(duration_sec)) {
		CMTime curr_time = m_player.currentTime;
		double curr_time_sec = CMTimeGetSeconds(curr_time);
		CMTime seek_time = CMTimeMakeWithSeconds(std::min(curr_time_sec + 5, duration_sec), NSEC_PER_SEC);
		[m_player seekToTime:seek_time toleranceBefore:kCMTimeZero toleranceAfter:kCMTimeZero];
	}
}

- (void)onBtnRewindBackPressed:(NSIconPushButton*)sender {
	// TODO: cache duration value
	CMTime duration = m_player.currentItem.duration;
	double duration_sec = CMTimeGetSeconds(duration);
	if (std::isfinite(duration_sec)) {
		CMTime curr_time = m_player.currentTime;
		double curr_time_sec = CMTimeGetSeconds(curr_time);
		CMTime seek_time = CMTimeMakeWithSeconds(std::max(curr_time_sec - 5, 0.0), NSEC_PER_SEC);
		[m_player seekToTime:seek_time toleranceBefore:kCMTimeZero toleranceAfter:kCMTimeZero];
	}
}

- (void)onSliderVideoChanged:(NSStyledSlider*)sender {
	// TODO: cache duration value
	CMTime duration = m_player.currentItem.duration;
	double duration_sec = CMTimeGetSeconds(duration);
	if (std::isfinite(duration_sec)) {
		double seek_sec = (sender.doubleValue / sender.maxValue) * duration_sec;
		CMTime seek_time = CMTimeMakeWithSeconds(seek_sec, NSEC_PER_SEC);
		[m_player seekToTime:seek_time toleranceBefore:kCMTimeZero toleranceAfter:kCMTimeZero];
	}
}

- (void)onSliderVolumeChanged:(NSStyledSlider*)sender {
	[self setVolume:(sender.doubleValue / sender.maxValue)];
}

- (void)onPlayerTimeChanged:(CMTime)time {
	// TODO: cache duration value
	CMTime duration = m_player.currentItem.duration;
	double duration_sec = CMTimeGetSeconds(duration);
	if (std::isfinite(duration_sec)) {
		// update slider position
		double time_sec = CMTimeGetSeconds(time);
		double time_value = (time_sec / duration_sec) * m_footer->m_slider_video.maxValue;
		[m_footer->m_slider_video setDoubleValue:time_value];
		// update time label
		[m_footer->m_time_label setTime:time_sec];
	}
}

- (void)observeValueForKeyPath:(NSString*)key_path ofObject:(id)object change:(NSDictionary<NSString*, id>*)change context:(void*)context {
	if ([key_path isEqualToString:@"rate"]) {
		float rate = [change[NSKeyValueChangeNewKey] floatValue];
		[m_footer updatePlayPauseButton:rate];
	} else if ([key_path isEqualToString:@"volume"]) {
		double volume = [change[NSKeyValueChangeNewKey] floatValue] * m_footer->m_slider_volume.maxValue;
		[m_footer->m_slider_volume setDoubleValue:volume];
		[m_footer updateVolumeButton:volume];
	}
}

- (void)removeObservers {
	[m_player removeObserver:self forKeyPath:@"rate"];
	[m_player removeObserver:self forKeyPath:@"volume"];
	[m_player removeTimeObserver:m_time_observer_token];
	m_time_observer_token = nil;
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
