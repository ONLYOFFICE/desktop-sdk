#import "playercontroller.h"

#include <cmath>
#include <algorithm>

@interface NSPlayerControllerBridge ()
{
	AVPlayer* m_player;
	NSVideoView* m_video_view;
	NSFooterPanel* m_footer;
	// observers
	id m_time_observer_token;
	// media info
	double m_duration_sec;
	bool m_is_video;
	// time slider dragging info
	bool m_is_drag;
	float m_rate_before_drag;
}
// button callbacks
- (void)onBtnPlayPausePressed:(NSIconPushButton*)sender;
- (void)onBtnVolumePressed:(NSIconPushButton*)sender;
- (void)onBtnRewindForwardPressed:(NSIconPushButton*)sender;
- (void)onBtnRewindBackPressed:(NSIconPushButton*)sender;
// slider callbacks
- (void)onSliderVideoChanged:(NSStyledSlider*)sender;
- (void)onSliderVideoPressed:(NSStyledSlider*)sender;
- (void)onSliderVideoReleased:(NSStyledSlider*)sender;
- (void)onSliderVolumeChanged:(NSStyledSlider*)sender;
// player callbacks
- (void)onPlayerTimeChanged:(CMTime)time;
@end

@implementation NSPlayerControllerBridge

- (instancetype)initWithPlayer:(AVPlayer*)player videoView:(NSVideoView*)video_view footer:(NSFooterPanel*)footer {
	self = [super init];
	if (self) {
		m_player = player;
		m_video_view = video_view;
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
		// UI slider actions
		[m_footer->m_slider_video setTarget:self];
		[m_footer->m_slider_video setAction:@selector(onSliderVideoChanged:)];
		[m_footer->m_slider_video setActionPress:@selector(onSliderVideoPressed:)];
		[m_footer->m_slider_video setActionRelease:@selector(onSliderVideoReleased:)];
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
	CMTime curr_time = m_player.currentTime;
	double curr_time_sec = CMTimeGetSeconds(curr_time);
	CMTime seek_time = CMTimeMakeWithSeconds(std::min(curr_time_sec + 5, m_duration_sec), NSEC_PER_SEC);
	[m_player seekToTime:seek_time toleranceBefore:kCMTimeZero toleranceAfter:kCMTimeZero];
}

- (void)onBtnRewindBackPressed:(NSIconPushButton*)sender {
	CMTime curr_time = m_player.currentTime;
	double curr_time_sec = CMTimeGetSeconds(curr_time);
	CMTime seek_time = CMTimeMakeWithSeconds(std::max(curr_time_sec - 5, 0.0), NSEC_PER_SEC);
	[m_player seekToTime:seek_time toleranceBefore:kCMTimeZero toleranceAfter:kCMTimeZero];
}

- (void)onSliderVideoChanged:(NSStyledSlider*)sender {
	double seek_sec = (sender.doubleValue / sender.maxValue) * m_duration_sec;
	CMTime seek_time = CMTimeMakeWithSeconds(seek_sec, NSEC_PER_SEC);
	[m_player seekToTime:seek_time toleranceBefore:kCMTimeZero toleranceAfter:kCMTimeZero];
}

- (void)onSliderVideoPressed:(NSStyledSlider*)sender {
	m_is_drag = true;
	// save current rate and pause player
	m_rate_before_drag = m_player.rate;
	[m_player pause];
}

- (void)onSliderVideoReleased:(NSStyledSlider*)sender {
	m_is_drag = false;
	// restore previous player rate
	[m_player setRate:m_rate_before_drag];
}

- (void)onSliderVolumeChanged:(NSStyledSlider*)sender {
	[self setVolume:(sender.doubleValue / sender.maxValue)];
}

- (void)onPlayerTimeChanged:(CMTime)time {
	// show video view if it's hidden
	if (m_is_video) {
		[m_video_view setHidden:NO];
	}
	double time_sec = CMTimeGetSeconds(time);
	// update slider position unless video slider is being dragged
	if (!m_is_drag) {
		double time_value = (time_sec / m_duration_sec) * m_footer->m_slider_video.maxValue;
		[m_footer->m_slider_video setDoubleValue:time_value];
	}
	// update time label
	[m_footer->m_time_label setTime:time_sec];
}

// the main function for general observer handles
- (void)observeValueForKeyPath:(NSString*)key_path ofObject:(id)object change:(NSDictionary<NSString*, id>*)change context:(void*)context {
	if ([key_path isEqualToString:@"rate"]) {
		// player rate changed
		// show video view if it's hidden
		if (m_is_video) {
			[m_video_view setHidden:NO];
		}
		float rate = [change[NSKeyValueChangeNewKey] floatValue];
		[m_footer updatePlayPauseButton:rate];
	} else if ([key_path isEqualToString:@"volume"]) {
		// player volume changed
		double volume = [change[NSKeyValueChangeNewKey] floatValue] * m_footer->m_slider_volume.maxValue;
		[m_footer->m_slider_volume setDoubleValue:volume];
		[m_footer updateVolumeButton:volume];
	} else if ([key_path isEqualToString:@"status"]) {
		// player item status changed
		AVPlayerItemStatus media_status = (AVPlayerItemStatus)[change[NSKeyValueChangeNewKey] integerValue];
		if (media_status == AVPlayerItemStatusUnknown) {
			// what for parsing to end
			return;
		}
		// unregister current observer
		[object removeObserver:self forKeyPath:@"status"];
		if (media_status == AVPlayerItemStatusFailed) {
			// log error and do nothing
			NSLog(@"Error: Media parsing failed");
			return;
		}
		// it case everything went good, save media duration
		AVPlayerItem* item = (AVPlayerItem*)object;
		CMTime duration = item.duration;
		m_duration_sec = CMTimeGetSeconds(duration);
		if (m_duration_sec == 0.0 || !std::isfinite(m_duration_sec)) {
			// NOTE: is it the real case? Handle it somehow ???
			NSLog(@"Error: Media duration was not determined successfully");
		}
		// save media type flag (video or audio)
		AVAsset* asset = item.asset;
		NSArray* video_tracks = [asset tracksWithMediaType:AVMediaTypeVideo];
		if (video_tracks.count > 0) {
			m_is_video = true;
		} else {
			m_is_video = false;
		}
	}
}

- (void)removeObservers {
	[m_player removeObserver:self forKeyPath:@"rate"];
	[m_player removeObserver:self forKeyPath:@"volume"];
	[m_player removeTimeObserver:m_time_observer_token];
	m_time_observer_token = nil;
}

- (BOOL)setMedia:(NSString*)media_path {
	NSURL* url = [NSURL fileURLWithPath:media_path];
	NSError* err;
	if (url && [url checkResourceIsReachableAndReturnError:&err]) {
		AVPlayerItem* player_item = [AVPlayerItem playerItemWithURL:url];
		// add media status observer
		[player_item addObserver:self forKeyPath:@"status" options:NSKeyValueObservingOptionNew context:nil];
		// set this item for the player
		[m_player replaceCurrentItemWithPlayerItem:player_item];
	} else {
		NSLog(@"Error: Could not open file: %@", media_path);
		return NO;
	}
	return YES;
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
	[m_video_view setHidden:YES];
	[m_player pause];
	[m_player seekToTime:kCMTimeZero toleranceBefore:kCMTimeZero toleranceAfter:kCMTimeZero];
}

- (void)dealloc {
#if !__has_feature(objc_arc)
	[super dealloc];
#endif
}
@end
