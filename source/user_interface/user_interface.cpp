/*******************************************************************************
FILE : user_interface.c

LAST MODIFIED : 26 May 2005

DESCRIPTION :
Functions for opening and closing the user interface.
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
#include <stddef.h>
#include <stdlib.h>
/*???debug */
#include <stdio.h>
#include <ctype.h>
#if defined (CONSOLE_USER_INTERFACE)
#include <unistd.h>
#endif /* defined (CONSOLE_USER_INTERFACE) */
#if __GLIBC__ >= 2
#include <gnu/libc-version.h>
#endif
#if defined (MOTIF)
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/Xlib.h>
#include <X11/cursorfont.h>
#include <Xm/Xm.h>
#include <Xm/RowColumn.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Mrm/MrmPublic.h>
#endif /* defined (MOTIF) */
#include "general/debug.h"
#include "general/myio.h"
#if defined (MOTIF)
#if defined (EXT_INPUT)
#include "io_devices/input_module.h"
#endif /* defined (EXT_INPUT) */
#endif /* defined (MOTIF) */
#if defined (LINK_CMISS)
#include "link/cmiss.h"
#endif /* defined (LINK_CMISS) */
#if defined (MOTIF)
#if !defined (USE_XTAPP_CONTEXT)
#include "user_interface/call_work_procedures.h"
#endif /* !defined (USE_XTAPP_CONTEXT) */
#endif /* defined (MOTIF) */
#if defined (GTK_USER_INTERFACE)
#include <gtk/gtk.h>
#include <glib.h>
#endif /* defined (GTK_USER_INTERFACE) */
#include "user_interface/event_dispatcher.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/* These functions are not ANSI so don't get included in stdlib.h */
extern long a64l(const char *);
extern char *l64a(long);

/*
Module types
------------
*/
struct User_interface
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
==============================================================================*/
{
#if defined (WIN32_USER_INTERFACE)
	HINSTANCE instance;
	HWND main_window;
	int main_window_state,widget_spacing;
	LPSTR command_line;
#else /* defined (WIN32_USER_INTERFACE) */
	char *application_name,**argv,*class_name;
	int *argc_address;
#endif /* defined (WIN32_USER_INTERFACE) */
#if defined (MOTIF) /* switch (USER_INTERFACE) */
	Cursor busy_cursor;
	Display *display;
	int screen_height,screen_width,widget_spacing;
	/* to avoid large gaps on the right of cascade buttons (option menus) */
	Pixmap no_cascade_pixmap;
	/* for communication with other applications */
	Property_notify_callback property_notify_callback;
	void *property_notify_data;
	Widget property_notify_widget;
	Widget application_shell;
	XFontStruct *button_font,*heading_font,*list_font,*menu_font,*normal_font,
		*normal_non_proportional_font,*small_font;
	XmFontList button_fontlist, normal_fontlist;
	XtAppContext application_context;
#if ! defined (USE_XTAPP_CONTEXT)
	int main_x_file_descriptor;
	int x_app_pending;
	struct Event_dispatcher_descriptor_callback *main_x_connection_callback;
	struct Event_dispatcher_idle_callback *special_idle_x_callback;
	struct Event_dispatcher_timeout_callback *timeout_x_callback;
#endif /* ! defined (USE_XTAPP_CONTEXT) */
#endif /* defined (MOTIF) */
#if defined (GTK_USER_INTERFACE)
	GtkWidget *main_window;
#if ! defined (USE_GTK_MAIN_STEP)
#if GTK_MAJOR_VERSION >= 2
	GMainContext *g_context;
#endif /* GTK_MAJOR_VERSION >= 2 */
#define MAX_GTK_FDS (50)
	GPollFD gtk_fds[MAX_GTK_FDS];
	gint records_in_gtk_fds;
	struct Event_dispatcher_descriptor_callback *gtk_descriptor_callback;
	int g_context_needs_prepare_and_query;
	gint g_context_timeout;
	gint max_priority;
#endif /* ! defined (USE_GTK_MAIN_STEP) */
#endif /* defined (GTK_USER_INTERFACE) */

	struct Event_dispatcher *event_dispatcher;
	struct Machine_information *local_machine_info;
	struct Shell_list_item *shell_list;
	struct Shell_stack_item *active_shell_stack;
}; /* struct User_interface */

struct Shell_stack_item
/*******************************************************************************
LAST MODIFIED : 25 March 1997

DESCRIPTION :
Used in conjunction with <busy_cursor_on> and <busy_cursor_off>.
???DB.  Move in with windowing macros ?
==============================================================================*/
{
#if defined (MOTIF)
	Widget shell;
#endif /* defined (MOTIF) */
	struct Shell_stack_item *next;
}; /* struct Shell_stack_item */

struct Shell_list_item
/*******************************************************************************
LAST MODIFIED : 23 June 1998

DESCRIPTION :
For keeping track of the all the shell widgets so that the busy cursor can be
set.
???DB.  Move in with windowing macros ?
???DB.  Moved from user_interface.h
==============================================================================*/
{
#if defined (MOTIF)
	Widget *shell_address;
#endif /* defined (MOTIF) */
	struct Shell_list_item *previous;
	struct Shell_list_item *next;
	/*???DB.  Needed because shells don't have to have UserData */
	struct User_interface *user_interface;
}; /* struct Shell_list_item */

#if defined (MOTIF)
typedef struct User_interface User_settings;
#endif /* defined (MOTIF) */

/*
Module variables
----------------
*/
#if defined (OLD_CODE)
/*???DB.  Have been moved into struct User_interface */
#if defined (MOTIF)
Cursor busy_cursor=(Cursor)NULL;
#endif /* defined (MOTIF) */
struct Shell_list_item *shell_list=(struct Shell_list_item *)NULL;
struct Shell_stack_item *active_shell_stack=(struct Shell_stack_item *)NULL;
#endif /* defined (OLD_CODE) */

#if defined (MOTIF)
#if defined (EXT_INPUT)
static unsigned long input_module_delay_s;
static unsigned long input_module_delay_ns;
	/*???DB.  Is there another way of doing this ? */
#endif /* defined (EXT_INPUT) */
#endif /* defined (MOTIF) */
#if defined (LINK_CMISS)
static unsigned long cmiss_link_delay_s;
static unsigned long cmiss_link_delay_ns;
extern struct CMISS_connection *CMISS;
	/*???GMH.  This is a hack - when we register it will disappear (defined in
		command.c) */
#endif /* defined (LINK_CMISS) */
/*???GMH.  We need to generalise the periodic update thingy.  What we need
	is an interface that allows a tool to register a function to get called
	at specific time intervals */

/*
Module functions
----------------
*/

#if defined (MOTIF)
#if ! defined (USE_XTAPP_CONTEXT)
static int User_interface_X_query_callback(
	struct Event_dispatcher_descriptor_set *descriptor_set, void *user_interface_void)
