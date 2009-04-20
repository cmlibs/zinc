/*******************************************************************************
FILE : time.h

LAST MODIFIED : 28 December 1999

DESCRIPTION :
This provides an object which supplies a concept of time to Cmgui
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
#if !defined (TIME_TIME_H) /* Distinguish general/time.h and time/time.h */
#define TIME_TIME_H

#include "api/cmiss_time.h"
#include "general/object.h"
#include "time/time_keeper.h"

#define Time_object Cmiss_time_notifier
#define Time_object_create_regular Cmiss_time_notifier_create_regular
#define Time_object_add_callback Cmiss_time_notifier_add_callback
#define Time_object_remove_callback Cmiss_time_notifier_remove_callback
#define Time_object_set_update_frequency Cmiss_time_notifier_set_frequency
#define Time_object_set_offset Cmiss_time_notifier_set_offset

struct Time_object;

typedef int (*Time_object_callback)(struct Time_object *time,
	double current_time, void *user_data);
typedef double (*Time_object_next_time_function)(double time_after,
	enum Time_keeper_play_direction play_direction, void *user_data);

PROTOTYPE_OBJECT_FUNCTIONS(Time_object);
PROTOTYPE_GET_OBJECT_NAME_FUNCTION(Time_object);

/***************************************************************************//**
 * Create a time object with regular update time.
 *
 * @param update_frequency  The number of times which time notifier will receive
 *    callback per unit of time in the time keeper.
 * @param time_offset  the offset value for the time object to receive time 
 *    callback.
 * @return  The time object if successfully create a time object otherwise 
 *    NULL.
 */
struct Time_object *Time_object_create_regular(double update_frequency, 
	double time_offset);

/***************************************************************************//**
 * Set the name of time object.
 *
 * @param time  The time object to set the name for.
 * @param name  The name to be set for time object.
 * @return  1 if successfully set name for time object, otherwise 0.
 */
int Time_object_set_name(struct Time_object *time, const char *name);

double Time_object_get_current_time(struct Time_object *time);
/*******************************************************************************
LAST MODIFIED : 29 September 1998

DESCRIPTION :
==============================================================================*/

/***************************************************************************//**
 * Check either the time specified is a valid callback time or not.
 *
 * @return  1 if the specified time is a valid callback time, otherwise 0.
 */
int Time_object_check_valid_callback_time(struct Time_object *time_object,
	double time,enum Time_keeper_play_direction play_direction);

double Time_object_get_next_callback_time(struct Time_object *time,
	double time_after,enum Time_keeper_play_direction play_direction);
/*******************************************************************************
LAST MODIFIED : 9 December 1998

DESCRIPTION :
==============================================================================*/

int Time_object_set_current_time_privileged(struct Time_object *time,
	double new_time);
/*******************************************************************************
LAST MODIFIED : 17 January 2002

DESCRIPTION :
This routine allows the timekeeper to explicitly set the time.
Separated Time_object_notify_clients_privileged so that all the clients
can be updated with the new time before any of them call back to their clients.
Users of a time object that is controlled by a timekeeper should set the time 
through the timekeeper.
==============================================================================*/

int Time_object_notify_clients_privileged(struct Time_object *time);
/*******************************************************************************
LAST MODIFIED : 17 January 2002

DESCRIPTION :
This routine allows the timekeeper to tell the time_object to notify its clients.
Separated off from Time_object_set_current_time_privileged so that all the clients
can be updated with the new time before any of them call back to their clients.
Users of a time object that is controlled by a timekeeper should set the time 
through the timekeeper.
==============================================================================*/

int Time_object_set_update_frequency(struct Time_object *time,double frequency);
/*******************************************************************************
LAST MODIFIED : 6 October 1998

DESCRIPTION :
This controls the rate per second which the time depedent object is called back
when in play mode.
==============================================================================*/

int Time_object_set_offset(struct Time_object *time,double time_offset);

int Time_object_set_next_time_function(struct Time_object *time,
	Time_object_next_time_function next_time_function,void *user_data);
/*******************************************************************************
LAST MODIFIED : 18 December 1998

DESCRIPTION :
By setting this function when the time_keeper requests the update_frequency is
not used.  Instead the next_time_function is called to evaluate the next valid
time.
==============================================================================*/

struct Time_keeper *Time_object_get_time_keeper(struct Time_object *time);
/*******************************************************************************
LAST MODIFIED : 29 September 1998

DESCRIPTION :
==============================================================================*/

int Time_object_add_callback(struct Time_object *time,
	Time_object_callback callback,void *user_data);
/*******************************************************************************
LAST MODIFIED : 29 September 1998

DESCRIPTION :
Adds a callback routine which is called whenever the current time is changed.
==============================================================================*/

int Time_object_remove_callback(struct Time_object *time,
	Time_object_callback callback,void *user_data);
/*******************************************************************************
LAST MODIFIED : 29 September 1998

DESCRIPTION :
Removes a callback which was added previously
==============================================================================*/

#endif /* !defined (TIME_TIME_H) */
