/*******************************************************************************
FILE : timer.c

LAST MODIFIED : 11 April 2005

DESCRIPTION :
This provides code for handling timer events.
==============================================================================*/
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
