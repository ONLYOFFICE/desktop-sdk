/*
 * (c) Copyright Ascensio System SIA 2010-2019
 *
 * This program is a free software product. You can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License (AGPL)
 * version 3 as published by the Free Software Foundation. In accordance with
 * Section 7(a) of the GNU AGPL its Section 15 shall be amended to the effect
 * that Ascensio System SIA expressly excludes the warranty of non-infringement
 * of any third-party rights.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR  PURPOSE. For
 * details, see the GNU AGPL at: http://www.gnu.org/licenses/agpl-3.0.html
 *
 * You can contact Ascensio System SIA at 20A-12 Ernesta Birznieka-Upisha
 * street, Riga, Latvia, EU, LV-1050.
 *
 * The  interactive user interfaces in modified source and object code versions
 * of the Program must display Appropriate Legal Notices, as required under
 * Section 5 of the GNU AGPL version 3.
 *
 * Pursuant to Section 7(b) of the License you must retain the original Product
 * logo when distributing the program. Pursuant to Section 7(e) we decline to
 * grant you any rights under trademark law for use of our trademarks.
 *
 * All the Product's GUI elements, including illustrations and icon sets, as
 * well as technical writing content are licensed under the terms of the
 * Creative Commons Attribution-ShareAlike 4.0 International. See the License
 * terms at http://creativecommons.org/licenses/by-sa/4.0/legalcode
 *
 */
#ifndef MAC_CEF_VIEW_H
#define MAC_CEF_VIEW_H

#include "./cefview.h"
#import <Cocoa/Cocoa.h>

class CCefViewWrapper : public CCefViewWidgetImpl
{
public:
    CCefView* m_pCefView;
    NSView* m_pParent;

public:
    CCefViewWrapper(NSView* pView) : CCefViewWidgetImpl()
    {
        m_pParent = pView;
        m_pCefView = NULL;

        cef_handle = (__bridge WindowHandleId)m_pParent;
    }
    virtual ~CCefViewWrapper()
    {
    }

    void SetBackgroundCefColor(unsigned char r, unsigned char g, unsigned char b)
    {
    }

    CCefView* GetCefView()
    {
        return m_pCefView;
    }

    void focusInEvent()
    {
        if (NULL != m_pCefView)
            m_pCefView->focus();
    }

    void resizeEvent()
    {
        UpdateGeometry();
        if (NULL != m_pCefView)
            m_pCefView->resizeEvent();
    }

    void moveEvent()
    {
        UpdateGeometry();
        if (NULL != m_pCefView)
            m_pCefView->moveEvent();
    }

public:
    virtual void UpdateSize()
    {
        NSView* child = nil;

        if ([[m_pParent subviews] count] > 0)
            child = [m_pParent subviews][0];

        NSRect childRect = m_pParent.frame;
        childRect.origin.x = 0;
        childRect.origin.y = 0;
        child.frame = childRect;
    }

protected:
    void UpdateGeometry()
    {
        //CGFloat koef = [[NSScreen mainScreen] backingScaleFactor];
        CGFloat koef = 1;
        cef_x = 0;
        cef_y = 0;
        cef_width = (int)(m_pParent.frame.size.width * koef);
        cef_height = (int)(m_pParent.frame.size.height * koef);
    }
};

#endif
