#ifndef MAC_CEF_VIEW_MEDIA_H
#define MAC_CEF_VIEW_MEDIA_H

#import "mac_cefview.h"

#include "../src/mac_videoplayer/playerview.h"

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
	CPlayerView* m_player_view = nullptr;
};

#endif	// MAC_CEF_VIEW_MEDIA_H
