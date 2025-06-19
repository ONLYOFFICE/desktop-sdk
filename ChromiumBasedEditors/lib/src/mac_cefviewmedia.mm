#import "../include/mac_cefviewmedia.h"

#import "../../../../core/DesktopEditor/common/Mac/NSString+StringUtils.h"

CCefViewMedia::CCefViewMedia(NSView* view) : CCefViewWrapper(view)
{
}

CCefViewMedia::~CCefViewMedia()
{
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
	if (m_media_view)
		return;

	// if media is not selected, don't do anything
	if (!data->get_IsSelected())
		return;

	// TODO: main logic here
	NSURL* url = [NSURL fileURLWithPath:[NSString stringWithWString:data->get_Url()]];

	NSError* err;
	if (url && [url checkResourceIsReachableAndReturnError:&err])
	{
		m_player = [AVPlayer playerWithURL:url];
		NSRect media_view_rect = NSMakeRect(data->get_FrameRectX(), cef_height - data->get_FrameRectH() - data->get_FrameRectY(), data->get_FrameRectW(), data->get_FrameRectH());
		m_media_view = [[NSView alloc] initWithFrame:media_view_rect];
		// set black background (transparent by default)
		AVPlayerLayer* layer = [AVPlayerLayer playerLayerWithPlayer:m_player];
		CGColorRef bg_color = [[NSColor blackColor] CGColor];
		[layer setBackgroundColor:bg_color];
		[m_media_view setWantsLayer:YES];
		[m_media_view setLayer:layer];
		// attach as subview
		[m_pParent addSubview:m_media_view positioned:NSWindowAbove relativeTo:nil];
		// start playback
		[m_player play];

		// add control rect
		NSRect control_view_rect = NSMakeRect(data->get_ControlRectX(), cef_height - data->get_ControlRectH() - data->get_ControlRectY(), data->get_ControlRectW(), data->get_ControlRectH());
		m_control_view = [[NSView alloc] initWithFrame:control_view_rect];
		[m_control_view setWantsLayer:YES];
		m_control_view.layer.cornerRadius = 8.0;
		m_control_view.layer.masksToBounds = YES;
		[m_control_view.layer setBackgroundColor:bg_color];
		[m_pParent addSubview:m_control_view positioned:NSWindowAbove relativeTo:nil];
	}
}

void CCefViewMedia::hideMediaControl()
{
	if (!m_player || !m_media_view)
		return;

	// TODO: main logic here
	if (m_player)
	{
		[m_player pause];
		m_player = nil;
	}

	if (m_media_view)
	{
		[m_media_view removeFromSuperview];
		m_media_view = nil;
	}

	if (m_control_view)
	{
		[m_control_view removeFromSuperview];
		m_control_view = nil;
	}
}

void CCefViewMedia::updateGeometry(NSEditorApi::CAscExternalMediaPlayerCommand* data)
{
	// TODO
}
