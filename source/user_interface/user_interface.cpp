/*******************************************************************************
FILE : user_interface.c

LAST MODIFIED : 27 December 1999

DESCRIPTION :
Functions for opening and closing the user interface.
==============================================================================*/
#include <stddef.h>
#include <stdlib.h>
/*???debug */
#include <stdio.h>
#include <ctype.h>
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
#if !defined (WINDOWS_DEV_FLAG)
#if defined (EXT_INPUT)
#include "io_devices/input_module.h"
#endif /* defined (EXT_INPUT) */
#endif /* !defined (WINDOWS_DEV_FLAG) */
#if defined (LINK_CMISS)
#include "link/cmiss.h"
#endif /* defined (LINK_CMISS) */
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/* These functions are not ANSI so don't get included in stdlib.h */
extern long a64l(const char *);
extern char *l64a(long);

/*
Module types
------------
*/
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
static unsigned long input_module_delay;
	/*???DB.  Is there another way of doing this ? */
#endif /* defined (EXT_INPUT) */
#endif /* defined (MOTIF) */
#if defined (LINK_CMISS)
static unsigned long cmiss_link_delay;
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
#if defined (EXT_INPUT)
static void process_external_input(XtPointer user_interface_void)
/*******************************************************************************
LAST MODIFIED : 24 June 1996

DESCRIPTION :
Calls the input module update function, so we can get input from external
devices, and then resets the timeout.
==============================================================================*/
{
	struct User_interface *user_interface;

	ENTER(process_external_input);
	if (user_interface=(struct User_interface *)user_interface_void)
	{
		input_module_update(user_interface);
		XtAppAddTimeOut(user_interface->application_context,input_module_delay,
			(XtTimerCallbackProc)process_external_input,user_interface_void);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"process_external_input.  Missing user_interface");
	}
	LEAVE;
} /* process_external_input */
#endif /* defined (EXT_INPUT) */
#endif /* defined (MOTIF) */

#if defined (LINK_CMISS)
#if defined (MOTIF)
static void process_cmiss_link(XtPointer user_interface_void)
/*******************************************************************************
LAST MODIFIED : 18 January 1997

DESCRIPTION :
Calls the input module update function, so we can get input from external
devices, and then resets the timeout.
==============================================================================*/
{
	struct User_interface *user_interface;

	ENTER(process_cmiss_link);
	if (user_interface=(struct User_interface *)user_interface_void)
	{
		/* ???GMH.  This changing of delay is a hack to avoid registering and
		deregistering this callback when CMISS_connection's are created */
		if (CMISS)
		{
			CMISS_connection_update(CMISS);
			/* if connection, then check every tenth of a second */
			cmiss_link_delay=100;
		}
		else
		{
			/* if no connection, then check every 5 seconds */
			cmiss_link_delay=5000;
		}
		XtAppAddTimeOut(user_interface->application_context,cmiss_link_delay,
			(XtTimerCallbackProc)process_cmiss_link,user_interface_void);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"process_cmiss_link.  Missing user_interface");
	}
	LEAVE;
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

