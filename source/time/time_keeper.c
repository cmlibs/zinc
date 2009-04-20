/*******************************************************************************
FILE : time_keeper.c

LAST MODIFIED : 21 January 2003

DESCRIPTION :
This object defines a relationship between a bunch of time objects, keeps them
in sync and allows control such as play, rewind and fast forward.
This is intended to be multithreaded......
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
#include <math.h>
#include <stdio.h>
#include "general/debug.h"
#include "general/list_private.h"
#include "general/object.h"
#include "general/time.h"
#include "user_interface/event_dispatcher.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"
#include "time/time.h"
#include "time/time_private.h"
#include "time/time_keeper.h"

static int Time_keeper_set_play_timeout(struct Time_keeper *time_keeper);
/* Declaration for circular reference between this and the event handler and
   play private */

#if defined (MOTIF)
struct Time_keeper_defaults
{
	Boolean play_every_frame;
	float speed;
};
#endif /* defined (MOTIF) */

struct Time_keeper_callback_data
{
	Time_keeper_callback callback;
	void *callback_user_data;
	enum Time_keeper_event event_mask;

	struct Time_keeper_callback_data *next;
};

struct Time_object_info
{
	struct Time_object *time_object;
	double next_callback_due;
	struct Time_object_info *next;
};

struct Time_keeper
{
	char *name;
	double time;
	double real_time;
	struct Time_object_info *time_object_info_list;
	enum Time_keeper_play_mode play_mode;
	int minimum_set, maximum_set, play_remaining;
	double minimum, maximum;
	double speed;
	double step;
	enum Time_keeper_play_direction play_direction;
	int play_every_frame;
	time_t play_start_seconds;
	long play_start_microseconds;
	int playing;
	struct Event_dispatcher_timeout_callback *timeout_callback_id;
	struct Event_dispatcher *event_dispatcher;
	struct Time_keeper_callback_data *callback_list;
	
	int access_count;
};

DECLARE_OBJECT_FUNCTIONS(Time_keeper)

static int Time_keeper_notify_clients(struct Time_keeper *time_keeper,
	enum Time_keeper_event event_mask)
