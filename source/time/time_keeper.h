/*******************************************************************************
FILE : time_keeper.h

LAST MODIFIED : 9 December 1998

DESCRIPTION :
This object defines a relationship between a bunch of time objects, keeps them
in sync and allows control such as play, rewind and fast forward.
This is intended to be multithreaded......
******************************************************************************/
#if !defined (TIME_KEEPER_H)
#define TIME_KEEPER_H

#include "api/cmiss_time_keeper.h"
#include "user_interface/user_interface.h"

#define Time_keeper Cmiss_time_keeper
#define Time_object Cmiss_time_object

#define Time_keeper_get_minimum Cmiss_time_keeper_get_minimum
#define Time_keeper_set_minimum Cmiss_time_keeper_set_minimum
#define Time_keeper_get_maximum Cmiss_time_keeper_get_maximum
#define Time_keeper_set_maximum Cmiss_time_keeper_set_maximum
#define Time_keeper_stop 	Cmiss_time_keeper_stop
#define Time_keeper_get_time Cmiss_time_keeper_get_time
#define Time_keeper_request_new_time Cmiss_time_keeper_set_time
#define Time_keeper_get_speed Cmiss_time_keeper_get_speed
#define Time_keeper_set_speed Cmiss_time_keeper_set_speed
#define Time_keeper_is_playing Cmiss_time_keeper_is_playing
#define Time_keeper_add_time_object Cmiss_time_keeper_add_time_object
#define Time_keeper_remove_time_object Cmiss_time_keeper_remove_time_object

struct Time_object; /* Either this or #include "time/time.h" */
struct Time_keeper;

enum Time_keeper_play_direction
{
	TIME_KEEPER_PLAY_FORWARD,
	TIME_KEEPER_PLAY_BACKWARD
};

enum Time_keeper_event
{
	/* These constants are bit masked and so should be powers of two */
	TIME_KEEPER_NEW_TIME = 1,
	TIME_KEEPER_STARTED = 2,
	TIME_KEEPER_STOPPED = 4,
	TIME_KEEPER_CHANGED_DIRECTION = 8,
	TIME_KEEPER_NEW_MINIMUM = 16,
	TIME_KEEPER_NEW_MAXIMUM = 32
};

enum Time_keeper_play_mode
{
	TIME_KEEPER_PLAY_ONCE,
	TIME_KEEPER_PLAY_LOOP,
	TIME_KEEPER_PLAY_SWING
};


typedef int (*Time_keeper_callback)(struct Time_keeper *time_keeper,
	enum Time_keeper_event event, void *user_data);

PROTOTYPE_OBJECT_FUNCTIONS(Time_keeper);

