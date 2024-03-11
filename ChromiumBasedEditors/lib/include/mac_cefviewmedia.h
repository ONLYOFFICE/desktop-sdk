#ifndef MAC_CEF_VIEW_MEDIA_H
#define MAC_CEF_VIEW_MEDIA_H

#import "mac_cefview.h"

#import <AVFoundation/AVFoundation.h>

@interface NSMediaView : NSView
{
	NSView* m_pPlayerView;
	NSRect m_oFrame;
}
- (void) setPlayerLayer:(AVPlayerLayer*)layer;
@end

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
	NSMediaView* m_pMediaView;
};

#endif	// MAC_CEF_VIEW_MEDIA_H
