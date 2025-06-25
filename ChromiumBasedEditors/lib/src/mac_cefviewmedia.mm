#import "../include/mac_cefviewmedia.h"

#import "../../../../core/DesktopEditor/common/Mac/NSString+StringUtils.h"

CCefViewMedia::CCefViewMedia(NSView* view) : CCefViewWrapper(view)
{
}

CCefViewMedia::~CCefViewMedia()
{
	hideMediaControl();
}

void CCefViewMedia::OnMediaPlayerCommand(NSEditorApi::CAscExternalMediaPlayerCommand* data)
{
	std::string cmd = data->get_Cmd();
	if (cmd == "showMediaControl")
	{
		NSLog(@"debug: show media control");
		showMediaControl(data);
	}
	else if (cmd == "hideMediaControl")
	{
		NSLog(@"debug: hide media control");
		hideMediaControl();
	}
	else if (cmd == "update")
	{
		NSLog(@"debug: update");
		updateGeometry(data);
	}
	else
	{
		NSLog(@"debug: unsupported player command");
	}
}

void CCefViewMedia::showMediaControl(NSEditorApi::CAscExternalMediaPlayerCommand* data)
{
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

	m_player_view->SetMedia(data->get_Url());
	m_player_view->Play();
}

void CCefViewMedia::hideMediaControl()
{
	if (!m_player_view)
		return;

	delete m_player_view;
	m_player_view = nullptr;
}

void CCefViewMedia::updateGeometry(NSEditorApi::CAscExternalMediaPlayerCommand* data)
{
	// TODO
}
