/*******************************************************************************
FILE : timer.c

LAST MODIFIED : 11 April 2005

DESCRIPTION :
This provides code for handling timer events.
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
#include "api/cmiss_timer.h"
#include "timer.h"
#include "general/debug.h"
#include "event_dispatcher.h"

Timer_package_id CREATE(Timer_package)(
	struct Event_dispatcher *event_dispatcher)
/*******************************************************************************
LAST MOFIFIED : 11 April 2005

DESCRIPTION :
This creates an Timer_package, given an event_dispatcher.
==============================================================================*/
{
	ENTER(CREATE(Timer_package));
	LEAVE;

	return (Timer_package_id)(event_dispatcher);
}

int DESTROY(Timer_package)(Timer_package_id *pkg)
/*******************************************************************************
LAST MOFIFIED : 11 April 2005

DESCRIPTION :
This destroys an Timer_package.
==============================================================================*/
{
	ENTER(DESTROY(Timer_package));
	*pkg = NULL;
	LEAVE;

	return (1);
}

Timer_callback_id Timer_package_add_callback(Timer_package_id pkg,
	unsigned long secs, unsigned long nsecs,
	Timer_callback_function *callback,
	void *user_data)
/*******************************************************************************
LAST MODIFIED : 11 April 2005

DESCRIPTION :
Sets a timer callback.
==============================================================================*/
{
	struct Event_dispatcher_timeout_callback* ret;

	ENTER(Timer_package_add_callback);
	ret = Event_dispatcher_add_timeout_callback
	      (
	       (struct Event_dispatcher*)pkg,
	       secs, nsecs,
	       callback, user_data
	      );
	LEAVE;

	return (Timer_callback_id)(ret);
}

int DESTROY(Timer_callback)(Timer_package_id pkg, Timer_callback_id *callback)
/*******************************************************************************
LAST MODIFIED : 11 April 2005

DESCRIPTION :
Destroys a timer callback, causing it to not be called any longer.
==============================================================================*/
{
	int ret;

	ENTER(DESTROY(Timer_callback));
	ret = Event_dispatcher_remove_timeout_callback
		(
			(struct Event_dispatcher*)pkg,
			(struct Event_dispatcher_timeout_callback*)*callback
		);
	*callback = NULL;
	LEAVE;

	return (ret);
}