struct Time_keeper *CREATE(Time_keeper)(char *name,
	struct Event_dispatcher *event_dispatcher,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 15 March 2002

DESCRIPTION :
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

int Time_keeper_add_time_object(struct Time_keeper *time_keeper,
	struct Time_object *time_object);
/*******************************************************************************
LAST MODIFIED : 29 September 1998

DESCRIPTION :
==============================================================================*/

int Time_keeper_remove_time_object(struct Time_keeper *time_keeper,
	struct Time_object *time_object);
/*******************************************************************************
LAST MODIFIED : 29 September 1998

DESCRIPTION :
==============================================================================*/

int Time_keeper_add_callback(struct Time_keeper *time,
	Time_keeper_callback callback, void *user_data,
	enum Time_keeper_event event_mask);
/*******************************************************************************
LAST MODIFIED : 9 December 1998

DESCRIPTION :
Adds a callback routine which is called when the state of the time_keeper changes.
i.e. play, stop, direction_change.
The event mask allows each client to request only relevant events.
==============================================================================*/

int Time_keeper_remove_callback(struct Time_keeper *time,
	Time_keeper_callback callback, void *user_data);
/*******************************************************************************
LAST MODIFIED : 15 October 1998

DESCRIPTION :
Removes a callback which was added previously
==============================================================================*/

int Time_keeper_request_new_time(struct Time_keeper *time_keeper, double new_time);
/*******************************************************************************
LAST MODIFIED : 29 September 1998

DESCRIPTION :
==============================================================================*/

double Time_keeper_get_time(struct Time_keeper *time_keeper);
/*******************************************************************************
LAST MODIFIED : 14 October 1998
DESCRIPTION :
==============================================================================*/

int Time_keeper_play(struct Time_keeper *time_keeper,
	enum Time_keeper_play_direction play_direction);
/*******************************************************************************
LAST MODIFIED : 9 December 1998

DESCRIPTION :
==============================================================================*/

int Time_keeper_is_playing(struct Time_keeper *time_keeper);
/*******************************************************************************
LAST MODIFIED : 1 October 1998

DESCRIPTION :
==============================================================================*/

int Time_keeper_stop(struct Time_keeper *time_keeper);
/*******************************************************************************
LAST MODIFIED : 1 October 1998

DESCRIPTION :
==============================================================================*/

double Time_keeper_get_speed(struct Time_keeper *time_keeper);
/*******************************************************************************
LAST MODIFIED : 6 October 1998
DESCRIPTION :
==============================================================================*/

int Time_keeper_set_speed(struct Time_keeper *time_keeper, double speed);
/*******************************************************************************
LAST MODIFIED : 6 October 1998
DESCRIPTION :
==============================================================================*/

double Time_keeper_get_maximum(struct Time_keeper *time_keeper);
/*******************************************************************************
LAST MODIFIED : 4 February 1999
DESCRIPTION :
==============================================================================*/

int Time_keeper_set_maximum(struct Time_keeper *time_keeper, double maximum);
/*******************************************************************************
LAST MODIFIED : 4 February 1999
DESCRIPTION :
==============================================================================*/

double Time_keeper_get_minimum(struct Time_keeper *time_keeper);
/*******************************************************************************
LAST MODIFIED : 4 February 1999
DESCRIPTION :
==============================================================================*/

int Time_keeper_set_minimum(struct Time_keeper *time_keeper, double minimum);
/*******************************************************************************
LAST MODIFIED : 4 February 1999
DESCRIPTION :
==============================================================================*/

int Time_keeper_set_play_once(struct Time_keeper *time_keeper);
/*******************************************************************************
LAST MODIFIED : 4 February 1999
DESCRIPTION :
==============================================================================*/

int Time_keeper_set_play_loop(struct Time_keeper *time_keeper);
/*******************************************************************************
LAST MODIFIED : 4 February 1999
DESCRIPTION :
==============================================================================*/

int Time_keeper_set_play_swing(struct Time_keeper *time_keeper);
/*******************************************************************************
LAST MODIFIED : 12 February 1999
DESCRIPTION :
==============================================================================*/

int Time_keeper_set_play_every_frame(struct Time_keeper *time_keeper);
/*******************************************************************************
LAST MODIFIED : 6 October 1998
DESCRIPTION :
==============================================================================*/

int Time_keeper_get_play_every_frame(struct Time_keeper *time_keeper);
/*******************************************************************************
LAST MODIFIED : 10 December 1998
DESCRIPTION :
==============================================================================*/

int Time_keeper_set_play_skip_frames(struct Time_keeper *time_keeper);
/*******************************************************************************
LAST MODIFIED : 6 October 1998
DESCRIPTION :
==============================================================================*/

enum Time_keeper_play_direction Time_keeper_get_play_direction(
	struct Time_keeper *time_keeper);
/*******************************************************************************
LAST MODIFIED : 9 December 1998
DESCRIPTION :
==============================================================================*/

int DESTROY(Time_keeper)(struct Time_keeper **time_keeper);
/*******************************************************************************
LAST MODIFIED : 29 September 1998

DESCRIPTION :
Destroys a Time_keeper object
x==============================================================================*/

int Time_keeper_has_time_object(struct Time_keeper *time_keeper);
/*******************************************************************************
LAST MODIFIED : 31 Oct 2007

DESCRIPTION :
Check if time keeper has time object
x==============================================================================*/

enum Time_keeper_play_mode Time_keeper_get_play_mode(
	struct Time_keeper *time_keeper);
/*******************************************************************************
LAST MODIFIED : 6 Mar 2009

DESCRIPTION :
Get the play mode of time keeper
x==============================================================================*/

#endif /* !defined (TIME_KEEPER_H) */

