#include "playerview.h"

#include <iostream>

#import "NSString+StringUtils.h"

CPlayerView::CPlayerView(NSRect video_view_rect, NSRect footer_panel_rect, NSView* parent) {
	// create player
	m_player = [[AVPlayer alloc] init];

	// add video view
	m_video_view = [[NSVideoView alloc] initWithFrame:video_view_rect player:m_player];
	[parent addSubview:m_video_view positioned:NSWindowAbove relativeTo:nil];

	// add footer panel
	m_footer = [[NSFooterPanel alloc] initWithFrame:footer_panel_rect];
	[parent addSubview:m_footer positioned:NSWindowAbove relativeTo:nil];
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

void CPlayerView::ChangeVolume(int new_value) {
	// TODO
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
		std::wcerr << L"Error: could not open file: " << media_path << std::endl;
		return false;
	}
	return true;
}

void CPlayerView::Stop() {
	// TODO
}
