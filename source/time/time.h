/*******************************************************************************
FILE : time.h

LAST MODIFIED : 26 April 1999

DESCRIPTION :
This provides an object which supplies a concept of time to Cmgui
==============================================================================*/
#if !defined (TIME_H)
#define TIME_H

#include "general/object.h"
#include "time/time_keeper.h"

struct Time_object;

typedef int (*Time_object_callback)(struct Time_object *time,
	double current_time, void *user_data);
typedef double (*Time_object_next_time_function)(double time_after,
	enum Time_keeper_play_direction play_direction, void *user_data);

PROTOTYPE_OBJECT_FUNCTIONS(Time_object);
PROTOTYPE_GET_OBJECT_NAME_FUNCTION(Time_object);

struct Time_object *CREATE(Time_object)(char *name);
/*******************************************************************************
LAST MODIFIED : 29 September 1998

DESCRIPTION :
==============================================================================*/

double Time_object_get_current_time(struct Time_object *time);
/*******************************************************************************
LAST MODIFIED : 29 September 1998
DESCRIPTION :
==============================================================================*/

double Time_object_get_next_callback_time(struct Time_object *time,
	double time_after, enum Time_keeper_play_direction play_direction);
/*******************************************************************************
LAST MODIFIED : 9 December 1998
DESCRIPTION :
==============================================================================*/

int Time_object_set_current_time_privileged(struct Time_object *time,
	double new_time);
/*******************************************************************************
LAST MODIFIED : 29 September 1998
DESCRIPTION :
==============================================================================*/

int Time_object_set_update_frequency(struct Time_object *time, double frequency);
/*******************************************************************************
LAST MODIFIED : 6 October 1998
DESCRIPTION :
This controls the rate per second which the time depedent object is called back
when in play mode.
==============================================================================*/

int Time_object_set_next_time_function(struct Time_object *time,
	Time_object_next_time_function next_time_function, void *user_data);
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

int Time_object_set_time_keeper(struct Time_object *time, struct Time_keeper *time_keeper);
/*******************************************************************************
LAST MODIFIED : 29 September 1998
DESCRIPTION :
==============================================================================*/

int Time_object_add_callback(struct Time_object *time,
	Time_object_callback callback, void *user_data);
/*******************************************************************************
LAST MODIFIED : 29 September 1998

DESCRIPTION :
Adds a callback routine which is called whenever the current time is changed.
==============================================================================*/

int Time_object_remove_callback(struct Time_object *time,
	Time_object_callback callback, void *user_data);
/*******************************************************************************
LAST MODIFIED : 29 September 1998

DESCRIPTION :
Removes a callback which was added previously
==============================================================================*/

int DESTROY(Time_object)(struct Time_object **time);
/*******************************************************************************
LAST MODIFIED : 29 September 1998

DESCRIPTION :
Destroys a Time_object object
x==============================================================================*/
#endif /* !defined (TIME_H) */

