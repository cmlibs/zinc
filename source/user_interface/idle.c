/*******************************************************************************
FILE : idle.c

LAST MODIFIED : 21 March 2005

DESCRIPTION :
This provides code for handling idle events.
==============================================================================*/
#include "api/cmiss_idle.h"
#include "idle.h"
#include "general/debug.h"

Idle_package_id CREATE(Idle_package)(
	struct Event_dispatcher *event_dispatcher)
/*******************************************************************************
LAST MOFIFIED : 21 March 2005

DESCRIPTION :
This creates an Idle_package, given an event_dispatcher.
==============================================================================*/
{
	ENTER(CREATE(Idle_package));
	LEAVE;

	return (Idle_package_id)(event_dispatcher);
}

int DESTROY(Idle_package)(Idle_package_id *pkg)
/*******************************************************************************
LAST MOFIFIED : 21 March 2005

DESCRIPTION :
This destroys an Idle_package.
==============================================================================*/
{
	ENTER(DESTROY(Idle_package));
	*pkg = NULL;
	LEAVE;

	return (1);
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

	ENTER(Idle_package_add_callback);
	ret = Event_dispatcher_add_idle_callback((struct Event_dispatcher*)pkg,
		callback, user_data, EVENT_DISPATCHER_X_PRIORITY);
	LEAVE;

	return (Idle_callback_id)(ret);
}

int DESTROY(Idle_callback)(Idle_package_id pkg, Idle_callback_id *callback)
/*******************************************************************************
LAST MODIFIED : 21 March 2005

DESCRIPTION :
Destroys an idle callback, causing it to not be called any longer.
==============================================================================*/
{
	int ret;

	ENTER(DESTROY(Idle_callback));
	ret = Event_dispatcher_remove_idle_callback((struct Event_dispatcher*)pkg,
		(struct Event_dispatcher_idle_callback*)*callback);
	*callback = NULL;
	LEAVE;

	return (ret);
}
