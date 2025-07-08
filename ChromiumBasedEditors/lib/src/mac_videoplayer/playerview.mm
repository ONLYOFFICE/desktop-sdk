#include "playerview.h"

#import "NSString+StringUtils.h"

CPlayerView::CPlayerView(NSRect video_view_rect, NSRect footer_panel_rect, NSView* parent) {
	// create player
	m_player = [[AVPlayer alloc] init];
	// add video view
	m_video_view = [[NSVideoView alloc] initWithFrame:video_view_rect player:m_player superview:parent];
	[m_video_view setHidden:YES];
	// add footer panel
	m_footer = [[NSFooterPanel alloc] initWithFrame:footer_panel_rect superview:parent];
	// initialize player controller (it should be initialized AFTER the player and footer panel)
	m_controller = [[NSPlayerControllerBridge alloc] initWithPlayer:m_player videoView:m_video_view footer:m_footer];
}

CPlayerView::~CPlayerView() {
	// we MUST remove any existing observers first
	[m_controller removeObservers];

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

#if !__has_feature(objc_arc)
	[m_controller release];
#endif
	m_controller = nil;
}

NSVideoView* CPlayerView::VideoView() {
	return m_video_view;
}

NSFooterPanel* CPlayerView::Footer() {
	return m_footer;
}

void CPlayerView::Play() {
	[m_controller play];
}

void CPlayerView::Pause() {
	[m_controller pause];
}

void CPlayerView::TogglePause() {
	[m_controller togglePause];
}

void CPlayerView::ChangeVolume(double new_value) {
	[m_controller setVolume:new_value];
}

void CPlayerView::ToggleMute() {
	[m_controller toggleMute];
}

bool CPlayerView::SetMedia(const std::wstring& media_path) {
	// TODO: stringWithWString causes memory leaks ???
	NSString* path = [NSString stringWithWString:media_path];
	return [m_controller setMedia:path];
}

void CPlayerView::Stop() {
	[m_controller stop];
}
