#ifndef MAC_CEF_VIEW_MEDIA_H
#define MAC_CEF_VIEW_MEDIA_H

#import "mac_cefview.h"

#import <AVFoundation/AVFoundation.h>

class DESKTOP_DECL CCefViewMedia : public CCefViewWrapper
{
public:
	CCefViewMedia(NSView* view);
	virtual ~CCefViewMedia();

public:
	virtual void OnMediaPlayerCommand(NSEditorApi::CAscExternalMediaPlayerCommand* data) override;

	void showMediaControl(NSEditorApi::CAscExternalMediaPlayerCommand* data);
	void hideMediaControl();
	void updateGeometry(NSEditorApi::CAscExternalMediaPlayerCommand* data);

private:
	AVPlayer* m_player = nil;
	NSView* m_media_view = nil;
	NSView* m_control_view = nil;
};

#endif	// MAC_CEF_VIEW_MEDIA_H
