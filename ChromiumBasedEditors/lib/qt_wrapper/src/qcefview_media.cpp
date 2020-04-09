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

#include "./../include/qcefview_media.h"

void QCefView_SetProperty(QObject* w, const QVariant& p)
{
    w->setProperty("native_dpi", p);
    QObjectList childs = w->children();
    for (int i = childs.count() - 1; i >= 0; --i)
    {
        QCefView_SetProperty(childs.at(i), p);
    }
}

void QCefView_SetDPI(QWidget* w, const double& v)
{
    QVariant p(v);
    QCefView_SetProperty(w, p);
}

QCefView_Media::QCefView_Media(QWidget* parent) : QCefView(parent)
{
    m_pMediaView = NULL;
}

QCefView_Media::~QCefView_Media()
{
    OnMediaEnd();
}

void QCefView_Media::OnMediaStart(NSEditorApi::CAscExternalMedia* data)
{
    if (m_pMediaView)
        return;

    m_pMediaView = new QAscVideoView(this, 49, 52, 55);
    m_pMediaView->setPlayListUsed(false);
    m_pMediaView->setFullScreenUsed(false);
    m_pMediaView->setPresentationMode(true);
    QCefView_SetDPI(this, m_pCefView->GetDeviceScale());
    m_pMediaView->setGeometry(data->get_BoundsX(), data->get_BoundsY(), data->get_BoundsW(), data->get_BoundsH());
    m_pMediaView->setMedia(QString::fromStdWString(data->get_Url()));
    //m_pMediaView->show();
}
void QCefView_Media::OnMediaEnd()
{
    if (!m_pMediaView)
        return;

    m_pMediaView->Stop();
    m_pMediaView->hide();
    m_pMediaView->deleteLater();
    m_pMediaView = NULL;
}
