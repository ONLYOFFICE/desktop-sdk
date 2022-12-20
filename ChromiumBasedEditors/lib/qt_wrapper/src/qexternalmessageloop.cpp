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

#include "./../include/qexternalmessageloop.h"

QExternalMessageLoop::QExternalMessageLoop(CAscApplicationManager* manager)
{
	m_manager = manager;
	m_timer.setSingleShot(true);
	QObject::connect(&m_timer, SIGNAL(timeout()), this, SLOT(slot_onTimeout()), Qt::QueuedConnection);
	QObject::connect(this, SIGNAL(onExecuteCommand(void*)), this, SLOT(slot_onExecuteCommand(void*)), Qt::QueuedConnection);
}
QExternalMessageLoop::~QExternalMessageLoop()
{
	m_timer.stop();
}

void QExternalMessageLoop::Run()
{
	QApplication::instance()->exec();
}
void QExternalMessageLoop::Execute(void* message)
{
	emit onExecuteCommand(message);
}
void QExternalMessageLoop::SetTimer(long delay)
{
	m_timer.start((int)delay);
}
void QExternalMessageLoop::KillTimer()
{
	m_timer.stop();
}
void QExternalMessageLoop::Exit()
{
	QApplication::instance()->quit();
}

void QExternalMessageLoop::slot_onExecuteCommand(void* message)
{
	m_manager->ExternalMessageLoop_OnExecute(message);
}
void QExternalMessageLoop::slot_onTimeout()
{
	m_manager->ExternalMessageLoop_OnTimeout();
}
