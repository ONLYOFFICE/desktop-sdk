#import "../include/mac_cefviewmedia.h"

CCefViewMedia::CCefViewMedia(NSView* view) : CCefViewWrapper(view) {}

CCefViewMedia::~CCefViewMedia() {
	hideMediaControl();
}

void CCefViewMedia::OnMediaPlayerCommand(NSEditorApi::CAscExternalMediaPlayerCommand* data) {
	std::string cmd = data->get_Cmd();
	if (cmd == "showMediaControl") {
		showMediaControl(data);
	} else if (cmd == "hideMediaControl") {
		hideMediaControl();
	} else if (cmd == "update") {
		updateGeometry(data);
	} else {
		// handle the possible case when player is not yet initialized
		if (m_player_view == nullptr) {
			return;
		}
		// handle player commands
		if (cmd == "play" || cmd == "resume") {
			m_player_view->Play();
		} else if (cmd == "pause") {
			m_player_view->Pause();
		} else if (cmd == "stop") {
			m_player_view->Stop();
		} else if (cmd == "togglePause") {
			m_player_view->TogglePause();
		} else {
			NSLog(@"Error: Media player command %s can not be handled.", cmd.c_str());
		}
	}
}

void CCefViewMedia::showMediaControl(NSEditorApi::CAscExternalMediaPlayerCommand* data) {
	if (m_player_view)
		return;

	// if media is not selected, don't do anything
	if (!data->get_IsSelected())
		return;

	// calculate rects
	NSRect video_view_rect = NSMakeRect(data->get_FrameRectX(), cef_height - data->get_FrameRectH() - data->get_FrameRectY(), data->get_FrameRectW(), data->get_FrameRectH());
	NSRect footer_panel_rect = NSMakeRect(data->get_ControlRectX(), cef_height - data->get_ControlRectH() - data->get_ControlRectY(), data->get_ControlRectW(), data->get_ControlRectH());
	// create and initialize player view
	m_player_view = new CPlayerView(video_view_rect, footer_panel_rect, m_pParent);

	// set footer skin
	NSFooterPanel* footer = m_player_view->Footer();
	std::string theme = data->get_Theme();
	if (theme == "dark") {
		[footer applySkin:CFooterSkin::Type::kDark];
	} else {
		[footer applySkin:CFooterSkin::Type::kLight];
	}

	// set media
	m_player_view->SetMedia(data->get_Url());
	// set volume
	int volume = data->get_Volume();
	if (data->get_Mute() || m_pCefView->IsPresentationReporter()) {
		m_player_view->ToggleMute();
	} else if (volume != -1) {
		m_player_view->ChangeVolume(volume);
	}
}

void CCefViewMedia::hideMediaControl() {
	if (!m_player_view)
		return;

	delete m_player_view;
	m_player_view = nullptr;
}

void CCefViewMedia::updateGeometry(NSEditorApi::CAscExternalMediaPlayerCommand* data) {
	if (!m_player_view)
		return;

	// update frame of the video view
	NSRect video_view_rect = NSMakeRect(data->get_FrameRectX(), cef_height - data->get_FrameRectH() - data->get_FrameRectY(), data->get_FrameRectW(), data->get_FrameRectH());
	[m_player_view->VideoView() setFrame:video_view_rect];
	// update frame of the footer panel
	NSRect footer_panel_rect = NSMakeRect(data->get_ControlRectX(), cef_height - data->get_ControlRectH() - data->get_ControlRectY(), data->get_ControlRectW(), data->get_ControlRectH());
	[m_player_view->Footer() setFrame:footer_panel_rect];
}
