/*******************************************************************************
FILE : time.c

LAST MODIFIED : 25 November 1999

DESCRIPTION :
This provides an object which supplies a concept of time to Cmgui
==============================================================================*/
#include <math.h>
#include <stdio.h>
#include "general/debug.h"
#include "general/object.h"
#include "user_interface/message.h"
#include "time/time.h"
#include "time/time_keeper.h"

enum Time_object_type
{
	TIME_OBJECT_UPDATE_FREQUENCY,
	TIME_OBJECT_NEXT_TIME_FUNCTION
};

struct Time_object_callback_data
{
	Time_object_callback callback;
	void *callback_user_data;

	struct Time_object_callback_data *next;
};

struct Time_object
{
	char *name;
	double current_time;
	double update_frequency;
	enum Time_object_type type;
	struct Time_object_callback_data *callback_list;
	struct Time_keeper *time_keeper;
	Time_object_next_time_function next_time_function;
	void *next_time_user_data;

	int access_count;
};

DECLARE_OBJECT_FUNCTIONS(Time_object)
DECLARE_DEFAULT_GET_OBJECT_NAME_FUNCTION(Time_object)

struct Time_object *CREATE(Time_object)(char *name)
/*******************************************************************************
LAST MODIFIED : 29 September 1998

DESCRIPTION :
==============================================================================*/
{
	struct Time_object *time;

	ENTER(CREATE(Time_object));

	if(name)
	{
		if (ALLOCATE(time, struct Time_object, 1) &&
			ALLOCATE(time->name, char, strlen(name) + 1))
		{
			strcpy(time->name, name);
			time->current_time = 0.0;
			time->time_keeper = (struct Time_keeper *)NULL;
			time->callback_list = (struct Time_object_callback_data *)NULL;
			time->update_frequency = 10.0;
			time->type = TIME_OBJECT_UPDATE_FREQUENCY;
			time->next_time_function = (Time_object_next_time_function)NULL;
			time->next_time_user_data = NULL;
			time->access_count = 0;
		}
		else
		{
			display_message(ERROR_MESSAGE,"CREATE(Time_object). Unable to allocate buffer structure");
			time = (struct Time_object *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(Time_object). Invalid arguments");
		time = (struct Time_object *)NULL;
	}
	LEAVE;

	return (time);
} /* CREATE(Time_object) */

double Time_object_get_current_time(struct Time_object *time)
/*******************************************************************************
LAST MODIFIED : 29 September 1998

DESCRIPTION :
==============================================================================*/
{
	double return_code;

	ENTER(Time_object_get_current_time);

	if (time)
	{
		return_code = time->current_time;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Time_object_get_current_time. Invalid time object");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Time_object_get_current_time */

double Time_object_get_next_callback_time(struct Time_object *time,
	double time_after, enum Time_keeper_play_direction play_direction)
/*******************************************************************************
LAST MODIFIED : 9 December 1998

DESCRIPTION :
==============================================================================*/
{
	double return_code;

	ENTER(Time_object_get_next_callback_time);

	if (time)
	{
		switch(time->type)
		{
			default:
			{
				switch(play_direction)
				{
					case TIME_KEEPER_PLAY_FORWARD:
					{
						return_code = (1.0 +
							floor(time_after * time->update_frequency)) / time->update_frequency;
					} break;
					case TIME_KEEPER_PLAY_BACKWARD:
					{
						return_code = (-1.0 +
							ceil(time_after * time->update_frequency)) / time->update_frequency;
					} break;
					default:
					{
						display_message(ERROR_MESSAGE,
							"Time_object_get_next_callback_time.  Unknown play direction");
						return_code=0;
					} break;
				}
			} break;
			case TIME_OBJECT_NEXT_TIME_FUNCTION:
			{
				if(time->next_time_function)
				{
					return_code = (time->next_time_function)(time_after, play_direction,
						time->next_time_user_data);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Time_object_get_next_callback_time.  type TIME_OBJECT_NEXT_TIME_FUNCTION but no function");
					return_code=0;
				}
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Time_object_get_next_callback_time.  Invalid time object");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Time_object_get_next_callback_time */

int Time_object_set_current_time_privileged(struct Time_object *time,
	double new_time)
/*******************************************************************************
LAST MODIFIED : 17 January 2002

DESCRIPTION :
This routine allows the timekeeper to explicitly set the time.
Separated Time_object_notify_clients_privileged so that all the clients
can be updated with the new time before any of them call back to their clients.
Users of a time object that is controlled by a timekeeper should set the time 
through the timekeeper.
==============================================================================*/
{
	int return_code;

	ENTER(Time_object_set_current_time_privileged);

	if (time)
	{
		time->current_time = new_time;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Time_object_set_current_time_privileged. Invalid time object");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Time_object_set_current_time_privileged */

int Time_object_notify_clients_privileged(struct Time_object *time)
/*******************************************************************************
LAST MODIFIED : 17 January 2002

DESCRIPTION :
This routine allows the timekeeper to tell the time_object to notify its clients.
Separated off from Time_object_set_current_time_privileged so that all the clients
can be updated with the new time before any of them call back to their clients.
Users of a time object that is controlled by a timekeeper should set the time 
through the timekeeper.
==============================================================================*/
{
	int return_code;
	struct Time_object_callback_data *callback_data;

	ENTER(Time_object_set_current_time_privileged);

	if (time)
	{
		callback_data = time->callback_list;
		while(callback_data)
		{
			(callback_data->callback)(time, time->current_time,
				callback_data->callback_user_data);
			callback_data = callback_data->next;
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Time_object_set_current_time_privileged. Invalid time object");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Time_object_set_current_time_privileged */

int Time_object_set_update_frequency(struct Time_object *time, double frequency)
/*******************************************************************************
LAST MODIFIED : 25 November 1999

DESCRIPTION :
This controls the rate per second which the time depedent object is called back
when in play mode.
==============================================================================*/
{
	int return_code;

	ENTER(Time_object_set_update_frequency);
	if (time)
	{
		time->update_frequency = frequency;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Time_object_set_update_frequency. Invalid time object");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Time_object_set_update_frequency */

int Time_object_set_next_time_function(struct Time_object *time,
	Time_object_next_time_function next_time_function, void *user_data)
/*******************************************************************************
LAST MODIFIED : 25 November 1999

DESCRIPTION :
By setting this function when the time_keeper requests the update_frequency is
not used.  Instead the next_time_function is called to evaluate the next valid
time.
==============================================================================*/
{
	int return_code;

	ENTER(Time_object_set_update_frequency);
	if (time && next_time_function)
	{
		time->type = TIME_OBJECT_NEXT_TIME_FUNCTION;
		time->next_time_function = next_time_function;
		time->next_time_user_data = user_data;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Time_object_set_next_time_function. Invalid arguments");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Time_object_set_next_time_function */

struct Time_keeper *Time_object_get_time_keeper(struct Time_object *time)
/*******************************************************************************
LAST MODIFIED : 29 September 1998

DESCRIPTION :
==============================================================================*/
{
	struct Time_keeper *return_code;

	ENTER(Time_object_get_time_keeper);

	if (time)
	{
		return_code = time->time_keeper;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Time_object_get_time_keeper. Invalid time object");
		return_code=(struct Time_keeper *)NULL;
	}
	LEAVE;

	return (return_code);
} /* Time_object_get_time_keeper */

int Time_object_set_time_keeper(struct Time_object *time,
	struct Time_keeper *time_keeper)
/*******************************************************************************
LAST MODIFIED : 29 September 1998

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Time_object_set_time_keeper);

	if (time)
	{
		if(time_keeper != time->time_keeper)
		{
			if(time->time_keeper)
			{
				Time_keeper_remove_time_object(time->time_keeper, time);
				DEACCESS(Time_keeper)(&(time->time_keeper));
			}
			if(time_keeper)
			{
				time->time_keeper = ACCESS(Time_keeper)(time_keeper);
				Time_keeper_add_time_object(time_keeper, time);
			}
			else
			{
				time->time_keeper = (struct Time_keeper *)NULL;
			}
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Time_object_set_time_keeper. Invalid time object");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Time_object_set_time_keeper */

int Time_object_add_callback(struct Time_object *time,
	Time_object_callback callback, void *user_data)
/*******************************************************************************
LAST MODIFIED : 29 September 1998

DESCRIPTION :
Adds a callback routine which is called whenever the current time is changed.
==============================================================================*/
{
	int return_code;
	struct Time_object_callback_data *callback_data, *previous;

	ENTER(Time_object_add_callback);

	if (time && callback)
	{
		if(ALLOCATE(callback_data, struct Time_object_callback_data, 1))
		{
			callback_data->callback = callback;
			callback_data->callback_user_data = user_data;
			callback_data->next = (struct Time_object_callback_data *)NULL;
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
				"Time_object_add_callback.  Unable to allocate callback data structure");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Time_object_add_callback.  Missing time object or callback");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Time_object_add_callback */

int Time_object_remove_callback(struct Time_object *time,
	Time_object_callback callback, void *user_data)
/*******************************************************************************
LAST MODIFIED : 29 September 1998

DESCRIPTION :
Removes a callback which was added previously
==============================================================================*/
{
	int return_code;
	struct Time_object_callback_data *callback_data, *previous;

	ENTER(Time_object_remove_callback);

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
					"Time_object_remove_callback.  Unable to find callback and user_data specified");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Time_object_remove_callback.  Missing time, callback or callback list");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Time_object_remove_callback */

int DESTROY(Time_object)(struct Time_object **time)
/*******************************************************************************
LAST MODIFIED : 29 September 1998

DESCRIPTION :
Destroys a Time_object object
==============================================================================*/
{
	int return_code;
	struct Time_object_callback_data *callback_data, *next;

	ENTER(DESTROY(Time_object));

	if (time && *time)
	{
		return_code=1;

		if((*time)->time_keeper)
		{
			Time_keeper_remove_time_object((*time)->time_keeper, *time);
			DEACCESS(Time_keeper)(&((*time)->time_keeper));
		}

		callback_data = (*time)->callback_list;
		while(callback_data)
		{
			next = callback_data->next;
			DEALLOCATE(callback_data);
			callback_data = next;
		}

		DEALLOCATE((*time)->name);
		DEALLOCATE(*time);
		*time = (struct Time_object *)NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Time_object).  Missing time object");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Time_object) */

