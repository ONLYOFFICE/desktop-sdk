#ifndef MAC_CEF_VIEW_MEDIA_H
#define MAC_CEF_VIEW_MEDIA_H

#import "mac_cefview.h"

#import <AVFoundation/AVFoundation.h>

class DESKTOP_DECL CCefViewMedia : public CCefViewWrapper
{
public:
	CCefViewMedia(NSView* pView);
	virtual ~CCefViewMedia();

public:
	virtual void OnMediaStart(NSEditorApi::CAscExternalMedia* data);
	virtual void OnMediaEnd();

private:
	AVPlayer* m_pPlayer;
	NSView* m_pMediaView;
};

#endif	// MAC_CEF_VIEW_MEDIA_H
