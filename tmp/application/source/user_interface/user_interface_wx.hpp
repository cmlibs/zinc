/*******************************************************************************
FILE : user_interface.hpp

DESCRIPTION :
wxWidgets c++ definitions for the user interface.
==============================================================================*/
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Cmgui.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2011
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
#if !defined (USER_INTERFACE_WX_H)
#define USER_INTERFACE_WX_H

#if 1
#include "configure/cmgui_configure.h"
#endif /* defined (1) */

#if defined (WX_USER_INTERFACE)
# include <wx/wx.h>

struct Event_dispatcher;

class wxCmguiApp : public wxApp
{
	Event_dispatcher *event_dispatcher;

public:
	wxCmguiApp() 
		: wxApp()
		, event_dispatcher(0)
	{
	}

	virtual ~wxCmguiApp()
	{
	}

	virtual bool OnInit();

	virtual wxAppTraits * CreateTraits();

	void OnIdle(wxIdleEvent& event);

	void SetEventDispatcher(Event_dispatcher *event_dispatcher_in);

#if defined (__WXDEBUG__)
	void OnAssertFailure(const wxChar *file, int line, const wxChar* func, const wxChar* cond, const wxChar *msg)
	{
		USE_PARAMETER(file);
		USE_PARAMETER(line);
		USE_PARAMETER(func);
		USE_PARAMETER(cond);
		USE_PARAMETER(msg);
	}
#endif /* defined (__WXDEBUG__) */

	DECLARE_EVENT_TABLE();
};	

#endif /*defined (WX_USER_INTERFACE)*/

#endif /* !defined (USER_INTERFACE_WX_H) */
