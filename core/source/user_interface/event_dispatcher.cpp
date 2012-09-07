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
#if defined (BUILD_WITH_CMAKE)
#include "configure/zinc_configure.h"
#endif /* defined (BUILD_WITH_CMAKE) */
//-- extern "C" {
#include <math.h>
#include <stdio.h>
#include "general/time.h"
#include "general/compare.h"
#include "general/debug.h"
#include "general/list.h"
#include "general/indexed_list_private.h"
#include "general/object.h"
#include "general/message.h"
#include "user_interface/event_dispatcher.h"
//-- }

/* After the event_dispatcher.h has set up these variables */
#if defined (WX_USER_INTERFACE) /* switch (USER_INTERFACE) */
#include <wx/wx.h>
#include <wx/apptrait.h>
//-- extern "C" {
#include "user_interface/user_interface.h"
//-- }
#elif defined (WIN32_USER_INTERFACE) /* switch (USER_INTERFACE) */
//-- extern "C" {
//#define WINDOWS_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include "general/callback.h"
//-- }
#elif defined (CARBON_USER_INTERFACE) /* switch (USER_INTERFACE) */
#include "carbon/carbon.h"
#elif defined (USE_GTK_MAIN_STEP) /* switch (USER_INTERFACE) */
//-- extern "C" {
#include <gtk/gtk.h>
//-- }
#endif /* switch (USER_INTERFACE) */

/*
Module types
------------
*/

class wxEventTimer;

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
#if defined (CARBON_USER_INTERFACE)
	EventLoopTimerRef carbon_timer_ref;
#endif /* defined (CARBON_USER_INTERFACE) */
#if defined (USE_GTK_MAIN_STEP)
	guint gtk_timeout_id;
#endif /* defined (USE_GTK_MAIN_STEP) */
#if defined (WX_USER_INTERFACE)
	wxEventTimer *wx_timer;
#endif /* defined (WX_USER_INTERFACE) */
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
	int special_idle_callback_pending;
	struct Event_dispatcher_idle_callback *special_idle_callback;
#if defined (WIN32_USER_INTERFACE)
	HWND networkWindowHandle;
#endif /* defined (WIN32_USER_INTERFACE) */
};

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
#if defined (CARBON_USER_INTERFACE)
		timeout_callback->carbon_timer_ref = (EventLoopTimerRef)NULL;
#endif /* defined (CARBON_USER_INTERFACE) */
#if defined (WX_USER_INTERFACE)
		timeout_callback->wx_timer = (wxEventTimer *)NULL;
#endif /* defined (WX_USER_INTERFACE) */
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
			else if (((idle_one->timestamp).dwHighDateTime>
				(idle_two->timestamp).dwHighDateTime)||
				(((idle_one->timestamp).dwHighDateTime==
				(idle_two->timestamp).dwHighDateTime)&&
				((idle_one->timestamp).dwLowDateTime>
				(idle_two->timestamp).dwLowDateTime)))
#else /* defined (WIN32_SYSTEM) */
			else if (idle_one->timestamp > idle_two->timestamp)
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

#if defined (USE_GTK_MAIN_STEP)
gboolean Event_dispatcher_gtk_idle_callback(
	gpointer idle_callback_void)
/*******************************************************************************
LAST MODIFIED : 10 July 2002

DESCRIPTION :
==============================================================================*/
{
	gboolean return_code = FALSE;
	struct Event_dispatcher_idle_callback *idle_callback;

	ENTER(Event_dispatcher_gtk_idle_callback);
	idle_callback = (struct Event_dispatcher_idle_callback *)idle_callback_void;
	if (idle_callback)
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
			display_message(ERROR_MESSAGE,
				"Event_dispatcher_carbon_idle_callback.  Callback function failed.");
		}
		else
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
		idle_callback = FIRST_OBJECT_IN_LIST_THAT(Event_dispatcher_idle_callback)
			((LIST_CONDITIONAL_FUNCTION(Event_dispatcher_idle_callback) *)NULL,
				(void *)NULL, event_dispatcher->idle_list);
		if(idle_callback != NULL)
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
		event_dispatcher->networkWindowHandle = (HWND)NULL;
#endif /* defined (WIN32_USER_INTERFACE) */
#if defined (USE_GENERIC_EVENT_DISPATCHER)
		event_dispatcher->descriptor_list = 
			CREATE(LIST(Event_dispatcher_descriptor_callback))();