#if defined (NEW_BUSY_CURSOR)
int busy_cursor_on(
#if defined (MOTIF)
	Widget excluded_shell,
#endif /* defined (MOTIF) */
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 29 April 1998

DESCRIPTION :
Switchs from the default cursor to the busy cursor for all shells except the
<excluded_shell>.
???DB.  Move in with windowing macros ?
???SAB.  Trying to avoid the need to have a list maintained by us, rather just
	get the available shells.  The problem is that although the parent of each
	shell is the application shell the application shell has no children!
???DB.  Doesn't currently work
==============================================================================*/
{
	int i,number_of_children,return_code;
#if defined (MOTIF)
	Widget *child_list;
	Window window;
#endif /* defined (MOTIF) */

	ENTER(busy_cursor_on);
#if defined (MOTIF)
	XtVaGetValues(user_interface->application_shell,
		XmNchildren,&child_list,
		XmNnumChildren,&number_of_children,
		NULL);
	for (i=0;i<number_of_children;i++)
	{
		if ((excluded_shell!=child_list[i])&&(window=XtWindow(child_list[i])))
		{
			XDefineCursor(XtDisplay(child_list[i]),window,busy_cursor);
		}
	}
	return_code=1;
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
	return_code=0;
#endif /* defined (WINDOWS) */
	LEAVE;

	return (return_code);
} /* busy_cursor_on */
#else /* defined (NEW_BUSY_CURSOR) */
int busy_cursor_on(
#if defined (MOTIF)
	Widget excluded_shell,
#endif /* defined (MOTIF) */
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 23 June 1998

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
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
	return_code=0;
#endif /* defined (WINDOWS) */
	LEAVE;

	return (return_code);
} /* busy_cursor_on */
#endif /* defined (NEW_BUSY_CURSOR) */

#if defined (NEW_BUSY_CURSOR)
int busy_cursor_off(
#if defined (MOTIF)
	Widget excluded_shell,
#endif /* defined (MOTIF) */
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 29 April 1998

DESCRIPTION :
Switchs from the busy cursor to the default cursor for all shells except the
<excluded_shell>.
???DB.  Move in with windowing macros ?
???DB.  Doesn't currently work
==============================================================================*/
{
	int i,number_of_children,return_code;
#if defined (MOTIF)
	Widget *child_list;
	Window window;
#endif /* defined (MOTIF) */

	ENTER(busy_cursor_off);
#if defined (MOTIF)
	XtVaGetValues(user_interface->application_shell,
		XmNchildren,&child_list,
		XmNnumChildren,&number_of_children,
		NULL);
	for (i=0;i<number_of_children;i++)
	{
		if ((excluded_shell!=child_list[i])&&(window=XtWindow(child_list[i])))
		{
			XDefineCursor(XtDisplay(child_list[i]),window,None);
		}
	}
	return_code=1;

#endif /* defined (MOTIF) */
#if defined (WINDOWS)
	return_code=0;
#endif /* defined (WINDOWS) */
	LEAVE;

	return (return_code);
} /* busy_cursor_off */
#else /* defined (NEW_BUSY_CURSOR) */
int busy_cursor_off(
#if defined (MOTIF)
	Widget excluded_shell,
#endif /* defined (MOTIF) */
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 25 November 1999

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
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
	return_code=0;
#endif /* defined (WINDOWS) */
	LEAVE;

	return (return_code);
} /* busy_cursor_off */
#endif /* defined (NEW_BUSY_CURSOR) */

#if defined (MOTIF)
#if defined (TEST_TRUE_COLOUR_VISUAL)
/*???debug.  To test true colour visuals */
Colormap default_colour_map;
int default_depth;
Visual *default_visual;
#endif /* defined (TEST_TRUE_COLOUR_VISUAL) */
#endif /* defined (MOTIF) */

int open_user_interface(struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 27 December 1999

DESCRIPTION :
Open the <user_interface>.
???DB.  Could use to set globals if too much work to pass.
==============================================================================*/
{
	int return_code;
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
			"*-Helvetica-medium-R-*--*-120-*"
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
			"*-Helvetica-medium-r-narrow--*-80-*"
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

	ENTER(open_user_interface);
	/* check arguments */
	if (user_interface)
	{
		user_interface->continue_interface = 1;
		user_interface->shell_list=(struct Shell_list_item *)NULL;
		user_interface->active_shell_stack=(struct Shell_stack_item *)NULL;
#if defined (MOTIF)
		user_interface->no_cascade_pixmap=XmUNSPECIFIED_PIXMAP;
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
#if defined (EXT_INPUT)
				/* conditional stuff in case any graphics routines void the ports */
				/* set the delay for the input_module callback */
				input_module_delay=10;
				/* we need to initialise the input module */
				input_module_init(user_interface);
				XtAppAddTimeOut(user_interface->application_context,input_module_delay,
					(XtTimerCallbackProc)process_external_input,
					(XtPointer)user_interface);
#endif /* defined (EXT_INPUT) */
#if defined (LINK_CMISS)
				/* set the delay for the input_module callback */
				cmiss_link_delay=1000;
				XtAppAddTimeOut(user_interface->application_context,cmiss_link_delay,
					(XtTimerCallbackProc)process_cmiss_link,
					(XtPointer)user_interface);
#endif /* defined (LINK_CMISS) */
				/* retrieve settings */
				XtVaGetApplicationResources(user_interface->application_shell,
					user_interface,resources,XtNumber(resources),NULL);
				/* register identifiers in the global name table */
				identifiers[0].value=(XtPointer)user_interface->widget_spacing;
					/*???DB.  widget_spacing based on screen_width ? */
				identifiers[1].value=(XtPointer)XmFontListCreate(
					user_interface->menu_font,XmSTRING_DEFAULT_CHARSET);
				identifiers[2].value=(XtPointer)XmFontListCreate(
					user_interface->heading_font,XmSTRING_DEFAULT_CHARSET);
				identifiers[3].value=(XtPointer)XmFontListCreate(
					user_interface->normal_font,XmSTRING_DEFAULT_CHARSET);
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
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"open_user_interface.  Unable to register identifiers");
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"open_user_interface.  Unable to create application shell");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"open_user_interface.  Unable to open display");
			return_code=0;
		}
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
		user_interface->widget_spacing=5;
		return_code=1;
#endif /* defined (WINDOWS) */
	}
	else
	{
		display_message(ERROR_MESSAGE,"open_user_interface.  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* open_user_interface */

int close_user_interface(struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 27 December 1999

DESCRIPTION :
???DB.  What should this do ?  Should it be called from inside the main
	application loop to terminate the main application loop ?
==============================================================================*/
{
	int return_code;

	ENTER(close_user_interface);
	return_code=0;
	if (user_interface)
	{
#if defined (LINK_CMISS)
		if (CMISS)
		{
			DESTROY(CMISS_connection)(&CMISS);
		}
#endif /* defined (LINK_CMISS) */
#if defined (MOTIF)
		if ((user_interface->no_cascade_pixmap)&&(XmUNSPECIFIED_PIXMAP!=
			user_interface->no_cascade_pixmap))
		{
			XFreePixmap(user_interface->display,user_interface->no_cascade_pixmap);
			user_interface->no_cascade_pixmap=XmUNSPECIFIED_PIXMAP;
		}
		user_interface->continue_interface = 0;
#if defined (OLD_CODE)
/*???DB.  Bad ! */
		exit(0);
#endif /* defined (OLD_CODE) */
#endif /* defined (MOTIF) */
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"close_user_interface.  Invalid argument");
	}
	LEAVE;

	return (return_code);
} /* close_user_interface */

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
#if defined (MOTIF)
	XEvent event;
#endif /* defined (MOTIF) */

	ENTER(application_main_step);
#if defined (MOTIF)
	XtAppNextEvent(user_interface->application_context,&event);
	/* for communication with other applications */
	if ((user_interface->property_notify_callback)&&
		(PropertyNotify==event.type))
	{
		(user_interface->property_notify_callback)((XPropertyEvent *)(&event),
			user_interface->property_notify_data,user_interface);
	}
#if defined (EXT_INPUT)
	input_module_process(&event,user_interface);
#endif
	XtDispatchEvent(&event);
#endif /* defined (MOTIF) */
	return_code=1;
	LEAVE;

	return (return_code);
} /* application_main_step */

int
#if defined (WINDOWS)
	WINAPI
#endif /* defined (WINDOWS) */
	application_main_loop(struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 25 July 1998

DESCRIPTION :
???DB.  Does the main window need to be passed to this ?
???DB.  Should we have our own "WINAPI" (size specifier) ?
==============================================================================*/
{
	int
#if defined (WINDOWS)
		WINAPI
#endif /* defined (WINDOWS) */
		return_code;
#if defined (WINDOWS)
	MSG message;
#endif /* defined (WINDOWS) */

	ENTER(application_main_loop);
	/* check arguments */
	if (user_interface)
	{
#if defined (MOTIF)
		/* infinite loop */
			/*???DB.  This should not be an infinite loop.  Set exit ? */
		while (user_interface->continue_interface)
		{
			application_main_step(user_interface);
		}
		return_code=1;
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
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
#endif /* defined (WINDOWS) */
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
int set_property_notify_callback(struct User_interface *user_interface,
	Property_notify_callback property_notify_callback,void *property_notify_data)
/*******************************************************************************
LAST MODIFIED : 18 November 1997

DESCRIPTION :
Sets the <property_notify_callback> for the <user_interface>.  This is used for
communication with other applications.
???DB.  At present only one (not a list)
==============================================================================*/
{
	int return_code;

	ENTER(set_property_notify_callback);
	return_code=0;
	/* check arguments */
	if (user_interface&&property_notify_callback)
	{
		user_interface->property_notify_callback=property_notify_callback;
		user_interface->property_notify_data=property_notify_data;
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
int MrmOpenHierarchy_base64_string(char *base64_string,
	MrmHierarchy *hierarchy,int *hierarchy_open)
/*******************************************************************************
LAST MODIFIED : 25 November 1999

DESCRIPTION :
This wrapper allows the passing of the <base64_string> which is intended to
contain a uid file converted to base64.  This function converts it back to
binary and writes a temporary file which is read using the normal 
MrmOpenHierarchy.
This allows the uid binaries to be kept inside the executable rather than bound
at run time!
If <*hierarchy_open> then 1 is returned, otherwise the full <uid_file_names> are
constructed and the <hierarchy> for those files opened.  1 is returned for
success and 0 for failure.
==============================================================================*/
{
	char *ptr_temp_uid_name,temp_uid_name[L_tmpnam],*total_uid;
	FILE *uid_file;
	int i,j,return_code,string_length;
	long uid_long_data;

	ENTER(MrmOpenHierarchy_base64_string);
	/* check arguments */
	if (base64_string&&hierarchy&&hierarchy_open)
	{
		if (*hierarchy_open)
		{
			return_code=1;
		}
		else
		{
			string_length=strlen(base64_string);
			if (tmpnam(temp_uid_name) && (uid_file=fopen(temp_uid_name, "w"))
			  && ALLOCATE(total_uid, char, string_length))
			{
#if defined (DEBUG)
				printf("MrmOpenHierarchy_base64_string.  %s\n", temp_uid_name);
#endif /* defined (DEBUG) */
				j=0;
				for (i=0;i<string_length-1;i+=6)
				{
#if defined (__BYTE_ORDER)
#if (1234==__BYTE_ORDER)
				  char tmp_string[6];
				  tmp_string[0]=base64_string[i + 5];
				  tmp_string[1]=base64_string[i + 4];
				  tmp_string[2]=base64_string[i + 3];
				  tmp_string[3]=base64_string[i + 2];
				  tmp_string[4]=base64_string[i + 1];
				  tmp_string[5]=base64_string[i];
					uid_long_data=a64l(tmp_string);
#else /* (1234==__BYTE_ORDER) */
					uid_long_data=a64l(base64_string + i);
#endif /* (1234==__BYTE_ORDER) */
#else /* defined (__BYTE_ORDER) */
					uid_long_data=a64l(base64_string + i);
#endif /* defined (__BYTE_ORDER) */
					total_uid[j]=(char)(255 & uid_long_data);
					j++;
					total_uid[j]=(char)(255 & (uid_long_data >> 8));
					j++;
					total_uid[j]=(char)(255 & (uid_long_data >> 16));
					j++;
					total_uid[j]=(char)(255 & (uid_long_data >> 24));
					j++;
				}
				fwrite(total_uid,1,4*string_length/6,uid_file);
				DEALLOCATE(total_uid);
				fclose(uid_file);
				ptr_temp_uid_name=temp_uid_name;
				if (MrmSUCCESS==MrmOpenHierarchy(1, &ptr_temp_uid_name,
					NULL,hierarchy))
				{
#if defined (DEBUG)
					printf(
						"MrmOpenHierarchy_base64_string.  opened temp file hierachy\n");
#endif /* defined (DEBUG) */
					*hierarchy_open=1;
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"MrmOpenHierarchy_base64_string.  Could not open hierarchy from internal string");
					return_code=0;
				}
				
				remove(temp_uid_name);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MrmOpenHierarchy_base64_string.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MrmOpenHierarchy_base64_string */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
int MrmOpenHierarchy_base64_multiple_strings(int number_of_strings, 
	char **base64_strings,
	MrmHierarchy *hierarchy,int *hierarchy_open)
/*******************************************************************************
LAST MODIFIED : 19 April 1999

DESCRIPTION :
This wrapper allows the passing of an array of <base64_strings> which are 
intended to contain uid files converted to base64.  
This function converts them all back to binary and writes a temporary files
which are read using the normal MrmOpenHierarchy.
This allows the uid binaries to be kept inside the executable rather than bound
at run time!
If <*hierarchy_open> then 1 is returned, otherwise the full <uid_file_names> are
constructed and the <hierarchy> for those files opened.  1 is returned for
success and 0 for failure.
==============================================================================*/
{
	char **temp_uid_names,*total_uid;
	FILE *uid_file;
	int i,j,k,length,max_string_length,return_code;
	long uid_long_data;

	ENTER(MrmOpenHierarchy_base64_multiple_strings);
	/* check arguments */
	if (base64_strings&&(number_of_strings > 0)&&hierarchy&&hierarchy_open)
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
				max_string_length=0;
				for (k=0;k<number_of_strings && return_code;k++)
				{
					length=strlen(base64_strings[k]);
					if (ALLOCATE(temp_uid_names[k], char, L_tmpnam) 
						&& tmpnam(temp_uid_names[k]))
					{
						if (length > max_string_length)
						{
							max_string_length=length;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"MrmOpenHierarchy_base64_multiple_strings.  Unable to allocate temp filename memory.");
						return_code=0;
					}
				}
				if (return_code)
				{
					if (ALLOCATE(total_uid, char, max_string_length))
					{
						for (k=0;k<number_of_strings && return_code;k++)
						{
							if (uid_file=fopen(temp_uid_names[k], "w"))
							{
								j=0;
								length=strlen(base64_strings[k]);
								for (i=0;i<length-1;i+=6)
								{
#if defined (__BYTE_ORDER)
#if (1234==__BYTE_ORDER)
									char tmp_string[6];
									tmp_string[0]=base64_strings[k][i + 5];
									tmp_string[1]=base64_strings[k][i + 4];
									tmp_string[2]=base64_strings[k][i + 3];
									tmp_string[3]=base64_strings[k][i + 2];
									tmp_string[4]=base64_strings[k][i + 1];
									tmp_string[5]=base64_strings[k][i];
									uid_long_data=a64l(tmp_string);
#else /* (1234==__BYTE_ORDER) */
									uid_long_data=a64l(base64_strings[k] + i);
#endif /* (1234==__BYTE_ORDER) */
#else /* defined (__BYTE_ORDER) */
									uid_long_data=a64l(base64_strings[k] + i);
#endif /* defined (__BYTE_ORDER) */
									total_uid[j]=(char)(255 & uid_long_data);
									j++;
									total_uid[j]=(char)(255 & (uid_long_data >> 8));
									j++;
									total_uid[j]=(char)(255 & (uid_long_data >> 16));
									j++;
									total_uid[j]=(char)(255 & (uid_long_data >> 24));
									j++;
								}
								fwrite(total_uid, 1, 4*length/6,
									uid_file);
								fclose(uid_file);
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"MrmOpenHierarchy_base64_multiple_strings.  Unable to open temporary file %s",
									temp_uid_names[k]);
								return_code=0;
							}
						}
						DEALLOCATE(total_uid);
					}
				
					if (return_code)
					{
						if (MrmSUCCESS==MrmOpenHierarchy(number_of_strings, temp_uid_names,
							NULL,hierarchy))
						{
#if defined (DEBUG)
							printf("MrmOpenHierarchy_base64_multiple_strings.  opened temp file hierachy\n");
#endif /* defined (DEBUG) */
							*hierarchy_open=1;
							return_code=1;
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"MrmOpenHierarchy_base64_multiple_strings.  Could not open hierarchy from internal string");
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
					"MrmOpenHierarchy_base64_multiple_strings.  Could not allocate temp name pointer array");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MrmOpenHierarchy_base64_multiple_strings.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MrmOpenHierarchy_base64_multiple_strings */
#endif /* defined (MOTIF) */

#if defined (OLD_CODE)
#if defined (MOTIF)
int specialized_MrmOpenHierarchy(int number_of_uid_files,char **uid_file_names,
	MrmHierarchy *hierarchy,int *hierarchy_open)
/*******************************************************************************
LAST MODIFIED : 13 December 1996

DESCRIPTION :
If <*hierarchy_open> then 1 is returned, otherwise the full <uid_file_names> are
constructed and the <hierarchy> for those files opened.  1 is returned for
success and 0 for failure.
==============================================================================*/
{
	char uid_data[4], **full_uid_file_names, 
		*ptr_temp_uid_name, temp_uid_name[L_tmpnam], *total_uid;
	int i, j, length_of_uid_directory,return_code;
	long uid_long_data;
	FILE *uid_file;

	ENTER(specialized_MrmOpenHierarchy);
	/* check arguments */
	if ((0<number_of_uid_files)&&uid_file_names&&hierarchy&&hierarchy_open)
	{
		if (*hierarchy_open)
		{
			return_code=1;
		}
		else
		{
#if defined INTERNAL_UIDS
			return_code=0;
			if ((number_of_uid_files==1)&&
				(!strcmp(uid_file_names[0],"command_window.uid")))
			{
				if (tmpnam(temp_uid_name) && (uid_file=fopen(temp_uid_name, "w"))
					&& ALLOCATE(total_uid, char, sizeof(command_window_uid64)))
				{
					printf("specialized_MrmOpenHierarchy.  %s\n", temp_uid_name);
					j=0;
					for (i=0;i<sizeof(command_window_uid64)-1;i += 6)
					{
						uid_long_data=a64l(command_window_uid64 + i);
						total_uid[j]=255 & uid_long_data;
						j++;
						total_uid[j]=255 & (uid_long_data >> 8);
						j++;
						total_uid[j]=255 & (uid_long_data >> 16);
						j++;
						total_uid[j]=255 & (uid_long_data >> 24);
						j++;

					}
					fwrite(total_uid, sizeof(command_window_uid64)-1/6, 4,
						uid_file);

					fclose(uid_file);

					ptr_temp_uid_name=temp_uid_name;
					if (MrmSUCCESS==MrmOpenHierarchy(1, &ptr_temp_uid_name,
						NULL,hierarchy))
					{
						printf("specialized_MrmOpenHierarchy.  opened temp file hierachy\n");
						*hierarchy_open=1;
						return_code=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"specialized_MrmOpenHierarchy.  Could not open hierarchy for pseudo_terminal");
						return_code=0;
					}

					remove(temp_uid_name);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"specialized_MrmOpenHierarchy.  Unable to create temp uid file\ntrying normal file load for uid %s", uid_file_names[0]);
					return_code=0;
				}
			}
			if (!return_code)			
			{
#endif /* defined INTERNAL_UIDS */
			if (uid_directory)
			{
				if (ALLOCATE(full_uid_file_names,char *,number_of_uid_files))
				{
					i=0;
					length_of_uid_directory=strlen(uid_directory);
					while ((i<number_of_uid_files)&&uid_file_names[i]&&
						(ALLOCATE(full_uid_file_names[i],char,length_of_uid_directory+
						strlen(uid_file_names[i])+1)))
					{
						strcpy(full_uid_file_names[i],uid_directory);
						strcat(full_uid_file_names[i],uid_file_names[i]);
						i++;
					}
					if (i<number_of_uid_files)
					{
						while (i>0)
						{
							i--;
							DEALLOCATE(full_uid_file_names[i]);
						}
						DEALLOCATE(full_uid_file_names);
						display_message(ERROR_MESSAGE,
					"specialized_MrmOpenHierarchy.  Error allocating full uid file name");
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
			"specialized_MrmOpenHierarchy.  Could not allocate full uid file names");
					return_code=0;
				}
			}
			else
			{
				full_uid_file_names=uid_file_names;
			}
			if (full_uid_file_names)
			{
				if (MrmSUCCESS==MrmOpenHierarchy(number_of_uid_files,
					full_uid_file_names,NULL,hierarchy))
				{
					*hierarchy_open=1;
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"specialized_MrmOpenHierarchy.  Could not open hierarchy");
					return_code=0;
				}
				if (uid_directory)
				{
					for (i=0;i<number_of_uid_files;i++)
					{
						DEALLOCATE(full_uid_file_names[i]);
					}
					DEALLOCATE(full_uid_file_names);
				}
			}
#if defined INTERNAL_UIDS
			}
#endif /* defined INTERNAL_UIDS */
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"specialized_MrmOpenHierarchy.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* specialized_MrmOpenHierarchy */
#endif /* defined (MOTIF) */
#endif /* defined (OLD_CODE) */

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