/*******************************************************************************
LAST MODIFIED : 

DESCRIPTION :
Sends any interested clients the callback message.
==============================================================================*/
{
	int return_code;
	struct Time_keeper_callback_data *callback_data;

	ENTER(Time_keeper_notify_clients);

	if (time_keeper)
	{
		callback_data = time_keeper->callback_list;
		while(callback_data)
		{
			if(callback_data->event_mask & event_mask)
			{
				(callback_data->callback)(time_keeper,
					event_mask, callback_data->callback_user_data);
			}
			callback_data = callback_data->next;
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Time_keeper_notify_clients.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Time_keeper_notify_clients */

static int Time_keeper_play_private(struct Time_keeper *time_keeper)
/*******************************************************************************
LAST MODIFIED : 16 October 1998

DESCRIPTION :
Starts the time_keeper from playing without notifying the interested clients.
Normally use Time_keeper_play
==============================================================================*/
{
	int return_code, looping = 0;
	struct Time_object_info *object_info;
	struct timeval timeofday;

	ENTER(Time_keeper_play_private);

	if (time_keeper)
	{
		if(!time_keeper->timeout_callback_id)
		{
			switch(time_keeper->play_direction)
			{
				case TIME_KEEPER_PLAY_FORWARD:
				{
					if(time_keeper->minimum_set && 
						(time_keeper->time < time_keeper->minimum))
					{
						time_keeper->time = time_keeper->minimum;
					}
					if (time_keeper->maximum_set &&
						(time_keeper->time >= time_keeper->maximum))
					{
							time_keeper->time = time_keeper->minimum;
							looping =1;
					}
				} break;
				case TIME_KEEPER_PLAY_BACKWARD:
				{
					if(time_keeper->maximum_set && 
						(time_keeper->time > time_keeper->maximum))
					{
						time_keeper->time = time_keeper->maximum;
					}
					if(time_keeper->minimum_set && 
						(time_keeper->time <= time_keeper->minimum))
					{
						time_keeper->time = time_keeper->maximum;
						looping =1;
					}
				} break;
			}
			gettimeofday(&timeofday, (struct timezone *)NULL);
			time_keeper->play_start_seconds = timeofday.tv_sec;
			time_keeper->play_start_microseconds = timeofday.tv_usec;
			time_keeper->real_time = time_keeper->time;

			object_info = time_keeper->time_object_info_list;
			while(object_info)
			{
				Time_object_set_current_time_privileged(object_info->time_object, time_keeper->time);
				/* if loop then check the current time is a valid callback time of time object,
					 if it is a valid callback time then notify clients for the change */
				if (looping && Time_object_check_valid_callback_time(
					object_info->time_object, time_keeper->time,
					time_keeper->play_direction))
				{
					Time_object_notify_clients_privileged(object_info->time_object);
				}
				object_info->next_callback_due = Time_object_get_next_callback_time(
					object_info->time_object, time_keeper->time,
					time_keeper->play_direction);
				object_info = object_info->next;
			}

			Time_keeper_notify_clients(time_keeper, TIME_KEEPER_NEW_TIME);
			
			return_code = Time_keeper_set_play_timeout(time_keeper);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Time_keeper_play_private.  Timekeeper is already playing");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Time_keeper_play_private.  Missing time_keeper");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Time_keeper_play_private */

static int Time_keeper_stop_private(struct Time_keeper *time_keeper)
/*******************************************************************************
LAST MODIFIED : 16 October 1998

DESCRIPTION :
Stops the time_keeper from playing without notifying the interested clients.
Normally use Time_keeper_stop
==============================================================================*/
{
	int return_code;

	ENTER(Time_keeper_stop_private);

	if (time_keeper)
	{
		if(time_keeper->timeout_callback_id)
		{
			Event_dispatcher_remove_timeout_callback(time_keeper->event_dispatcher,
				time_keeper->timeout_callback_id);
			time_keeper->timeout_callback_id = (struct Event_dispatcher_timeout_callback *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Time_keeper_stop_private.  Missing time_keeper");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Time_keeper_stop_private */

struct Time_keeper *CREATE(Time_keeper)(char *name,
	struct Event_dispatcher *event_dispatcher,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 15 March 2002

DESCRIPTION :
==============================================================================*/
{
#if defined (MOTIF)
#define XmNtimePlaySpeed "timePlaySpeed"
#define XmCtimePlaySpeed "TimePlaySpeed"
#define XmNtimePlayEveryFrame "timePlayEveryFrame"
#define XmCtimePlayEveryFrame "TimePlayEveryFrame"
	struct Time_keeper_defaults time_keeper_defaults;
	static XtResource resources[]=
	{
		{
			XmNtimePlaySpeed,
			XmCtimePlaySpeed,
			XmRFloat,
			sizeof(float),
			XtOffsetOf(struct Time_keeper_defaults,speed),
			XmRString,
			"1.0"
		},
		{
			XmNtimePlayEveryFrame,
			XmCtimePlayEveryFrame,
			XmRBoolean,
			sizeof(Boolean),
			XtOffsetOf(struct Time_keeper_defaults,play_every_frame),
			XmRString,
			"false"
		}
	};
#endif /* defined (MOTIF) */
	struct Time_keeper *time_keeper;

	ENTER(CREATE(Time_keeper));
#if !defined (MOTIF)
	USE_PARAMETER(user_interface);
#endif /* defined (MOTIF) */
	if(name)
	{
		if (ALLOCATE(time_keeper, struct Time_keeper, 1) &&
			ALLOCATE(time_keeper->name, char, strlen(name) + 1))
		{
#if defined (MOTIF)
			time_keeper_defaults.speed = 1.0;
			time_keeper_defaults.play_every_frame = False;
			if (user_interface)
			{
				XtVaGetApplicationResources(User_interface_get_application_shell(user_interface),
					&time_keeper_defaults,resources,XtNumber(resources),NULL);
			}
			time_keeper->speed = time_keeper_defaults.speed;
			if(time_keeper_defaults.play_every_frame)
			{
				time_keeper->play_every_frame = 1;
			}
			else
			{
				time_keeper->play_every_frame = 0;
			}
#else /* defined (MOTIF) */
			time_keeper->speed = 1.0;
			time_keeper->play_every_frame = 0;
#endif /* defined (MOTIF) */
			strcpy(time_keeper->name, name);
			time_keeper->time_object_info_list = (struct Time_object_info *)NULL;
			time_keeper->time = 0.0;
			time_keeper->play_mode = TIME_KEEPER_PLAY_LOOP;
			time_keeper->maximum_set = 0;
			time_keeper->minimum_set = 0;
			time_keeper->play_remaining = 0;
			time_keeper->maximum = 0.0;
			time_keeper->minimum = 0.0;
			time_keeper->real_time = 0.0;
			time_keeper->step = 0.0;
			time_keeper->play_direction = TIME_KEEPER_PLAY_FORWARD;
			time_keeper->play_start_seconds = 0;
			time_keeper->play_start_microseconds = 0;
			time_keeper->timeout_callback_id = (struct Event_dispatcher_timeout_callback *)NULL;
			time_keeper->playing = 0;
			time_keeper->event_dispatcher = event_dispatcher;
			time_keeper->callback_list = (struct Time_keeper_callback_data *)NULL;
			time_keeper->access_count = 0;
		}
		else
		{
			display_message(ERROR_MESSAGE,"CREATE(Time_keeper). Unable to allocate buffer structure");
			time_keeper = (struct Time_keeper *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(Time_keeper). Invalid arguments");
		time_keeper = (struct Time_keeper *)NULL;
	}
	LEAVE;

	return (time_keeper);
} /* CREATE(Time_keeper) */

int Time_keeper_add_time_object(struct Time_keeper *time_keeper,
	struct Time_object *time_object)
/*******************************************************************************
LAST MODIFIED : 6 October 1998

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Time_object_info *object_info, *previous;

	ENTER(Time_keeper_add_time_object);

	if (time_keeper && time_object)
	{
		if (!Time_object_get_time_keeper(time_object))
		{
			if(ALLOCATE(object_info, struct Time_object_info, 1))
			{
				object_info->time_object = time_object;
				Time_object_set_current_time_privileged(time_object, time_keeper->time);
				Time_object_notify_clients_privileged(time_object);
				if (time_keeper->playing)
				{
					switch(time_keeper->play_direction)
					{
						case TIME_KEEPER_PLAY_FORWARD:
						{
							object_info->next_callback_due = Time_object_get_next_callback_time(
								object_info->time_object, time_keeper->time + 0.01 * time_keeper->speed,
								TIME_KEEPER_PLAY_FORWARD);
						} break;
						case TIME_KEEPER_PLAY_BACKWARD:
						{
							object_info->next_callback_due = Time_object_get_next_callback_time(
								object_info->time_object, time_keeper->time - 0.01 * time_keeper->speed,
								TIME_KEEPER_PLAY_BACKWARD);
						} break;
					}
				}
				object_info->next = (struct Time_object_info *)NULL;
				if(time_keeper->time_object_info_list)
				{
					previous = time_keeper->time_object_info_list;
					while(previous->next)
					{
						previous = previous->next;
					}
					previous->next = object_info;
				}
				else
				{
					time_keeper->time_object_info_list = object_info;
				}
				Time_object_set_time_keeper(time_object, time_keeper);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Time_keeper_add_time_object.  Unable to allocate time object info structure");
			}
		}
		else
		{
				display_message(ERROR_MESSAGE,
					"Time_keeper_add_time_object.  Time object already has a time keeper.");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Time_keeper_add_time_object.  Missing time_keeper or time object");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Time_keeper_add_time_object */

int Time_keeper_remove_time_object(struct Time_keeper *time_keeper,
	struct Time_object *time_object)
/*******************************************************************************
LAST MODIFIED : 6 October 1998

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Time_object_info *object_info, *previous;

	ENTER(Time_keeper_remove_time_object);

	if (time_keeper && time_object)
	{
		if (time_keeper == Time_object_get_time_keeper(time_object))
		{
			return_code = 0;
			object_info = time_keeper->time_object_info_list;
			return_code = 0;
			while(!return_code && object_info)
			{
				if (object_info->time_object == time_object)
				{
					if (object_info == time_keeper->time_object_info_list)
					{
						time_keeper->time_object_info_list = object_info->next;
					}
					else
					{
						previous->next = object_info->next;
					}
					Time_object_set_time_keeper(object_info->time_object,
						(struct Time_keeper *)NULL);
					DEALLOCATE(object_info);
					return_code = 1;
				}
				else
				{
					previous = object_info;
					object_info = object_info->next;
				}
			}
			if (!return_code)
			{
				display_message(ERROR_MESSAGE,
					"Time_keeper_remove_time_object.  Unable to find time object specified");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Time_keeper_remove_time_object.  Time keeper does not match with the time keeper"
				"in time object.");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Time_keeper_remove_time_object.  Missing time_keeper or time object");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Time_keeper_remove_time_object */

int Time_keeper_add_callback(struct Time_keeper *time,
	Time_keeper_callback callback, void *user_data,
	enum Time_keeper_event event_mask)
/*******************************************************************************
LAST MODIFIED : 15 October 1998

DESCRIPTION :
Adds a callback routine which is called whenever the current time is changed.
==============================================================================*/
{
	int return_code;
	struct Time_keeper_callback_data *callback_data, *previous;

	ENTER(Time_keeper_add_callback);

	if (time && callback)
	{
		if(ALLOCATE(callback_data, struct Time_keeper_callback_data, 1))
		{
			callback_data->callback = callback;
			callback_data->callback_user_data = user_data;
			callback_data->event_mask = event_mask;
			callback_data->next = (struct Time_keeper_callback_data *)NULL;
			if(time->callback_list)
			{
				previous = time->callback_list;
				while(previous->next)
				{
					previous = previous->next;
				}
				previous->next = callback_data;
			}
			else
			{
				time->callback_list = callback_data;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Time_keeper_add_callback.  Unable to allocate callback data structure");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Time_keeper_add_callback.  Missing time keeper or callback");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Time_keeper_add_callback */

int Time_keeper_remove_callback(struct Time_keeper *time,
	Time_keeper_callback callback, void *user_data)
/*******************************************************************************
LAST MODIFIED : 15 October 1998

DESCRIPTION :
Removes a callback which was added previously
==============================================================================*/
{
	int return_code;
	struct Time_keeper_callback_data *callback_data, *previous;

	ENTER(Time_keeper_remove_callback);

	if (time && callback && time->callback_list)
	{
		callback_data = time->callback_list;
		if((callback_data->callback == callback)
			&& (callback_data->callback_user_data == user_data))
		{
			time->callback_list = callback_data->next;
			DEALLOCATE(callback_data);
			return_code = 1;
		}
		else
		{
			return_code = 0;
			while(!return_code && callback_data->next)
			{
				previous = callback_data;
				callback_data = callback_data->next;
				if((callback_data->callback == callback)
					&& (callback_data->callback_user_data == user_data))
				{
					previous->next = callback_data->next;
					DEALLOCATE(callback_data);
					return_code = 1;		
				}
			}
			if (!return_code)
			{
				display_message(ERROR_MESSAGE,
					"Time_keeper_remove_callback.  Unable to find callback and user_data specified");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Time_keeper_remove_callback.  Missing time, callback or callback list");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Time_keeper_remove_callback */

int Time_keeper_request_new_time(struct Time_keeper *time_keeper, double new_time)
/*******************************************************************************
LAST MODIFIED : 6 October 1998

DESCRIPTION :
==============================================================================*/
{
	int playing, return_code;
	static int recursive_check = 0;
	struct Time_object_info *object_info;

	ENTER(Time_keeper_request_new_time);

	if (time_keeper)
	{		
		/* SAB Some things in UNEMAP expect this to callback even when the
			value hasn't changed */
		/* Ensure that this new time request isn't generated from the callbacks
			inside this routine, otherwise an infinite loop could occur. */
		if(!recursive_check)
		{
			recursive_check = 1;

			time_keeper->time = new_time;
			/* If the object is playing */
			if(time_keeper->timeout_callback_id)
			{
				playing = 1;

				/* Using stop_private and play_private doesn't notify clients as
					we aren't really stopping */
				Time_keeper_stop_private(time_keeper);
			}
			else
			{
				playing = 0;
			}

			/* Update the times in all the clients and then get them to call their clients */
			object_info = time_keeper->time_object_info_list;
			while(object_info)
			{
				Time_object_set_current_time_privileged(object_info->time_object, new_time);
				object_info = object_info->next;
			}

			object_info = time_keeper->time_object_info_list;
			while(object_info)
			{
				Time_object_notify_clients_privileged(object_info->time_object);
				object_info = object_info->next;
			}

			Time_keeper_notify_clients(time_keeper, TIME_KEEPER_NEW_TIME);

			if(playing)
			{
				Time_keeper_play_private(time_keeper);
			}

			recursive_check = 0;
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Time_keeper_request_new_time.  Recursive entry disabled");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Time_keeper_request_new_time.  Missing time_keeper");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Time_keeper_request_new_time */

double Time_keeper_get_time(struct Time_keeper *time_keeper)
/*******************************************************************************
LAST MODIFIED : 14 October 1998
DESCRIPTION :
==============================================================================*/
{
	double return_time;

	ENTER(Time_keeper_get_time);

	if (time_keeper)
	{
		return_time = time_keeper->time;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Time_keeper_get_time. Invalid time object");
		return_time = 0.0;
	}
	LEAVE;

	return (return_time);
} /* Time_keeper_get_time */

static int Time_keeper_timer_event_handler(void *time_keeper_void)
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
Responds to Xttimer_Events generated by the Time_keeper xttimer_event queue.
==============================================================================*/
{
	double event_time, real_time_elapsed, closest_object_time, event_interval;
	int first_event_time, return_code;
	struct Time_keeper *time_keeper;
	struct Time_object_info *object_info;
	struct timeval timeofday;

	ENTER(Time_keeper_xttimer_event_handler);

	if (time_keeper = (struct Time_keeper *)time_keeper_void)
	{
		if(time_keeper->timeout_callback_id)
		{
			time_keeper->timeout_callback_id = (struct Event_dispatcher_timeout_callback *)NULL;
			first_event_time = 1;

			gettimeofday(&timeofday, (struct timezone *)NULL);
			real_time_elapsed = (double)(timeofday.tv_sec - 
				time_keeper->play_start_seconds) + ((double)(timeofday.tv_usec
				- time_keeper->play_start_microseconds) / 1000000.0);
			real_time_elapsed *= time_keeper->speed;
			time_keeper->play_start_seconds = timeofday.tv_sec;
			time_keeper->play_start_microseconds = timeofday.tv_usec;
			/* Set an interval from within which we will do every event pending event.
				When we are playing every frame we want this to be much smaller */
			if (time_keeper->play_every_frame)
			{
				event_interval = 0.00001 * time_keeper->speed;				
			}
			else
			{
				event_interval = 0.01 * time_keeper->speed;
			}
#if defined (DEBUG)
			printf("Time_keeper_xttimer_event_handler. real_time %lf  stepped time %lf\n", 
				time_keeper->time + real_time_elapsed, time_keeper->time + time_keeper->step);
#endif /* defined (DEBUG) */
			switch(time_keeper->play_direction)
			{
				case TIME_KEEPER_PLAY_FORWARD:
				{
					if(time_keeper->play_every_frame)
					{
						time_keeper->real_time += time_keeper->step;
					}
					else
					{
						time_keeper->real_time += real_time_elapsed;
					}
#if defined (DEBUG)
					if(real_time_elapsed < time_keeper->step)
					{
						printf("time_keeper_xttimer_event_handler. real_time %lf  stepped time %lf\n",
							real_time_elapsed, time_keeper->step);
					}
					else
					{
						printf("   time_keeper_xttimer_event_handler. real_time %lf  stepped time %lf\n",
							real_time_elapsed, time_keeper->step);
					}
#endif /* defined (DEBUG) */
					/* Record the time_keeper->time so that if a callback changes it
						then we do a full restart */
					time_keeper->time = time_keeper->real_time; 

					object_info = time_keeper->time_object_info_list;
					while(object_info)
					{
						/* Do all the events in the next event interval */
						if(object_info->next_callback_due < time_keeper->time + event_interval)
						{
							if(!time_keeper->play_every_frame)
							{
								/* Then look for the event that should have occurred most
									recently */
								closest_object_time =
									Time_object_get_next_callback_time(
									object_info->time_object, time_keeper->time + event_interval,
									TIME_KEEPER_PLAY_BACKWARD);
								if(closest_object_time >= object_info->next_callback_due)
								{
									object_info->next_callback_due = closest_object_time;
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"Time_keeper_xttimer_event_handler.  Closest_object_time occurs after next_callback_due");
								}
							}
							if(first_event_time || object_info->next_callback_due > event_time)
							{
								first_event_time = 0;
								event_time = object_info->next_callback_due;
							}
							if (object_info->next_callback_due < time_keeper->maximum)
							{
								Time_object_set_current_time_privileged(object_info->time_object,
									object_info->next_callback_due);
								Time_object_notify_clients_privileged(object_info->time_object);
							}
							object_info->next_callback_due = Time_object_get_next_callback_time(
								object_info->time_object, time_keeper->time + event_interval,
								TIME_KEEPER_PLAY_FORWARD);
#if defined (DEBUG)
							printf("Time_keeper_xttimer_event_handler. callback %p\n", object_info->time_object);
#endif /* defined (DEBUG) */
						}
						else if (time_keeper->play_remaining && time_keeper->maximum_set &&
							time_keeper->time + event_interval>time_keeper->maximum)
						{
							/* when playing the remaining time, the difference between 
								 next_callback_due and current time is normally large then the event interval,
								 so the actual event time needs to be set to maximum here */
							event_time = time_keeper->maximum;
							time_keeper->play_remaining = 0;
						}
						object_info = object_info->next;
					}
					
				} break;
				case TIME_KEEPER_PLAY_BACKWARD:
				{
					if(time_keeper->play_every_frame)
					{
						time_keeper->real_time -= time_keeper->step;
					}
					else
					{
						time_keeper->real_time -= real_time_elapsed;
					}
					/* Record the time_keeper->time so that if a callback changes it
						then we do a full restart */
					time_keeper->time = time_keeper->real_time;

					object_info = time_keeper->time_object_info_list;
					while(object_info)
					{
						/* Do all the events in the next 10 milliseconds */
						if(object_info->next_callback_due > time_keeper->time - event_interval)
						{
							if(!time_keeper->play_every_frame)
							{
								/* Then look for the event that should have occurred most
									recently */
								closest_object_time =
									Time_object_get_next_callback_time(
									object_info->time_object, time_keeper->time - event_interval,
									TIME_KEEPER_PLAY_FORWARD);
								if(closest_object_time <= object_info->next_callback_due)
								{
									object_info->next_callback_due = closest_object_time;
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"Time_keeper_xttimer_event_handler.  Closest_object_time occurs after next_callback_due");
								}
							}
							if(first_event_time || object_info->next_callback_due < event_time)
							{
								first_event_time = 0;
								event_time = object_info->next_callback_due;
							}
							if (object_info->next_callback_due >= time_keeper->minimum)
							{
								Time_object_set_current_time_privileged(object_info->time_object,
									object_info->next_callback_due);
								Time_object_notify_clients_privileged(object_info->time_object);
							}
							object_info->next_callback_due = Time_object_get_next_callback_time(
								object_info->time_object, time_keeper->time - event_interval,
								TIME_KEEPER_PLAY_BACKWARD);
#if defined (DEBUG)
							printf("Time_keeper_xttimer_event_handler. callback %p\n", object_info->time_object);
#endif /* defined (DEBUG) */
						}
						else if (time_keeper->play_remaining && time_keeper->minimum_set &&
							(time_keeper->time - event_interval)<time_keeper->minimum)
						{
							/* when playing the remaining time, the difference between 
								 next_callback_due and current time is normally large then the event interval,
								 so the actual event time needs to be set to minimum here */
							event_time = time_keeper->minimum;
							time_keeper->play_remaining = 0;
						}
						object_info = object_info->next;
					}
				} break;
			}

			if(time_keeper->time == time_keeper->real_time)
			{
				/* We want it to appear to the clients that only actual event times
					have occured */
				time_keeper->time = event_time;
				if(!first_event_time)
				{
					Time_keeper_notify_clients(time_keeper, TIME_KEEPER_NEW_TIME);
				}
#if defined (DEBUG)
				/* SAB Sometimes the XtTimer returns too early, ignore this error and
					just reschedule another timeout */
				else
				{
					display_message(ERROR_MESSAGE,
						"Time_keeper_xttimer_event_handler.  Timer returned but no callbacks due");
				}
#endif /* defined (DEBUG) */
				if(time_keeper->time == event_time)
				{
					Time_keeper_set_play_timeout(time_keeper);
				}
				else
				{
					Time_keeper_play_private(time_keeper);
				}
			}
			else
			{
				Time_keeper_play_private(time_keeper);
			}
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Time_keeper_xttimer_event_handler.  Unknown source for callback");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Time_keeper_xttimer_event_handler.  Missing time_keeper");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Time_keeper_xttimer_event_handler */

static int Time_keeper_set_play_timeout(struct Time_keeper *time_keeper)
/*******************************************************************************
LAST MODIFIED : 21 January 2003

DESCRIPTION :
==============================================================================*/
{
	double next_time, real_time_elapsed, sleep;
	int return_code;
	struct Time_object_info *object_info;
	struct timeval timeofday;
	unsigned long sleep_s, sleep_ns;

	ENTER(Time_keeper_play);
	
	if (time_keeper && time_keeper->event_dispatcher)
	{
		object_info = time_keeper->time_object_info_list;
		if(object_info)
		{
			switch(time_keeper->play_direction)
			{
				case TIME_KEEPER_PLAY_FORWARD:
				{
					next_time = object_info->next_callback_due;
					object_info = object_info->next;
					while(object_info)
					{
						if(object_info->next_callback_due < next_time )
						{
							next_time = object_info->next_callback_due;
						}
						object_info = object_info->next;
					}
					
#if defined (DEBUG)
					printf("Time_keeper_set_play_timeout.  Next time %lf\n", next_time);	
#endif /* defined (DEBUG) */
 
					if (time_keeper->maximum_set && 
						(next_time > time_keeper->maximum) && 
						(time_keeper->time < time_keeper->maximum))
					{
						next_time = time_keeper->maximum;
						time_keeper->play_remaining = 1;
					}
					if(time_keeper->maximum_set && (next_time > time_keeper->maximum) && 
						!time_keeper->play_remaining)
					{
						if(time_keeper->time > time_keeper->minimum)
						{
							switch(time_keeper->play_mode)
							{
								case TIME_KEEPER_PLAY_LOOP:
								{		 
									return_code = Time_keeper_play_private(time_keeper);
								} break ;
								case TIME_KEEPER_PLAY_SWING:
								{
									time_keeper->time = time_keeper->maximum;
									time_keeper->play_direction = TIME_KEEPER_PLAY_BACKWARD;
									Time_keeper_notify_clients(time_keeper, TIME_KEEPER_CHANGED_DIRECTION);
									return_code = Time_keeper_play_private(time_keeper);
								} break ;
								case TIME_KEEPER_PLAY_ONCE:
								{
									Time_keeper_stop(time_keeper);
									return_code = 0;
								} break;
								default:
								{
									Time_keeper_stop(time_keeper);
									display_message(ERROR_MESSAGE,
										"Time_keeper_set_play_timeout.  Unknown play mode");
									return_code = 0;
								} break;
							}
						}
						else
						{
							Time_keeper_stop(time_keeper);
							return_code = 0;
						}
					}
					else
					{
						/*???DB.  Changed from > to >= */
						if(next_time >= time_keeper->real_time)
						{
							time_keeper->step = next_time - time_keeper->real_time;
							
							gettimeofday(&timeofday, (struct timezone *)NULL);
							real_time_elapsed = (double)(timeofday.tv_sec - 
								time_keeper->play_start_seconds) + ((double)(timeofday.tv_usec
									- time_keeper->play_start_microseconds) / 1000000.0);
							sleep = time_keeper->step / time_keeper->speed - real_time_elapsed;
							if (sleep > 0)
							{
								sleep_s = (unsigned long)floor(sleep);
								sleep_ns = (unsigned long)((sleep - floor(sleep))*1e9);
							}
							else
							{
								sleep_s = 0;
								sleep_ns = 0;
							}
							if((sleep_s < 1) && (sleep_ns < 3000000))
							{
								/* Ensure all the events from the previous timestamp are 
									processed before the events from this next callback occur */
								sleep_ns = 3000000;
							}
							time_keeper->timeout_callback_id = Event_dispatcher_add_timeout_callback(
								time_keeper->event_dispatcher, (unsigned long)sleep_s, sleep_ns, 
								Time_keeper_timer_event_handler, (void *)time_keeper);
							return_code=1;
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Time_keeper_set_play_timeout.  Next time is in the past");
							return_code=0;
						}
					}
				} break;
				case TIME_KEEPER_PLAY_BACKWARD:
				{
					next_time = object_info->next_callback_due;
					object_info = object_info->next;
					while(object_info)
					{
						if(object_info->next_callback_due > next_time )
						{
							next_time = object_info->next_callback_due;
						}
						object_info = object_info->next;
					}
					
#if defined (DEBUG)
					printf("Time_keeper_set_play_timeout.  Next time %lf\n", next_time);
#endif /* defined (DEBUG) */
					if (time_keeper->minimum_set && 
						(next_time < time_keeper->minimum) && 
						(time_keeper->time > time_keeper->minimum))
					{
						next_time = time_keeper->minimum;
						time_keeper->play_remaining = 1;
					}
					if(time_keeper->minimum_set && (next_time < time_keeper->minimum) &&
						!time_keeper->play_remaining)
					{
						if(time_keeper->time < time_keeper->maximum)
						{
							switch(time_keeper->play_mode)
							{
								case TIME_KEEPER_PLAY_LOOP:
								{
									return_code = Time_keeper_play_private(time_keeper);
								} break ;
								case TIME_KEEPER_PLAY_SWING:
								{
									time_keeper->time = time_keeper->minimum;
									time_keeper->play_direction = TIME_KEEPER_PLAY_FORWARD;
									Time_keeper_notify_clients(time_keeper, TIME_KEEPER_CHANGED_DIRECTION);
									return_code = Time_keeper_play_private(time_keeper);
								} break ;
								case TIME_KEEPER_PLAY_ONCE:
								{
									Time_keeper_stop(time_keeper);
									return_code=0;
								} break;
								default:
								{
									Time_keeper_stop(time_keeper);
									display_message(ERROR_MESSAGE,
										"Time_keeper_set_play_timeout.  Unknown play mode");
									return_code=0;
								} break;
							}
						}
						else
						{
							Time_keeper_stop(time_keeper);
							return_code=0;
						}
					}
					else
					{
						/*???DB.  Changed from < to <= */
						if(next_time <= time_keeper->real_time)
						{
							time_keeper->step = time_keeper->real_time - next_time;

							gettimeofday(&timeofday, (struct timezone *)NULL);
							real_time_elapsed = (double)(timeofday.tv_sec - 
								time_keeper->play_start_seconds) + ((double)(timeofday.tv_usec
									- time_keeper->play_start_microseconds) / 1000000.0);
							sleep = time_keeper->step / time_keeper->speed - real_time_elapsed;
							if (sleep > 0)
							{
								sleep_s = (unsigned long)floor(sleep);
								sleep_ns = (unsigned long)((sleep - floor(sleep))*1e9);
							}
							else
							{
								sleep_s = 0;
								sleep_ns = 0;
							}
							if((sleep_s < 1) && (sleep_ns < 3000000))
							{
								/* Ensure all the events from the previous timestamp are 
									processed before the events from this next callback occur */
								sleep_ns = 3000000;
							}
							time_keeper->timeout_callback_id = Event_dispatcher_add_timeout_callback(
								time_keeper->event_dispatcher, sleep_s, sleep_ns, 
								Time_keeper_timer_event_handler, (void *)time_keeper);
							return_code=1;
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Time_keeper_set_play_timeout.  Playing back %lf and next time %lf is in the future (real_time %lf)",
								time_keeper->time, next_time, time_keeper->real_time);
							return_code=0;
						}
					}
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"Time_keeper_set_play_timeout.  Unknown play direction");
					return_code=0;
				} break;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Time_keeper_set_play_timeout.  No time objects in timekeeper");
			return_code=0;
		}		
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Time_keeper_set_play_timeout.  Missing time_keeper or event_dispatcher");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Time_keeper_set_play_timeout */


int Time_keeper_play(struct Time_keeper *time_keeper,
	enum Time_keeper_play_direction play_direction)
/*******************************************************************************
LAST MODIFIED : 7 December 2001

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Time_keeper_play);

	if (time_keeper)
	{
		if(!Time_keeper_is_playing(time_keeper))
		{
			time_keeper->play_direction = play_direction;	
			/*notify clients before playing. If play fails, notify of stop*/
			Time_keeper_notify_clients(time_keeper, TIME_KEEPER_STARTED);
			if(Time_keeper_play_private(time_keeper))
			{
				time_keeper->playing = 1;
			}
			else
			{
				time_keeper->playing = 0;
				Time_keeper_notify_clients(time_keeper, TIME_KEEPER_STOPPED);
			}
		}
		else
		{
			if(play_direction != time_keeper->play_direction)
			{
				Time_keeper_stop_private(time_keeper);
				time_keeper->play_direction = play_direction;
				Time_keeper_play_private(time_keeper);	
				Time_keeper_notify_clients(time_keeper, TIME_KEEPER_CHANGED_DIRECTION);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Time_keeper_play.  Timekeeper is already playing");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Time_keeper_play.  Missing time_keeper");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Time_keeper_play */

int Time_keeper_is_playing(struct Time_keeper *time_keeper)
/*******************************************************************************
LAST MODIFIED : 1 October 1998

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Time_keeper_play);

	if (time_keeper)
	{
		if(time_keeper->playing)
		{
			return_code = 1;
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Time_keeper_is_playing.  Missing time_keeper or event_dispatcher");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Time_keeper_is_playing */

int Time_keeper_stop(struct Time_keeper *time_keeper)
/*******************************************************************************
LAST MODIFIED : 1 October 1998

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Time_keeper_stop);

	if (time_keeper)
	{
		if(Time_keeper_is_playing(time_keeper))
		{
			return_code = Time_keeper_stop_private(time_keeper);

			time_keeper->playing = 0;

			Time_keeper_notify_clients(time_keeper, TIME_KEEPER_STOPPED);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Time_keeper_stop.  Missing time_keeper");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Time_keeper_stop */

double Time_keeper_get_speed(struct Time_keeper *time_keeper)
/*******************************************************************************
LAST MODIFIED : 6 October 1998
DESCRIPTION :
==============================================================================*/
{
	double return_speed;

	ENTER(Time_keeper_get_speed);

	if (time_keeper)
	{
		return_speed = time_keeper->speed;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Time_keeper_get_speed. Invalid time object");
		return_speed = 0.0;
	}
	LEAVE;

	return (return_speed);
} /* Time_keeper_get_speed */

int Time_keeper_set_speed(struct Time_keeper *time_keeper, double speed)
/*******************************************************************************
LAST MODIFIED : 6 October 1998
DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Time_object_set_time_keeper);

	if (time_keeper)
	{
		time_keeper->speed = speed;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Time_keeper_set_speed. Invalid time object");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Time_keeper_set_speed */

double Time_keeper_get_maximum(struct Time_keeper *time_keeper)
/*******************************************************************************
LAST MODIFIED : 4 February 1999
DESCRIPTION :
==============================================================================*/
{
	double return_maximum;

	ENTER(Time_keeper_get_maximum);

	if (time_keeper)
	{
		return_maximum = time_keeper->maximum;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Time_keeper_get_maximum. Invalid time object");
		return_maximum = 0.0;
	}
	LEAVE;

	return (return_maximum);
} /* Time_keeper_get_maximum */

int Time_keeper_set_maximum(struct Time_keeper *time_keeper, double maximum)
/*******************************************************************************
LAST MODIFIED : 4 February 1999
DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Time_object_set_time_keeper);

	if (time_keeper)
	{		
		time_keeper->maximum_set = 1;
		time_keeper->maximum = maximum;
		Time_keeper_notify_clients(time_keeper, TIME_KEEPER_NEW_MAXIMUM);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Time_keeper_set_maximum. Invalid time object");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Time_keeper_set_maximum */

double Time_keeper_get_minimum(struct Time_keeper *time_keeper)
/*******************************************************************************
LAST MODIFIED : 4 February 1999
DESCRIPTION :
==============================================================================*/
{
	double return_minimum;

	ENTER(Time_keeper_get_minimum);

	if (time_keeper)
	{
		return_minimum = time_keeper->minimum;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Time_keeper_get_minimum. Invalid time object");
		return_minimum = 0.0;
	}
	LEAVE;

	return (return_minimum);
} /* Time_keeper_get_minimum */

int Time_keeper_set_minimum(struct Time_keeper *time_keeper, double minimum)
/*******************************************************************************
LAST MODIFIED : 4 February 1999
DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Time_object_set_time_keeper);

	if (time_keeper)
	{	
		time_keeper->minimum_set = 1;
		time_keeper->minimum = minimum;
		Time_keeper_notify_clients(time_keeper, TIME_KEEPER_NEW_MINIMUM);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Time_keeper_set_minimum. Invalid time object");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Time_keeper_set_minimum */

int Time_keeper_set_play_loop(struct Time_keeper *time_keeper)
/*******************************************************************************
LAST MODIFIED : 4 February 1999
DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Time_keeper_set_play_loop);

	if (time_keeper)
	{
		time_keeper->play_mode = TIME_KEEPER_PLAY_LOOP;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Time_keeper_set_play_loop. Invalid time keeper");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Time_keeper_set_play_loop */

int Time_keeper_set_play_once(struct Time_keeper *time_keeper)
/*******************************************************************************
LAST MODIFIED : 4 February 1999
DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Time_keeper_set_play_once);

	if (time_keeper)
	{
		time_keeper->play_mode = TIME_KEEPER_PLAY_ONCE;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Time_keeper_set_play_once. Invalid time keeper");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Time_keeper_set_play_once */

int Time_keeper_set_play_swing(struct Time_keeper *time_keeper)
/*******************************************************************************
LAST MODIFIED : 12 February 1999
DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Time_keeper_set_play_once);

	if (time_keeper)
	{
		time_keeper->play_mode = TIME_KEEPER_PLAY_SWING;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Time_keeper_set_play_once. Invalid time keeper");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Time_keeper_set_play_once */

int Time_keeper_set_play_every_frame(struct Time_keeper *time_keeper)
/*******************************************************************************
LAST MODIFIED : 6 October 1998
DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Time_keeper_set_play_every_frame);

	if (time_keeper)
	{
		time_keeper->play_every_frame = 1;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Time_keeper_set_play_every_frame. Invalid time keeper");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Time_keeper_set_play_every_frame */

int Time_keeper_get_play_every_frame(struct Time_keeper *time_keeper)
/*******************************************************************************
LAST MODIFIED : 10 December 1998
DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Time_object_get_play_every_frame);

	if (time_keeper)
	{
		return_code = time_keeper->play_every_frame;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Time_keeper_get_play_every_frame. Invalid time keeper");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Time_keeper_get_play_every_frame */

int Time_keeper_set_play_skip_frames(struct Time_keeper *time_keeper)
/*******************************************************************************
LAST MODIFIED : 6 October 1998
DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Time_object_set_time_keeper);

	if (time_keeper)
	{
		time_keeper->play_every_frame = 0;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Time_keeper_set_play_skip_frames. Invalid time object");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Time_keeper_set_play_skip_frames */

enum Time_keeper_play_direction Time_keeper_get_play_direction(
	struct Time_keeper *time_keeper)
/*******************************************************************************
LAST MODIFIED : 9 December 1998
DESCRIPTION :
==============================================================================*/
{
	enum Time_keeper_play_direction return_direction;

	ENTER(Time_keeper_get_play_direction);

	if (time_keeper)
	{
		return_direction = time_keeper->play_direction;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Time_keeper_get_play_direction. Invalid time keeper");
		return_direction = TIME_KEEPER_PLAY_FORWARD;
	}
	LEAVE;

	return (return_direction);
} /* Time_keeper_get_play_direction */

int DESTROY(Time_keeper)(struct Time_keeper **time_keeper)
/*******************************************************************************
LAST MODIFIED : 29 September 1998

DESCRIPTION :
Destroys a Time_keeper object
x==============================================================================*/
{
	int return_code;
	struct Time_object_info *object_info, *next;
	struct Time_keeper_callback_data *callback_data, *next_callback;


	ENTER(DESTROY(Time_keeper));

	if (time_keeper && *time_keeper)
	{
		return_code=1;
		Time_keeper_stop(*time_keeper);

		callback_data = (*time_keeper)->callback_list;
		while(callback_data)
		{
			next_callback = callback_data->next;
			DEALLOCATE(callback_data);
			callback_data = next_callback;
		}

		object_info = (*time_keeper)->time_object_info_list;
		while(object_info)
		{
			if (object_info->time_object)
			{
				Time_object_set_time_keeper(object_info->time_object,
					(struct Time_keeper *)NULL);
				object_info->time_object = (struct Time_object *)NULL;
			}
			next = object_info->next;
			DEALLOCATE(object_info);
			object_info = next;
 		}

		if ((*time_keeper)->name)
		{
			DEALLOCATE((*time_keeper)->name);
		}
		DEALLOCATE(*time_keeper);
		*time_keeper = (struct Time_keeper *)NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Time_keeper).  Missing time_keeper object");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Time_keeper) */

int Time_keeper_has_time_object(struct Time_keeper *time_keeper)
/*******************************************************************************
LAST MODIFIED : 31 Oct 2007

DESCRIPTION :
Check if time keeper has time object
x==============================================================================*/
{
	 int return_code;
	 
	 ENTER(Time_object_has_time_object);
	 if (time_keeper)
	 {
			if (time_keeper->time_object_info_list)
			{
				 return_code = 1;
			}
			else
			{
				 return_code = 0;
			}
	 }
	 else
	 {
			display_message(ERROR_MESSAGE,
				 "Time_keeper_set_play_skip_frames. Invalid time object");
			return_code=0;
	 }
	 LEAVE;
	 
	return (return_code);
} /* Time_keeper_has_time_object */

enum Time_keeper_play_mode Time_keeper_get_play_mode(
	struct Time_keeper *time_keeper)
{
	enum Time_keeper_play_mode play_mode;

	ENTER(Time_keeper_get_play_mode);
	if (time_keeper)
	{
		play_mode = time_keeper->play_mode;

	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Time_keeper_set_play_skip_frames. Invalid time object");
		play_mode = TIME_KEEPER_PLAY_ONCE;
	}
	LEAVE;

	return (play_mode);
}
