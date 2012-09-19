/*******************************************************************************
FILE : user_interface.cpp

LAST MODIFIED : 8 November 2006

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

#if defined (GTK_USER_INTERFACE)
#include <gtk/gtk.h>
#include <glib.h>
#if ( GTK_MAJOR_VERSION < 2 ) || defined (WIN32_SYSTEM)
	/* GTKGLAREA */
#else
#include <gtk/gtkgl.h>
#endif
#endif /* defined (GTK_USER_INTERFACE) */
#if defined (CARBON_USER_INTERFACE)
#include <carbon/carbon.h>
#endif /* defined (CARBON_USER_INTERFACE) */

#include "api/cmiss_zinc_configure.h"
#include "api/cmiss_zinc_ui_configure.h"

#include "general/debug.h"
#include "general/myio.h"
#include "general/message.h"
#include "user_interface/event_dispatcher.h"
#include "user_interface/user_interface.h"

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
#if defined (WX_USER_INTERFACE) || (!defined (WIN32_USER_INTERFACE) && !defined (_MSC_VER))
	const char *application_name;
	char **argv;
	const char *class_name;
	int *argc_address;
#else /* defined (WX_USER_INTERFACE) || (!defined (WIN32_USER_INTERFACE) && !defined (_MSC_VER)) */
	HINSTANCE instance;
	HWND main_window;
	int main_window_state,widget_spacing;
	LPSTR command_line;
#endif /* defined (WIN32_USER_INTERFACE) || defined (_MSC_VER) */
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
	struct Shell_list_item *previous;
	struct Shell_list_item *next;
	/*???DB.  Needed because shells don't have to have UserData */
	struct User_interface *user_interface;
}; /* struct Shell_list_item */

/*???GMH.  We need to generalise the periodic update thingy.  What we need
	is an interface that allows a tool to register a function to get called
	at specific time intervals */

/*
Module functions
----------------
*/


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
#if defined (DEBUG_CODE)
						display_message(ERROR_MESSAGE,
							"User_interface_gtk_gpoll_callback.  "
							"Requested a file descriptor not in our list");
#endif /* defined (DEBUG_CODE) */
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

#if defined (CARBON_USER_INTERFACE)
static OSStatus User_interface_carbon_application_event_handler(
	EventHandlerCallRef handler, EventRef event, void* userData)
{
    OSStatus result = eventNotHandledErr;

    return result;
}
#endif /* defined (CARBON_USER_INTERFACE) */

/*
Global functions
----------------
*/


struct Shell_list_item *create_Shell_list_item(
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

	ENTER(create_Shell_list_item);
	if (ALLOCATE(list_item,struct Shell_list_item,1))
	{
		list_item->user_interface=user_interface;
		/* add item to list */
		list_item->previous=(struct Shell_list_item *)NULL;
		list_item->next=user_interface->shell_list;
		/* point previous first item back to new first item */
		if (list_item->next)
		{
			list_item->next->previous=list_item;
		}
		user_interface->shell_list=list_item;
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

#if defined (WX_USER_INTERFACE) || (!defined (WIN32_USER_INTERFACE) && !defined (_MSC_VER))
struct User_interface *CREATE(User_interface)(int *argc_address, char **argv, 
	struct Event_dispatcher *event_dispatcher, const char *class_name, 
	const char *application_name)
#else /* !defined (WIN32_USER_INTERFACE) && !defined (_MSC_VER) */
struct User_interface *CREATE(User_interface)(HINSTANCE current_instance,
	HINSTANCE previous_instance, LPSTR command_line,int initial_main_window_state,
	int *argc_address, char **argv, struct Event_dispatcher *event_dispatcher)
#endif /* !defined (WIN32_USER_INTERFACE) && !defined (_MSC_VER) */
/*******************************************************************************
LAST MODIFIED : 10 October 2003

DESCRIPTION :
Open the <user_interface>.
==============================================================================*/
{
	struct User_interface *user_interface;

	ENTER(CREATE(User_interface));
#if !defined (WX_USER_INTERFACE) && (defined (WIN32_USER_INTERFACE) || defined (_MSC_VER))
	USE_PARAMETER(previous_instance);
#endif /* defined (WIN32_USER_INTERFACE) || defined (_MSC_VER) */
	if (ALLOCATE(user_interface, struct User_interface, 1))
	{
#if defined (WX_USER_INTERFACE) || (!defined (WIN32_USER_INTERFACE) && !defined (_MSC_VER))
		user_interface->argc_address = argc_address;
		user_interface->argv=argv;
		user_interface->application_name=application_name;
		user_interface->class_name=class_name;
#else /* defined (WIN32_USER_INTERFACE) || defined (_MSC_VER) */
		user_interface->instance=current_instance;
		user_interface->main_window=(HWND)NULL;
		user_interface->main_window_state=initial_main_window_state;
		user_interface->command_line=command_line;
#endif /* defined (WIN32_USER_INTERFACE) || defined (_MSC_VER) */
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
		user_interface->local_machine_info = NULL;
		user_interface->event_dispatcher = event_dispatcher;
		user_interface->shell_list = (struct Shell_list_item *)NULL;
		user_interface->active_shell_stack = (struct Shell_stack_item *)NULL;

		/* get the name of the machine we are running on */
		if (!(user_interface->local_machine_info=CREATE(Machine_information)()))
		{
			display_message(WARNING_MESSAGE,
				"Could not determine local machine information");
		}
#if !defined (WX_USER_INTERFACE) && (defined (WIN32_USER_INTERFACE) || defined (_MSC_VER))
		user_interface->widget_spacing=5;
#endif /* defined (WIN32_USER_INTERFACE) || defined (_MSC_VER) */
#if defined (GTK_USER_INTERFACE)
		/* Initialize i18n support */
		gtk_set_locale ();
		
		/* Initialize the widget set */
		gtk_init (argc_address, &argv);

#if ( GTK_MAJOR_VERSION < 2 ) || defined (WIN32_SYSTEM)
	/* GTKGLAREA */
#else
		/* Initialise gtkglext, stop the gtkglext calling XmuLookupStandardColormap()
			which appears to be buggy on NVIDIA and ATI OpenGL, you have to restart the
			X server after getting BadColor (invalid Colormap parameter) */
		putenv(const_cast<char *>("GDK_GL_NO_STANDARD_COLORMAP=1"));
		gtk_gl_init(argc_address, &argv);
#endif

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

#if defined (CARBON_USER_INTERFACE)
        EventTypeSpec event_type;
        OSStatus error = noErr;
        EventHandlerUPP appCommandProcess;

        if(appCommandProcess = NewEventHandlerUPP(
				  User_interface_carbon_application_event_handler))
		  {
            event_type.eventClass = kEventClassCommand;
            event_type.eventKind = kEventCommandProcess;
            InstallApplicationEventHandler(appCommandProcess, 1, 
					&event_type, NULL, NULL);
        }
		  else
		  {
            error = memFullErr;
		  }
#endif /* defined (CARBON_USER_INTERFACE) */
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
		if (user_interface->local_machine_info)
		{
			DESTROY(Machine_information)(&(user_interface->local_machine_info));
		}
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

#if defined (WIN32_USER_INTERFACE)
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
#endif /* defined (WIN32_USER_INTERFACE) */

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
#endif /* defined (WIN32_USER_INTERFACE) */

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

