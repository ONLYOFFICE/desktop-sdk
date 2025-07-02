#import "../include/mac_cefviewmedia.h"

#import "../../../../core/DesktopEditor/common/Mac/NSString+StringUtils.h"

CCefViewMedia::CCefViewMedia(NSView* view) : CCefViewWrapper(view) {}

CCefViewMedia::~CCefViewMedia() {
	hideMediaControl();
}

void CCefViewMedia::OnMediaPlayerCommand(NSEditorApi::CAscExternalMediaPlayerCommand* data) {
	std::string cmd = data->get_Cmd();
	// TODO: remove debug logs
	if (cmd == "showMediaControl") {
		NSLog(@"debug: show media control");
		showMediaControl(data);
	} else if (cmd == "hideMediaControl") {
		NSLog(@"debug: hide media control");
		hideMediaControl();
	} else if (cmd == "update") {
		NSLog(@"debug: update");
		updateGeometry(data);
	} else {
		NSLog(@"debug: unsupported player command");
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
	std::string sTheme = data->get_Theme();
	if (sTheme == "dark") {
		[footer applySkin:CFooterSkin::Type::kDark];
	} else {
		[footer applySkin:CFooterSkin::Type::kLight];
	}

	m_player_view->SetMedia(data->get_Url());
	// TODO: remove play and mute
	m_player_view->Play();
	m_player_view->ChangeVolume(0);

	int nVolume = data->get_Volume();
	if (data->get_Mute() || m_pCefView->IsPresentationReporter()) {
		m_player_view->ToggleMute();
	} else if (nVolume != -1) {
		m_player_view->ChangeVolume(nVolume);
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
