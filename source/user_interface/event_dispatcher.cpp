/*******************************************************************************
FILE : event_dispatcher.c

LAST MODIFIED : 17 January 2006

DESCRIPTION :
This provides an object which interfaces between a event_dispatcher and Cmgui
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
extern "C" {
#include <math.h>
#include <stdio.h>
#include <general/time.h>

#include "general/compare.h"
#include "general/debug.h"
#include "general/list.h"
#include "general/indexed_list_private.h"
#include "general/object.h"
#include "user_interface/message.h"
#include "user_interface/event_dispatcher.h"
}

/* After the event_dispatcher.h has set up these variables */
#if defined (USE_XTAPP_CONTEXT) /* switch (USER_INTERFACE) */
extern "C" {
#include <Xm/Xm.h>
}
#elif defined (WX_USER_INTERFACE) /* switch (USER_INTERFACE) */
#include <wx/wx.h>
#include <wx/apptrait.h>
extern "C" {
#include "user_interface/user_interface.h"
}
#elif defined (WIN32_USER_INTERFACE) /* switch (USER_INTERFACE) */
extern "C" {
#include <windows.h>
#include "general/callback.h"
}
#elif defined (CARBON_USER_INTERFACE) /* switch (USER_INTERFACE) */
#include "carbon/carbon.h"
#elif defined (USE_GTK_MAIN_STEP) /* switch (USER_INTERFACE) */
extern "C" {
#include <gtk/gtk.h>
}
#endif /* switch (USER_INTERFACE) */

/*
Module types
------------
*/

#if defined (USE_GENERIC_EVENT_DISPATCHER)
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
	/* When using the set_socket_* callbacks the user_data is an 
		internal ALLOCATED structure. This flag is set so that the DESTROY
	   DEALLOCATES this user_data.
         */
	int deallocate_user_data_on_destroy;
	Event_dispatcher_descriptor_query_function *query_callback;
	Event_dispatcher_descriptor_check_function *check_callback;
	Event_dispatcher_descriptor_dispatch_function *dispatch_callback;
}; /* struct Event_dispatcher_descriptor_callback */

PROTOTYPE_OBJECT_FUNCTIONS(Event_dispatcher_descriptor_callback);
DECLARE_LIST_TYPES(Event_dispatcher_descriptor_callback);
FULL_DECLARE_INDEXED_LIST_TYPE(Event_dispatcher_descriptor_callback);
#endif /* defined (USE_GENERIC_EVENT_DISPATCHER) */

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
#if defined (CARBON_USER_INTERFACE)
	EventLoopTimerRef carbon_timer_ref;
#endif /* defined (CARBON_USER_INTERFACE) */
#if defined (USE_GTK_MAIN_STEP)
	guint gtk_timeout_id;
#endif /* defined (USE_GTK_MAIN_STEP) */
}; /* struct Event_dispatcher_timeout_callback */

PROTOTYPE_OBJECT_FUNCTIONS(Event_dispatcher_timeout_callback);
DECLARE_LIST_TYPES(Event_dispatcher_timeout_callback);
FULL_DECLARE_INDEXED_LIST_TYPE(Event_dispatcher_timeout_callback);

struct Event_dispatcher_idle_callback
/*******************************************************************************
LAST MODIFIED : 1 June 2003

DESCRIPTION :
Contains all information necessary for a file descriptor callback.
==============================================================================*/
{
	struct Event_dispatcher_idle_callback *self;	
	int access_count;
#if defined (WIN32_SYSTEM)
	FILETIME timestamp;
#else /* defined (WIN32_SYSTEM) */
	long timestamp;
#endif /* defined (WIN32_SYSTEM) */
	enum Event_dispatcher_idle_priority priority;
	Event_dispatcher_idle_function *idle_function;
	void *user_data;
#if defined (USE_XTAPP_CONTEXT)
	XtWorkProcId xt_idle_id;
#endif /* defined (USE_XTAPP_CONTEXT) */
#if defined (USE_GTK_MAIN_STEP)
	guint gtk_idle_id;
#endif /* defined (USE_GTK_MAIN_STEP) */
#if defined (CARBON_USER_INTERFACE)
	EventLoopTimerRef carbon_timer_ref;
#endif /* defined (CARBON_USER_INTERFACE) */
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
#if defined(WIN32_USER_INTERFACE)
	struct LIST(Fdio) *socket_list;
#else
	struct LIST(Event_dispatcher_descriptor_callback) *descriptor_list;
#endif
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
#if defined (WIN32_USER_INTERFACE)
	HWND networkWindowHandle;
#endif /* defined (WIN32_USER_INTERFACE) */
};

struct Fdio_callback_data
/*******************************************************************************
LAST MODIFIED : 28 February, 2005

DESCRIPTION :
Contains data for a callback function in the I/O callback API.
==============================================================================*/
{
	Fdio_callback function;
	void *app_user_data;
};

struct Fdio
/*******************************************************************************
LAST MODIFIED : 17 January 2006

DESCRIPTION :
This structure is equivalent to a filedescriptor/socket, and holds all the
state we need to perform callbacks when the descriptor is ready to be
read from or written to.
==============================================================================*/
{
	struct Event_dispatcher *event_dispatcher;
	Cmiss_native_socket_t descriptor;
	struct Fdio_callback_data read_data;
	struct Fdio_callback_data write_data;
#if defined(USE_GENERIC_EVENT_DISPATCHER)
	int is_reentrant, signal_to_destroy;
	struct Event_dispatcher_descriptor_callback *callback;
	int ready_to_read, ready_to_write;
#elif defined(WIN32_USER_INTERFACE)
	int wantevents;
#elif defined(USE_GTK_MAIN_STEP)
	GIOChannel* iochannel;
	guint read_source_tag, write_source_tag;
#elif defined(XTAPP_CONTEXT)
	XtInputId read_input, write_input;
#endif /* defined(USE_GENERIC_EVENT_DISPATCHER) */
	int access_count;
};

DECLARE_OBJECT_FUNCTIONS(Fdio)
FULL_DECLARE_INDEXED_LIST_TYPE(Fdio);
DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(Fdio,
	descriptor,Cmiss_native_socket_t,compare_int)
DECLARE_INDEXED_LIST_FUNCTIONS(Fdio)
DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(Fdio,
	descriptor,Cmiss_native_socket_t,compare_int)

/*
Module functions
----------------
*/
#if defined (WIN32_USER_INTERFACE)
static int Event_dispatcher_win32_idle_callback(void *idle_callback_void);
/*******************************************************************************
LAST MODIFIED : 9 August 2004

DESCRIPTION :
==============================================================================*/
#endif /* defined (WIN32_USER_INTERFACE) */

#if defined (WIN32_USER_INTERFACE)
static LRESULT CALLBACK Event_dispatcher_handle_win32_network_event(HWND hwnd,
	UINT uMsg, WPARAM wParam, LPARAM lParam)
/*******************************************************************************
LAST MODIFIED : 16 May 2005

DESCRIPTION :
Processes window events on the network-only window.
==============================================================================*/
{
	struct Event_dispatcher *dispatcher;
	Fdio_id fdio;
	SOCKET sock;
	LRESULT ret = 0;

	ENTER(Event_dispatcher_handle_win32_network_event);
	if (hwnd != NULL && uMsg == UWM_NETWORK)
	{
		dispatcher = (struct Event_dispatcher*)GetWindowLong(hwnd, 0);
		sock = (SOCKET)wParam;
		fdio = FIND_BY_IDENTIFIER_IN_LIST
			(Fdio, descriptor)
			(sock, dispatcher->socket_list);
		if (fdio)
		{
			switch (WSAGETSELECTEVENT(lParam))
			{
			case FD_READ:
			case FD_CLOSE:
				if (fdio->read_data.function != NULL)
					fdio->read_data.function(fdio, fdio->read_data.app_user_data);
				break;
			case FD_WRITE:
				if (fdio->write_data.function != NULL)
					fdio->write_data.function(fdio, fdio->write_data.app_user_data);
				break;
			}
		}
	}
        else if (uMsg == UWM_IDLE)
        {
		Event_dispatcher_win32_idle_callback((void *)lParam);
		ret = TRUE;
        }
	else
		ret = TRUE;
	LEAVE;

	return (ret);
}

static int Event_dispatcher_ensure_network_window(struct Event_dispatcher *dispatcher)
/*******************************************************************************
LAST MODIFIED : 16 February 2005

DESCRIPTION :
Ensures that dispatcher->networkWindowHandle is set.
==============================================================================*/
{
	int ret;
	HINSTANCE hInstance;
	WNDCLASS class_data;

	ENTER(Event_dispatcher_ensure_network_window);
	if (dispatcher->networkWindowHandle == NULL)
	{
		hInstance = GetModuleHandle(NULL);
		class_data.style = CS_CLASSDC;
		class_data.lpfnWndProc = Event_dispatcher_handle_win32_network_event;
		class_data.cbClsExtra = 0;
		class_data.cbWndExtra = sizeof(dispatcher);
		class_data.hInstance = hInstance;
		class_data.hIcon = NULL;
		class_data.hCursor = NULL;
		class_data.hbrBackground = NULL;
		class_data.lpszMenuName = NULL;
		class_data.lpszClassName = "cmgui_networkonly_class";
		/* Don't worry if it fails, it may have already been registered. */
		RegisterClass(&class_data);
		dispatcher->networkWindowHandle =
			CreateWindow("cmgui_networkonly_class",
				"This is supposed to be an invisible window!",
				0,
				0, 0, 0, 0,
				NULL,
				NULL,
				hInstance,
				NULL
			);
		if (dispatcher->networkWindowHandle != NULL)
		{
			/* XXX FIXME for 64 bit windows */
			SetWindowLong(dispatcher->networkWindowHandle, 0,
				(LONG)dispatcher);
			ret = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"DESTROY(Event_dispatcher_ensure_network_window).  "
				"Can't create a network-only window.");
			ret = 0;
		}
	}
	else
		ret = 1;

	LEAVE;
	return (ret);
}
#else /* defined(WIN32_USER_INTERFACE) */