/*******************************************************************************
LAST MODIFIED : 18 November 2002

DESCRIPTION :
This function is called to add the main file descriptor for the X connection to
those processed by the event dispatcher.
==============================================================================*/
{
	int return_code;
	struct User_interface *user_interface;

	ENTER(User_interface_X_query_callback);
	if (user_interface=(struct User_interface *)user_interface_void)
	{
		return_code = 1;
		FD_SET(user_interface->main_x_file_descriptor, &(descriptor_set->read_set));
		if (XtAppPending(user_interface->application_context))
		{
			user_interface->x_app_pending = 1;
			if (descriptor_set->max_timeout_ns != 0)
			{
				descriptor_set->max_timeout_ns = 0;
			}
		}
		else
		{
			user_interface->x_app_pending = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"User_interface_X_query_callback.  Missing user_interface");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* User_interface_X_query_callback */
#endif /* ! defined (USE_XTAPP_CONTEXT) */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
#if ! defined (USE_XTAPP_CONTEXT)
static int User_interface_X_check_callback(
	struct Event_dispatcher_descriptor_set *descriptor_set,
	void *user_interface_void)
/*******************************************************************************
LAST MODIFIED : 13 November 2002

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct User_interface *user_interface;

	ENTER(User_interface_X_check_callback);
	if (user_interface=(struct User_interface *)user_interface_void)
	{
		return_code = 0;
		if (user_interface->x_app_pending || 
			(FD_ISSET(user_interface->main_x_file_descriptor, 
			&(descriptor_set->read_set))))
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
			"User_interface_X_check_callback.  Missing user_interface");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* User_interface_X_check_callback */
#endif /* ! defined (USE_XTAPP_CONTEXT) */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
#if ! defined (USE_XTAPP_CONTEXT)
static int User_interface_X_dispatch_callback(void *user_interface_void)
/*******************************************************************************
LAST MODIFIED : 6 March 2002

DESCRIPTION :
This function is called to process X connections.
==============================================================================*/
{
	int return_code;
	struct User_interface *user_interface;

	ENTER(User_interface_X_callback);
	if (user_interface=(struct User_interface *)user_interface_void)
	{
		XtAppProcessEvent(user_interface->application_context, XtIMAll);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"User_interface_X_callback.  Missing user_interface");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* User_interface_X_callback */
#endif /* ! defined (USE_XTAPP_CONTEXT) */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
#if ! defined (USE_XTAPP_CONTEXT)
static int User_interface_timeout_X_callback(void *user_interface_void)
/*******************************************************************************
LAST MODIFIED : 6 March 2002

DESCRIPTION :
This function is called to process X connections.
==============================================================================*/
{
	int return_code;
	struct User_interface *user_interface;

	ENTER(User_interface_timeout_X_callback);
	if (user_interface=(struct User_interface *)user_interface_void)
	{
		return_code = 1;
		user_interface->timeout_x_callback =
			(struct Event_dispatcher_timeout_callback *)NULL;
		if (XtAppPending(user_interface->application_context))
		{
			XtAppProcessEvent(user_interface->application_context, XtIMAll);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"User_interface_timeout_X_callback.  Missing user_interface");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* User_interface_timeout_X_callback */
#endif /* ! defined (USE_XTAPP_CONTEXT) */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
#if ! defined (USE_XTAPP_CONTEXT)
static int User_interface_idle_X_callback(void *user_interface_void)
/*******************************************************************************
LAST MODIFIED : 6 March 2002

DESCRIPTION :
This function is called to process X connections.
==============================================================================*/
{
	int return_code;
	long sec, usec;
	struct User_interface *user_interface;

	ENTER(User_interface_idle_X_callback);
	if (user_interface=(struct User_interface *)user_interface_void)
	{
		if (XtAppPending(user_interface->application_context))
		{
			XtAppProcessEvent(user_interface->application_context, XtIMAll);
			/* Call us back again next time we are idle */
			return_code = 1;
		}
		else
		{
			if (XtTimeForTimeout(user_interface->application_context, &sec, &usec))
			{
				user_interface->timeout_x_callback = 
					Event_dispatcher_add_timeout_callback_at_time(
					user_interface->event_dispatcher, sec, 1000*usec,
					User_interface_timeout_X_callback, user_interface_void);
			}

			if (XtCallWorkProc(user_interface->application_context))
			{
				/* There are X Work Procedures so keep the idle callback coming */
				return_code = 1;
			}
			else
			{
				/* This idle callback is finished, it will sleep 
				   if we return 0 */
				return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"User_interface_idle_X_callback.  Missing user_interface");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* User_interface_idle_X_callback */
#endif /* ! defined (USE_XTAPP_CONTEXT) */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
#if ! defined (USE_XTAPP_CONTEXT)
struct additional_X_connection
{
	Fdio_id fdio;
	struct User_interface *user_interface;
	Cmiss_native_socket_t fd;
};

static int User_interface_additional_X_callback(Fdio_id fdio,
	void *axc_void)
/*******************************************************************************
LAST MODIFIED : 6 March 2002

DESCRIPTION :
This function is called to process X connections.
==============================================================================*/
{
	int return_code;
	struct additional_X_connection *axc;

	ENTER(User_interface_X_callback);
	USE_PARAMETER(fdio);

	if ((axc=(struct additional_X_connection *)axc_void))
	{
		XProcessInternalConnection(axc->user_interface->display,
			axc->fd);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"User_interface_X_callback.  Missing user_interface");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* User_interface_X_callback */
#endif /* ! defined (USE_XTAPP_CONTEXT) */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
#if ! defined (USE_XTAPP_CONTEXT)
static void User_interface_X_connection_callback(Display *display, 
	XPointer user_interface_void, int file_descriptor, Bool opening, 
	XPointer *watch_data)
/*******************************************************************************
LAST MODIFIED : 6 March 2002

DESCRIPTION :
This function is called to register and deregister X connections.
==============================================================================*/
{
	Fdio_id fdio;
	struct additional_X_connection *axc;
	struct User_interface *user_interface;

	ENTER(User_interface_X_connection_callback);
	USE_PARAMETER(display);
	if ((user_interface=(struct User_interface *)user_interface_void))
	{
		switch (opening)
		{
			case True:
			{
				fdio = Event_dispatcher_create_Fdio(
					user_interface->event_dispatcher,
					file_descriptor);
				if (!fdio)
				{
					*watch_data = NULL;
				}
				else
				{
					ALLOCATE(axc,
						struct additional_X_connection, 1);
					axc->fdio = fdio;
					axc->user_interface = user_interface;
					axc->fd = file_descriptor;
					*watch_data = (void*)axc;
					Fdio_set_read_callback(fdio,
						User_interface_additional_X_callback,
						user_interface_void);
				}
			} break;
			case False:
			{
				axc = (struct additional_X_connection*)*watch_data;

				DESTROY(Fdio)(&axc->fdio);
				DEALLOCATE(axc);
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"User_interface_X_connection_callback.  Invalid mode.");
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"User_interface_X_connection_callback.  Missing user_interface");
	}
	LEAVE;
} /* User_interface_X_connection_callback */
#endif /* ! defined (USE_XTAPP_CONTEXT) */
#endif /* defined (MOTIF) */

#if defined (GTK_USER_INTERFACE)
#if ! defined (USE_GTK_MAIN_STEP)
#if GTK_MAJOR_VERSION < 2
static struct User_interface *polling_user_interface =
   (struct User_interface *)NULL;

static gint User_interface_gtk_gpoll_callback(GPollFD *ufds, guint nfsd, 
	gint timeout)
/*******************************************************************************
LAST MODIFIED : 19 November 2002

DESCRIPTION :
A dummy poll function used to find out which file descriptors that gtk is
interested in and then in the check phase to return the results of our 
actual polling.  Unnecessary in Gtk2 as there is a much more complete interface.
==============================================================================*/
{
	int j;
	gint return_code;
	struct User_interface *user_interface;
	unsigned int i;

	ENTER(User_interface_gtk_query_callback);
	if (user_interface = polling_user_interface)
	{
		if (user_interface->g_context_needs_prepare_and_query)
		{
			if (nfsd < MAX_GTK_FDS)
			{
				user_interface->records_in_gtk_fds = nfsd;
				for (i = 0 ; i < nfsd ; i++)
				{
					user_interface->gtk_fds[i].fd = ufds[i].fd;
					user_interface->gtk_fds[i].events = ufds[i].events;
					user_interface->gtk_fds[i].revents = ufds[i].revents;
				}
				user_interface->g_context_timeout = timeout;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"User_interface_gtk_gpoll_callback.  "
					"Internal array size is too small for number of active descriptors");
				return_code = 0;
			}
			/* Always return no results this time, we just want to
				harvest the file descriptor numbers */
			return_code = 0;
		}
		else
		{
			if (nfsd < MAX_GTK_FDS)
			{
				/* Increment the return code for every fd found with
					pending events */
				return_code = 0;
				for (i = 0 ; i < nfsd ; i++)
				{
					/* We can't assume we have the same lists so lets search */
					j = 0;
					while ((ufds[i].fd != user_interface->gtk_fds[j].fd)
						&& (j < user_interface->records_in_gtk_fds))
					{
						j++;
					}
					if (j < user_interface->records_in_gtk_fds)
					{
						ufds[i].revents = user_interface->gtk_fds[j].revents;
						if (user_interface->gtk_fds[j].revents)
						{
							return_code++;
						}
					}
					else
					{
						/* Requested file descriptor isn't in the list we were
							asked about in the query, hopefully this doesn't happen
							too often, and even when it does that another query/check
						   loop will happen so that the descriptor can be included. */
#if defined (DEBUG)
						display_message(ERROR_MESSAGE,
							"User_interface_gtk_gpoll_callback.  "
							"Requested a file descriptor not in our list");
#endif /* defined (DEBUG) */
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"User_interface_gtk_gpoll_callback.  "
					"Internal array size is too small for number of active descriptors");
				return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"User_interface_gtk_gpoll_callback.  Missing user_interface");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* User_interface_gtk_gpoll_callback */
#endif /* GTK_MAJOR_VERSION < 2 */
#endif /* ! defined (USE_GTK_MAIN_STEP) */
#endif /* defined (GTK_USER_INTERFACE) */

#if defined (GTK_USER_INTERFACE)
#if ! defined (USE_GTK_MAIN_STEP)
static int User_interface_gtk_query_callback(
	struct Event_dispatcher_descriptor_set *descriptor_set, void *user_interface_void)
/*******************************************************************************
LAST MODIFIED : 13 November 2002

DESCRIPTION :
This function is called to add file descriptors from a gtk context to
those processed by the event dispatcher.
==============================================================================*/
{
	int i, return_code;
	struct User_interface *user_interface;

	ENTER(User_interface_gtk_query_callback);
	if (user_interface=(struct User_interface *)user_interface_void)
	{
		if (user_interface->g_context_needs_prepare_and_query)
		{
#if GTK_MAJOR_VERSION >= 2
			if (g_main_context_prepare(user_interface->g_context,
				&user_interface->max_priority))
			{
				/* display_message(INFORMATION_MESSAGE,
					"User_interface_gtk_query_callback.  "
					"Sources ready for dispatch in prepare.\n"); */
			}

			user_interface->records_in_gtk_fds = g_main_context_query(user_interface->g_context,
				user_interface->max_priority, &user_interface->g_context_timeout,
				user_interface->gtk_fds, MAX_GTK_FDS);
#else /* GTK_MAJOR_VERSION >= 2 */
			/* We can't query or check the records directly so we overload the 
				g_poll_func and see what filehandles the function wants to
				poll.  The results are basically we return to the poll are invalid.
				Then we do our own poll and use the results to reply correctly
				when Gtk calls us back with a poll from the check callback.
				The g_context_needs_prepare_and_query flag is used to control
				what mode the g_poll function has so it must be updated after
				the call to g_main_pending.
			   The user_interface is passed to the poll function as a static
			   module variable.... yuk. */
			user_interface->records_in_gtk_fds = 0;
			user_interface->g_context_timeout = -1;
			/* Check the static pointer to ensure we are taking turns correctly */
			if (!polling_user_interface)
			{
				/* g_main_set_poll_func(User_interface_gtk_gpoll_callback); */

				polling_user_interface = user_interface;
				/* The result of this pending isn't very valuable as we didn't
					really poll for it anyway */
				g_main_pending();
				polling_user_interface = (struct User_interface *)NULL;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"User_interface_gtk_query_callback.  "
					"Already waiting for a poll callback.");
				return_code = 0;
			}
#endif /* GTK_MAJOR_VERSION >= 2 */

			user_interface->g_context_needs_prepare_and_query = 0;
		}

		if (user_interface->records_in_gtk_fds <= MAX_GTK_FDS)
		{
			return_code = 1;
			for (i = 0 ; i < user_interface->records_in_gtk_fds ; i++)
			{
				if (user_interface->gtk_fds[i].events & G_IO_IN)
				{
					FD_SET(user_interface->gtk_fds[i].fd, &(descriptor_set->read_set));
				}
				if (user_interface->gtk_fds[i].events & G_IO_OUT)
				{
					FD_SET(user_interface->gtk_fds[i].fd, &(descriptor_set->write_set));
				}
				if (user_interface->gtk_fds[i].events & G_IO_ERR)
				{
					FD_SET(user_interface->gtk_fds[i].fd, &(descriptor_set->error_set));
				}
			}
			if ((user_interface->g_context_timeout >= 0) && 
				((descriptor_set->max_timeout_ns < 0) || 
				(user_interface->g_context_timeout < descriptor_set->max_timeout_ns)))
			{
				descriptor_set->max_timeout_ns = user_interface->g_context_timeout * 1000;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"User_interface_gtk_query_callback.  "
				"Insufficient file descriptor records.");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"User_interface_gtk_query_callback.  Missing user_interface");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* User_interface_gtk_query_callback */
#endif /* ! defined (USE_GTK_MAIN_STEP) */
#endif /* defined (GTK_USER_INTERFACE) */

#if defined (GTK_USER_INTERFACE)
#if ! defined (USE_GTK_MAIN_STEP)
static int User_interface_gtk_check_callback(
	struct Event_dispatcher_descriptor_set *descriptor_set,
	void *user_interface_void)
/*******************************************************************************
LAST MODIFIED : 13 November 2002

DESCRIPTION :
==============================================================================*/
{
	int i, return_code;
	struct User_interface *user_interface;

	ENTER(User_interface_gtk_check_callback);
	if (user_interface=(struct User_interface *)user_interface_void)
	{
		for (i = 0 ; i < user_interface->records_in_gtk_fds ; i++)
		{
			user_interface->gtk_fds[i].revents = 0;
			if (FD_ISSET(user_interface->gtk_fds[i].fd, &(descriptor_set->read_set)))
			{
				user_interface->gtk_fds[i].revents |= G_IO_IN;
			}
			if (FD_ISSET(user_interface->gtk_fds[i].fd, &(descriptor_set->write_set)))
			{
				user_interface->gtk_fds[i].revents |= G_IO_OUT;
			}
			if (FD_ISSET(user_interface->gtk_fds[i].fd, &(descriptor_set->error_set)))
			{
				user_interface->gtk_fds[i].revents |= G_IO_ERR;
			}
		}
#if GTK_MAJOR_VERSION >= 2
		if (g_main_context_check(user_interface->g_context, 
			user_interface->max_priority, user_interface->gtk_fds, 
			user_interface->records_in_gtk_fds))
		{
			return_code = 1;
		}
		else
		{
			return_code = 0;
		}
#else /* GTK_MAJOR_VERSION >= 2 */
		/* We can't query or check the records directly so we overload the 
			g_poll_func and see what filehandles the function wants to
			poll.  The results are basically we return to the poll are invalid.
			Then we do our own poll and use the results to reply correctly
			when Gtk calls us back with a poll from the check callback.
			The g_context_needs_prepare_and_query flag is used to control
			what mode the g_poll function has so it must be updated after
			the call to g_main_pending.
			The user_interface is passed to the poll function as a static
			module variable.... yuk. */
		/* Check the static pointer to ensure we are taking turns correctly */
		if (!polling_user_interface)
		{
			polling_user_interface = user_interface;
			if (g_main_pending())
			{
				return_code = 1;
			}
			else
			{
				return_code = 0;
			}
			polling_user_interface = (struct User_interface *)NULL;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"User_interface_gtk_check_callback.  "
				"Already waiting for a poll callback.");
			return_code = 0;
		}
#endif /* GTK_MAJOR_VERSION >= 2 */
		user_interface->g_context_needs_prepare_and_query = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"User_interface_gtk_check_callback.  Missing user_interface");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* User_interface_gtk_check_callback */
#endif /* ! defined (USE_GTK_MAIN_STEP) */
#endif /* defined (GTK_USER_INTERFACE) */

#if defined (GTK_USER_INTERFACE)
#if ! defined (USE_GTK_MAIN_STEP)
static int User_interface_gtk_dispatch_callback(void *user_interface_void)
/*******************************************************************************
LAST MODIFIED : 13 November 2002

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct User_interface *user_interface;

	ENTER(User_interface_gtk_dispatch_callback);
	if (user_interface=(struct User_interface *)user_interface_void)
	{
#if GTK_MAJOR_VERSION >= 2
		g_main_context_dispatch(user_interface->g_context);
#else /* GTK_MAJOR_VERSION >= 2 */
		/* Can't call the dispatch directly unfortunately... */
		if (!polling_user_interface)
		{
			polling_user_interface = user_interface;
			g_main_iteration (/*block*/FALSE);
			polling_user_interface = (struct User_interface *)NULL;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"User_interface_gtk_dispatch_callback.  "
				"Already waiting for a poll callback.");
			return_code = 0;
		}
#endif /* GTK_MAJOR_VERSION >= 2 */
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"User_interface_gtk_dispatch_callback.  Missing user_interface");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* User_interface_gtk_dispatch_callback */
#endif /* ! defined (USE_GTK_MAIN_STEP) */
#endif /* defined (GTK_USER_INTERFACE) */

#if defined (MOTIF)
#if defined (EXT_INPUT)
static int process_external_input(void *user_interface_void)
/*******************************************************************************
LAST MODIFIED : 6 March 2002

DESCRIPTION :
Calls the input module update function, so we can get input from external
devices, and then resets the timeout.
==============================================================================*/
{
	int return_code;
	struct User_interface *user_interface;

	ENTER(process_external_input);
	if (user_interface=(struct User_interface *)user_interface_void)
	{
		input_module_update(user_interface);
		Event_dispatcher_add_timeout_callback(user_interface->event_dispatcher,
			input_module_delay_s, input_module_delay_ns, process_external_input, 
			user_interface_void);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"process_external_input.  Missing user_interface");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* process_external_input */
#endif /* defined (EXT_INPUT) */
#endif /* defined (MOTIF) */

#if defined (LINK_CMISS)
#if defined (MOTIF)
static int process_cmiss_link(void *user_interface_void)
/*******************************************************************************
LAST MODIFIED : 6 March 2002

DESCRIPTION :
Calls the input module update function, so we can get input from external
devices, and then resets the timeout.
==============================================================================*/
{
	int return_code;
	struct User_interface *user_interface;

	ENTER(process_cmiss_link);
	if (user_interface=(struct User_interface *)user_interface_void)
	{
		/* ???GMH.  This changing of delay is a hack to avoid registering and
		deregistering this callback when CMISS_connection's are created */
		if (CMISS)
		{
			CMISS_connection_update(&CMISS);
			/* if connection, then check every tenth of a second */
			cmiss_link_delay_s=0;
			cmiss_link_delay_ns=100000000;
		}
		else
		{
			/* if no connection, then check every 5 seconds */
			cmiss_link_delay_s=5;
			cmiss_link_delay_ns=0;
		}
		Event_dispatcher_add_timeout_callback(user_interface->event_dispatcher,
			cmiss_link_delay_s, cmiss_link_delay_ns, process_cmiss_link, user_interface_void);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"process_cmiss_link.  Missing user_interface");
		return_code = 0;
	}
	LEAVE;
	
	return (return_code);
} /* process_cmiss_link */
#endif /* defined (MOTIF) */
#endif /* defined (LINK_CMISS) */

/*
Global functions
----------------
*/
#if defined (MOTIF)
int x_error_handler(Display *display, XErrorEvent *error)
/*******************************************************************************
LAST MODIFIED : 15 September 1999 

DESCRIPTION :
Responds to nonfatal XErrors and allows cmgui to continue.
==============================================================================*/
{
	char msg[80];
	XGetErrorText(display, error->error_code, msg, 80);
	display_message(ERROR_MESSAGE, "x_error_handler:  %s", msg);
	return(0);	
} /* x_error_handler */
#endif /* defined (MOTIF) */

struct Shell_list_item *create_Shell_list_item(
#if defined (MOTIF)
	Widget *shell_address,
#endif /* defined (MOTIF) */
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 28 October 1998

DESCRIPTION :
This function allocates memory for a shell list item, initializes the <shell>
field to the specified value and adds the item to the beginning of the shell
list.  It returns a pointer to the created item if successful and NULL if
unsuccessful.
???DB.  Move in with windowing macros ?
==============================================================================*/
{
	struct Shell_list_item *list_item;
#if defined (MOTIF)
	Widget shell;
	Window window;
#endif /* defined (MOTIF) */

	ENTER(create_Shell_list_item);
#if defined (DEBUG)
#if defined (MOTIF)
printf("enter create_Shell_list_item.  shell_address=%p, user_interface=%p, shell=%p\n",
	shell_address,user_interface,*shell_address);
#endif /* defined (MOTIF) */
#endif /* defined (DEBUG) */
	if (ALLOCATE(list_item,struct Shell_list_item,1))
	{
		list_item->user_interface=user_interface;
#if defined (MOTIF)
		/* initialize shell field */
		list_item->shell_address=shell_address;
#endif /* defined (MOTIF) */
		/* add item to list */
		list_item->previous=(struct Shell_list_item *)NULL;
		list_item->next=user_interface->shell_list;
		/* point previous first item back to new first item */
		if (list_item->next)
		{
			list_item->next->previous=list_item;
		}
		user_interface->shell_list=list_item;
#if defined (MOTIF)
		/* set the cursor */
		if (shell_address&&(shell= *shell_address)&&
			(user_interface->active_shell_stack)&&
			(shell!=user_interface->active_shell_stack->shell)&&
			(window=XtWindow(shell)))
		{
			XDefineCursor(XtDisplay(shell),window,user_interface->busy_cursor);
		}
#endif /* defined (MOTIF) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_Shell_list_item.  Could not allocate memory for list_item");
	}
	LEAVE;

	return (list_item);
} /* create_Shell_list_item */

int destroy_Shell_list_item(struct Shell_list_item **list_item)
/*******************************************************************************
LAST MODIFIED : 4 June 1999

DESCRIPTION :
This function removes the <list_item> from the shell list and frees the memory
for the <list_item>.  <*list_item> is set to NULL.
???DB.  Move in with windowing macros ?
==============================================================================*/
{
	int return_code;
	struct Shell_list_item *item;
	struct User_interface *user_interface;

	ENTER(destroy_Shell_list_item);
#if defined (DEBUG)
#if defined (MOTIF)
printf("enter destroy_Shell_list_item.  shell_address=%p, user_interface=%p\n",
	(*list_item)->shell_address,(*list_item)->user_interface);
#endif /* defined (MOTIF) */
#endif /* defined (DEBUG) */
	if (list_item&&(item= *list_item)&&(user_interface=item->user_interface))
	{
		/* remove item from shell list */
		if (item->previous)
		{
			item->previous->next=item->next;
		}
		else
		{
			user_interface->shell_list=item->next;
		}
		if (item->next)
		{
			item->next->previous=item->previous;
		}
		/* free memory */
		DEALLOCATE(item);
		*list_item=(struct Shell_list_item *)NULL;
		return_code=1;
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* destroy_Shell_list_item */

#if defined (MOTIF)
int destroy_Shell_list_item_from_shell(Widget *shell_address,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 23 June 1998

DESCRIPTION :
This function removes the list_item which refers to <shell> from the shell list
and frees the memory for the <list_item>.
???DB.  Move in with windowing macros ?
???DB.  Does not need user_interface argument anymore
==============================================================================*/
{
	int return_code;
	struct Shell_list_item *item;

	ENTER(destroy_Shell_list_item_from_shell);
#if defined (DEBUG)
printf("enter destroy_Shell_list_item_from_shell.  shell_address=%p, user_interface=%p\n",
	shell_address,user_interface);
#endif /* defined (DEBUG) */
	if (shell_address&&user_interface)
	{
		item=user_interface->shell_list;
		while (item&&(item->shell_address!=shell_address))
		{
			item=item->next;
		}
		if (item)
		{
			/* remove item from shell list */
			if (item->previous)
			{
				item->previous->next=item->next;
			}
			else
			{
				user_interface->shell_list=item->next;
			}
			if (item->next)
			{
				item->next->previous=item->previous;
			}
			/* free memory */
			DEALLOCATE(item);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"destroy_Shell_list_item_from_shell.  "
				"Unable to find the shell in the shell list");
			return_code=0;
		}
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* destroy_Shell_list_item */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
void destroy_window_shell(Widget widget,XtPointer list_item,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 25 November 1999

DESCRIPTION :
This function removes the <list_item> from the shell list, frees the memory
for the <list_item> and sets <*(list_item->address)> to NULL.
???DB.  Move in with windowing macros ?
==============================================================================*/
{
	struct Shell_list_item *item;
	struct User_interface *user_interface;

	ENTER(destroy_window_shell);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if ((item=(struct Shell_list_item *)list_item)&&
		(user_interface=item->user_interface))
	{
		/* remove item from shell list */
		if (item->previous)
		{
			item->previous->next=item->next;
		}
		else
		{
			user_interface->shell_list=item->next;
		}
		if (item->next)
		{
			item->next->previous=item->previous;
		}
		/* set the shell widget to NULL */
		if (item->shell_address)
		{
			/*???DB.  Used to be */
/*			item->shell_address=(Widget *)NULL;*/
			*(item->shell_address)=(Widget)NULL;
		}
		/* free memory */
		DEALLOCATE(item);
	}
	else
	{
		if (item)
		{
			display_message(ERROR_MESSAGE,
				"destroy_window_shell.  Unable to get user_interface");
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"destroy_window_shell.  Unable to get item");
		}
	}
	LEAVE;
} /* destroy_window_shell */
#endif /* defined (MOTIF) */

int busy_cursor_on(
#if defined (MOTIF)
	Widget excluded_shell,
#endif /* defined (MOTIF) */
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 24 July 2002

DESCRIPTION :
Switchs from the default cursor to the busy cursor for all shells except the
<excluded_shell>.
???DB.  Move in with windowing macros ?
==============================================================================*/
{
	int return_code;
#if defined (MOTIF)
	struct Shell_list_item *item;
	struct Shell_stack_item *stack_item;
	Widget shell;
	Window window;
#endif /* defined (MOTIF) */

	ENTER(busy_cursor_on);
#if !defined (MOTIF)
	USE_PARAMETER(user_interface);
#endif /* !defined (MOTIF) */
#if defined (MOTIF)
	if (user_interface&&(user_interface->busy_cursor))
	{
		/* add item to active shell stack */
		if (ALLOCATE(stack_item,struct Shell_stack_item,1))
		{
			if (user_interface->active_shell_stack)
			{
				if ((shell=user_interface->active_shell_stack->shell)
					&&(window=XtWindow(shell)))
				{
					XDefineCursor(XtDisplay(shell),window,user_interface->busy_cursor);
				}
				if (excluded_shell&&(window=XtWindow(excluded_shell)))
				{
					XDefineCursor(XtDisplay(excluded_shell),window,None);
				}
			}
			else
			{
				item=user_interface->shell_list;
				while (item)
				{
					if (((shell= *(item->shell_address))!=excluded_shell)&&
						(window=XtWindow(shell)))
					{
						XDefineCursor(XtDisplay(shell),window,user_interface->busy_cursor);
					}
					item=item->next;
				}
			}
			stack_item->shell=excluded_shell;
			stack_item->next=user_interface->active_shell_stack;
			user_interface->active_shell_stack=stack_item;
			/*???SAB.  Trying to make it modal */
			if (excluded_shell)
			{
				XtAddGrab(excluded_shell,True,False);
			}
			else
			{
				XtAddGrab(user_interface->application_shell,True,False);
			}
			XFlush(user_interface->display);
			return_code=1;
		}
		else
		{
			return_code=0;
		}
	}
	else
	{
		return_code=0;
	}
#else /* defined (MOTIF) */
	return_code=0;
#endif /* defined (MOTIF) */
	LEAVE;

	return (return_code);
} /* busy_cursor_on */

int busy_cursor_off(
#if defined (MOTIF)
	Widget excluded_shell,
#endif /* defined (MOTIF) */
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 24 July 2002

DESCRIPTION :
Switchs from the busy cursor to the default cursor for all shells except the
<excluded_shell>.
???DB.  Move in with windowing macros ?
==============================================================================*/
{
	int return_code;
#if defined (MOTIF)
	struct Shell_list_item *item;
	struct Shell_stack_item *stack_item,**stack_item_address;
	Widget shell;
	Window window;
#if defined (OLD_CODE)
	XEvent event;
#endif /* defined (OLD_CODE) */
#endif /* defined (MOTIF) */

	ENTER(busy_cursor_off);
#if !defined (MOTIF)
	USE_PARAMETER(user_interface);
#endif /* !defined (MOTIF) */
#if defined (MOTIF)
	if (user_interface&&(user_interface->busy_cursor))
	{
		if (stack_item=user_interface->active_shell_stack)
		{
			stack_item_address= &(user_interface->active_shell_stack);
			while ((excluded_shell!=stack_item->shell)&&(stack_item->next))
			{
				stack_item_address= &(stack_item->next);
				stack_item=stack_item->next;
			}
			if (excluded_shell==stack_item->shell)
			{
				*stack_item_address=stack_item->next;
				if (stack_item_address== &(user_interface->active_shell_stack))
				{
					if (user_interface->active_shell_stack)
					{
						if ((shell=stack_item->shell)&&(window=XtWindow(shell)))
						{
							XDefineCursor(XtDisplay(shell),window,
								user_interface->busy_cursor);
						}
						if ((shell=user_interface->active_shell_stack->shell)
							&&(window=XtWindow(shell)))
						{
							XDefineCursor(XtDisplay(shell),window,None);
						}
					}
					else
					{
						item=user_interface->shell_list;
						while (item)
						{
							if (((shell= *(item->shell_address))!=excluded_shell)&&
								(window=XtWindow(shell)))
							{
								XUndefineCursor(XtDisplay(shell),window);
							}
							item=item->next;
						}
					}
				}
				/*???SAB.  Trying to make it modal */
				if (excluded_shell)
				{
					XtRemoveGrab(excluded_shell);
				}
				else
				{
					XtRemoveGrab(user_interface->application_shell);
				}
				DEALLOCATE(stack_item);
				return_code=1;
			}
			else
			{
				return_code=0;
			}
		}
		else
		{
			return_code=0;
		}
		XFlush(user_interface->display);
#if defined (OLD_CODE)
		/*???SAB.  This is to try and get rid of XEvents that were created
			while the busy cursor was on, doesn't seem to work */
		while (XCheckMaskEvent(user_interface->display,
			KeyPress|KeyRelease|ButtonPress|ButtonRelease|MotionNotify,
			&event))
		{
			/* Do nothing to throw the event away */
		}
#endif /* defined (OLD_CODE) */
	}
	else
	{
		if (user_interface)
		{
			display_message(ERROR_MESSAGE,
				"busy_cursor_off.  Unable to get busy cursor");
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"busy_cursor_off.  Unable to get user_interface");
		}
		return_code=0;
	}
#else /* defined (MOTIF) */
	return_code=0;
#endif /* defined (MOTIF) */
	LEAVE;

	return (return_code);
} /* busy_cursor_off */

#if defined (MOTIF)
#if defined (TEST_TRUE_COLOUR_VISUAL)
/*???debug.  To test true colour visuals */
Colormap default_colour_map;
int default_depth;
Visual *default_visual;
#endif /* defined (TEST_TRUE_COLOUR_VISUAL) */
#endif /* defined (MOTIF) */

#if !defined (WIN32_USER_INTERFACE)
struct User_interface *CREATE(User_interface)(int *argc_address, char **argv, 
	struct Event_dispatcher *event_dispatcher, char *class_name, 
	char *application_name)
#else /* !defined (WIN32_USER_INTERFACE) */
struct User_interface *CREATE(User_interface)(HINSTANCE current_instance,
	HINSTANCE previous_instance, LPSTR command_line,int initial_main_window_state,
	struct Event_dispatcher *event_dispatcher)
#endif /* !defined (WIN32_USER_INTERFACE) */
/*******************************************************************************
LAST MODIFIED : 10 October 2003

DESCRIPTION :
Open the <user_interface>.
==============================================================================*/
{
#if defined (MOTIF)
	char bitmap_data;
	int screen_number;
	static MrmRegisterArg identifiers[]=
	{
		{"widget_spacing",(XtPointer)NULL},
		{"menu_font",(XtPointer)NULL},
		{"heading_font",(XtPointer)NULL},
		{"normal_font",(XtPointer)NULL},
		{"normal_non_proportional_font",(XtPointer)NULL},
		{"button_font",(XtPointer)NULL},
		{"list_font",(XtPointer)NULL},
		{"small_font",(XtPointer)NULL}
	};
	static char *fallback_resources[]=
	{
		"*.time_editor_play.accelerators: <KeyPress>f:   ArmAndActivate()",
		"*.time_editor_play_reverse.accelerators: <KeyPress>b:   ArmAndActivate()",
		"*.time_editor_stop.accelerators: <KeyPress>s:   ArmAndActivate()",
		"*.time_editor_step.accelerators: <KeyPress>j:   ArmAndActivate()",
		/*SAB #Override is needed for when a drawing area has focus
		  alternatively could set no traversal for drawing area*/
		"*.analysis_previous_button.accelerators: #override <Key>p: ArmAndActivate()",
		"*.analysis_next_button.accelerators: #override <Key>n: ArmAndActivate()",
		"*.analysis_accept_button.accelerators: #override <Key>a: ArmAndActivate()",
		"*.analysis_reject_button.accelerators: #override <Key>r: ArmAndActivate()",
		"*.trace_edit_previous_button.accelerators: #override <Key>p: ArmAndActivate()",
		"*.trace_edit_next_button.accelerators: #override <Key>n: ArmAndActivate()",
		"*.trace_edit_accept_button.accelerators: #override <Key>a: ArmAndActivate()",
		"*.trace_edit_reject_button.accelerators: #override <Key>r: ArmAndActivate()",
		NULL /* Must be NULL terminated */
	};
#define XmNbuttonFont "buttonFont"
#define XmCButtonFont "ButtonFont"
#define XmNheadingFont "headingFont"
#define XmCHeadingFont "HeadingFont"
#define XmNlistFont "listFont"
#define XmCListFont "ListFont"
#define XmNmenuFont "menuFont"
#define XmCMenuFont "MenuFont"
#define XmNnormalFont "normalFont"
#define XmCNormalFont "NormalFont"
#define XmNnormalNonProportionalFont "normalNonProportionalFont"
#define XmCNormalNonProportionalFont "NormalNonProportionalFont"
#define XmNsmallFont "smallFont"
#define XmCSmallFont "SmallFont"
#define XmNwidgetSpacing "widgetSpacing"
#define XmCWidgetSpacing "WidgetSpacing"
	static XtResource resources[]=
	{
		{
			XmNmenuFont,
			XmCMenuFont,
			XmRFontStruct,
			sizeof(XFontStruct *),
			XtOffsetOf(User_settings,menu_font),
			XmRString,
			"*-Helvetica-medium-R-*--*-120-*"
		},
		{
			XmNheadingFont,
			XmCHeadingFont,
			XmRFontStruct,
			sizeof(XFontStruct *),
			XtOffsetOf(User_settings,heading_font),
			XmRString,
			"-*-HELVETICA-BOLD-R-*--*-120-*"
		},
		{
			XmNnormalFont,
			XmCNormalFont,
			XmRFontStruct,
			sizeof(XFontStruct *),
			XtOffsetOf(User_settings,normal_font),
			XmRString,
			"*-helvetica-medium-r-normal--14-*-*"
		},
		{
			XmNnormalNonProportionalFont,
			XmCNormalNonProportionalFont,
			XmRFontStruct,
			sizeof(XFontStruct *),
			XtOffsetOf(User_settings,normal_non_proportional_font),
			XmRString,
			"*-courier-medium-r-normal--12-*"
		},
		{
			XmNbuttonFont,
			XmCButtonFont,
			XmRFontStruct,
			sizeof(XFontStruct *),
			XtOffsetOf(User_settings,button_font),
			XmRString,
			"*-Helvetica-medium-r-*--*-120-*"
		},
		{
			XmNlistFont,
			XmCListFont,
			XmRFontStruct,
			sizeof(XFontStruct *),
			XtOffsetOf(User_settings,list_font),
			XmRString,
			"*-Helvetica-medium-R-*--*-120-*"
		},
		{
			XmNsmallFont,
			XmCSmallFont,
			XmRFontStruct,
			sizeof(XFontStruct *),
			XtOffsetOf(User_settings,small_font),
			XmRString,
			"*-Helvetica-medium-R-*--*-120-*"
		},
		{
			XmNwidgetSpacing,
			XmCWidgetSpacing,
			XmRInt,
			sizeof(int),
			XtOffsetOf(User_settings,widget_spacing),
			XmRString,
			"5"
		}
	};
#endif /* defined (MOTIF) */
	struct User_interface *user_interface;

	ENTER(CREATE(User_interface));
#if defined (WIN32_USER_INTERFACE)
	USE_PARAMETER(previous_instance);
#endif /* defined (WIN32_USER_INTERFACE) */
	if (ALLOCATE(user_interface, struct User_interface, 1))
	{
#if defined (MOTIF)
		user_interface->application_context=(XtAppContext)NULL;
		user_interface->application_shell=(Widget)NULL;
		user_interface->display=(Display *)NULL;
#if ! defined (USE_XTAPP_CONTEXT)
		user_interface->main_x_connection_callback = 
			(struct Event_dispatcher_descriptor_callback *)NULL;
		user_interface->special_idle_x_callback = 
			(struct Event_dispatcher_idle_callback *)NULL;
		user_interface->timeout_x_callback = 
			(struct Event_dispatcher_timeout_callback *)NULL;
#endif /* ! defined (USE_XTAPP_CONTEXT) */
#endif /* defined (MOTIF) */
#if defined (WIN32_USER_INTERFACE)
		user_interface->instance=current_instance;
		user_interface->main_window=(HWND)NULL;
		user_interface->main_window_state=initial_main_window_state;
		user_interface->command_line=command_line;
#else /* defined (WIN32_USER_INTERFACE) */
		user_interface->argc_address= argc_address;
		user_interface->argv=argv;
		user_interface->application_name=application_name;
		user_interface->class_name=class_name;
#endif /* defined (WIN32_USER_INTERFACE) */
#if defined (GTK_USER_INTERFACE)
		user_interface->main_window = (GtkWidget *)NULL;
#if ! defined (USE_GTK_MAIN_STEP)
#if GTK_MAJOR_VERSION >= 2
		user_interface->g_context = (GMainContext *)NULL;
#endif /* GTK_MAJOR_VERSION >= 2 */
		user_interface->records_in_gtk_fds = 0;
		user_interface->gtk_descriptor_callback = 
			(struct Event_dispatcher_descriptor_callback *)NULL;
		user_interface->g_context_needs_prepare_and_query = 1;
		user_interface->g_context_timeout = -1;
		user_interface->max_priority = 0;
#endif /* ! defined (USE_GTK_MAIN_STEP) */
#endif /* defined (GTK_USER_INTERFACE) */

		user_interface->event_dispatcher = event_dispatcher;
		user_interface->shell_list = (struct Shell_list_item *)NULL;
		user_interface->active_shell_stack = (struct Shell_stack_item *)NULL;

		/* get the name of the machine we are running on */
		if (!(user_interface->local_machine_info=CREATE(Machine_information)()))
		{
			display_message(WARNING_MESSAGE,
				"Could not determine local machine information");
		}

#if defined (MOTIF)
		user_interface->no_cascade_pixmap=XmUNSPECIFIED_PIXMAP;
		/* XtSetLanguageProc(NULL,NULL,NULL); */
		/* initialize the Motif resource manager */
		MrmInitialize();
		/* initialize the X toolkit */
		XtToolkitInitialize();
		/* create the application context */
		user_interface->application_context=XtCreateApplicationContext();
		XtAppSetFallbackResources(user_interface->application_context,
			fallback_resources);
		/* open the display */
		if (user_interface->display=XtOpenDisplay(
				 user_interface->application_context,NULL,user_interface->application_name,
				 user_interface->class_name,NULL,0,user_interface->argc_address,
				 user_interface->argv))
		{
			/* determine the screen size */
			screen_number=XDefaultScreen(user_interface->display);
			user_interface->screen_width=XDisplayWidth(user_interface->display,
				screen_number);
			user_interface->screen_height=XDisplayHeight(user_interface->display,
				screen_number);
			/* create the busy cursor */
			user_interface->busy_cursor=XCreateFontCursor(user_interface->display,
				XC_watch);
			/* set the multi-click time */
			if (XtGetMultiClickTime(user_interface->display)<500)
			{
				XtSetMultiClickTime(user_interface->display,500);
			}
#if defined (USE_XTAPP_CONTEXT)
			Event_dispatcher_set_application_context(event_dispatcher,
				user_interface->application_context);
#else /* defined (USE_XTAPP_CONTEXT) */
		/* ask X for it's file handle connections */
			user_interface->main_x_file_descriptor = 
				ConnectionNumber(user_interface->display);
			user_interface->x_app_pending = 1;
			if (user_interface->main_x_connection_callback = 
				Event_dispatcher_add_descriptor_callback(
				user_interface->event_dispatcher, User_interface_X_query_callback,
				User_interface_X_check_callback, User_interface_X_dispatch_callback,
				(void *)user_interface))
			{
				if (user_interface->special_idle_x_callback = Event_dispatcher_set_special_idle_callback(
						 user_interface->event_dispatcher, User_interface_idle_X_callback, 
						 (void *)user_interface, EVENT_DISPATCHER_X_PRIORITY))
				{
					if (!XAddConnectionWatch(user_interface->display, 
							 User_interface_X_connection_callback, (XPointer)user_interface))
					{
						display_message(ERROR_MESSAGE,
							"CREATE(User_interface).  Unable to register connection watch.");
						DEALLOCATE(user_interface);
						user_interface = (struct User_interface *)NULL;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"CREATE(User_interface).  Unable to register special X connection.");
					DEALLOCATE(user_interface);
					user_interface = (struct User_interface *)NULL;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"CREATE(User_interface).  Unable to register main X connection.");
				DEALLOCATE(user_interface);
				user_interface = (struct User_interface *)NULL;
			}
#endif /* defined (USE_XTAPP_CONTEXT) */

			if (user_interface)
			{
#if defined (TEST_TRUE_COLOUR_VISUAL)
				/*???debug.  To test true colour visuals */
				{
					int number_of_visuals;
					XVisualInfo *visual_info,visual_info_template;

					visual_info_template.class=TrueColor;
					default_depth=24;
					visual_info_template.depth=default_depth;
					if ((visual_info=XGetVisualInfo(user_interface->display,
							  VisualClassMask|VisualDepthMask,&visual_info_template,&number_of_visuals))&&
						(0<number_of_visuals))
					{
						default_visual=visual_info->visual;
						if (!(default_colour_map=XCreateColormap(user_interface->display,
									XRootWindow(user_interface->display,screen_number),default_visual,
									AllocNone)))
						{
							printf("Could not create colour map\n");
							default_depth=XDefaultDepth(user_interface->display,screen_number);
							default_visual=XDefaultVisual(user_interface->display,screen_number);
							default_colour_map=XDefaultColormap(user_interface->display,
								screen_number);
						}
					}
					else
					{
						printf("Could not find visual\n");
						default_depth=XDefaultDepth(user_interface->display,screen_number);
						default_visual=XDefaultVisual(user_interface->display,screen_number);
						default_colour_map=XDefaultColormap(user_interface->display,screen_number);
					}
				}
#endif /* defined (TEST_TRUE_COLOUR_VISUAL) */
				/* create the application shell (allow children to resize) */
				/*???DB.  Is allowShellResize necessary */
				if (user_interface->application_shell=XtVaAppCreateShell(
						 user_interface->application_name,user_interface->class_name,
						 applicationShellWidgetClass,user_interface->display,
						 XtNallowShellResize,True,
						 XtNtitle,"CMISS",
#if defined (TEST_TRUE_COLOUR_VISUAL)
						 /*???debug.  To test true colour visuals */
						 XmNcolormap,default_colour_map,
						 XmNdepth,default_depth,
						 XmNvisual,default_visual,
#endif /* defined (TEST_TRUE_COLOUR_VISUAL) */
						 NULL))
				{
					/* to avoid large gaps on the right of cascade gadgets (option menus) */
					bitmap_data=(char)0x0;
					if (!(user_interface->no_cascade_pixmap=XCreatePixmapFromBitmapData(
								user_interface->display,XRootWindow(user_interface->display,
									screen_number),&bitmap_data,1,1,0,XWhitePixel(user_interface->display,
										screen_number),XDefaultDepth(user_interface->display,screen_number))))
					{
						user_interface->no_cascade_pixmap=XmUNSPECIFIED_PIXMAP;
					}
					/* for communication with other applications */
					user_interface->property_notify_callback=(Property_notify_callback)NULL;
					user_interface->property_notify_data=(void *)NULL;
					user_interface->property_notify_widget=(Widget)NULL;
#if defined (EXT_INPUT)
					/* conditional stuff in case any graphics routines void the ports */
					/* set the delay for the input_module callback */
					input_module_delay_s = 0;
					input_module_delay_ns = 10000000;
					/* we need to initialise the input module */
					input_module_init(user_interface);
					Event_dispatcher_add_timeout_callback(event_dispatcher,
						input_module_delay_s, input_module_delay_ns, process_external_input, 
						(void *)user_interface);
#endif /* defined (EXT_INPUT) */
#if defined (LINK_CMISS)
					/* set the delay for the cmiss link callback */
					cmiss_link_delay_s = 1;
					cmiss_link_delay_ns = 0;
					Event_dispatcher_add_timeout_callback(event_dispatcher, cmiss_link_delay_s,
						cmiss_link_delay_ns, process_cmiss_link, (void *)user_interface);
#endif /* defined (LINK_CMISS) */
					/* retrieve settings */
					XtVaGetApplicationResources(user_interface->application_shell,
						user_interface,resources,XtNumber(resources),NULL);
					/* register identifiers in the global name table */
					user_interface->normal_fontlist = XmFontListCreate(
						user_interface->normal_font, XmSTRING_DEFAULT_CHARSET);
					user_interface->button_fontlist = XmFontListCreate(
						user_interface->button_font, XmSTRING_DEFAULT_CHARSET);
					identifiers[0].value = INT2VOIDPTR(user_interface->widget_spacing);
					/*???DB.  widget_spacing based on screen_width ? */
					identifiers[1].value = (XtPointer)XmFontListCreate(
						user_interface->menu_font,XmSTRING_DEFAULT_CHARSET);
					identifiers[2].value = (XtPointer)XmFontListCreate(
						user_interface->heading_font,XmSTRING_DEFAULT_CHARSET);
					identifiers[3].value = (XtPointer)user_interface->normal_fontlist;
					identifiers[4].value=(XtPointer)XmFontListCreate(
						user_interface->normal_non_proportional_font,
						XmSTRING_DEFAULT_CHARSET);
					identifiers[5].value=(XtPointer)XmFontListCreate(
						user_interface->button_font,XmSTRING_DEFAULT_CHARSET);
					identifiers[6].value=(XtPointer)XmFontListCreate(
						user_interface->list_font,XmSTRING_DEFAULT_CHARSET);
					identifiers[7].value=(XtPointer)XmFontListCreate(
						user_interface->small_font,XmSTRING_DEFAULT_CHARSET);
					/* register the identifiers in the global name table*/
					if (MrmSUCCESS==MrmRegisterNames(identifiers,XtNumber(identifiers)))
					{
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"CREATE(User_interface).  Unable to register identifiers");
						DEALLOCATE(user_interface);
						user_interface = (struct User_interface *)NULL;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"CREATE(User_interface).  Unable to create application shell");
					DEALLOCATE(user_interface);
					user_interface = (struct User_interface *)NULL;
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(User_interface).  Unable to open display");
			DEALLOCATE(user_interface);
			user_interface = (struct User_interface *)NULL;
		}
#endif /* defined (MOTIF) */
#if defined (WIN32_USER_INTERFACE)
		user_interface->widget_spacing=5;
#endif /* defined (WIN32_USER_INTERFACE) */
#if defined (GTK_USER_INTERFACE)
		/* Initialize i18n support */
		gtk_set_locale ();
		
		/* Initialize the widget set */
		gtk_init (argc_address, &argv);

		/* Create the main window */
		user_interface->main_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

#if ! defined (USE_GTK_MAIN_STEP)
#if GTK_MAJOR_VERSION >= 2
		if ((user_interface->g_context = g_main_context_default())
			&& (g_main_context_acquire(user_interface->g_context)))
		{
			g_main_context_ref(user_interface->g_context);
			user_interface->gtk_descriptor_callback = 
				Event_dispatcher_add_descriptor_callback(
				user_interface->event_dispatcher,
				User_interface_gtk_query_callback,
				User_interface_gtk_check_callback,
				User_interface_gtk_dispatch_callback,
				user_interface); 
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(User_interface).  Unable to acquire main g_context");
			DESTROY(User_interface)(&user_interface);
			user_interface = (struct User_interface *)NULL;
		}
#else /* GTK_MAJOR_VERSION >= 2 */
		g_main_set_poll_func(User_interface_gtk_gpoll_callback);
		user_interface->gtk_descriptor_callback = 
			Event_dispatcher_add_descriptor_callback(
			user_interface->event_dispatcher,
			User_interface_gtk_query_callback,
			User_interface_gtk_check_callback,
			User_interface_gtk_dispatch_callback,
			user_interface);
#endif /* GTK_MAJOR_VERSION >= 2 */
#endif /* ! defined (USE_GTK_MAIN_STEP) */
#endif /* defined (GTK_USER_INTERFACE) */

	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(User_interface).  "
			"Unable to allocate memory for structure.");
		user_interface = (struct User_interface *)NULL;
	}
	LEAVE;

	return (user_interface);
} /* CREATE(User_interface) */

int DESTROY(User_interface)(struct User_interface **user_interface_address)
/*******************************************************************************
LAST MODIFIED : 24 July 2002

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct User_interface *user_interface;

	ENTER(DESTROY(User_interface));
	if (user_interface_address && (user_interface = *user_interface_address))
	{
#if defined (LINK_CMISS)
		if (CMISS)
		{
			DESTROY(CMISS_connection)(&CMISS);
		}
#endif /* defined (LINK_CMISS) */
#if defined (MOTIF)
		if ((user_interface->property_notify_callback)&&
			(user_interface->property_notify_widget))
		{
			set_property_notify_callback(user_interface,
				(Property_notify_callback)NULL,NULL,(Widget)NULL);
		}
		if (user_interface->local_machine_info)
		{
			DESTROY(Machine_information)(&(user_interface->local_machine_info));
		}
		if (user_interface->normal_fontlist)
		{
			XmFontListFree(user_interface->normal_fontlist);
			user_interface->normal_fontlist = (XmFontList)NULL;
		}
		if (user_interface->button_fontlist)
		{
			XmFontListFree(user_interface->button_fontlist);
			user_interface->button_fontlist = (XmFontList)NULL;
		}
		if ((user_interface->no_cascade_pixmap)&&(XmUNSPECIFIED_PIXMAP!=
			user_interface->no_cascade_pixmap))
		{
			XFreePixmap(user_interface->display,user_interface->no_cascade_pixmap);
			user_interface->no_cascade_pixmap=XmUNSPECIFIED_PIXMAP;
		}
		XtDestroyApplicationContext(user_interface->application_context);
		XtCloseDisplay(user_interface->display);
#if ! defined (USE_XTAPP_CONTEXT)
		if (user_interface->main_x_connection_callback)
		{
			Event_dispatcher_remove_descriptor_callback(
				user_interface->event_dispatcher,
				user_interface->main_x_connection_callback);
		}
#endif /* ! defined (USE_XTAPP_CONTEXT) */
#endif /* defined (MOTIF) */
#if defined (GTK_USER_INTERFACE)
		if (user_interface->main_window)
		{
			gtk_widget_destroy(user_interface->main_window);
		}
#if ! defined (USE_GTK_MAIN_STEP)
#if GTK_MAJOR_VERSION >= 2
		if (user_interface->g_context)
		{
			g_main_context_release(user_interface->g_context);
			g_main_context_unref(user_interface->g_context);
		}
#endif /* GTK_MAJOR_VERSION >= 2 */
		if (user_interface->gtk_descriptor_callback)
		{
			Event_dispatcher_remove_descriptor_callback(
				user_interface->event_dispatcher,
				user_interface->gtk_descriptor_callback);
		}
#endif /* ! defined (USE_GTK_MAIN_STEP) */
#endif /* defined (GTK_USER_INTERFACE) */
		DEALLOCATE(user_interface);
		*user_interface_address=(struct User_interface *)NULL;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"DESTROY(User_interface).  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(User_interface) */

int User_interface_end_application_loop(struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 7 July 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(User_interface_end_application_loop);
	if (user_interface)
	{
		Event_dispatcher_end_main_loop(user_interface->event_dispatcher);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"User_interface_end_application_loop.  "
			"Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* User_interface_end_application_loop */

#if defined (MOTIF)
Widget User_interface_get_application_shell(struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
Returns the application shell widget
==============================================================================*/
{
	Widget widget;

	ENTER(User_interface_get_application_shell);
	if (user_interface)
	{
		widget = user_interface->application_shell;
	}
	else
	{
		display_message(ERROR_MESSAGE,"User_interface_get_application_shell.  Invalid argument");
		widget = (Widget)NULL;
	}
	LEAVE;

	return (widget);
} /* User_interface_get_application_shell */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
XFontStruct *User_interface_get_normal_font(struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
Returns the application shell widget
==============================================================================*/
{
	XFontStruct *normal_font;

	ENTER(User_interface_get_normal_font);
	if (user_interface)
	{
		normal_font = user_interface->normal_font;
	}
	else
	{
		display_message(ERROR_MESSAGE,"User_interface_get_normal_font.  Invalid argument");
		normal_font = (XFontStruct *)NULL;
	}
	LEAVE;

	return (normal_font);
} /* User_interface_get_normal_font */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
XmFontList User_interface_get_normal_fontlist(struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
Returns the application shell widget
==============================================================================*/
{
	XmFontList normal_fontlist;

	ENTER(User_interface_get_normal_fontlist);
	if (user_interface)
	{
		normal_fontlist = user_interface->normal_fontlist;
	}
	else
	{
		display_message(ERROR_MESSAGE,"User_interface_get_normal_fontlist.  Invalid argument");
		normal_fontlist = (XmFontList)NULL;
	}
	LEAVE;

	return (normal_fontlist);
} /* User_interface_get_normal_fontlist */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
XmFontList User_interface_get_button_fontlist(struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
Returns the application shell widget
==============================================================================*/
{
	XmFontList button_fontlist;

	ENTER(User_interface_get_button_fontlist);
	if (user_interface)
	{
		button_fontlist = user_interface->button_fontlist;
	}
	else
	{
		display_message(ERROR_MESSAGE,"User_interface_get_button_fontlist.  Invalid argument");
		button_fontlist = (XmFontList)NULL;
	}
	LEAVE;

	return (button_fontlist);
} /* User_interface_get_button_fontlist */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
Pixmap User_interface_get_no_cascade_pixmap(struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
Returns the application shell widget
==============================================================================*/
{
	Pixmap pixmap;

	ENTER(User_interface_get_no_cascade_pixmap);
	if (user_interface)
	{
		pixmap = user_interface->no_cascade_pixmap;
	}
	else
	{
		display_message(ERROR_MESSAGE,"User_interface_get_no_cascade_pixmap.  Invalid argument");
		pixmap = XmUNSPECIFIED_PIXMAP;
	}
	LEAVE;

	return (pixmap);
} /* User_interface_get_no_cascade_pixmap */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
Display *User_interface_get_display(struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
Returns the application shell widget
==============================================================================*/
{
	Display *display;

	ENTER(User_interface_get_display);
	if (user_interface)
	{
		display = user_interface->display;
	}
	else
	{
		display_message(ERROR_MESSAGE,"User_interface_get_display.  Invalid argument");
		display = (Display *)NULL;
	}
	LEAVE;

	return (display);
} /* User_interface_get_display */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
int User_interface_get_screen_width(struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
Returns the application shell widget
==============================================================================*/
{
	int screen_width;

	ENTER(User_interface_get_screen_width);
	if (user_interface)
	{
		screen_width = user_interface->screen_width;
	}
	else
	{
		display_message(ERROR_MESSAGE,"User_interface_get_screen_width.  Invalid argument");
		screen_width = 0;
	}
	LEAVE;

	return (screen_width);
} /* User_interface_get_screen_width */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
int User_interface_get_screen_height(struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
Returns the application shell widget
==============================================================================*/
{
	int screen_height;

	ENTER(User_interface_get_screen_height);
	if (user_interface)
	{
		screen_height = user_interface->screen_height;
	}
	else
	{
		display_message(ERROR_MESSAGE,"User_interface_get_screen_height.  "
			"Invalid argument");
		screen_height = 0;
	}
	LEAVE;

	return (screen_height);
} /* User_interface_get_screen_height */
#endif /* defined (MOTIF) */

#if defined (MOTIF) || defined (WIN32_USER_INTERFACE)
int User_interface_get_widget_spacing(struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 24 July 2002

DESCRIPTION :
Returns the widget spacing.
==============================================================================*/
{
	int widget_spacing;

	ENTER(User_interface_get_widget_spacing);
	if (user_interface)
	{
		widget_spacing = user_interface->widget_spacing;
	}
	else
	{
		display_message(ERROR_MESSAGE,"User_interface_get_widget_spacing.  "
			"Invalid argument");
		widget_spacing = 0;
	}
	LEAVE;

	return (widget_spacing);
} /* User_interface_get_widget_spacing */
#endif /* defined (MOTIF) || defined (WIN32_USER_INTERFACE) */

#if defined (WIN32_USER_INTERFACE)
HINSTANCE User_interface_get_instance(struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 20 June 2002

DESCRIPTION :
Returns the application shell widget
==============================================================================*/
{
	HINSTANCE instance;

	ENTER(User_interface_get_instance);
	if (user_interface)
	{
		instance = user_interface->instance;
	}
	else
	{
		display_message(ERROR_MESSAGE,"User_interface_get_instance.  "
			"Invalid argument");
		instance = 0;
	}
	LEAVE;

	return (instance);
} /* User_interface_get_instance */
#endif /* defined (MOTIF) */

#if defined (GTK_USER_INTERFACE)
GtkWidget *User_interface_get_main_window(struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 9 July 2002

DESCRIPTION :
Returns the main window widget
==============================================================================*/
{
	GtkWidget *main_window;

	ENTER(User_interface_get_main_window);
	if (user_interface)
	{
		main_window = user_interface->main_window;
	}
	else
	{
		display_message(ERROR_MESSAGE,"User_interface_get_main_window.  Invalid argument");
		main_window = (GtkWidget *)NULL;
	}
	LEAVE;

	return (main_window);
} /* User_interface_get_main_window */
#endif /* defined (GTK_USER_INTERFACE) */

int User_interface_get_local_machine_name(struct User_interface *user_interface,
	char **name_ptr)
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
If the local machine name is know ALLOCATES and returns a string containing that
name.
==============================================================================*/
{
	char *name;
	int return_code;

	ENTER(User_interface_get_local_machine_name);
	if (user_interface && name_ptr)
	{
		if (user_interface->local_machine_info && 
			user_interface->local_machine_info->name)
		{
			if (ALLOCATE(name, char, strlen(
				user_interface->local_machine_info->name) + 1))
			{
				strcpy(name, user_interface->local_machine_info->name);
				*name_ptr = name;
				return_code = 1;				
			}
			else
			{
				display_message(ERROR_MESSAGE,"User_interface_get_local_machine_name.  "
					"Unable to allocate string for name.");
				return_code = 0;
			}
		}
		else
		{
			/* Name unknown */
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"User_interface_get_local_machine_name.  "
			"Invalid arguments");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* User_interface_get_local_machine_name */

struct Event_dispatcher *User_interface_get_event_dispatcher(
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
Returns the application shell widget
==============================================================================*/
{
	struct Event_dispatcher *event_dispatcher;

	ENTER(User_interface_get_event_dispatcher);
	if (user_interface)
	{
		event_dispatcher = user_interface->event_dispatcher;
	}
	else
	{
		display_message(ERROR_MESSAGE,"User_interface_get_event_dispatcher.  "
			"Invalid argument");
		event_dispatcher = (struct Event_dispatcher *)NULL;
	}
	LEAVE;

	return (event_dispatcher);
} /* User_interface_get_event_dispatcher */

int application_main_step(struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 25 July 1998

DESCRIPTION :
Performs one step of the application_main_step update allowing the programmer
to execute the same main loop elsewhere under special conditions (i.e. waiting
for a response from a modal dialog).
==============================================================================*/
{
	int return_code;
#if defined (OLD_CODE)
#if defined (MOTIF)
	XEvent event;
#endif /* defined (MOTIF) */
#endif /* defined (OLD_CODE) */

	ENTER(application_main_step);
#if defined (OLD_CODE)
#if defined (MOTIF)
	/* SAB We want the function not to block after processing a timer
		event or an input event so I use XtAppProcessEvent instead */
	XtAppNextEvent(user_interface->application_context,&event);
	/* for communication with other applications */
	/* SAB Replaced with an XtEventHandler registered with the correct shell */
	if ((user_interface->property_notify_callback)&&
		(PropertyNotify==event.type))
	{
		(user_interface->property_notify_callback)((XPropertyEvent *)(&event),
			user_interface->property_notify_data,user_interface);
	}
#if defined (EXT_INPUT)
	/* Spaceball and dials so not used */
	input_module_process(&event,user_interface);
#endif
	XtDispatchEvent(&event);
#endif /* defined (MOTIF) */
	XtAppProcessEvent(user_interface->application_context, XtIMAll);
#endif /* defined (OLD_CODE) */
	Event_dispatcher_do_one_event(user_interface->event_dispatcher);
	return_code=1;
	LEAVE;

	return (return_code);
} /* application_main_step */

int
#if defined (WIN32_USER_INTERFACE)
	WINAPI
#endif /* defined (WIN32_USER_INTERFACE) */
	application_main_loop(struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 23 January 2002

DESCRIPTION :
???DB.  Does the main window need to be passed to this ?
???DB.  Should we have our own "WINAPI" (size specifier) ?
==============================================================================*/
{
	int return_code;
#if defined (WIN32_USER_INTERFACE)
	MSG message;
#endif /* defined (WIN32_USER_INTERFACE) */

	ENTER(application_main_loop);
	/* check arguments */
	if (user_interface)
	{
#if defined (MOTIF)
		return_code=1;
#endif /* defined (MOTIF) */
#if defined (WIN32_USER_INTERFACE)
		while (TRUE==GetMessage(&message,NULL,0,0))
		{
#if defined (OLD_CODE)
			/*???DB.  Glen had this.  IsDialogMessage checks if the message is for the
				specified dialog and if it is, processes the message.  The reason for
				doing it was that, if you don't keystrokes such as up arrow and tab get
				translated into change of focus messages rather than being left as
				keystroke messages.  There should be another way of doing this,
				otherwise we're going to have to have similar code for other dialogs
				here */
			/*???DB.  Find out more about TranslateAccelerator */
			/*???DB.  Find out more about dialogs */
			/*???DB.  IsDialogMessage translates arrow keys and tabs into selections,
				but checks to make sure the application wants this via a WM_GETDLGCODE
				message.  Shouldn't this mean that the translations are not done if
				IsDialogMessage is not called ? */
			/*???DB.  Should the command window be a dialog ? */
			if (!global_Command_window||
				(TRUE!=IsDialogMessage(global_Command_window->dialog,&message)))
			{
#endif /* defined (OLD_CODE) */
				TranslateMessage(&message);
				DispatchMessage(&message);
#if defined (OLD_CODE)
			}
#endif /* defined (OLD_CODE) */
		}
		return_code=message.wParam;
#endif /* defined (WIN32_USER_INTERFACE) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"application_main_loop.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* application_main_loop */

#if defined (MOTIF)
static void User_interface_property_notify_callback(Widget w, 
	XtPointer user_interface_void, XEvent *event, Boolean *f)
/*******************************************************************************
LAST MODIFIED : 1 March 2002

DESCRIPTION :
Wrap the old callback so that we can use the XtEvent mechanism instead but keep
the interface basically the same.
==============================================================================*/
{
	struct User_interface *user_interface;

	USE_PARAMETER(w);
	USE_PARAMETER(f);
	ENTER(User_interface_property_notify_callback);
	if (user_interface = (struct User_interface *)user_interface_void)
	{
		if ((user_interface->property_notify_callback)&&
			(PropertyNotify==event->type))
		{
			(user_interface->property_notify_callback)((XPropertyEvent *)(event),
				user_interface->property_notify_data,user_interface);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_property_notify_callback.  Invalid argument(s)");
	}
	LEAVE;

} /* User_interface_property_notify_callback */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
int set_property_notify_callback(struct User_interface *user_interface,
	Property_notify_callback property_notify_callback,void *property_notify_data,
	Widget widget)
/*******************************************************************************
LAST MODIFIED : 8 March 2002

DESCRIPTION :
Sets the <property_notify_callback> for the <user_interface>.  This is used for
communication with other applications.
???DB.  At present only one (not a list)
To clear out the callback pass NULL the <property_notify_callback> and 
<property_notify_data>.
==============================================================================*/
{
	int return_code;

	ENTER(set_property_notify_callback);
	return_code=0;
	/* check arguments */
	if (user_interface)
	{
		if (property_notify_callback)
		{
			user_interface->property_notify_callback=property_notify_callback;
			user_interface->property_notify_data=property_notify_data;
			user_interface->property_notify_widget=widget;
			XtAddEventHandler(widget, PropertyChangeMask, False, 
				User_interface_property_notify_callback, (XtPointer)user_interface);
		}
		else
		{
			XtRemoveEventHandler(user_interface->property_notify_widget, PropertyChangeMask, False, 
				User_interface_property_notify_callback, (XtPointer)user_interface);
			user_interface->property_notify_callback=(Property_notify_callback)NULL;
			user_interface->property_notify_data=NULL;
			user_interface->property_notify_widget=(Widget)NULL;
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_property_notify_callback.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_property_notify_callback */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
int MrmOpenHierarchy_binary_string(char *binary_string, int string_length,
	MrmHierarchy *hierarchy,int *hierarchy_open)
/*******************************************************************************
LAST MODIFIED : 23 December 2005

DESCRIPTION :
This wrapper allows the passing of the <binary_string> which contains a uid file.
<string_length> is required as the binary string may contain NULL characters.
This function writes it to a temporary file which is read using the normal 
MrmOpenHierarchy.
This allows the uid binaries to be kept inside the executable rather than bound
at run time!
If <*hierarchy_open> then 1 is returned, otherwise the full <uid_file_names> are
constructed and the <hierarchy> for those files opened.  1 is returned for
success and 0 for failure.
==============================================================================*/
{
	char *ptr_temp_uid_name,temp_uid_name[L_tmpnam];
	FILE *uid_file;
	int return_code;

	ENTER(MrmOpenHierarchy_binary_string);
	/* check arguments */
	if (binary_string&&hierarchy&&hierarchy_open)
	{
		if (*hierarchy_open)
		{
			return_code=1;
		}
		else
		{
			if (tmpnam(temp_uid_name) && (uid_file=fopen(temp_uid_name, "w")))
			{
				fwrite(binary_string,1,string_length,uid_file);
				fclose(uid_file);
				ptr_temp_uid_name=temp_uid_name;
				if (MrmSUCCESS==MrmOpenHierarchy(1, &ptr_temp_uid_name,
					NULL,hierarchy))
				{
#if defined (DEBUG)
					printf(
						"MrmOpenHierarchy_binary_string.  opened temp file hierachy\n");
#endif /* defined (DEBUG) */
					*hierarchy_open=1;
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"MrmOpenHierarchy_binary_string.  Could not open hierarchy from internal string");
					return_code=0;
				}
				
				remove(temp_uid_name);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MrmOpenHierarchy_binary_string.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MrmOpenHierarchy_binary_string */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
int MrmOpenHierarchy_binary_multiple_strings(int number_of_strings, 
	char **binary_strings, int *string_lengths,
	MrmHierarchy *hierarchy,int *hierarchy_open)
/*******************************************************************************
LAST MODIFIED : 23 December 2005

DESCRIPTION :
This wrapper allows the passing of an array of <binary_strings> which are 
intended to contain a uid files.  
The <string_lengths> are required as the binary string may contain NULL characters.
This function writes a temporary file which is read using the normal 
MrmOpenHierarchy.
This allows the uid binaries to be kept inside the executable rather than bound
at run time!
If <*hierarchy_open> then 1 is returned, otherwise the full <uid_file_names> are
constructed and the <hierarchy> for those files opened.  1 is returned for
success and 0 for failure.
==============================================================================*/
{
	char **temp_uid_names;
	FILE *uid_file;
	int k,length,return_code;

	ENTER(MrmOpenHierarchy_binary_multiple_strings);
	/* check arguments */
	if (binary_strings&&(number_of_strings > 0)&&hierarchy&&hierarchy_open)
	{
		if (*hierarchy_open)
		{
			return_code=1;
		}
		else
		{
			return_code=1;
			if (ALLOCATE(temp_uid_names, char *, number_of_strings))
			{
				for (k=0;k<number_of_strings && return_code;k++)
				{
					if (!(ALLOCATE(temp_uid_names[k], char, L_tmpnam) 
						&& tmpnam(temp_uid_names[k])))
					{
						display_message(ERROR_MESSAGE,
							"MrmOpenHierarchy_binary_multiple_strings.  Unable to allocate temp filename memory.");
						return_code=0;
					}
				}
				if (return_code)
				{
					for (k=0;k<number_of_strings && return_code;k++)
					{
						if (uid_file=fopen(temp_uid_names[k], "w"))
						{
							length=string_lengths[k];
							fwrite(binary_strings[k], 1, length,
								uid_file);
							fclose(uid_file);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"MrmOpenHierarchy_binary_multiple_strings.  Unable to open temporary file %s",
								temp_uid_names[k]);
							return_code=0;
						}
					}
					
					if (return_code)
					{
						if (MrmSUCCESS==MrmOpenHierarchy(number_of_strings, temp_uid_names,
								NULL,hierarchy))
						{
#if defined (DEBUG)
							printf("MrmOpenHierarchy_binary_multiple_strings.  opened temp file hierachy\n");
#endif /* defined (DEBUG) */
							*hierarchy_open=1;
							return_code=1;
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"MrmOpenHierarchy_binary_multiple_strings.  Could not open hierarchy from internal string");
							return_code=0;
						}
						
						for (k=0;k<number_of_strings;k++)
						{
							remove(temp_uid_names[k]);
							DEALLOCATE(temp_uid_names[k]);
						}
						DEALLOCATE(temp_uid_names);
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"MrmOpenHierarchy_binary_multiple_strings.  Could not allocate temp name pointer array");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MrmOpenHierarchy_binary_multiple_strings.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MrmOpenHierarchy_binary_multiple_strings */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
int install_accelerators(Widget widget, Widget top_widget)
/*******************************************************************************
LAST MODIFIED : 24 December 1998

DESCRIPTION :
This travels down the widget tree from <widget> and installs all
accelerators in any subwidgets of <top_widget> in every appropriate subwidget of
<widget>.
==============================================================================*/
{
	int i, num_children, return_code, valid;
	unsigned char row_column_type;
	Widget *child_list;
	WidgetClass widget_class;

	ENTER(install_accelerators);
	if (widget)
	{
		/*don't do for gadgets, as can't have accelerators for them */
		if(True==XtIsWidget(widget))
		{
			valid=1;
			widget_class=XtClass(widget);
			if (widget_class==xmRowColumnWidgetClass)
			{
				XtVaGetValues(widget,
					XmNrowColumnType, &row_column_type,
					NULL);
				if ((XmWORK_AREA!=row_column_type)&&(XmMENU_OPTION!=row_column_type))
				{
					valid=0;
				}
			}
			if ((xmTextWidgetClass==widget_class)||
				(xmTextFieldWidgetClass==widget_class))
			{
				valid=0;
			}
			if (valid)
			{
				XtInstallAllAccelerators(widget, top_widget);				
				if (XtIsComposite(widget))
				{
					XtVaGetValues(widget,
						XmNchildren,&child_list,
						XmNnumChildren,&num_children,
						NULL);
					for (i=0;i<num_children;i++)
					{				
						install_accelerators(child_list[i], top_widget);	
					}
				}			
			}
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"install_accelerators.  Invalid argument(s)");
	}
	LEAVE;

	return(return_code);
} /* install_accelerators */
#endif /* defined (MOTIF) */

