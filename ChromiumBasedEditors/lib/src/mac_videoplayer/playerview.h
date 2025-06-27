#ifndef PLAYER_VIEW_H_
#define PLAYER_VIEW_H_

#include <string>

#import <AVFoundation/AVFoundation.h>

#import "videoview.h"
#import "footerpanel.h"

/*
 * Main class that owns footer panel and video frame and
 * handles audio/video playback functionality.
 */
class CPlayerView {
public:
	CPlayerView(NSRect video_view_rect, NSRect footer_panel_rect, NSView* parent);
	~CPlayerView();
	// not copyable
	CPlayerView(const CPlayerView& other) = delete;
	CPlayerView& operator=(const CPlayerView& other) = delete;
	// not movable
	CPlayerView(CPlayerView&& other) = delete;
	CPlayerView& operator=(CPlayerView&& other) = delete;

public:
	// getters
	NSVideoView* VideoView();
	NSFooterPanel* Footer();

	// player commands
	void Play();
	void Pause();
	void TogglePause();
	void ChangeVolume(int new_value);
	void ToggleMute();
	bool SetMedia(const std::wstring& media_path);
	void Stop();

private:
	AVPlayer* m_player = nil;
	NSVideoView* m_video_view = nil;
	NSFooterPanel* m_footer = nil;
};

#endif	// PLAYER_VIEW_H_