#if defined (USE_GENERIC_EVENT_DISPATCHER)
static struct Event_dispatcher_descriptor_callback *CREATE(Event_dispatcher_descriptor_callback)(
	Event_dispatcher_descriptor_query_function *query_function,
	Event_dispatcher_descriptor_check_function *check_function,
	Event_dispatcher_descriptor_dispatch_function *dispatch_function,
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
		callback->query_callback = query_function;
		callback->check_callback = check_function;
		callback->dispatch_callback = dispatch_function;
		callback->user_data = user_data;
		callback->pending = 1;
		callback->deallocate_user_data_on_destroy = 0;
		callback->access_count = 0;
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
#endif /* defined (USE_GENERIC_EVENT_DISPATCHER) */

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

#if defined (USE_GENERIC_EVENT_DISPATCHER)
DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(Event_dispatcher_descriptor_callback, \
	self,struct Event_dispatcher_descriptor_callback *,compare_pointer)
DECLARE_OBJECT_FUNCTIONS(Event_dispatcher_descriptor_callback)
DECLARE_INDEXED_LIST_FUNCTIONS(Event_dispatcher_descriptor_callback)
DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(Event_dispatcher_descriptor_callback, \
	self,struct Event_dispatcher_descriptor_callback *,compare_pointer)
#endif /* defined (USE_GENERIC_EVENT_DISPATCHER) */
#endif /* defined (WIN32_USER_INTERFACE) else */

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
#if defined (USE_XTAPP_CONTEXT)
		timeout_callback->xt_timeout_id = (XtIntervalId)NULL;
#endif /* defined (USE_XTAPP_CONTEXT) */
#if defined (CARBON_USER_INTERFACE)
		timeout_callback->carbon_timer_ref = (EventLoopTimerRef)NULL;
#endif /* defined (CARBON_USER_INTERFACE) */
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
LAST MODIFIED : 1 June 2003

DESCRIPTION :
Create a single object that belongs to a specific file descriptor.
==============================================================================*/
{
	struct Event_dispatcher_idle_callback *idle_callback;
#if !defined (WIN32_SYSTEM)
	struct tms times_buffer;
#endif /* !defined (WIN32_SYSTEM) */

	ENTER(CREATE(Event_dispatcher_idle_callback));

	if (ALLOCATE(idle_callback, struct Event_dispatcher_idle_callback, 1))
	{
		idle_callback->self = idle_callback;
		idle_callback->priority = priority;
#if defined (WIN32_SYSTEM)
		GetSystemTimeAsFileTime(&(idle_callback->timestamp));
#else /* defined (WIN32_SYSTEM) */
		idle_callback->timestamp = (long)times(&times_buffer);
#endif /* defined (WIN32_SYSTEM) */
		idle_callback->idle_function = idle_function;
		idle_callback->user_data = user_data;
		idle_callback->access_count = 0;

#if defined (USE_XTAPP_CONTEXT)
		idle_callback->xt_idle_id = (XtWorkProcId)NULL;
#endif /* defined (USE_XTAPP_CONTEXT) */
#if defined (USE_GTK_MAIN_STEP)
		idle_callback->gtk_idle_id = 0;
#endif /* defined (USE_GTK_MAIN_STEP) */
#if defined (CARBON_USER_INTERFACE)
		idle_callback->carbon_timer_ref = (EventLoopTimerRef)NULL;
#endif /* defined (CARBON_USER_INTERFACE) */

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

#if defined (CARBON_USER_INTERFACE)
		if ((*idle_callback_address)->carbon_timer_ref)
		{
			RemoveEventLoopTimer((*idle_callback_address)->carbon_timer_ref);
		}
#endif /* defined (CARBON_USER_INTERFACE) */

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
#if defined (WIN32_SYSTEM)
			if (((idle_one->timestamp).dwHighDateTime<
				(idle_two->timestamp).dwHighDateTime)||
				(((idle_one->timestamp).dwHighDateTime==
				(idle_two->timestamp).dwHighDateTime)&&
				((idle_one->timestamp).dwLowDateTime<
				(idle_two->timestamp).dwLowDateTime)))
#else /* defined (WIN32_SYSTEM) */
			if (idle_one->timestamp < idle_two->timestamp)
#endif /* defined (WIN32_SYSTEM) */
			{
				return_code = -1;
			}
#if defined (WIN32_SYSTEM)
			if (((idle_one->timestamp).dwHighDateTime>
				(idle_two->timestamp).dwHighDateTime)||
				(((idle_one->timestamp).dwHighDateTime==
				(idle_two->timestamp).dwHighDateTime)&&
				((idle_one->timestamp).dwLowDateTime>
				(idle_two->timestamp).dwLowDateTime)))
#else /* defined (WIN32_SYSTEM) */
			if (idle_one->timestamp > idle_two->timestamp)
#endif /* defined (WIN32_SYSTEM) */
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

#if defined (WIN32_USER_INTERFACE)
static int Event_dispatcher_win32_idle_callback(
	void *idle_callback_void)
/*******************************************************************************
LAST MODIFIED : 9 August 2004

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Event_dispatcher_idle_callback *idle_callback;

	ENTER(Event_dispatcher_win32_idle_callback);
	if (idle_callback = (struct Event_dispatcher_idle_callback *)idle_callback_void)
	{
		if (idle_callback->idle_function &&
		    (*idle_callback->idle_function)(idle_callback->user_data))
		{
			display_message(ERROR_MESSAGE,
				"Event_dispatcher_win32_idle_callback.  Callback function failed.");
			return_code = 0;
		}
		else
		{
			return_code = 1;
		}
		DEACCESS(Event_dispatcher_idle_callback)(&idle_callback);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Event_dispatcher_win32_idle_callback.  Invalid arguments.");
	}
	LEAVE;

	return (return_code);
} /* Event_dispatcher_win32_idle_callback */
#endif /* defined (WIN32_USER_INTERFACE) */

#if defined (CARBON_USER_INTERFACE)
static void Event_dispatcher_Carbon_idle_callback(
	EventLoopTimerRef timer, EventLoopIdleTimerMessage state,
	void *idle_callback_void)
/*******************************************************************************
LAST MODIFIED : 24 November 2006

DESCRIPTION :
==============================================================================*/
{
	struct Event_dispatcher_idle_callback *idle_callback;

	ENTER(Event_dispatcher_gtk_idle_callback);
	if (idle_callback = (struct Event_dispatcher_idle_callback *)idle_callback_void)
	{
		if ((*idle_callback->idle_function)(idle_callback->user_data))
		{
			RemoveEventLoopTimer(idle_callback->carbon_timer_ref);
			idle_callback->carbon_timer_ref = (EventLoopTimerRef)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Event_dispatcher_gtk_idle_callback.  Invalid arguments.");
	}
	LEAVE;
} /* Event_dispatcher_Carbon_idle_callback */
#endif /* defined (CARBON_USER_INTERFACE) */

#if defined (WX_USER_INTERFACE)
static int Event_dispatcher_do_idle_event(struct Event_dispatcher *event_dispatcher)
/*******************************************************************************
LAST MODIFIED : 8 December 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	int callback_code;
	struct Event_dispatcher_idle_callback *idle_callback;
#if !defined (WIN32_SYSTEM)
	struct tms times_buffer;
#endif /* !defined (WIN32_SYSTEM) */

	ENTER(Event_dispatcher_do_idle_event);

	if (event_dispatcher)
	{
		if(idle_callback = FIRST_OBJECT_IN_LIST_THAT(Event_dispatcher_idle_callback)
			((LIST_CONDITIONAL_FUNCTION(Event_dispatcher_idle_callback) *)NULL,
				(void *)NULL, event_dispatcher->idle_list))
		{
			ACCESS(Event_dispatcher_idle_callback)(idle_callback);
			callback_code = (*idle_callback->idle_function)(idle_callback->user_data);
			if (IS_OBJECT_IN_LIST(Event_dispatcher_idle_callback)
				(idle_callback, event_dispatcher->idle_list))
			{
				REMOVE_OBJECT_FROM_LIST(Event_dispatcher_idle_callback)
					(idle_callback, event_dispatcher->idle_list);
				if (callback_code != 0)
				{
#if defined (WIN32_SYSTEM)
					GetSystemTimeAsFileTime(&(idle_callback->timestamp));
#else /* defined (WIN32_SYSTEM) */
					idle_callback->timestamp = (long)times(&times_buffer);
#endif /* defined (WIN32_SYSTEM) */
					ADD_OBJECT_TO_LIST(Event_dispatcher_idle_callback)
						(idle_callback, event_dispatcher->idle_list);
				}
			}
			DEACCESS(Event_dispatcher_idle_callback)(&idle_callback);
		}
		if (0<NUMBER_IN_LIST(Event_dispatcher_idle_callback)(event_dispatcher->idle_list))
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
			"Event_dispatcher_register_do_idle_event.  Invalid arguments.");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Event_dispatcher_do_idle_event */

class wxCmguiApp : public wxApp
{
	Event_dispatcher *event_dispatcher;

public:
	wxCmguiApp() : wxApp()
	{
		event_dispatcher = static_cast<Event_dispatcher *>(NULL);
	}

    virtual bool OnInit()
	{
		return (true);
	}

	virtual ~wxCmguiApp()
	{
	}
	
	virtual wxAppTraits * CreateTraits()
	{
		return new wxGUIAppTraits;
	}

	void OnIdle(wxIdleEvent& event)
	{
		if (event_dispatcher)
		{
			if (Event_dispatcher_do_idle_event(event_dispatcher))
			{
				event.RequestMore();
			}
		}
	}

	void SetEventDispatcher(Event_dispatcher *event_dispatcher_in)
	{
		event_dispatcher = event_dispatcher_in;
	}

   DECLARE_EVENT_TABLE();
};	

IMPLEMENT_APP_NO_MAIN(wxCmguiApp)

BEGIN_EVENT_TABLE(wxCmguiApp, wxApp)
	EVT_IDLE(wxCmguiApp::OnIdle)
END_EVENT_TABLE()

#endif /* defined (WX_USER_INTERFACE) */

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
#if defined (WIN32_USER_INTERFACE)
		event_dispatcher->socket_list = 
			CREATE(LIST(Fdio))();
#endif /* defined (WIN32_USER_INTERFACE) */
#if defined (USE_GENERIC_EVENT_DISPATCHER)
		event_dispatcher->descriptor_list = 
			CREATE(LIST(Event_dispatcher_descriptor_callback))();
#endif /* defined (USE_GENERIC_EVENT_DISPATCHER) */
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
#if defined(WIN32_USER_INTERFACE)
		event_dispatcher->networkWindowHandle = (HWND)NULL;
#endif /* defined(WIN32_USER_INTERFACE) */
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
#if defined (WIN32_USER_INTERFACE)
		if (event_dispatcher->socket_list)
		{
			DESTROY(LIST(Fdio))
				(&event_dispatcher->socket_list);
		}
#endif /* defined (WIN32_USER_INTERFACE) */
#if defined (USE_GENERIC_EVENT_DISPATCHER)
		if (event_dispatcher->descriptor_list)
		{
			DESTROY(LIST(Event_dispatcher_descriptor_callback))
				(&event_dispatcher->descriptor_list);
		}
#endif /* defined (USE_GENERIC_EVENT_DISPATCHER) */
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

#if defined (WX_USER_INTERFACE)
int Event_dispatcher_initialise_wx_app(struct Event_dispatcher *event_dispatcher)
/*******************************************************************************
LAST MODIFIED : 30 April 2007

DESCRIPTION :
==============================================================================*/
{
	 int return_code;

	ENTER(Event_dispatcher_initialise_wx_app);

	if (event_dispatcher)
	{
		 wxCmguiApp &app = wxGetApp();
		 if (&app)
		 {
				app.SetEventDispatcher(event_dispatcher);
				return_code = 1;
		 }
		 else
		 {
				display_message(ERROR_MESSAGE,
					 "Event_dispatcher_initialise_wx_app.  wxCmguiApp not initialised.");
				return_code = 0;
		 }
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Event_dispatcher_initialise_wx_app.  Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Event_dispatcher_initialise_wx_app */
#endif /* defined (WX_USER_INTERFACE) */

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

#endif /* defined(USE_GENERIC_EVENT_DISPATCHER) */

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
		timeout_callback = Event_dispatcher_add_timeout_callback(
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

#if defined (WIN32_USER_INTERFACE)

static void CALLBACK Event_dispatcher_process_win32_timeout(
	HWND hWnd, UINT msg, UINT TimerID, DWORD Time)
/*******************************************************************************
LAST MODIFIED : 13 June 2005

DESCRIPTION :
Processes a Win32 timer.
==============================================================================*/
{
  struct Event_dispatcher_timeout_callback *timeout_callback;

  ENTER(Event_dispatcher_process_win32_timeout);
  USE_PARAMETER(msg);
  USE_PARAMETER(Time);
  /* Our timers are one-shot, Win32 timers recur... */
  KillTimer(hWnd, TimerID);

  timeout_callback = (struct Event_dispatcher_timeout_callback*)TimerID;
  (*timeout_callback->timeout_function)(
	timeout_callback->user_data);
  
  DESTROY(Event_dispatcher_timeout_callback)(&timeout_callback);

  LEAVE;
}

struct Event_dispatcher_timeout_callback *Event_dispatcher_add_timeout_callback_at_time(
	struct Event_dispatcher *event_dispatcher, unsigned long timeout_s, unsigned long timeout_ns,
	Event_dispatcher_timeout_function *timeout_function, void *user_data)
/*******************************************************************************
LAST MODIFIED : 13 June 2005

DESCRIPTION :
Set a timeout on Win32...
==============================================================================*/
{
	struct Event_dispatcher_timeout_callback *timeout_callback;
	ULONGLONG system_time, event_time, event_time_delta_millis;
  
	ENTER(Event_dispatcher_register_descriptor_callback);

	if (event_dispatcher && timeout_function)
	{
		if (timeout_callback = CREATE(Event_dispatcher_timeout_callback)(
			timeout_s, timeout_ns, timeout_function, user_data))
		{
			GetSystemTimeAsFileTime((FILETIME *)&system_time);
		 
 			/* Change the epoch to avoid overflow... */
 			system_time -= 119603304000000000LL;
			event_time = (ULONGLONG)timeout_s * 10000000L + (ULONGLONG)timeout_ns / 100L;
			if (system_time < event_time)
				event_time_delta_millis = (event_time - system_time) / 10000;
			else
				event_time_delta_millis = 0;

			Event_dispatcher_ensure_network_window(event_dispatcher);
		  
			SetTimer(event_dispatcher->networkWindowHandle,
				(ULONG)timeout_callback,
				(ULONG)event_time_delta_millis,
				Event_dispatcher_process_win32_timeout
				);
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

	return timeout_callback;
}

#elif defined (CARBON_USER_INTERFACE)

static void Event_dispatcher_process_Carbon_timeout(
	EventLoopTimerRef timer, void *user_data)
/*******************************************************************************
LAST MODIFIED : 1 December 2006

DESCRIPTION :
Processes a Carbon timer.
==============================================================================*/
{
  struct Event_dispatcher_timeout_callback *timeout_callback;

  ENTER(Event_dispatcher_process_win32_timeout);
  USE_PARAMETER(timer);

  timeout_callback = (struct Event_dispatcher_timeout_callback*)user_data;

  /* Our timers are one-shot, Carbon timers recur... */
  RemoveEventLoopTimer(timeout_callback->carbon_timer_ref);

  (*timeout_callback->timeout_function)(
	timeout_callback->user_data);
  
  DESTROY(Event_dispatcher_timeout_callback)(&timeout_callback);

  LEAVE;
}

static struct Event_dispatcher_timeout_callback *Event_dispatcher_add_Carbon_timeout_callback(
	struct Event_dispatcher *event_dispatcher, unsigned long timeout_s, unsigned long timeout_ns,
	Event_dispatcher_timeout_function *timeout_function, void *user_data)
/*******************************************************************************
LAST MODIFIED : 1 December 2006

DESCRIPTION :
Set a timeout on Carbon...
==============================================================================*/
{
	struct Event_dispatcher_timeout_callback *timeout_callback;
  
	ENTER(Event_dispatcher_register_descriptor_callback);

	if (event_dispatcher && timeout_function)
	{
		if (timeout_callback = CREATE(Event_dispatcher_timeout_callback)(
			timeout_s, timeout_ns, timeout_function, user_data))
		{
			EventLoopRef main_loop;
			EventLoopTimerUPP timer_UPP;

			main_loop = GetMainEventLoop();
			timer_UPP = NewEventLoopTimerUPP(Event_dispatcher_process_Carbon_timeout);

			InstallEventLoopTimer(main_loop,
				timeout_s * kEventDurationSecond +
				timeout_ns * kEventDurationNanosecond,
				kEventDurationSecond, timer_UPP, timeout_callback,
				&timeout_callback->carbon_timer_ref);
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

	return timeout_callback;
}

#elif defined (WX_USER_INTERFACE)

class wxEventTimer : public wxTimer
{
  struct Event_dispatcher_timeout_callback *timeout_callback;


	~wxEventTimer()
	{
		DESTROY(Event_dispatcher_timeout_callback)(&timeout_callback);
	}

	void Notify()
	{
		(*timeout_callback->timeout_function)(
			timeout_callback->user_data);
	}
	
public:
	wxEventTimer(struct Event_dispatcher_timeout_callback *timeout_callback):
		timeout_callback(timeout_callback)
	{
	}
}; // class wxEventTimer

static struct Event_dispatcher_timeout_callback *Event_dispatcher_add_wx_timeout_callback(
	struct Event_dispatcher *event_dispatcher, unsigned long timeout_s,
	unsigned long timeout_ns,
	Event_dispatcher_timeout_function *timeout_function, void *user_data)
/*******************************************************************************
LAST MODIFIED : 1 December 2006

DESCRIPTION :
Set a timeout on wx widgets
==============================================================================*/
{
	struct Event_dispatcher_timeout_callback *timeout_callback;
  
	ENTER(Event_dispatcher_register_descriptor_callback);

	if (event_dispatcher && timeout_function)
	{
		if (timeout_callback = CREATE(Event_dispatcher_timeout_callback)(
			timeout_s, timeout_ns, timeout_function, user_data))
		{
			wxEventTimer *timer = new wxEventTimer(timeout_callback);
			timer->Start(timeout_s * 1000 + timeout_ns / 1000000, /*OneShot*/true);
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

	return timeout_callback;
}

#elif defined (USE_GTK_MAIN_STEP)

static gboolean Event_dispatcher_process_gtk_timeout(gpointer user_data)
/*******************************************************************************
LAST MODIFIED : 22 December 2006

DESCRIPTION :
Processes a Gtk timer.
==============================================================================*/
{
  struct Event_dispatcher_timeout_callback *timeout_callback;

  ENTER(Event_dispatcher_process_gtk_timeout);

  timeout_callback = (struct Event_dispatcher_timeout_callback*)user_data;

  (*timeout_callback->timeout_function)(timeout_callback->user_data);
  
  DESTROY(Event_dispatcher_timeout_callback)(&timeout_callback);

  LEAVE;

  return(FALSE);  /* Don't recur */
}

static struct Event_dispatcher_timeout_callback *Event_dispatcher_add_gtk_timeout_callback(
	struct Event_dispatcher *event_dispatcher, unsigned long timeout_s, unsigned long timeout_ns,
	Event_dispatcher_timeout_function *timeout_function, void *user_data)
/*******************************************************************************
LAST MODIFIED : 22 December 2006

DESCRIPTION :
Set a timeout on Gtk main loop
==============================================================================*/
{
	guint32 interval;
	struct Event_dispatcher_timeout_callback *timeout_callback;
  
	ENTER(Event_dispatcher_add_timeout_callback_at_time);

	if (event_dispatcher && timeout_function)
	{
		if (timeout_callback = CREATE(Event_dispatcher_timeout_callback)(
			timeout_s, timeout_ns, timeout_function, user_data))
		{
			interval = timeout_s * 1000 + timeout_ns / 1000000;
			timeout_callback->gtk_timeout_id = gtk_timeout_add(interval,
				Event_dispatcher_process_gtk_timeout, timeout_callback);
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

	return timeout_callback;
}

#elif defined (USE_GENERIC_EVENT_DISPATCHER)

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

#endif /* switch (USER_INTERFACE) */

struct Event_dispatcher_timeout_callback *Event_dispatcher_add_timeout_callback(
	struct Event_dispatcher *event_dispatcher, unsigned long timeout_s, unsigned long timeout_ns,
	Event_dispatcher_timeout_function *timeout_function, void *user_data)
/*******************************************************************************
LAST MODIFIED : 1 June 2003

DESCRIPTION :
==============================================================================*/
{
	struct Event_dispatcher_timeout_callback *timeout_callback;
#if defined (WX_USER_INTERFACE)
#elif defined (WIN32_SYSTEM)
	ULONGLONG system_time;
#elif defined (USE_GENERIC_EVENT_DISPATCHER)
	struct timeval timeofday;
#endif /* switch (USER_INTERFACE) */

	ENTER(Event_dispatcher_register_descriptor_callback);
	if (event_dispatcher && timeout_function)
	{
#if defined (USE_GTK_MAIN_STEP)
		/* This should preempt the WIN32_SYSTEM version */
		timeout_callback = Event_dispatcher_add_gtk_timeout_callback(
			event_dispatcher, timeout_s, 
			timeout_ns, 
			timeout_function, user_data);
#elif defined (CARBON_USER_INTERFACE)
		timeout_callback = Event_dispatcher_add_Carbon_timeout_callback(
			event_dispatcher, timeout_s, 
			timeout_ns, 
			timeout_function, user_data);
#elif defined (WX_USER_INTERFACE)
		timeout_callback = Event_dispatcher_add_wx_timeout_callback(
			event_dispatcher, timeout_s, 
			timeout_ns, 
			timeout_function, user_data);
#elif defined (WIN32_SYSTEM)
		GetSystemTimeAsFileTime((FILETIME *)&system_time);
 		system_time -= 119603304000000000LL;
		timeout_callback = Event_dispatcher_add_timeout_callback_at_time(
			event_dispatcher, timeout_s +
			(unsigned long)(system_time/10000000L),
			timeout_ns +
			100*(unsigned long)(system_time%10000000L),
			timeout_function, user_data);
#elif defined (USE_GENERIC_EVENT_DISPATCHER)
		gettimeofday(&timeofday, NULL);
		timeout_callback = Event_dispatcher_add_timeout_callback_at_time(
			event_dispatcher, timeout_s + (unsigned long)timeofday.tv_sec, 
			timeout_ns + 1000*(unsigned long)timeofday.tv_usec, 
			timeout_function, user_data);
#else /* switch (USER_INTERFACE) */
#error Timeout callbacks not defined on this platform
#endif /* defined (WIN32_SYSTEM) */
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
#if defined (USE_GTK_MAIN_STEP)
		gtk_timeout_remove(callback_id->gtk_timeout_id);
#elif defined (CARBON_USER_INTERFACE)
		RemoveEventLoopTimer(callback_id->carbon_timer_ref);
		callback_id->carbon_timer_ref = (EventLoopTimerRef)NULL;
#elif defined (WX_USER_INTERFACE)
#elif defined (WIN32_USER_INTERFACE)
		return_code = 1;
		KillTimer(event_dispatcher->networkWindowHandle, (ULONG)callback_id);
#elif defined (USE_GENERIC_EVENT_DISPATCHER)
		return_code = REMOVE_OBJECT_FROM_LIST(Event_dispatcher_timeout_callback)
			(callback_id, event_dispatcher->timeout_list);
#else /* switch (USER_INTERFACE) */
#error remove timeout callbacks not defined on this platform
#endif /* switch (USER_INTERFACE) */
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
#elif defined (CARBON_USER_INTERFACE)
			else
			{
				EventLoopRef main_loop = GetMainEventLoop();
				EventLoopIdleTimerUPP idle_timer_function = 
					NewEventLoopIdleTimerUPP(Event_dispatcher_Carbon_idle_callback);

				InstallEventLoopIdleTimer(main_loop,
					/*FireDelay*/0, /*Interval*/10,
					idle_timer_function, idle_callback,
					&idle_callback->carbon_timer_ref);
			}
#elif defined (WX_USER_INTERFACE)
#elif defined (WIN32_SYSTEM)
			else
			{
				Event_dispatcher_ensure_network_window(event_dispatcher);
				if(!(PostMessage(event_dispatcher->networkWindowHandle,
						UWM_IDLE, 0, (LPARAM)idle_callback)))
				{
					display_message(ERROR_MESSAGE,
						"Event_dispatcher_add_idle_event_callback.  "
						"Could not post idle_callback message.");
					idle_callback = (struct Event_dispatcher_idle_callback *)NULL;
				}
				else
				{
					ACCESS(Event_dispatcher_idle_callback)(idle_callback);
				}
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
#elif defined (CARBON_USER_INTERFACE)
		RemoveEventLoopTimer(callback_id->carbon_timer_ref);
		callback_id->carbon_timer_ref = (EventLoopTimerRef)NULL;
#endif /* defined (USE_XTAPP_CONTEXT) */
		callback_id->idle_function = NULL;
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
#elif defined (CARBON_USER_INTERFACE) /* switch (USER_INTERFACE) */
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
#if ! defined (WX_USER_INTERFACE)
#  if ! defined (CARBON_USER_INTERFACE)
		while(event_dispatcher->continue_flag)
		{
			Event_dispatcher_do_one_event(event_dispatcher);
		}
#  else /* ! defined (CARBON_USER_INTERFACE) */
		RunApplicationEventLoop(); // Process events until time to quit
#  endif /* ! defined (CARBON_USER_INTERFACE) */
#else /* ! defined (WX_USER_INTERFACE) */

		wxCmguiApp &app = wxGetApp();
		app.OnRun();
		return_code = 1;

#endif /* ! defined (WX_USER_INTERFACE) */
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
#if ! defined (WX_USER_INTERFACE)
		event_dispatcher->continue_flag = 0;
#else /* ! defined (WX_USER_INTERFACE) */
		wxCmguiApp &app = wxGetApp();
		app.ExitMainLoop();
		return_code = 1;
#endif /* ! defined (WX_USER_INTERFACE) */
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

Fdio_package_id CREATE(Fdio_package)(
	struct Event_dispatcher *event_dispatcher)
/*******************************************************************************
LAST MODIFIED : 28 February 2005

DESCRIPTION :
Creates a new Fdio_package, given an event dispatcher.
==============================================================================*/
{
	ENTER(CREATE(Fdio_package));
	LEAVE;

	return (Fdio_package_id)(event_dispatcher);
} /* CREATE(Fdio_package) */

int DESTROY(Fdio_package)(Fdio_package_id *pkg)
/*******************************************************************************
LAST MODIFIED : 28 February 2005

DESCRIPTION :
Destroys the Fdio package object. This causes cmgui to forget about the descriptor,
but the descriptor itself must still be closed. This should be called as soon as
the application is notified by the operating system of a closure event.
==============================================================================*/
{
	ENTER(DESTROY(Fdio_package));
	*pkg = NULL;
	LEAVE;

	return (1);
}

Fdio_id Fdio_package_create_Fdio(Fdio_package_id package,
	Cmiss_native_socket_t descriptor)
/*******************************************************************************
LAST MODIFIED : 16 May, 2005

DESCRIPTION :
This function creates an Fdio object, given a Fdio_package and a descriptor.
==============================================================================*/
{
	Fdio_id ret;

	ENTER(Fdio_package_create_Fdio);
	ret = Event_dispatcher_create_Fdio((struct Event_dispatcher*)package,
		descriptor);

	LEAVE;

	return (ret);
}

/* This implementation is so dependent on USE_GENERIC_EVENT_DISPATCHER that it is
 * easier to write the whole thing again for each option than to put
 * preprocessor conditionals in every single function.
*/
#if defined(USE_GENERIC_EVENT_DISPATCHER)
static int Fdio_event_dispatcher_query_function(
	struct Event_dispatcher_descriptor_set *descriptor_set,
	void *user_data
	);
/*******************************************************************************
LAST MODIFIED : 28 February, 2005

DESCRIPTION :
This function is the "query" callback function whenever we need to call FD_SET
etc... on a socket using this API.
==============================================================================*/

static int Fdio_event_dispatcher_check_function(
	struct Event_dispatcher_descriptor_set *descriptor_set,
	void *user_data
	);
/*******************************************************************************
LAST MODIFIED : 28 February, 2005

DESCRIPTION :
This function is called whenever we need to call FD_ISSET etc... on a socket
using this API.
==============================================================================*/

static int Fdio_event_dispatcher_dispatch_function(
	void *user_data
	);
/*******************************************************************************
LAST MODIFIED : 28 February, 2005

DESCRIPTION :
This function is called whenever we need to dispatch callbacks out to the
application.
==============================================================================*/

static int Fdio_set_callback(struct Fdio_callback_data *callback_data,
	Fdio_callback callback,
	void *app_user_data);
/*******************************************************************************
LAST MODIFIED : 28 February, 2005

DESCRIPTION :
This function sets the callback and user data for a given callback_data structure.
==============================================================================*/

Fdio_id Event_dispatcher_create_Fdio(struct Event_dispatcher *dispatcher,
	Cmiss_native_socket_t descriptor)
/*******************************************************************************
LAST MODIFIED : 28 February 2005

DESCRIPTION :
Creates a new Fdio, given an event dispatcher.
==============================================================================*/
{
	struct Cmiss_fdio *io;

	ENTER(CREATE(Event_dispatcher_create_fdio));

	if (ALLOCATE(io, struct Fdio, 1))
	{
		memset(io, 0, sizeof(*io));
		io->event_dispatcher = dispatcher;
		io->descriptor = descriptor;
		io->access_count = 0;
		if (!(io->callback = Event_dispatcher_add_descriptor_callback(
			io->event_dispatcher,
			Fdio_event_dispatcher_query_function,
			Fdio_event_dispatcher_check_function,
			Fdio_event_dispatcher_dispatch_function,
			io
			)))
		{
			display_message(ERROR_MESSAGE,
				"Event_dispatcher_create_fdio.  "
				"Event_dispatcher_add_descriptor_callback failed.");
			DEALLOCATE(io);
			io = (Fdio_id)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "CREATE(Fdio).  "
			"Unable to allocate structure");
	}
	LEAVE;

	return (io);
} /* Event_dispatcher_create_fdio */

int DESTROY(Fdio)(Fdio_id *io)
/*******************************************************************************
LAST MODIFIED : 28 February 2005

DESCRIPTION :
Destroys the IO object. This causes cmgui to forget about the descriptor, but the
descriptor itself must still be closed. This should be called as soon as the
application is notified by the operating system of a closure event.
==============================================================================*/
{
	ENTER(DESTROY(Fdio));
	if ((*io)->is_reentrant)
		(*io)->signal_to_destroy = 1;
	else
	{
		Event_dispatcher_remove_descriptor_callback((*io)->event_dispatcher,
			(*io)->callback);
		DEALLOCATE(*io);
	}

	*io = NULL;

	LEAVE;

	return (1);
} /* DESTROY(Fdio) */

int Fdio_set_read_callback(Fdio_id handle,
	Fdio_callback callback,
	void *user_data)
/*******************************************************************************
LAST MODIFIED : 28 February 2005

DESCRIPTION :
Sets a read callback on the specified IO handle. This callback is called at
least once after a read function indicates it would block. An application
should not rely upon it being called more than once without attempting a
read between the calls. This read should occur after Fdio_set_read_callback
is called. The callback will also be called if the underlying descriptor is
closed by the peer. The callback is not one-shot, and the callback remains in
effect until it is explicitly cancelled.

There may be at most one read callback set per I/O handle at any one time. If
this function is passed NULL as the callback parameter, the read callback
previously set will be cancelled.
==============================================================================*/
{
	ENTER(Fdio_set_read_callback);
	Fdio_set_callback(&handle->read_data, callback, user_data);
	LEAVE;

	return (1);
} /* Fdio_set_read_callback */

int Fdio_set_write_callback(Fdio_id handle,
	Fdio_callback callback,
	void *user_data)
/*******************************************************************************
LAST MODIFIED : 28 February 2005

DESCRIPTION :
Sets a write callback on the specified Fdio handle. This callback is called at
least once after a write function indicates it would block. An application
should not rely upon it being called more than once without attempting a
write between the calls. This write should occur after Fdio_set_write_callback
is called. The callback is not one-shot, and the callback remains in
effect until it is explicitly cancelled.

There may be at most one write callback set per I/O handle at any one time. If
this function is passed NULL as the callback parameter, the write callback
previously set will be cancelled.
==============================================================================*/
{
	ENTER(Fdio_set_write_callback);
	Fdio_set_callback(&handle->read_data, callback, user_data);
	LEAVE;

	return (1);
} /* Fdio_set_write_callback */

static int Fdio_event_dispatcher_query_function(
	struct Event_dispatcher_descriptor_set *descriptor_set,
	void *user_data
	)
/*******************************************************************************
LAST MODIFIED : 28 February 2005

DESCRIPTION :
This function is the "query" callback function whenever we need to call FD_SET
etc... on a socket using this API.
==============================================================================*/
{
	Fdio_id io;

	ENTER(Fdio_event_dispatcher_query_function);
	io = (Fdio_id)user_data;

	if (io->read_data.function)
		FD_SET(io->descriptor, &descriptor_set->read_set);

	if (io->write_data.function)
		FD_SET(io->descriptor, &descriptor_set->write_set);

	LEAVE;

	return (1);
} /* Fdio_event_dispatcher_query_function */


static int Fdio_event_dispatcher_check_function(
	struct Event_dispatcher_descriptor_set *descriptor_set,
	void *user_data
	)
/*******************************************************************************
LAST MODIFIED : 28 February 2005

DESCRIPTION :
This function is called whenever we need to call FD_ISSET etc... on a socket
using this API.
==============================================================================*/
{
	Fdio_id io;

	ENTER(Fdio_event_dispatcher_check_function);
	io = (Fdio_id)user_data;

	io->ready_to_read = FD_ISSET(io->descriptor, &descriptor_set->read_set);
	io->ready_to_write = FD_ISSET(io->descriptor, &descriptor_set->write_set);

	LEAVE;

	return (io->ready_to_read || io->ready_to_write);
} /* Fdio_event_dispatcher_check_function */

static int Fdio_event_dispatcher_dispatch_function(
	void *user_data
	)
/*******************************************************************************
LAST MODIFIED : 28 February 2005

DESCRIPTION :
This function is called whenever we need to dispatch callbacks out to the
application.
==============================================================================*/
{
	Fdio_id io;

	ENTER(Fdio_event_dispatcher_dispatch_function);
	io = (Fdio_id)user_data;

	io->is_reentrant = 1;
	if (io->read_data.function && io->ready_to_read)
		io->read_data.function(io, io->read_data.app_user_data);
	if (io->write_data.function &&
		io->ready_to_write &&
		!io->signal_to_destroy)
		io->write_data.function(io, io->write_data.app_user_data);
	io->is_reentrant = 0;
	if (io->signal_to_destroy)
		DESTROY(Fdio)(&io);
	LEAVE;

	return (1);
} /* Fdio_event_dispatcher_dispatch_function */

static int Fdio_set_callback(struct Fdio_callback_data *callback_data,
	Fdio_callback callback,
	void *app_user_data)
/*******************************************************************************
LAST MODIFIED : 28 February 2005

DESCRIPTION :
This function sets the callback and user data for a given callback_data structure.
==============================================================================*/
{
	ENTER(Fdio_set_callback);
	callback_data->function = callback;
	callback_data->app_user_data = app_user_data;
	LEAVE;

	return (1);
} /* Fdio_set_callback */

#elif defined(WIN32_USER_INTERFACE)
Fdio_id Event_dispatcher_create_Fdio(struct Event_dispatcher *dispatcher,
	Cmiss_native_socket_t descriptor)
/*******************************************************************************
LAST MODIFIED : 28 February 2005

DESCRIPTION :
Creates a new Fdio, given an event dispatcher and a descriptor.
==============================================================================*/
{
	struct Fdio *io;

	ENTER(Event_dispatcher_create_fdio);
	ALLOCATE(io, struct Fdio, 1);
	if (io)
	{
		memset(io, 0, sizeof(*io));
		io->event_dispatcher = dispatcher;
		io->descriptor = descriptor;
		io->access_count = 0;
		ADD_OBJECT_TO_LIST(Fdio)
			(io, dispatcher->socket_list);

	}
	else
	{
		display_message(ERROR_MESSAGE, "Event_dispatcher_create_fdio.  "
			"Unable to allocate structure");
	}
	LEAVE;

	return (io);
} /* Event_dispatcher_create_fdio (win32 version) */

int DESTROY(Fdio)(Fdio_id *io)
/*******************************************************************************
LAST MODIFIED : 16 May 2005

DESCRIPTION :
Destroys the IO object. This causes cmgui to forget about the descriptor, but the
descriptor itself must still be closed. This should be called as soon as the
application is notified by the operating system of a closure event.
==============================================================================*/
{
	if ((*io)->read_data.function)
	{
		Fdio_set_read_callback(*io, NULL, NULL);
	}
	else if ((*io)->write_data.function)
	{
		Fdio_set_write_callback(*io, NULL, NULL);
	}

	REMOVE_OBJECT_FROM_LIST(Fdio)
		(*io, (*io)->event_dispatcher->socket_list);

	DEALLOCATE((*io));
	*io = NULL;
	return (1);
} /* DESTROY(Fdio) (win32 version) */

int Fdio_set_read_callback(Fdio_id handle, Fdio_callback callback,
	void *user_data)
/*******************************************************************************
LAST MODIFIED : 16 May 2005

DESCRIPTION :
Sets a read callback on the specified IO handle. This callback is called at
least once after a read function indicates it would block. An application
should not rely upon it being called more than once without attempting a
read between the calls. This read should occur after fdio_set_read_callback
is called. The callback will also be called if the underlying descriptor is
closed by the peer. The callback is not one-shot, and the callback remains in
effect until it is explicitly cancelled.

There may be at most one read callback set per I/O handle at any one time. If
this function is passed NULL as the callback parameter, the read callback
previously set will be cancelled.
==============================================================================*/
{
	ENTER(Fdio_set_read_callback);

	if (callback == NULL)
	{
		handle->read_data.function = NULL;
		handle->wantevents &= ~(FD_READ | FD_CLOSE);
		WSAAsyncSelect(handle->descriptor,
			handle->event_dispatcher->networkWindowHandle,
			handle->wantevents ? UWM_NETWORK : 0,
			handle->wantevents);
	}
	else
	{
		handle->read_data.function = callback;
		handle->read_data.app_user_data = user_data;
		handle->wantevents |= FD_READ | FD_CLOSE;
		WSAAsyncSelect(handle->descriptor,
			handle->event_dispatcher->networkWindowHandle,
			UWM_NETWORK,
			handle->wantevents);
	}

	LEAVE;

	return (1);
} /* Fdio_set_read_callback (win32 version) */

int Fdio_set_write_callback(Fdio_id handle, Fdio_callback callback,
	void *user_data)
/*******************************************************************************
LAST MODIFIED : 16 May 2005

DESCRIPTION :
Sets a write callback on the specified IO handle. This callback is called at
least once after a write function indicates it would block. An application
should not rely upon it being called more than once without attempting a
write between the calls. This write should occur after Fdio_set_write_callback
is called. The callback is not one-shot, and the callback remains in
effect until it is explicitly cancelled.

There may be at most one write callback set per I/O handle at any one time. If
this function is passed NULL as the callback parameter, the write callback
previously set will be cancelled.
==============================================================================*/
{
	ENTER(Fdio_set_write_callback);

	if (callback == NULL)
	{
		handle->write_data.function = NULL;
		handle->wantevents &= ~FD_WRITE;
		WSAAsyncSelect(handle->descriptor,
			handle->event_dispatcher->networkWindowHandle,
			handle->wantevents ? UWM_NETWORK : 0,
			handle->wantevents);
	}
	else
	{
		handle->write_data.function = callback;
		handle->write_data.app_user_data = user_data;
		handle->wantevents |= FD_WRITE;
		WSAAsyncSelect(handle->descriptor,
			handle->event_dispatcher->networkWindowHandle,
			UWM_NETWORK,
			handle->wantevents);
	}

	LEAVE;

	return (1);
} /* Fdio_set_write_callback (win32 version) */

#elif defined(WX_USER_INTERFACE)

Fdio_id Event_dispatcher_create_Fdio(struct Event_dispatcher *dispatcher,
	Cmiss_native_socket_t descriptor)
/*******************************************************************************
LAST MODIFIED : 16 May 2005

DESCRIPTION :
Creates a new Fdio, given an event dispatcher and a descriptor.
==============================================================================*/
{
	struct Fdio *io;

	ENTER(Event_dispatcher_create_fdio);
	ALLOCATE(io, struct Fdio, 1);
	if (io)
	{
		memset(io, 0, sizeof(*io));
		io->event_dispatcher = dispatcher;
		io->descriptor = descriptor;
		io->access_count = 0;
	}
	else
	{
		display_message(ERROR_MESSAGE, "Event_dispatcher_create_fdio.  "
			"Unable to allocate structure");
	}

	//	io->iochannel = g_io_channel_unix_new(descriptor);
	//	io->read_source_tag = 0;
	// io->write_source_tag = 0;

	LEAVE;

	return (io);
} /* Event_dispatcher_create_fdio (glib) */

int DESTROY(Fdio)(Fdio_id *io)
/*******************************************************************************
LAST MODIFIED : 16 May 2005

DESCRIPTION :
Destroys the IO object. This causes cmgui to forget about the descriptor, but the
descriptor itself must still be closed. This should be called as soon as the
application is notified by the operating system of a closure event.
==============================================================================*/
{
	if ((*io)->read_data.function)
	{
		Fdio_set_read_callback(*io, NULL, NULL);
	}
	else if ((*io)->write_data.function)
	{
		Fdio_set_write_callback(*io, NULL, NULL);
	}

	// g_io_channel_unref((*io)->iochannel);

	// if ((*io)->read_source_tag != 0)
	//	g_source_remove((*io)->read_source_tag);
	//if ((*io)->write_source_tag != 0)
	//	g_source_remove((*io)->write_source_tag);

	DEALLOCATE((*io));
	*io = NULL;
	return (1);
} /* DESTROY(Fdio) (glib) */

int Fdio_set_read_callback(Fdio_id handle, Fdio_callback callback,
	void *user_data)
/*******************************************************************************
LAST MODIFIED : 16 May 2005

DESCRIPTION :
Sets a read callback on the specified IO handle. This callback is called at
least once after a read function indicates it would block. An application
should not rely upon it being called more than once without attempting a
read between the calls. This read should occur after fdio_set_read_callback
is called. The callback will also be called if the underlying descriptor is
closed by the peer. The callback is not one-shot, and the callback remains in
effect until it is explicitly cancelled.

There may be at most one read callback set per I/O handle at any one time. If
this function is passed NULL as the callback parameter, the read callback
previously set will be cancelled.
==============================================================================*/
{
	ENTER(Fdio_set_read_callback);

	if (callback == NULL)
	{
		handle->read_data.function = NULL;
		//		if (handle->read_source_tag != 0)
		//{
		//	g_source_remove(handle->read_source_tag);
		//	handle->read_source_tag = 0;
		//}
	}
	else
	{
		handle->read_data.function = callback;
		handle->read_data.app_user_data = user_data;
		//if (handle->read_source_tag == 0)
		//	handle->read_source_tag =
		//		g_io_add_watch(handle->iochannel, G_IO_IN | G_IO_HUP,
		//			Fdio_glib_io_callback, handle);
	}

	LEAVE;

	return (1);
} /* Fdio_set_read_callback (glib version) */

int Fdio_set_write_callback(Fdio_id handle, Fdio_callback callback,
	void *user_data)
/*******************************************************************************
LAST MODIFIED : 16 May 2005

DESCRIPTION :
Sets a write callback on the specified IO handle. This callback is called at
least once after a write function indicates it would block. An application
should not rely upon it being called more than once without attempting a
write between the calls. This write should occur after Fdio_set_write_callback
is called. The callback is not one-shot, and the callback remains in
effect until it is explicitly cancelled.

There may be at most one write callback set per I/O handle at any one time. If
this function is passed NULL as the callback parameter, the write callback
previously set will be cancelled.
==============================================================================*/
{
	ENTER(Fdio_set_write_callback);

	if (callback == NULL)
	{
		handle->write_data.function = NULL;
		//if (handle->write_source_tag != 0)
		//{
		//	g_source_remove(handle->write_source_tag);
		//	handle->write_source_tag = 0;
		//}
	}
	else
	{
		handle->write_data.function = callback;
		handle->write_data.app_user_data = user_data;
		//if (handle->write_source_tag == 0)
		//	handle->write_source_tag =
		//		g_io_add_watch(handle->iochannel, G_IO_OUT,
		//			Fdio_glib_io_callback, handle);
	}

	LEAVE;

	return (1);
} /* Fdio_set_write_callback (glib version) */
#elif defined(USE_GTK_MAIN_STEP)

Fdio_id Event_dispatcher_create_Fdio(struct Event_dispatcher *dispatcher,
	Cmiss_native_socket_t descriptor)
/*******************************************************************************
LAST MODIFIED : 16 May 2005

DESCRIPTION :
Creates a new Fdio, given an event dispatcher and a descriptor.
==============================================================================*/
{
	struct Fdio *io;

	ENTER(Event_dispatcher_create_fdio);
	ALLOCATE(io, struct Fdio, 1);
	if (io)
	{
		memset(io, 0, sizeof(*io));
		io->event_dispatcher = dispatcher;
		io->descriptor = descriptor;
		io->access_count = 0;
	}
	else
	{
		display_message(ERROR_MESSAGE, "Event_dispatcher_create_fdio.  "
			"Unable to allocate structure");
	}

	io->iochannel = g_io_channel_unix_new(descriptor);
	io->read_source_tag = 0;
	io->write_source_tag = 0;

	LEAVE;

	return (io);
} /* Event_dispatcher_create_fdio (glib) */

int DESTROY(Fdio)(Fdio_id *io)
/*******************************************************************************
LAST MODIFIED : 16 May 2005

DESCRIPTION :
Destroys the IO object. This causes cmgui to forget about the descriptor, but the
descriptor itself must still be closed. This should be called as soon as the
application is notified by the operating system of a closure event.
==============================================================================*/
{
	if ((*io)->read_data.function)
	{
		Fdio_set_read_callback(*io, NULL, NULL);
	}
	else if ((*io)->write_data.function)
	{
		Fdio_set_write_callback(*io, NULL, NULL);
	}

	g_io_channel_unref((*io)->iochannel);

	if ((*io)->read_source_tag != 0)
		g_source_remove((*io)->read_source_tag);
	if ((*io)->write_source_tag != 0)
		g_source_remove((*io)->write_source_tag);

	DEALLOCATE((*io));
	*io = NULL;
	return (1);
} /* DESTROY(Fdio) (glib) */

static gboolean Fdio_glib_io_callback(GIOChannel *source,
	GIOCondition condition, gpointer data)
/*******************************************************************************
LAST MODIFIED : 16 May 2005

DESCRIPTION :
Called from glib whenever a socket is ready to read/write.
==============================================================================*/
{
	Fdio_id fdio;
	gboolean ret = FALSE;

	ENTER(Fdio_glib_io_callback);
	USE_PARAMETER(source);

	fdio = (Fdio_id)data;
	if ((condition & (G_IO_IN | G_IO_HUP)) &&
		fdio->read_data.function)
	{
		fdio->read_data.function(fdio,
			fdio->read_data.app_user_data);
		ret = TRUE;
	}
	else if ((condition & G_IO_OUT) &&
		fdio->write_data.function)
	{
		fdio->write_data.function(fdio,
			fdio->write_data.app_user_data);
		ret = TRUE;
	}

	if ((condition & (G_IO_IN | G_IO_HUP)) &&
	    !fdio->read_data.function)
		fdio->read_source_tag = 0;
	else if ((condition & G_IO_OUT) &&
		!fdio->write_data.function)
		fdio->write_source_tag = 0;

	LEAVE;

	return (ret);
} /* Fdio_glib_io_callback */

int Fdio_set_read_callback(Fdio_id handle, Fdio_callback callback,
	void *user_data)
/*******************************************************************************
LAST MODIFIED : 16 May 2005

DESCRIPTION :
Sets a read callback on the specified IO handle. This callback is called at
least once after a read function indicates it would block. An application
should not rely upon it being called more than once without attempting a
read between the calls. This read should occur after fdio_set_read_callback
is called. The callback will also be called if the underlying descriptor is
closed by the peer. The callback is not one-shot, and the callback remains in
effect until it is explicitly cancelled.

There may be at most one read callback set per I/O handle at any one time. If
this function is passed NULL as the callback parameter, the read callback
previously set will be cancelled.
==============================================================================*/
{
	ENTER(Fdio_set_read_callback);

	if (callback == NULL)
	{
		handle->read_data.function = NULL;
		if (handle->read_source_tag != 0)
		{
			g_source_remove(handle->read_source_tag);
			handle->read_source_tag = 0;
		}
	}
	else
	{
		handle->read_data.function = callback;
		handle->read_data.app_user_data = user_data;
		if (handle->read_source_tag == 0)
			handle->read_source_tag = g_io_add_watch(handle->iochannel,
				static_cast<GIOCondition>(G_IO_IN | G_IO_HUP),
				Fdio_glib_io_callback, handle);
	}

	LEAVE;

	return (1);
} /* Fdio_set_read_callback (glib version) */

int Fdio_set_write_callback(Fdio_id handle, Fdio_callback callback,
	void *user_data)
/*******************************************************************************
LAST MODIFIED : 16 May 2005

DESCRIPTION :
Sets a write callback on the specified IO handle. This callback is called at
least once after a write function indicates it would block. An application
should not rely upon it being called more than once without attempting a
write between the calls. This write should occur after Fdio_set_write_callback
is called. The callback is not one-shot, and the callback remains in
effect until it is explicitly cancelled.

There may be at most one write callback set per I/O handle at any one time. If
this function is passed NULL as the callback parameter, the write callback
previously set will be cancelled.
==============================================================================*/
{
	ENTER(Fdio_set_write_callback);

	if (callback == NULL)
	{
		handle->write_data.function = NULL;
		if (handle->write_source_tag != 0)
		{
			g_source_remove(handle->write_source_tag);
			handle->write_source_tag = 0;
		}
	}
	else
	{
		handle->write_data.function = callback;
		handle->write_data.app_user_data = user_data;
		if (handle->write_source_tag == 0)
			handle->write_source_tag =
				g_io_add_watch(handle->iochannel, G_IO_OUT,
					Fdio_glib_io_callback, handle);
	}

	LEAVE;

	return (1);
} /* Fdio_set_write_callback (glib version) */
#elif defined(CARBON_USER_INTERFACE)

Fdio_id Event_dispatcher_create_Fdio(struct Event_dispatcher *dispatcher,
	Cmiss_native_socket_t descriptor)
/*******************************************************************************
LAST MODIFIED : 16 May 2005

DESCRIPTION :
Creates a new Fdio, given an event dispatcher and a descriptor.
==============================================================================*/
{
	struct Fdio *io;

	ENTER(Event_dispatcher_create_fdio);
	ALLOCATE(io, struct Fdio, 1);
	if (io)
	{
		memset(io, 0, sizeof(*io));
		io->event_dispatcher = dispatcher;
		io->descriptor = descriptor;
		io->access_count = 0;
	}
	else
	{
		display_message(ERROR_MESSAGE, "Event_dispatcher_create_fdio.  "
			"Unable to allocate structure");
	}

	//	io->iochannel = g_io_channel_unix_new(descriptor);
	//	io->read_source_tag = 0;
	// io->write_source_tag = 0;

	LEAVE;

	return (io);
} /* Event_dispatcher_create_fdio (glib) */

int DESTROY(Fdio)(Fdio_id *io)
/*******************************************************************************
LAST MODIFIED : 16 May 2005

DESCRIPTION :
Destroys the IO object. This causes cmgui to forget about the descriptor, but the
descriptor itself must still be closed. This should be called as soon as the
application is notified by the operating system of a closure event.
==============================================================================*/
{
	if ((*io)->read_data.function)
	{
		Fdio_set_read_callback(*io, NULL, NULL);
	}
	else if ((*io)->write_data.function)
	{
		Fdio_set_write_callback(*io, NULL, NULL);
	}

	// g_io_channel_unref((*io)->iochannel);

	// if ((*io)->read_source_tag != 0)
	//	g_source_remove((*io)->read_source_tag);
	//if ((*io)->write_source_tag != 0)
	//	g_source_remove((*io)->write_source_tag);

	DEALLOCATE((*io));
	*io = NULL;
	return (1);
} /* DESTROY(Fdio) (glib) */

int Fdio_set_read_callback(Fdio_id handle, Fdio_callback callback,
	void *user_data)
/*******************************************************************************
LAST MODIFIED : 16 May 2005

DESCRIPTION :
Sets a read callback on the specified IO handle. This callback is called at
least once after a read function indicates it would block. An application
should not rely upon it being called more than once without attempting a
read between the calls. This read should occur after fdio_set_read_callback
is called. The callback will also be called if the underlying descriptor is
closed by the peer. The callback is not one-shot, and the callback remains in
effect until it is explicitly cancelled.

There may be at most one read callback set per I/O handle at any one time. If
this function is passed NULL as the callback parameter, the read callback
previously set will be cancelled.
==============================================================================*/
{
	ENTER(Fdio_set_read_callback);

	if (callback == NULL)
	{
		handle->read_data.function = NULL;
		//		if (handle->read_source_tag != 0)
		//{
		//	g_source_remove(handle->read_source_tag);
		//	handle->read_source_tag = 0;
		//}
	}
	else
	{
		handle->read_data.function = callback;
		handle->read_data.app_user_data = user_data;
		//if (handle->read_source_tag == 0)
		//	handle->read_source_tag =
		//		g_io_add_watch(handle->iochannel, G_IO_IN | G_IO_HUP,
		//			Fdio_glib_io_callback, handle);
	}

	LEAVE;

	return (1);
} /* Fdio_set_read_callback (glib version) */

int Fdio_set_write_callback(Fdio_id handle, Fdio_callback callback,
	void *user_data)
/*******************************************************************************
LAST MODIFIED : 16 May 2005

DESCRIPTION :
Sets a write callback on the specified IO handle. This callback is called at
least once after a write function indicates it would block. An application
should not rely upon it being called more than once without attempting a
write between the calls. This write should occur after Fdio_set_write_callback
is called. The callback is not one-shot, and the callback remains in
effect until it is explicitly cancelled.

There may be at most one write callback set per I/O handle at any one time. If
this function is passed NULL as the callback parameter, the write callback
previously set will be cancelled.
==============================================================================*/
{
	ENTER(Fdio_set_write_callback);

	if (callback == NULL)
	{
		handle->write_data.function = NULL;
		//if (handle->write_source_tag != 0)
		//{
		//	g_source_remove(handle->write_source_tag);
		//	handle->write_source_tag = 0;
		//}
	}
	else
	{
		handle->write_data.function = callback;
		handle->write_data.app_user_data = user_data;
		//if (handle->write_source_tag == 0)
		//	handle->write_source_tag =
		//		g_io_add_watch(handle->iochannel, G_IO_OUT,
		//			Fdio_glib_io_callback, handle);
	}

	LEAVE;

	return (1);
} /* Fdio_set_write_callback (glib version) */
#elif defined(USE_GTK_MAIN_STEP)

Fdio_id Event_dispatcher_create_Fdio(struct Event_dispatcher *dispatcher,
	Cmiss_native_socket_t descriptor)
/*******************************************************************************
LAST MODIFIED : 16 May 2005

DESCRIPTION :
Creates a new Fdio, given an event dispatcher and a descriptor.
==============================================================================*/
{
	struct Fdio *io;

	ENTER(Event_dispatcher_create_fdio);
	ALLOCATE(io, struct Fdio, 1);
	if (io)
	{
		memset(io, 0, sizeof(*io));
		io->event_dispatcher = dispatcher;
		io->descriptor = descriptor;
		io->access_count = 0;
	}
	else
	{
		display_message(ERROR_MESSAGE, "Event_dispatcher_create_fdio.  "
			"Unable to allocate structure");
	}

	io->iochannel = g_io_channel_unix_new(descriptor);
	io->read_source_tag = 0;
	io->write_source_tag = 0;

	LEAVE;

	return (io);
} /* Event_dispatcher_create_fdio (glib) */

int DESTROY(Fdio)(Fdio_id *io)
/*******************************************************************************
LAST MODIFIED : 16 May 2005

DESCRIPTION :
Destroys the IO object. This causes cmgui to forget about the descriptor, but the
descriptor itself must still be closed. This should be called as soon as the
application is notified by the operating system of a closure event.
==============================================================================*/
{
	if ((*io)->read_data.function)
	{
		Fdio_set_read_callback(*io, NULL, NULL);
	}
	else if ((*io)->write_data.function)
	{
		Fdio_set_write_callback(*io, NULL, NULL);
	}

	g_io_channel_unref((*io)->iochannel);

	if ((*io)->read_source_tag != 0)
		g_source_remove((*io)->read_source_tag);
	if ((*io)->write_source_tag != 0)
		g_source_remove((*io)->write_source_tag);

	DEALLOCATE((*io));
	*io = NULL;
	return (1);
} /* DESTROY(Fdio) (glib) */

static gboolean Fdio_glib_io_callback(GIOChannel *source,
	GIOCondition condition, gpointer data)
/*******************************************************************************
LAST MODIFIED : 16 May 2005

DESCRIPTION :
Called from glib whenever a socket is ready to read/write.
==============================================================================*/
{
	Fdio_id fdio;
	gboolean ret = FALSE;

	ENTER(Fdio_glib_io_callback);
	USE_PARAMETER(source);

	fdio = (Fdio_id)data;
	if ((condition & (G_IO_IN | G_IO_HUP)) &&
		fdio->read_data.function)
	{
		fdio->read_data.function(fdio,
			fdio->read_data.app_user_data);
		ret = TRUE;
	}
	else if ((condition & G_IO_OUT) &&
		fdio->write_data.function)
	{
		fdio->write_data.function(fdio,
			fdio->write_data.app_user_data);
		ret = TRUE;
	}

	if ((condition & (G_IO_IN | G_IO_HUP)) &&
	    !fdio->read_data.function)
		fdio->read_source_tag = 0;
	else if ((condition & G_IO_OUT) &&
		!fdio->write_data.function)
		fdio->write_source_tag = 0;

	LEAVE;

	return (ret);
} /* Fdio_glib_io_callback */

int Fdio_set_read_callback(Fdio_id handle, Fdio_callback callback,
	void *user_data)
/*******************************************************************************
LAST MODIFIED : 16 May 2005

DESCRIPTION :
Sets a read callback on the specified IO handle. This callback is called at
least once after a read function indicates it would block. An application
should not rely upon it being called more than once without attempting a
read between the calls. This read should occur after fdio_set_read_callback
is called. The callback will also be called if the underlying descriptor is
closed by the peer. The callback is not one-shot, and the callback remains in
effect until it is explicitly cancelled.

There may be at most one read callback set per I/O handle at any one time. If
this function is passed NULL as the callback parameter, the read callback
previously set will be cancelled.
==============================================================================*/
{
	ENTER(Fdio_set_read_callback);

	if (callback == NULL)
	{
		handle->read_data.function = NULL;
		if (handle->read_source_tag != 0)
		{
			g_source_remove(handle->read_source_tag);
			handle->read_source_tag = 0;
		}
	}
	else
	{
		handle->read_data.function = callback;
		handle->read_data.app_user_data = user_data;
		if (handle->read_source_tag == 0)
			handle->read_source_tag =
				g_io_add_watch(handle->iochannel, G_IO_IN | G_IO_HUP,
					Fdio_glib_io_callback, handle);
	}

	LEAVE;

	return (1);
} /* Fdio_set_read_callback (glib version) */

int Fdio_set_write_callback(Fdio_id handle, Fdio_callback callback,
	void *user_data)
/*******************************************************************************
LAST MODIFIED : 16 May 2005

DESCRIPTION :
Sets a write callback on the specified IO handle. This callback is called at
least once after a write function indicates it would block. An application
should not rely upon it being called more than once without attempting a
write between the calls. This write should occur after Fdio_set_write_callback
is called. The callback is not one-shot, and the callback remains in
effect until it is explicitly cancelled.

There may be at most one write callback set per I/O handle at any one time. If
this function is passed NULL as the callback parameter, the write callback
previously set will be cancelled.
==============================================================================*/
{
	ENTER(Fdio_set_write_callback);

	if (callback == NULL)
	{
		handle->write_data.function = NULL;
		if (handle->write_source_tag != 0)
		{
			g_source_remove(handle->write_source_tag);
			handle->write_source_tag = 0;
		}
	}
	else
	{
		handle->write_data.function = callback;
		handle->write_data.app_user_data = user_data;
		if (handle->write_source_tag == 0)
			handle->write_source_tag =
				g_io_add_watch(handle->iochannel, G_IO_OUT,
					Fdio_glib_io_callback, handle);
	}

	LEAVE;

	return (1);
} /* Fdio_set_write_callback (glib version) */
#elif defined(USE_XTAPP_CONTEXT)

Fdio_id Event_dispatcher_create_Fdio(struct Event_dispatcher *dispatcher,
	Cmiss_native_socket_t descriptor)
/*******************************************************************************
LAST MODIFIED : 16 May 2005

DESCRIPTION :
Creates a new Fdio, given an event dispatcher and a descriptor.
==============================================================================*/
{
	struct Fdio *io;

	ENTER(Event_dispatcher_create_fdio);
	ALLOCATE(io, struct Fdio, 1);
	if (io)
	{
		memset(io, 0, sizeof(*io));
		io->event_dispatcher = dispatcher;
		io->descriptor = descriptor;
		io->access_count = 0;
	}
	else
	{
		display_message(ERROR_MESSAGE, "Event_dispatcher_create_fdio.  "
			"Unable to allocate structure");
	}

	io->read_input = NULL;
	io->write_input = NULL;

	LEAVE;

	return (io);
} /* Event_dispatcher_create_fdio (Xt) */

int DESTROY(Fdio)(Fdio_id *io)
/*******************************************************************************
LAST MODIFIED : 16 May 2005

DESCRIPTION :
Destroys the IO object. This causes cmgui to forget about the descriptor, but the
descriptor itself must still be closed. This should be called as soon as the
application is notified by the operating system of a closure event.
==============================================================================*/
{
	if ((*io)->read_data.function)
	{
		Fdio_set_read_callback(*io, NULL, NULL);
	}
	else if ((*io)->write_data.function)
	{
		Fdio_set_write_callback(*io, NULL, NULL);
	}

	if ((*io)->read_input)
	{
		XtRemoveInput((*io)->read_input);
	}

	if ((*io)->write_input)
	{
		XtRemoveInput((*io)->write_input);
	}

	DEALLOCATE((*io));
	*io = NULL;
	return (1);
} /* DESTROY(Fdio) (Xt) */

static void Fdio_Xt_io_callback(XtPointer user_data, int* sourcefd,
	XtInputId *input)
/*******************************************************************************
LAST MODIFIED : 16 May 2005

DESCRIPTION :
Called from Xt whenever a socket is ready to read/write.
==============================================================================*/
{
	Fdio_id fdio = (Fdio_id)user_data;

	if (input == io->read_input &&
		fdio->read_data.function)
		fdio->read_data.function(fdio, fdio->read_data.app_user_data);
	else if (input == io->write_input &&
		fdio->write_data.function)
		fdio->write_data.function(fdio, fdio->write_data.app_user_data);
}

int Fdio_set_read_callback(Fdio_id handle, Fdio_callback callback,
	void *user_data)
/*******************************************************************************
LAST MODIFIED : 16 May 2005

DESCRIPTION :
Sets a read callback on the specified IO handle. This callback is called at
least once after a read function indicates it would block. An application
should not rely upon it being called more than once without attempting a
read between the calls. This read should occur after fdio_set_read_callback
is called. The callback will also be called if the underlying descriptor is
closed by the peer. The callback is not one-shot, and the callback remains in
effect until it is explicitly cancelled.

There may be at most one read callback set per I/O handle at any one time. If
this function is passed NULL as the callback parameter, the read callback
previously set will be cancelled.
==============================================================================*/
{
	ENTER(Fdio_set_read_callback);

	if (callback == NULL)
	{
		handle->read_data.function = NULL;
		if (handle->read_input)
		{
			XtRemoveInput(handle->read_input);
			handle->read_input = NULL;
		}
	}
	else
	{
		handle->read_data.function = callback;
		handle->read_data.app_user_data = user_data;
		if (handle->read_input == 0)
			handle->read_input = XtAppAddInput(handle->event_dispatcher
				->application_context, handle->descriptor,
				(XtPointer)(XtInputReadMask),
				Fdio_Xt_io_callback, handle);
	}

	LEAVE;

	return (1);
} /* Fdio_set_read_callback (Xt version) */

int Fdio_set_write_callback(Fdio_id handle, Fdio_callback callback,
	void *user_data)
/*******************************************************************************
LAST MODIFIED : 16 May 2005

DESCRIPTION :
Sets a write callback on the specified IO handle. This callback is called at
least once after a write function indicates it would block. An application
should not rely upon it being called more than once without attempting a
write between the calls. This write should occur after Fdio_set_write_callback
is called. The callback is not one-shot, and the callback remains in
effect until it is explicitly cancelled.

There may be at most one write callback set per I/O handle at any one time. If
this function is passed NULL as the callback parameter, the write callback
previously set will be cancelled.
==============================================================================*/
{
	ENTER(Fdio_set_read_callback);

	if (callback == NULL)
	{
		handle->write_data.function = NULL;
		if (handle->write_input)
		{
			XtRemoveInput(handle->write_input);
			handle->write_input = NULL;
		}
	}
	else
	{
		handle->write_data.function = callback;
		handle->write_data.app_user_data = user_data;
		if (handle->write_input == 0)
			handle->write_input = XtAppAddInput(handle->event_dispatcher
				->application_context, handle->descriptor,
				(XtPointer)(XtInputWriteMask),
				Fdio_Xt_io_callback, handle);
	}

	LEAVE;

	return (1);
} /* Fdio_set_write_callback (Xt version) */

#else
#error You are not using GENERIC_EVENT_DISPATCHER, WIN32_USER_INTERFACE, or USE_GTK_MAIN_STEP. Implement your platform in event_dispatcher.c
#endif /* defined(USE_GENERIC_EVENT_DISPATCHER) elif (WIN32_USER_INTERFACE|USE_GTK_MAIN_STEP) */
