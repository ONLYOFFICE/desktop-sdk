#import "../include/mac_cefviewmedia.h"

#import "../../../../core/DesktopEditor/common/Mac/NSString+StringUtils.h"

CCefViewMedia::CCefViewMedia(NSView* pView) : CCefViewWrapper(pView)
{
	m_pPlayer = nil;
	m_pMediaView = nil;
}

CCefViewMedia::~CCefViewMedia()
{
}

void CCefViewMedia::OnMediaStart(NSEditorApi::CAscExternalMedia* data)
{
	NSURL* url = [NSURL fileURLWithPath:[NSString stringWithWString:data->get_Url()]];

	NSError* err;
	if (url && [url checkResourceIsReachableAndReturnError:&err])
	{
		m_pPlayer = [AVPlayer playerWithURL:url];
		AVPlayerLayer* layer = [AVPlayerLayer playerLayerWithPlayer:m_pPlayer];
		[layer setBackgroundColor:CGColorCreateGenericRGB(0, 0, 0, 1.0)];

		m_pMediaView = [[NSView alloc] initWithFrame:NSMakeRect(data->get_BoundsX(), cef_height - data->get_BoundsH() - data->get_BoundsY(), data->get_BoundsW(), data->get_BoundsH())];
		[m_pMediaView setLayer:layer];

		[m_pParent addSubview:m_pMediaView positioned:NSWindowAbove relativeTo:nil];

		[m_pPlayer play];
	}
}

void CCefViewMedia::OnMediaEnd()
{
	if (m_pPlayer)
	{
		[m_pPlayer pause];
		m_pPlayer = nil;
	}

	if (m_pMediaView)
	{
		[m_pMediaView removeFromSuperview];
		m_pMediaView = nil;
	}
}
