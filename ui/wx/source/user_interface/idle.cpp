/*******************************************************************************
FILE : idle.c

LAST MODIFIED : 21 March 2005

DESCRIPTION :
This provides code for handling idle events.
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
 * The Original Code is cmgui.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2005
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
#include "api/cmiss_idle.h"
#include "idle.h"
#include "user_interface/event_dispatcher.h"

Idle_package_id create_idle_package(
	struct Event_dispatcher *event_dispatcher)
/*******************************************************************************
LAST MOFIFIED : 21 March 2005

DESCRIPTION :
This creates an Idle_package, given an event_dispatcher.
==============================================================================*/
{
	return (Idle_package_id)(event_dispatcher);
}

Idle_callback_id Idle_package_add_callback(Idle_package_id pkg,
	Idle_callback_function *callback,
	void *user_data)
/*******************************************************************************
LAST MODIFIED : 21 March 2005

DESCRIPTION :
Sets an idle callback.
==============================================================================*/
{
	struct Event_dispatcher_idle_callback* ret;

	ret = Event_dispatcher_add_idle_callback((struct Event_dispatcher*)pkg,
		callback, user_data, EVENT_DISPATCHER_X_PRIORITY);

	return (Idle_callback_id)(ret);
}

int Cmiss_idle_callback_destroy(Cmiss_idle_package_id pkg,
	Cmiss_idle_callback_id *callback)
{
	int ret;

	ret = Event_dispatcher_remove_idle_callback((struct Event_dispatcher*)pkg,
		(struct Event_dispatcher_idle_callback*)*callback);
	*callback = 0;

	return ret;
}

int Cmiss_idle_package_destroy(Cmiss_idle_package_id *pkg)
{
	*pkg = 0;

	return 1;
}
