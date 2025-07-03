#include "playerview.h"

#import "NSString+StringUtils.h"

CPlayerView::CPlayerView(NSRect video_view_rect, NSRect footer_panel_rect, NSView* parent) {
	// create player
	m_player = [[AVPlayer alloc] init];
	// add video view
	m_video_view = [[NSVideoView alloc] initWithFrame:video_view_rect player:m_player superview:parent];
	// add footer panel
	m_footer = [[NSFooterPanel alloc] initWithFrame:footer_panel_rect superview:parent];
}

CPlayerView::~CPlayerView() {
	[m_player pause];
#if !__has_feature(objc_arc)
	[m_player release];
#endif
	m_player = nil;

	[m_video_view removeFromSuperview];
#if !__has_feature(objc_arc)
	[m_video_view release];
#endif
	m_video_view = nil;

	[m_footer removeFromSuperview];
#if !__has_feature(objc_arc)
	[m_footer release];
#endif
	m_footer = nil;
}

NSVideoView* CPlayerView::VideoView() {
	return m_video_view;
}

NSFooterPanel* CPlayerView::Footer() {
	return m_footer;
}

void CPlayerView::Play() {
	[m_player play];
}

void CPlayerView::Pause() {
	[m_player pause];
}

void CPlayerView::TogglePause() {
	if (m_player.rate > 0.0) {
		Pause();
	} else {
		Play();
	}
}

void CPlayerView::ChangeVolume(CGFloat new_value) {
	m_player.volume = new_value;
}

void CPlayerView::ToggleMute() {
	// TODO
}

bool CPlayerView::SetMedia(const std::wstring& media_path) {
	// TODO: stringWithWString causes memory leaks ???
	NSString* path = [NSString stringWithWString:media_path];
	NSURL* url = [NSURL fileURLWithPath:path];

	NSError* err;
	if (url && [url checkResourceIsReachableAndReturnError:&err]) {
		AVPlayerItem* player_item = [AVPlayerItem playerItemWithURL:url];
		[m_player replaceCurrentItemWithPlayerItem:player_item];
	} else {
		NSLog(@"Error: could not open file: %@", path);
		return false;
	}
	return true;
}

void CPlayerView::Stop() {
	// TODO
}