#endif /* defined (USE_GENERIC_EVENT_DISPATCHER) */
		event_dispatcher->timeout_list = 
			CREATE(LIST(Event_dispatcher_timeout_callback))();
		event_dispatcher->idle_list = 
			CREATE(LIST(Event_dispatcher_idle_callback))();
		event_dispatcher->special_idle_callback_pending = 0;
		event_dispatcher->special_idle_callback = 
			(struct Event_dispatcher_idle_callback *)NULL;
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
#if defined (WIN32_USER_INTERFACE)
		if (event_dispatcher->socket_list)
		{
			DESTROY(LIST(Fdio))
				(&event_dispatcher->socket_list);
		}
		if (event_dispatcher->networkWindowHandle)
		{
			DestroyWindow(event_dispatcher->networkWindowHandle);
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
		if (event_dispatcher->special_idle_callback)
		{
			DEACCESS(Event_dispatcher_idle_callback)(
				&event_dispatcher->special_idle_callback);
		}

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
		callback = CREATE(Event_dispatcher_descriptor_callback)(
			query_function, check_function, dispatch_function, user_data);
		if (callback)
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

	void Notify()
	{
		(*timeout_callback->timeout_function)(
			timeout_callback->user_data);
		delete this;
	}
	
public:
	wxEventTimer(struct Event_dispatcher_timeout_callback *timeout_callback):
		timeout_callback(timeout_callback)
	{
	}

	~wxEventTimer()
	{
		DESTROY(Event_dispatcher_timeout_callback)(&timeout_callback);
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
		timeout_callback = CREATE(Event_dispatcher_timeout_callback)(
			timeout_s, timeout_ns, timeout_function, user_data);
		if (timeout_callback != NULL)
		{
			timeout_callback->wx_timer = new wxEventTimer(timeout_callback);
			timeout_callback->wx_timer->Start(timeout_s * 1000 + timeout_ns / 1000000, /*OneShot*/true);
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
		timeout_callback = CREATE(Event_dispatcher_timeout_callback)(
			timeout_s, timeout_ns, timeout_function, user_data);
		if (timeout_callback)
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
		timeout_callback = CREATE(Event_dispatcher_timeout_callback)(
					timeout_s, timeout_ns, timeout_function, user_data);
		if (timeout_callback)
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

int Event_dispatcher_remove_timeout_callback(
	struct Event_dispatcher *event_dispatcher, 
	struct Event_dispatcher_timeout_callback *callback_id)
/*******************************************************************************
LAST MODIFIED : 6 March 2002

DESCRIPTION :
==============================================================================*/
{
	int return_code = 0;

	ENTER(Event_dispatcher_remove_timeout_callback);

	if (event_dispatcher && event_dispatcher->timeout_list && callback_id)
	{
#if defined (USE_GTK_MAIN_STEP)
		gtk_timeout_remove(callback_id->gtk_timeout_id);
#elif defined (CARBON_USER_INTERFACE)
		RemoveEventLoopTimer(callback_id->carbon_timer_ref);
		callback_id->carbon_timer_ref = (EventLoopTimerRef)NULL;
#elif defined (WX_USER_INTERFACE)
		delete callback_id->wx_timer;
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
		idle_callback = CREATE(Event_dispatcher_idle_callback)(
			idle_function, user_data, priority);
		if (idle_callback != NULL)
		{
			if (!(ADD_OBJECT_TO_LIST(Event_dispatcher_idle_callback)(
				idle_callback, event_dispatcher->idle_list)))
			{
				DESTROY(Event_dispatcher_idle_callback)(&idle_callback);
				idle_callback = (struct Event_dispatcher_idle_callback *)NULL;
			}
#if defined (WIN32_SYSTEM)
			else
			{
				//Event_dispatcher_ensure_network_window(event_dispatcher);
				//if(!(PostMessage(event_dispatcher->networkWindowHandle,
				//		UWM_IDLE, 0, (LPARAM)idle_callback)))
				//{
				//	display_message(ERROR_MESSAGE,
				//		"Event_dispatcher_add_idle_event_callback.  "
				//		"Could not post idle_callback message.");
				//	idle_callback = (struct Event_dispatcher_idle_callback *)NULL;
				//}
				//else
				//{
				//	ACCESS(Event_dispatcher_idle_callback)(idle_callback);
				//}
			}
#endif /* defined (WIN32_SYSTEM) */
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

	if (event_dispatcher && idle_function)
	{
		idle_callback = CREATE(Event_dispatcher_idle_callback)(
			idle_function, user_data, priority);
		if (idle_callback != NULL) 
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
#if defined (USE_GTK_MAIN_STEP)
		gtk_idle_remove(callback_id->gtk_idle_id);
#elif defined (CARBON_USER_INTERFACE)
		RemoveEventLoopTimer(callback_id->carbon_timer_ref);
		callback_id->carbon_timer_ref = (EventLoopTimerRef)NULL;
#endif /* defined (USE_GTK_MAIN_STEP) */
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

/* This implementation is so dependent on USE_GENERIC_EVENT_DISPATCHER that it is
 * easier to write the whole thing again for each option than to put
 * preprocessor conditionals in every single function.
*/
#if defined(USE_GENERIC_EVENT_DISPATCHER)
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
#else
#error You are not using GENERIC_EVENT_DISPATCHER, WIN32_USER_INTERFACE, or USE_GTK_MAIN_STEP. Implement your platform in event_dispatcher.cpp
#endif /* defined(USE_GENERIC_EVENT_DISPATCHER) elif (WIN32_USER_INTERFACE|USE_GTK_MAIN_STEP) */

int Event_dispatcher_process_idle_event(struct Event_dispatcher *event_dispatcher)
{
	USE_PARAMETER(event_dispatcher);
#if defined (WX_USER_INTERFACE)
	return Event_dispatcher_do_idle_event(event_dispatcher);
#endif
	return 0;
}
