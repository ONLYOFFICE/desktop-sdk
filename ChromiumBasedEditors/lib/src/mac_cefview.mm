#import "../include/mac_cefview.h"

CCefViewWrapper::CCefViewWrapper(NSView* pView) : CCefViewWidgetImpl()
{
	m_pParent = pView;
	m_pCefView = NULL;

	cef_handle = (__bridge WindowHandleId)m_pParent;
}

CCefViewWrapper::~CCefViewWrapper()
{
}

void CCefViewWrapper::SetBackgroundCefColor(unsigned char r, unsigned char g, unsigned char b)
{
}

CCefView* CCefViewWrapper::GetCefView()
{
	return m_pCefView;
}

void CCefViewWrapper::focusInEvent()
{
	if (NULL != m_pCefView)
		m_pCefView->focus();
}

void CCefViewWrapper::resizeEvent()
{
	UpdateGeometry();
	if (NULL != m_pCefView)
		m_pCefView->resizeEvent();
}

void CCefViewWrapper::moveEvent()
{
	UpdateGeometry();
	if (NULL != m_pCefView)
		m_pCefView->moveEvent();
}

void CCefViewWrapper::OnMediaPlayerCommand(NSEditorApi::CAscExternalMediaPlayerCommand* data)
{
}

void CCefViewWrapper::UpdateSize()
{
	NSView* child = nil;

	if ([[m_pParent subviews] count] > 0)
		child = [m_pParent subviews][0];

	NSRect childRect = m_pParent.frame;
	childRect.origin.x = 0;
	childRect.origin.y = 0;
	child.frame = childRect;
}

void CCefViewWrapper::UpdateGeometry()
{
	//CGFloat koef = [[NSScreen mainScreen] backingScaleFactor];
	CGFloat koef = 1;
	cef_x = 0;
	cef_y = 0;
	cef_width = (int)(m_pParent.frame.size.width * koef);
	cef_height = (int)(m_pParent.frame.size.height * koef);
}
