/*******************************************************************************
FILE : event_dispatcher.c

LAST MODIFIED : 24 October 2002

DESCRIPTION :
This provides an object which interfaces between a event_dispatcher and Cmgui
==============================================================================*/
#include <math.h>
#include <stdio.h>
#include <general/time.h>
#include <sys/times.h>

#include "general/compare.h"
#include "general/debug.h"
#include "general/list.h"
#include "general/indexed_list_private.h"
#include "general/object.h"
#include "user_interface/message.h"
#include "user_interface/event_dispatcher.h"

/* After the event_dispatcher.h has set up these variables */
#if defined (USE_XTAPP_CONTEXT) /* switch (USER_INTERFACE) */
#include <Xm/Xm.h>
#elif defined (WIN32_USER_INTERFACE) /* switch (USER_INTERFACE) */
#include <windows.h>
#elif defined (USE_GTK_MAIN_STEP) /* switch (USER_INTERFACE) */
#include <gtk/gtk.h>
#endif /* switch (USER_INTERFACE) */

/*
Module types
------------
*/

struct Event_dispatcher_simple_descriptor_data
/*******************************************************************************
LAST MODIFIED : 13 November 2002

DESCRIPTION :
Contains data for the simple wrapper around the descriptor callbacks that 
handles a single file_descriptor and single callback.
==============================================================================*/
{
	int file_descriptor;
	Event_dispatcher_simple_descriptor_callback_function *simple_callback;
	void *user_data;
}; /* struct Event_dispatcher_simple_descriptor_data */

struct Event_dispatcher_descriptor_callback
/*******************************************************************************
LAST MODIFIED : 13 November 2002

DESCRIPTION :
Contains all information necessary for a descriptor callback.
==============================================================================*/
{
	struct Event_dispatcher_descriptor_callback *self;	
	int access_count;
	int pending;
	void *user_data;
	/* When using the simple descriptor callbacks the user_data is an 
		internal ALLOCATED structure. This flag is set so that the DESTROY
		DEALLOCATES this user_data. */
	int deallocate_user_data_on_destroy;
#if defined (USE_GENERIC_EVENT_DISPATCHER)
	Event_dispatcher_descriptor_query_function *query_callback;
	Event_dispatcher_descriptor_check_function *check_callback;
	Event_dispatcher_descriptor_dispatch_function *dispatch_callback;
#endif /* defined (USE_GENERIC_EVENT_DISPATCHER) */
#if defined (USE_XTAPP_CONTEXT)
	XtInputId xt_input_id;
#endif /* defined (USE_XTAPP_CONTEXT) */
#if defined (USE_GTK_MAIN_STEP)
	guint gtk_input_id;
#endif /* defined (USE_GTK_MAIN_STEP) */
}; /* struct Event_dispatcher_descriptor_callback */

PROTOTYPE_OBJECT_FUNCTIONS(Event_dispatcher_descriptor_callback);
DECLARE_LIST_TYPES(Event_dispatcher_descriptor_callback);
FULL_DECLARE_INDEXED_LIST_TYPE(Event_dispatcher_descriptor_callback);

struct Event_dispatcher_timeout_callback
/*******************************************************************************
LAST MODIFIED : 6 March 2002

DESCRIPTION :
Contains all information necessary for a file descriptor callback.
==============================================================================*/
{
	struct Event_dispatcher_timeout_callback *self;	
	int access_count;
	unsigned long timeout_s;
	unsigned long timeout_ns;
	Event_dispatcher_timeout_function *timeout_function;
	void *user_data;
#if defined (USE_XTAPP_CONTEXT)
	XtIntervalId xt_timeout_id;
#endif /* defined (USE_XTAPP_CONTEXT) */
}; /* struct Event_dispatcher_timeout_callback */

PROTOTYPE_OBJECT_FUNCTIONS(Event_dispatcher_timeout_callback);
DECLARE_LIST_TYPES(Event_dispatcher_timeout_callback);
FULL_DECLARE_INDEXED_LIST_TYPE(Event_dispatcher_timeout_callback);

struct Event_dispatcher_idle_callback
/*******************************************************************************
LAST MODIFIED : 6 March 2002

DESCRIPTION :
Contains all information necessary for a file descriptor callback.
==============================================================================*/
{
	struct Event_dispatcher_idle_callback *self;	
	int access_count;
	long timestamp;
	enum Event_dispatcher_idle_priority priority;
	Event_dispatcher_idle_function *idle_function;
	void *user_data;
#if defined (USE_XTAPP_CONTEXT)
	XtWorkProcId xt_idle_id;
#endif /* defined (USE_XTAPP_CONTEXT) */
#if defined (USE_GTK_MAIN_STEP)
	guint gtk_idle_id;
#endif /* defined (USE_GTK_MAIN_STEP) */
}; /* struct Event_dispatcher_idle_callback */

PROTOTYPE_OBJECT_FUNCTIONS(Event_dispatcher_idle_callback);
DECLARE_LIST_TYPES(Event_dispatcher_idle_callback);
FULL_DECLARE_INDEXED_LIST_TYPE(Event_dispatcher_idle_callback);

struct Event_dispatcher
/*******************************************************************************
LAST MODIFIED : 4 June 2002

DESCRIPTION :
==============================================================================*/
{
	int continue_flag;
	struct LIST(Event_dispatcher_descriptor_callback) *descriptor_list;
	struct LIST(Event_dispatcher_timeout_callback) *timeout_list;
	struct LIST(Event_dispatcher_idle_callback) *idle_list;
#if defined (USE_XTAPP_CONTEXT)
/* This implements nearly the same interface as the normal implementation
	but uses the Xt Application Context instead, thereby allowing us to test
	the Xt behaviour easily if required */
	XtAppContext application_context;
#else /* defined (USE_XTAPP_CONTEXT) */
/* This is the default system */
	int special_idle_callback_pending;
	struct Event_dispatcher_idle_callback *special_idle_callback;
#endif /* defined (USE_XTAPP_CONTEXT) */
};

/*
Module functions
----------------
*/

static struct Event_dispatcher_descriptor_callback *CREATE(Event_dispatcher_descriptor_callback)(
#if defined (USE_GENERIC_EVENT_DISPATCHER)
	Event_dispatcher_descriptor_query_function *query_function,
	Event_dispatcher_descriptor_check_function *check_function,
	Event_dispatcher_descriptor_dispatch_function *dispatch_function,
#endif /* defined (USE_GENERIC_EVENT_DISPATCHER) */
	void *user_data)
/*******************************************************************************
LAST MODIFIED : 4 March 2002

DESCRIPTION :
Create a single object that belongs to a specific file descriptor.
==============================================================================*/
{
	struct Event_dispatcher_descriptor_callback *callback;

	ENTER(CREATE(Event_dispatcher_descriptor_callback));

	if (ALLOCATE(callback, struct Event_dispatcher_descriptor_callback, 1))
	{
		callback->self = callback;
#if defined (USE_GENERIC_EVENT_DISPATCHER)
		callback->query_callback = query_function;
		callback->check_callback = check_function;
		callback->dispatch_callback = dispatch_function;
#endif /* defined (USE_GENERIC_EVENT_DISPATCHER) */
		callback->user_data = user_data;
		callback->pending = 0;
		callback->deallocate_user_data_on_destroy = 0;
		callback->access_count = 0;
#if defined (USE_XTAPP_CONTEXT)
		callback->xt_input_id = (XtInputId)NULL;
#endif /* defined (USE_XTAPP_CONTEXT) */
#if defined (USE_GTK_MAIN_STEP)
		callback->gtk_input_id = 0;
#endif /* defined (USE_GTK_MAIN_STEP) */
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(Event_dispatcher_descriptor_callback).  "
			"Unable to allocate structure");
		callback = (struct Event_dispatcher_descriptor_callback *)NULL;
	}
	LEAVE;

	return (callback);
} /* CREATE(Event_dispatcher_descriptor_callback) */

static int DESTROY(Event_dispatcher_descriptor_callback)(
	struct Event_dispatcher_descriptor_callback **callback_address)
/*******************************************************************************
LAST MODIFIED : 4 March 2002

DESCRIPTION :
Destroys the object associated with the file descriptor.
==============================================================================*/
{
	int return_code;

	ENTER(DESTROY(Event_dispatcher_descriptor_callback));

	if (callback_address)
	{
		return_code=1;

		if ((*callback_address)->deallocate_user_data_on_destroy &&
			(*callback_address)->user_data)
		{
			DEALLOCATE((*callback_address)->user_data);
		}

		DEALLOCATE(*callback_address);
		*callback_address = (struct Event_dispatcher_descriptor_callback *)NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Event_dispatcher_descriptor_callback).  Missing event_dispatcher object");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Event_dispatcher_descriptor_callback) */

#if defined (USE_GENERIC_EVENT_DISPATCHER)
static int Event_dispatcher_simple_descriptor_query_callback(
	struct Event_dispatcher_descriptor_set *descriptor_set, void *user_data)
/*******************************************************************************
LAST MODIFIED : 13 November 2002

DESCRIPTION :
Adds the file descriptor in <user_data> to the <descriptor_set>.
==============================================================================*/
{
	int return_code;
	struct Event_dispatcher_simple_descriptor_data *data;

	ENTER(Event_dispatcher_simple_descriptor_query_callback);

	if (descriptor_set &&
		(data = (struct Event_dispatcher_simple_descriptor_data *)user_data))
	{
		FD_SET(data->file_descriptor, &(descriptor_set->read_set));
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Event_dispatcher_simple_descriptor_query_callback.  "
			"Invalid arguments.");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Event_dispatcher_simple_descriptor_query_callback */
#endif /* defined (USE_GENERIC_EVENT_DISPATCHER) */

#if defined (USE_GENERIC_EVENT_DISPATCHER)
static int Event_dispatcher_simple_descriptor_check_callback(
	struct Event_dispatcher_descriptor_set *descriptor_set,
	void *user_data)
/*******************************************************************************
LAST MODIFIED : 13 November 2002

DESCRIPTION :
Checks if the file descriptor in <user_data> is set in the <descriptor_set>.
Returns true if it is and false if not.
==============================================================================*/
{
	int return_code;
	struct Event_dispatcher_simple_descriptor_data *data;

	ENTER(Event_dispatcher_simple_descriptor_check_callback);

	if (descriptor_set &&
		(data = (struct Event_dispatcher_simple_descriptor_data *)user_data))
	{
		return_code = FD_ISSET(data->file_descriptor, &(descriptor_set->read_set));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Event_dispatcher_simple_descriptor_check_callback.  "
			"Invalid arguments.");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Event_dispatcher_simple_descriptor_check_callback */
#endif /* defined (USE_GENERIC_EVENT_DISPATCHER) */

static int Event_dispatcher_simple_descriptor_dispatch_callback(void *user_data)
/*******************************************************************************
LAST MODIFIED : 13 November 2002

DESCRIPTION :
Calls the callback in <user_data>.
==============================================================================*/
{
	int return_code;
	struct Event_dispatcher_simple_descriptor_data *data;

	ENTER(Event_dispatcher_simple_descriptor_check_callback);

	if (data = (struct Event_dispatcher_simple_descriptor_data *)user_data)
	{
		return_code = data->simple_callback(data->file_descriptor, data->user_data);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Event_dispatcher_simple_descriptor_dispatch_callback.  "
			"Invalid arguments.");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Event_dispatcher_simple_descriptor_dispatch_callback */

#if defined (USE_GENERIC_EVENT_DISPATCHER)
static int Event_dispatcher_descriptor_do_query_callback(
	struct Event_dispatcher_descriptor_callback *callback,
	void *descriptor_set_void)
/*******************************************************************************
LAST MODIFIED : 13 November 2002

DESCRIPTION :
Calls the query callback for the <callback>.
==============================================================================*/
{
	int return_code;
	struct Event_dispatcher_descriptor_set *descriptor_set;

	ENTER(Event_dispatcher_descriptor_do_query_callback);

	if (callback && callback->query_callback && (descriptor_set = 
		(struct Event_dispatcher_descriptor_set *)descriptor_set_void))
	{
		return_code = callback->query_callback(descriptor_set, callback->user_data);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Event_dispatcher_do_query_callback.  "
			"Invalid arguments.");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Event_dispatcher_do_query_callback */
#endif /* defined (USE_GENERIC_EVENT_DISPATCHER) */

#if defined (USE_GENERIC_EVENT_DISPATCHER)
static int Event_dispatcher_descriptor_do_check_callback(
	struct Event_dispatcher_descriptor_callback *callback,
	void *descriptor_set_void)
/*******************************************************************************
LAST MODIFIED : 13 November 2002

DESCRIPTION :
Calls the check callback for the <callback>, if the check_callback function
returns true then the pending flag is set in the <callback>.
==============================================================================*/
{
	int return_code;
	struct Event_dispatcher_descriptor_set *descriptor_set;

	ENTER(Event_dispatcher_descriptor_do_check_callback);

	if (callback && callback->check_callback && (descriptor_set = 
		(struct Event_dispatcher_descriptor_set *)descriptor_set_void))
	{
		callback->pending = callback->check_callback(descriptor_set,
			callback->user_data);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Event_dispatcher_do_check_callback.  "
			"Invalid arguments.");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Event_dispatcher_do_check_callback */
#endif /* defined (USE_GENERIC_EVENT_DISPATCHER) */

#if defined (USE_GENERIC_EVENT_DISPATCHER)
static int Event_dispatcher_descriptor_callback_is_pending(
	struct Event_dispatcher_descriptor_callback *callback, void *user_data)
/*******************************************************************************
LAST MODIFIED : 13 November 2002

DESCRIPTION :
An iterator function that finds a pending callback.
==============================================================================*/
{
	int return_code;

	ENTER(Event_dispatcher_descriptor_callback_is_pending);
	USE_PARAMETER(user_data);
	if (callback)
	{	
		return_code=callback->pending;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Event_dispatcher_descriptor_callback_is_pending.  "
			"Invalid arguments.");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Event_dispatcher_descriptor_callback_is_pending */
#endif /* defined (USE_GENERIC_EVENT_DISPATCHER) */

DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(Event_dispatcher_descriptor_callback, \
	self,struct Event_dispatcher_descriptor_callback *,compare_pointer)
DECLARE_OBJECT_FUNCTIONS(Event_dispatcher_descriptor_callback)
DECLARE_INDEXED_LIST_FUNCTIONS(Event_dispatcher_descriptor_callback)
DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(Event_dispatcher_descriptor_callback, \
	self,struct Event_dispatcher_descriptor_callback *,compare_pointer)

static struct Event_dispatcher_timeout_callback *CREATE(Event_dispatcher_timeout_callback)(
	unsigned long timeout_s, unsigned long timeout_ns, 
	Event_dispatcher_timeout_function timeout_function, void *user_data)
/*******************************************************************************
LAST MODIFIED : 4 March 2002

DESCRIPTION :
Create a single object that belongs to a specific file descriptor.
==============================================================================*/
{
	struct Event_dispatcher_timeout_callback *timeout_callback;

	ENTER(CREATE(Event_dispatcher_timeout_callback));

	if (ALLOCATE(timeout_callback, struct Event_dispatcher_timeout_callback, 1))
	{
		/* This pointer is used as the identifier so the indexed list is sorted
			by the pointers */
		timeout_callback->self = timeout_callback;
		timeout_callback->timeout_s = timeout_s;
		timeout_callback->timeout_ns = timeout_ns;
		timeout_callback->timeout_function = timeout_function;
		timeout_callback->user_data = user_data;
		timeout_callback->access_count = 0;
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(Event_dispatcher_timeout_callback).  "
			"Unable to allocate structure");
		timeout_callback = (struct Event_dispatcher_timeout_callback *)NULL;
	}
	LEAVE;

	return (timeout_callback);
} /* CREATE(Event_dispatcher_timeout_callback) */

static int DESTROY(Event_dispatcher_timeout_callback)(
	struct Event_dispatcher_timeout_callback **timeout_callback_address)
/*******************************************************************************
LAST MODIFIED : 4 March 2002

DESCRIPTION :
Destroys the object associated with the file descriptor.
==============================================================================*/
{
	int return_code;

	ENTER(DESTROY(Event_dispatcher_timeout_callback));

	if (timeout_callback_address)
	{
		return_code=1;

		DEALLOCATE(*timeout_callback_address);
		*timeout_callback_address = (struct Event_dispatcher_timeout_callback *)NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Event_dispatcher_timeout_callback).  Missing event_dispatcher object");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Event_dispatcher_timeout_callback) */

static int Event_dispatcher_timeout_callback_compare(
	struct Event_dispatcher_timeout_callback *timeout_one, 
	struct Event_dispatcher_timeout_callback *timeout_two)
/*******************************************************************************
LAST MODIFIED : 7 March 2002

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Event_dispatcher_timeout_callback_compare);

	if (timeout_one && timeout_two)
	{
		if (timeout_one->timeout_s < timeout_two->timeout_s)
		{
			return_code = -1;
		}
		else if (timeout_one->timeout_s > timeout_two->timeout_s)
		{
			return_code = 1;
		}
		else
		{
			if (timeout_one->timeout_ns < timeout_two->timeout_ns)
			{
				return_code = -1;
			}
			else if (timeout_one->timeout_ns > timeout_two->timeout_ns)
			{
				return_code = 1;
			}
			else
			{
				if (timeout_one < timeout_two)
				{
					return_code = -1;
				}
				else if (timeout_one >  timeout_two)
				{
					return_code = 1;
				}
				else
				{
					return_code = 0;
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Event_dispatcher_timeout_callback_compare.  "
			"Invalid arguments.");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Event_dispatcher_timeout_callback_compare */

DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(Event_dispatcher_timeout_callback, \
	self,struct Event_dispatcher_timeout_callback *,Event_dispatcher_timeout_callback_compare)
DECLARE_OBJECT_FUNCTIONS(Event_dispatcher_timeout_callback)
DECLARE_INDEXED_LIST_FUNCTIONS(Event_dispatcher_timeout_callback)
DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(Event_dispatcher_timeout_callback, \
	self,struct Event_dispatcher_timeout_callback *,Event_dispatcher_timeout_callback_compare)

static struct Event_dispatcher_idle_callback *CREATE(Event_dispatcher_idle_callback)(
	Event_dispatcher_idle_function idle_function, void *user_data, 
	enum Event_dispatcher_idle_priority priority)
/*******************************************************************************
LAST MODIFIED : 4 March 2002

DESCRIPTION :
Create a single object that belongs to a specific file descriptor.
==============================================================================*/
{
	struct Event_dispatcher_idle_callback *idle_callback;
	struct tms times_buffer;

	ENTER(CREATE(Event_dispatcher_idle_callback));

	if (ALLOCATE(idle_callback, struct Event_dispatcher_idle_callback, 1))
	{
		idle_callback->self = idle_callback;
		idle_callback->priority = priority;
		idle_callback->timestamp = (long)times(&times_buffer);
		idle_callback->idle_function = idle_function;
		idle_callback->user_data = user_data;
		idle_callback->access_count = 0;

#if defined (USE_XTAPP_CONTEXT)
		idle_callback->xt_idle_id = (XtWorkProcId)NULL;
#endif /* defined (USE_XTAPP_CONTEXT) */
#if defined (USE_GTK_MAIN_STEP)
		idle_callback->gtk_idle_id = 0;
#endif /* defined (USE_GTK_MAIN_STEP) */
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(Event_dispatcher_idle_callback).  "
			"Unable to allocate structure");
		idle_callback = (struct Event_dispatcher_idle_callback *)NULL;
	}
	LEAVE;

	return (idle_callback);
} /* CREATE(Event_dispatcher_idle_callback) */

static int DESTROY(Event_dispatcher_idle_callback)(
	struct Event_dispatcher_idle_callback **idle_callback_address)
/*******************************************************************************
LAST MODIFIED : 4 March 2002

DESCRIPTION :
Destroys the object associated with the file descriptor.
==============================================================================*/
{
	int return_code;

	ENTER(DESTROY(Event_dispatcher_idle_callback));

	if (idle_callback_address)
	{
		return_code=1;

		DEALLOCATE(*idle_callback_address);
		*idle_callback_address = (struct Event_dispatcher_idle_callback *)NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Event_dispatcher_idle_callback).  Missing event_dispatcher object");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Event_dispatcher_idle_callback) */

static int Event_dispatcher_idle_callback_compare(
	struct Event_dispatcher_idle_callback *idle_one, 
	struct Event_dispatcher_idle_callback *idle_two)
/*******************************************************************************
LAST MODIFIED : 7 March 2002

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Event_dispatcher_idle_callback_compare);

	if (idle_one && idle_two)
	{
		if (idle_one->priority < idle_two->priority)
		{
			return_code = -1;
		}
		else if (idle_one->priority > idle_two->priority)
		{
			return_code = 1;
		}
		else
		{
			if (idle_one->timestamp < idle_two->timestamp)
			{
				return_code = -1;
			}
			if (idle_one->timestamp > idle_two->timestamp)
			{
				return_code = 1;
			}
			else
			{
				if (idle_one < idle_two)
				{
					return_code = -1;
				}
				else if (idle_one > idle_two)
				{
					return_code = 1;
				}
				else
				{
					return_code = 0;
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Event_dispatcher_idle_callback_compare.  "
			"Invalid arguments.");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Event_dispatcher_idle_callback_compare */

DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(Event_dispatcher_idle_callback, \
	self,struct Event_dispatcher_idle_callback *,Event_dispatcher_idle_callback_compare)
DECLARE_OBJECT_FUNCTIONS(Event_dispatcher_idle_callback)
DECLARE_INDEXED_LIST_FUNCTIONS(Event_dispatcher_idle_callback)
DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(Event_dispatcher_idle_callback, \
	self,struct Event_dispatcher_idle_callback *,Event_dispatcher_idle_callback_compare)

#if defined (USE_XTAPP_CONTEXT)
void Event_dispatcher_xt_input_callback(
	XtPointer callback_void, int *source, XtInputId *id)
/*******************************************************************************
LAST MODIFIED : 4 June 2002

DESCRIPTION :
==============================================================================*/
{
	struct Event_dispatcher_descriptor_callback *callback;

	ENTER(Event_dispatcher_xt_input_callback);
	USE_PARAMETER(source);
	USE_PARAMETER(id);
	if (callback = (struct Event_dispatcher_descriptor_callback *)callback_void)
	{
		Event_dispatcher_simple_descriptor_dispatch_callback(callback->user_data);		
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Event_dispatcher_xt_input_callback.  Invalid arguments.");
	}
	LEAVE;
} /* Event_dispatcher_xt_input_callback */
#endif /* defined (USE_XTAPP_CONTEXT) */

#if defined (USE_XTAPP_CONTEXT)
void Event_dispatcher_xt_timeout_callback(
	XtPointer timeout_callback_void, XtIntervalId *id)
/*******************************************************************************
LAST MODIFIED : 4 June 2002

DESCRIPTION :
==============================================================================*/
{
	struct Event_dispatcher_timeout_callback *timeout_callback;

	ENTER(Event_dispatcher_xt_timeout_callback);
	USE_PARAMETER(id);
	if (timeout_callback = (struct Event_dispatcher_timeout_callback *)timeout_callback_void)
	{
		(*timeout_callback->timeout_function)(timeout_callback->user_data);		
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Event_dispatcher_xt_timeout_callback.  Invalid arguments.");
	}
	LEAVE;
} /* Event_dispatcher_xt_timeout_callback */
#endif /* defined (USE_XTAPP_CONTEXT) */

#if defined (USE_XTAPP_CONTEXT)
Boolean Event_dispatcher_xt_idle_callback(
	XtPointer idle_callback_void)
/*******************************************************************************
LAST MODIFIED : 4 June 2002

DESCRIPTION :
==============================================================================*/
{
	Boolean return_code;
	struct Event_dispatcher_idle_callback *idle_callback;

	ENTER(Event_dispatcher_xt_idle_callback);
	if (idle_callback = (struct Event_dispatcher_idle_callback *)idle_callback_void)
	{
		if ((*idle_callback->idle_function)(idle_callback->user_data))
		{
			return_code = False;
		}
		else
		{
			return_code = True;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Event_dispatcher_xt_idle_callback.  Invalid arguments.");
	}
	LEAVE;

	return (return_code);
} /* Event_dispatcher_xt_idle_callback */
#endif /* defined (USE_XTAPP_CONTEXT) */

#if defined (USE_GTK_MAIN_STEP)
void Event_dispatcher_gtk_input_callback(
	gpointer callback_void, gint source, GdkInputCondition condition)
/*******************************************************************************
LAST MODIFIED : 11 July 2002

DESCRIPTION :
==============================================================================*/
{
	struct Event_dispatcher_descriptor_callback *callback;

	ENTER(Event_dispatcher_gtk_input_callback);
	USE_PARAMETER(source);
	USE_PARAMETER(condition);
	if (callback = (struct Event_dispatcher_descriptor_callback *)callback_void)
	{
		Event_dispatcher_simple_descriptor_dispatch_callback(callback->user_data);		
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Event_dispatcher_xt_input_callback.  Invalid arguments.");
	}
	LEAVE;

} /* Event_dispatcher_gtk_input_callback */
#endif /* defined (USE_GTK_MAIN_STEP) */

#if defined (USE_GTK_MAIN_STEP)
gboolean Event_dispatcher_gtk_idle_callback(
	gpointer idle_callback_void)
/*******************************************************************************
LAST MODIFIED : 10 July 2002

DESCRIPTION :
==============================================================================*/
{
	gboolean return_code;
	struct Event_dispatcher_idle_callback *idle_callback;

	ENTER(Event_dispatcher_gtk_idle_callback);
	if (idle_callback = (struct Event_dispatcher_idle_callback *)idle_callback_void)
	{
		if ((*idle_callback->idle_function)(idle_callback->user_data))
		{
			return_code = FALSE;
		}
		else
		{
			gtk_idle_remove(idle_callback->gtk_idle_id);
			idle_callback->gtk_idle_id = 0;
			return_code = TRUE;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Event_dispatcher_gtk_idle_callback.  Invalid arguments.");
	}
	LEAVE;

	return (return_code);
} /* Event_dispatcher_gtk_idle_callback */
#endif /* defined (USE_GTK_MAIN_STEP) */

/*
Global functions
----------------
*/

struct Event_dispatcher *CREATE(Event_dispatcher)(void)
/*******************************************************************************
LAST MODIFIED : 4 March 2002

DESCRIPTION :
Creates a connection to a event_dispatcher of the specified type.
==============================================================================*/
{
	struct Event_dispatcher *event_dispatcher;

	ENTER(CREATE(Event_dispatcher));

	if (ALLOCATE(event_dispatcher, struct Event_dispatcher, 1))
	{
		event_dispatcher->descriptor_list = 
			CREATE(LIST(Event_dispatcher_descriptor_callback))();
		event_dispatcher->timeout_list = 
			CREATE(LIST(Event_dispatcher_timeout_callback))();
		event_dispatcher->idle_list = 
			CREATE(LIST(Event_dispatcher_idle_callback))();
#if defined (USE_XTAPP_CONTEXT)
		event_dispatcher->application_context = (XtAppContext)NULL;
#else /* defined (USE_XTAPP_CONTEXT) */
		event_dispatcher->special_idle_callback_pending = 0;
		event_dispatcher->special_idle_callback = 
			(struct Event_dispatcher_idle_callback *)NULL;
#endif /* defined (USE_XTAPP_CONTEXT) */
		event_dispatcher->continue_flag = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(Event_dispatcher). Unable to allocate structure");
		event_dispatcher = (struct Event_dispatcher *)NULL;
	}
	LEAVE;

	return (event_dispatcher);
} /* CREATE(Event_dispatcher) */

int DESTROY(Event_dispatcher)(struct Event_dispatcher **event_dispatcher_address)
/*******************************************************************************
LAST MODIFIED : 4 March 2002

DESCRIPTION :
Destroys a Event_dispatcher object
==============================================================================*/
{
	int return_code;
	struct Event_dispatcher *event_dispatcher;

	ENTER(DESTROY(Event_dispatcher));

	if (event_dispatcher_address && (event_dispatcher = *event_dispatcher_address))
	{
		return_code=1;
		if (event_dispatcher->descriptor_list)
		{
			DESTROY(LIST(Event_dispatcher_descriptor_callback))
				(&event_dispatcher->descriptor_list);
		}
		if (event_dispatcher->timeout_list)
		{
			DESTROY(LIST(Event_dispatcher_timeout_callback))
				(&event_dispatcher->timeout_list);
		}
		if (event_dispatcher->idle_list)
		{
			DESTROY(LIST(Event_dispatcher_idle_callback))
				(&event_dispatcher->idle_list);
		}
#if ! defined (USE_XTAPP_CONTEXT)
		if (event_dispatcher->special_idle_callback)
		{
			DEACCESS(Event_dispatcher_idle_callback)(
				&event_dispatcher->special_idle_callback);
		}
#endif /* ! defined (USE_XTAPP_CONTEXT) */

		DEALLOCATE(*event_dispatcher_address);
		*event_dispatcher_address = (struct Event_dispatcher *)NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Event_dispatcher).  Missing event_dispatcher object");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Event_dispatcher) */

#if defined (USE_GENERIC_EVENT_DISPATCHER)
struct Event_dispatcher_descriptor_callback *Event_dispatcher_add_descriptor_callback(
	struct Event_dispatcher *event_dispatcher,
	Event_dispatcher_descriptor_query_function *query_function,
	Event_dispatcher_descriptor_check_function *check_function,
	Event_dispatcher_descriptor_dispatch_function *dispatch_function,
	void *user_data)
/*******************************************************************************
LAST MODIFIED : 13 November 2002

DESCRIPTION :
==============================================================================*/
{
	struct Event_dispatcher_descriptor_callback *callback;

	ENTER(Event_dispatcher_add_descriptor_callback);

	if (event_dispatcher && query_function && check_function && dispatch_function)
	{
		if (callback = CREATE(Event_dispatcher_descriptor_callback)(
			query_function, check_function, dispatch_function, user_data))
		{
			if (!(ADD_OBJECT_TO_LIST(Event_dispatcher_descriptor_callback)(
						callback, event_dispatcher->descriptor_list)))
			{
				DESTROY(Event_dispatcher_descriptor_callback)(&callback);
				callback = (struct Event_dispatcher_descriptor_callback *)NULL;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Event_dispatcher_add_descriptor_callback.  "
				"Could not create callback object.");
			callback = (struct Event_dispatcher_descriptor_callback *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Event_dispatcher_add_descriptor_callback.  Invalid arguments.");
		callback = (struct Event_dispatcher_descriptor_callback *)NULL;
	}
	LEAVE;

	return (callback);
} /* Event_dispatcher_add_descriptor_callback */
#endif /* defined (USE_GENERIC_EVENT_DISPATCHER) */

struct Event_dispatcher_descriptor_callback *Event_dispatcher_add_simple_descriptor_callback(
	struct Event_dispatcher *event_dispatcher, int file_descriptor,
	Event_dispatcher_simple_descriptor_callback_function *callback_function,
	void *user_data)
/*******************************************************************************
LAST MODIFIED : 13 November 2002

DESCRIPTION :
==============================================================================*/
{
	struct Event_dispatcher_simple_descriptor_data *data;
	struct Event_dispatcher_descriptor_callback *callback;

	ENTER(Event_dispatcher_add_simple_descriptor_callback);

	if (event_dispatcher && callback_function)
	{
		if (ALLOCATE(data, struct Event_dispatcher_simple_descriptor_data, 1))
		{
			data->file_descriptor = file_descriptor;
			data->simple_callback = callback_function;
			data->user_data = user_data;
#if defined (USE_GENERIC_EVENT_DISPATCHER)
			callback = Event_dispatcher_add_descriptor_callback(event_dispatcher,
				Event_dispatcher_simple_descriptor_query_callback,
				Event_dispatcher_simple_descriptor_check_callback,
				Event_dispatcher_simple_descriptor_dispatch_callback,
				data);
#else /* defined (USE_GENERIC_EVENT_DISPATCHER) */
			if (callback = CREATE(Event_dispatcher_descriptor_callback)(user_data))
			{
				if (!(ADD_OBJECT_TO_LIST(Event_dispatcher_descriptor_callback)(
							callback, event_dispatcher->descriptor_list)))
				{
					DESTROY(Event_dispatcher_descriptor_callback)(&callback);
					callback = (struct Event_dispatcher_descriptor_callback *)NULL;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Event_dispatcher_add_simple_descriptor_callback.  "
					"Could not create callback object.");
				callback = (struct Event_dispatcher_descriptor_callback *)NULL;
			}
#endif /* defined (USE_GENERIC_EVENT_DISPATCHER) */
			if (callback)
			{
				callback->deallocate_user_data_on_destroy = 1;
			}
#if defined (USE_XTAPP_CONTEXT) /* switch (USER_INTERFACE) */
			if (callback)
			{
				if (event_dispatcher->application_context)
				{
					callback->xt_input_id = XtAppAddInput(event_dispatcher->application_context, 
						file_descriptor, (XtPointer)XtInputReadMask, Event_dispatcher_xt_input_callback,
						callback);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Event_dispatcher_add_simple_descriptor_callback.  "
						"Missing application context.");
					callback = (struct Event_dispatcher_descriptor_callback *)NULL;
				}
			}
#elif defined (USE_GTK_MAIN_STEP) /* switch (USER_INTERFACE) */
			if (callback)
			{
				callback->gtk_input_id = gdk_input_add(file_descriptor,
					GDK_INPUT_READ, Event_dispatcher_gtk_input_callback,
					(void *)callback);
			}
#endif /* defined (USER_INTERFACE) */
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Event_dispatcher_add_simple_descriptor_callback.  "
				"Unable to allocate memory for data structure.");
			callback = (struct Event_dispatcher_descriptor_callback *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Event_dispatcher_add_simple_descriptor_callback.  Invalid arguments.");
		callback = (struct Event_dispatcher_descriptor_callback *)NULL;
	}
	LEAVE;

	return (callback);
} /* Event_dispatcher_add_simple_descriptor_callback */

int Event_dispatcher_remove_descriptor_callback(
	struct Event_dispatcher *event_dispatcher, 
	struct Event_dispatcher_descriptor_callback *callback_id)
/*******************************************************************************
LAST MODIFIED : 6 March 2002

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Event_dispatcher_remove_descriptor_callback);
	if (event_dispatcher && event_dispatcher->descriptor_list && callback_id)
	{
#if defined (USE_XTAPP_CONTEXT)
		XtRemoveInput(callback_id->xt_input_id);
#endif /* defined (USE_XTAPP_CONTEXT) */
		return_code = REMOVE_OBJECT_FROM_LIST(Event_dispatcher_descriptor_callback)
			(callback_id, event_dispatcher->descriptor_list);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Event_dispatcher_remove_descriptor_callback.  Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Event_dispatcher_remove_descriptor_callback */

#if defined (USE_XTAPP_CONTEXT)
struct Event_dispatcher_timeout_callback *Event_dispatcher_add_timeout_callback(
	struct Event_dispatcher *event_dispatcher, unsigned long timeout_s, unsigned long timeout_ns,
	Event_dispatcher_timeout_function *timeout_function, void *user_data)
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
==============================================================================*/
{
	long interval_ms;
	struct Event_dispatcher_timeout_callback *timeout_callback;

	ENTER(Event_dispatcher_register_descriptor_callback);

	if (event_dispatcher && timeout_function)
	{
		if (timeout_callback = CREATE(Event_dispatcher_timeout_callback)(
			timeout_s, timeout_ns, timeout_function, user_data))
		{
			if (!(ADD_OBJECT_TO_LIST(Event_dispatcher_timeout_callback)(
				timeout_callback, event_dispatcher->timeout_list)))
			{
				DESTROY(Event_dispatcher_timeout_callback)(&timeout_callback);
				timeout_callback = (struct Event_dispatcher_timeout_callback *)NULL;
			}
			else
			{
				if (event_dispatcher->application_context)
				{
					interval_ms = 
						timeout_ns / 1000000 + timeout_s * 1000;
					timeout_callback->xt_timeout_id = XtAppAddTimeOut(
						event_dispatcher->application_context,  interval_ms,
						Event_dispatcher_xt_timeout_callback, timeout_callback);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Event_dispatcher_add_timeout_callback.  "
						"Missing application context.");
					timeout_callback = (struct Event_dispatcher_timeout_callback *)NULL;
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Event_dispatcher_add_timeout_callback.  "
				"Could not create timeout_callback object.");
			timeout_callback = (struct Event_dispatcher_timeout_callback *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Event_dispatcher_add_timeout_callback.  Invalid arguments.");
		timeout_callback = (struct Event_dispatcher_timeout_callback *)NULL;
	}
	LEAVE;

	return (timeout_callback);
} /* Event_dispatcher_add_timeout_callback */

struct Event_dispatcher_timeout_callback *Event_dispatcher_add_timeout_callback_at_time(
	struct Event_dispatcher *event_dispatcher, unsigned long timeout_s, unsigned long timeout_ns,
	Event_dispatcher_timeout_function *timeout_function, void *user_data)
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
==============================================================================*/
{
	struct timeval timeofday;
	struct timezone timeofdayzone;	
	struct Event_dispatcher_timeout_callback *timeout_callback;

	ENTER(Event_dispatcher_add_timeout_callback_at_time);

	if (event_dispatcher && timeout_function)
	{
		gettimeofday(&timeofday, &timeofdayzone);
		while (timeout_s && (timeout_ns > 1000*(unsigned long)timeofday.tv_usec))
		{
			timeout_s--;
			timeout_ns += 1000000000 - 1000*timeofday.tv_usec;
		}
		timeout_callback = Event_dispatcher_add_timeout_callback_at_time(
			event_dispatcher, timeout_s - (unsigned long)timeofday.tv_sec, 
			timeout_ns - 1000*(unsigned long)timeofday.tv_usec, 
			timeout_function, user_data);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Event_dispatcher_add_timeout_callback_at_time.  Invalid arguments.");
		timeout_callback = (struct Event_dispatcher_timeout_callback *)NULL;
	}
	LEAVE;

	return (timeout_callback);
} /* Event_dispatcher_add_timeout_callback_at_time */
#else /* defined (USE_XTAPP_CONTEXT) */
struct Event_dispatcher_timeout_callback *Event_dispatcher_add_timeout_callback_at_time(
	struct Event_dispatcher *event_dispatcher, unsigned long timeout_s, unsigned long timeout_ns,
	Event_dispatcher_timeout_function *timeout_function, void *user_data)
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
==============================================================================*/
{
	struct Event_dispatcher_timeout_callback *timeout_callback;

	ENTER(Event_dispatcher_register_descriptor_callback);

	if (event_dispatcher && timeout_function)
	{
		if (timeout_callback = CREATE(Event_dispatcher_timeout_callback)(
			timeout_s, timeout_ns, timeout_function, user_data))
		{
			if (!(ADD_OBJECT_TO_LIST(Event_dispatcher_timeout_callback)(
				timeout_callback, event_dispatcher->timeout_list)))
			{
				DESTROY(Event_dispatcher_timeout_callback)(&timeout_callback);
				timeout_callback = (struct Event_dispatcher_timeout_callback *)NULL;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Event_dispatcher_add_timeout_callback_at_time.  "
				"Could not create timeout_callback object.");
			timeout_callback = (struct Event_dispatcher_timeout_callback *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Event_dispatcher_add_timeout_callback_at_time.  Invalid arguments.");
		timeout_callback = (struct Event_dispatcher_timeout_callback *)NULL;
	}
	LEAVE;

	return (timeout_callback);
} /* Event_dispatcher_add_timeout_callback_at_time */

struct Event_dispatcher_timeout_callback *Event_dispatcher_add_timeout_callback(
	struct Event_dispatcher *event_dispatcher, unsigned long timeout_s, unsigned long timeout_ns,
	Event_dispatcher_timeout_function *timeout_function, void *user_data)
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
==============================================================================*/
{
	struct Event_dispatcher_timeout_callback *timeout_callback;
	struct timeval timeofday;

	ENTER(Event_dispatcher_register_descriptor_callback);

	if (event_dispatcher && timeout_function)
	{
		gettimeofday(&timeofday, NULL);
		timeout_callback = Event_dispatcher_add_timeout_callback_at_time(
			event_dispatcher, timeout_s + (unsigned long)timeofday.tv_sec, 
			timeout_ns + 1000*(unsigned long)timeofday.tv_usec, 
			timeout_function, user_data);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Event_dispatcher_add_timeout_callback.  Invalid arguments.");
		timeout_callback = (struct Event_dispatcher_timeout_callback *)NULL;
	}
	LEAVE;

	return (timeout_callback);
} /* Event_dispatcher_add_timeout_callback */
#endif /* defined (USE_XTAPP_CONTEXT) */

int Event_dispatcher_remove_timeout_callback(
	struct Event_dispatcher *event_dispatcher, 
	struct Event_dispatcher_timeout_callback *callback_id)
/*******************************************************************************
LAST MODIFIED : 6 March 2002

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Event_dispatcher_remove_timeout_callback);

	if (event_dispatcher && event_dispatcher->timeout_list && callback_id)
	{
#if defined (USE_XTAPP_CONTEXT)
		XtRemoveTimeOut(callback_id->xt_timeout_id);
#endif /* defined (USE_XTAPP_CONTEXT) */
		return_code = REMOVE_OBJECT_FROM_LIST(Event_dispatcher_timeout_callback)
			(callback_id, event_dispatcher->timeout_list);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Event_dispatcher_remove_timeout_callback.  Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Event_dispatcher_remove_timeout_callback */

struct Event_dispatcher_idle_callback *Event_dispatcher_add_idle_callback(
	struct Event_dispatcher *event_dispatcher, 
	Event_dispatcher_idle_function *idle_function, void *user_data,
	enum Event_dispatcher_idle_priority priority)
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
==============================================================================*/
{
	struct Event_dispatcher_idle_callback *idle_callback;

	ENTER(Event_dispatcher_add_idle_event_callback);

	if (event_dispatcher && idle_function)
	{
		if (idle_callback = CREATE(Event_dispatcher_idle_callback)(
			idle_function, user_data, priority))
		{
			if (!(ADD_OBJECT_TO_LIST(Event_dispatcher_idle_callback)(
				idle_callback, event_dispatcher->idle_list)))
			{
				DESTROY(Event_dispatcher_idle_callback)(&idle_callback);
				idle_callback = (struct Event_dispatcher_idle_callback *)NULL;
			}
#if defined (USE_XTAPP_CONTEXT)
			else
			{
				if (event_dispatcher->application_context)
				{
					idle_callback->xt_idle_id = XtAppAddWorkProc(event_dispatcher->application_context, 
						Event_dispatcher_xt_idle_callback, idle_callback);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Event_dispatcher_add_idle_callback.  "
						"Missing application context.");
					idle_callback = (struct Event_dispatcher_idle_callback *)NULL;
				}
			}
#elif defined (USE_GTK_MAIN_STEP)
			else
			{
				idle_callback->gtk_idle_id = gtk_idle_add(
					Event_dispatcher_gtk_idle_callback, idle_callback);
			}
#endif /* defined (USE_XTAPP_CONTEXT) */
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Event_dispatcher_add_idle_event_callback.  "
				"Could not create idle_callback object.");
			idle_callback = (struct Event_dispatcher_idle_callback *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Event_dispatcher_add_idle_event_callback.  Invalid arguments.");
		idle_callback = (struct Event_dispatcher_idle_callback *)NULL;
	}
	LEAVE;

	return (idle_callback);
} /* Event_dispatcher_add_idle_event_callback */

struct Event_dispatcher_idle_callback *Event_dispatcher_set_special_idle_callback(
	struct Event_dispatcher *event_dispatcher, 
	Event_dispatcher_idle_function *idle_function, void *user_data,
	enum Event_dispatcher_idle_priority priority)
/*******************************************************************************
LAST MODIFIED : 12 March 2002

DESCRIPTION :
==============================================================================*/
{
	struct Event_dispatcher_idle_callback *idle_callback;

	ENTER(Event_dispatcher_set_special_idle_callback);

#if defined (USE_XTAPP_CONTEXT)
	USE_PARAMETER(user_data);
	USE_PARAMETER(priority);
#endif /* defined (USE_XTAPP_CONTEXT) */
	if (event_dispatcher && idle_function)
	{
#if defined (USE_XTAPP_CONTEXT)
		idle_callback = (struct Event_dispatcher_idle_callback *)NULL;
#else /* defined (USE_XTAPP_CONTEXT) */
		if (idle_callback = CREATE(Event_dispatcher_idle_callback)(
			idle_function, user_data, priority))
		{
			REACCESS(Event_dispatcher_idle_callback)(
				&event_dispatcher->special_idle_callback, idle_callback);
			event_dispatcher->special_idle_callback_pending = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Event_dispatcher_set_special_idle_callback.  "
				"Could not create idle_callback object.");
			idle_callback = (struct Event_dispatcher_idle_callback *)NULL;
		}
#endif /* defined (USE_XTAPP_CONTEXT) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Event_dispatcher_set_special_idle_callback.  Invalid arguments.");
		idle_callback = (struct Event_dispatcher_idle_callback *)NULL;
	}
	LEAVE;

	return (idle_callback);
} /* Event_dispatcher_set_special_idle_callback */

int Event_dispatcher_remove_idle_callback(
	struct Event_dispatcher *event_dispatcher, 
	struct Event_dispatcher_idle_callback *callback_id)
/*******************************************************************************
LAST MODIFIED : 4 March 2002

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Event_dispatcher_remove_idle_callback);

	if (event_dispatcher && event_dispatcher->timeout_list && callback_id)
	{
#if defined (USE_XTAPP_CONTEXT)
		XtRemoveWorkProc(callback_id->xt_idle_id);
#elif defined (USE_GTK_MAIN_STEP)
		gtk_idle_remove(callback_id->gtk_idle_id);
#endif /* defined (USE_XTAPP_CONTEXT) */
		return_code = REMOVE_OBJECT_FROM_LIST(Event_dispatcher_idle_callback)
			(callback_id, event_dispatcher->idle_list);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Event_dispatcher_remove_idle_callback.  Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Event_dispatcher_remove_idle_callback */

int Event_dispatcher_do_one_event(struct Event_dispatcher *event_dispatcher)
/*******************************************************************************
LAST MODIFIED : 24 October 2002

DESCRIPTION :
==============================================================================*/
{
	int return_code;
#if defined (USE_GENERIC_EVENT_DISPATCHER)
	int callback_code, select_code;
	struct Event_dispatcher_descriptor_set descriptor_set;
	struct timeval timeofday, timeout, *timeout_ptr;
	struct tms times_buffer;
	struct Event_dispatcher_descriptor_callback *descriptor_callback;
	struct Event_dispatcher_idle_callback *idle_callback;
	struct Event_dispatcher_timeout_callback *timeout_callback;
#endif /*  defined (USE_GENERIC_EVENT_DISPATCHER) */

	ENTER(Event_dispatcher_do_one_event);

	if (event_dispatcher)
	{
#if defined (WIN32_USER_INTERFACE) /* switch (USER_INTERFACE) */
		{
			MSG message;
			if (TRUE==GetMessage(&message,NULL,0,0))
			{
				TranslateMessage(&message);
				DispatchMessage(&message);
			}
			return_code=1;
		}
#elif defined (USE_GTK_MAIN_STEP) /* switch (USER_INTERFACE) */
		gtk_main_iteration();
		return_code = 1;
#elif defined (USE_XTAPP_CONTEXT) /* switch (USER_INTERFACE) */
		XtAppProcessEvent(event_dispatcher->application_context, XtIMAll);
		return_code = 1;
#elif defined (USE_GENERIC_EVENT_DISPATCHER) /* switch (USER_INTERFACE) */
		return_code=1;
		FD_ZERO(&(descriptor_set.read_set));
		FD_ZERO(&(descriptor_set.write_set));
		FD_ZERO(&(descriptor_set.error_set));
 		descriptor_set.max_timeout_ns = -1;
		FOR_EACH_OBJECT_IN_LIST(Event_dispatcher_descriptor_callback)
			(Event_dispatcher_descriptor_do_query_callback,
			&descriptor_set, event_dispatcher->descriptor_list);
		timeout_callback = FIRST_OBJECT_IN_LIST_THAT(Event_dispatcher_timeout_callback)
			((LIST_CONDITIONAL_FUNCTION(Event_dispatcher_timeout_callback) *)NULL,
			(void *)NULL, event_dispatcher->timeout_list);
		if (event_dispatcher->special_idle_callback_pending && event_dispatcher->special_idle_callback)
		{
			timeout.tv_sec = 0;
			timeout.tv_usec = 0;
			timeout_ptr = &timeout;
		}
		else
		{
			if (idle_callback = FIRST_OBJECT_IN_LIST_THAT(Event_dispatcher_idle_callback)
				((LIST_CONDITIONAL_FUNCTION(Event_dispatcher_idle_callback) *)NULL,
					(void *)NULL, event_dispatcher->idle_list))
			{
				timeout.tv_sec = 0;
				timeout.tv_usec = 0;
				timeout_ptr = &timeout;
			}
			else
			{
				/* Till the first timeout */
				if (timeout_callback)
				{
					gettimeofday(&timeofday, NULL);
					if ((timeout_callback->timeout_s < (unsigned long)timeofday.tv_sec) ||
						((timeout_callback->timeout_s == (unsigned long)timeofday.tv_sec) &&
							(timeout_callback->timeout_ns <= (unsigned long)1000*timeofday.tv_usec)))
					{
						timeout.tv_sec = 0;
						timeout.tv_usec = 0;
						timeout_ptr = &timeout;
					}
					else
					{
						timeout.tv_sec = (long)timeout_callback->timeout_s - timeofday.tv_sec;
						if (timeout_callback->timeout_ns/1000.0 > timeofday.tv_usec)
						{
							timeout.tv_usec = ((long)timeout_callback->timeout_ns)/1000 - timeofday.tv_usec;
						}
						else
						{
							timeout.tv_sec--;
							timeout.tv_usec = 1000000 + ((long)timeout_callback->timeout_ns)/1000
								- timeofday.tv_usec;
						}
						timeout_ptr = &timeout;
					}
				}
				else
				{
					if (descriptor_set.max_timeout_ns >= 0)
					{
						timeout.tv_sec = 0;
						timeout.tv_usec = descriptor_set.max_timeout_ns / 1000;
						timeout_ptr = &timeout;
					}
					else
					{
						/* Indefinite */
						timeout_ptr = (struct timeval *)NULL;
					}
				}
			}
		}
		select_code = 0;
		if (!(descriptor_callback = FIRST_OBJECT_IN_LIST_THAT(Event_dispatcher_descriptor_callback)
			(Event_dispatcher_descriptor_callback_is_pending,
			(void *)NULL, event_dispatcher->descriptor_list)))
		{
			if (-1 < (select_code = select(100, &(descriptor_set.read_set),
				&(descriptor_set.write_set), &(descriptor_set.error_set),
				timeout_ptr)))
			{
				/* The select leaves only those descriptors that are pending in the read set,
					we set the pending flag and then work through them one by one.  This makes sure
					that each file callback that is waiting gets an equal chance and we don't just
					call them directly from the iterator as any of the events could modify the list */
				/* For Gtk I need to call check so long as there wasn't an error,
					even if no file_descriptors selected at this level, Gtk will then
					change its maximum priority and ask for a new select */
				FOR_EACH_OBJECT_IN_LIST(Event_dispatcher_descriptor_callback)
					(Event_dispatcher_descriptor_do_check_callback,
					&descriptor_set, event_dispatcher->descriptor_list);
				descriptor_callback =
					FIRST_OBJECT_IN_LIST_THAT(Event_dispatcher_descriptor_callback)
					(Event_dispatcher_descriptor_callback_is_pending,
					(void *)NULL, event_dispatcher->descriptor_list);
			}
		}
		if (descriptor_callback)
		{
			if (event_dispatcher->special_idle_callback)
			{
				event_dispatcher->special_idle_callback_pending = 1;
			}
			/* Call the descriptor dispatch callback then */
			descriptor_callback->pending = 0;
			(*descriptor_callback->dispatch_callback)(descriptor_callback->user_data);
		}
		else
		{
			if (select_code == 0)
			{
				/* Look for ready timer callbacks first */
				gettimeofday(&timeofday, NULL);
				if (timeout_callback &&
					((timeout_callback->timeout_s < (unsigned long)timeofday.tv_sec) ||
					((timeout_callback->timeout_s == (unsigned long)timeofday.tv_sec) &&
					(timeout_callback->timeout_ns <= (unsigned long)1000*timeofday.tv_usec))))
				{
					if (event_dispatcher->special_idle_callback)
					{
						event_dispatcher->special_idle_callback_pending = 1;
					}
					/* Do it now */
					callback_code = (*timeout_callback->timeout_function)(
						timeout_callback->user_data);
					REMOVE_OBJECT_FROM_LIST(Event_dispatcher_timeout_callback)
						(timeout_callback, event_dispatcher->timeout_list);
				}
				else
				{
					if (event_dispatcher->special_idle_callback_pending && 
						event_dispatcher->special_idle_callback)
					{
						callback_code = (*event_dispatcher->special_idle_callback->idle_function)
							(event_dispatcher->special_idle_callback->user_data);
						if (callback_code == 0)
						{
							event_dispatcher->special_idle_callback_pending = 0;
						}
					}
					else
					{
						/* Now do idle callbacks */
						if (idle_callback)
						{
							ACCESS(Event_dispatcher_idle_callback)(idle_callback);
							callback_code = (*idle_callback->idle_function)(idle_callback->user_data);
							if (event_dispatcher->special_idle_callback)
							{
								event_dispatcher->special_idle_callback_pending = 1;
							}
							if (IS_OBJECT_IN_LIST(Event_dispatcher_idle_callback)
								(idle_callback, event_dispatcher->idle_list))
							{
								REMOVE_OBJECT_FROM_LIST(Event_dispatcher_idle_callback)
									(idle_callback, event_dispatcher->idle_list);
								if (callback_code != 0)
								{
									/* Not finished so add it back in, this is done rather
										than just leaving the old event in so that it gets
										a new timestamp and therefore a different idle event
										goes next.  It can't be done while it is in the list
									   as the timestamp is part of the list identifier */
									idle_callback->timestamp = (long)times(&times_buffer);
									ADD_OBJECT_TO_LIST(Event_dispatcher_idle_callback)
										(idle_callback, event_dispatcher->idle_list);
								}
							}
							DEACCESS(Event_dispatcher_idle_callback)(&idle_callback);
						}
					}
				}
			}
			else if (select_code == -1)
			{
				display_message(ERROR_MESSAGE,
					"Event_dispatcher_do_one_event.  "
					"Error on file descriptors.");
				return_code=0;
			}
		}
#endif /* switch (USER_INTERFACE) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Event_dispatcher_register_do_one_event.  Invalid arguments.");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Event_dispatcher_do_one_event */

int Event_dispatcher_main_loop(struct Event_dispatcher *event_dispatcher)
/*******************************************************************************
LAST MODIFIED : 4 March 2002

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Event_dispatcher_main_loop);
	if (event_dispatcher)
	{
		return_code=1;
		while(event_dispatcher->continue_flag)
		{
			Event_dispatcher_do_one_event(event_dispatcher);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Event_dispatcher_main_loop.  Invalid arguments.");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Event_dispatcher_main_loop */

int Event_dispatcher_end_main_loop(struct Event_dispatcher *event_dispatcher)
/*******************************************************************************
LAST MODIFIED : 4 March 2002

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Event_dispatcher_end_main_loop);

	if (event_dispatcher)
	{
		return_code=1;
		event_dispatcher->continue_flag = 0;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Event_dispatcher_end_main_loop.  Invalid arguments.");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Event_dispatcher_end_main_loop */

#if defined (USE_XTAPP_CONTEXT)
int Event_dispatcher_set_application_context(
	struct Event_dispatcher *event_dispatcher,XtAppContext application_context)
/*******************************************************************************
LAST MODIFIED : 4 June 2002

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Event_dispatcher_set_application_context);

	if (event_dispatcher)
	{
		return_code=1;
		event_dispatcher->application_context = application_context;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Event_dispatcher_end_main_loop.  Invalid arguments.");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Event_dispatcher_end_main_loop */
#endif /* defined (USE_XTAPP_CONTEXT) */
