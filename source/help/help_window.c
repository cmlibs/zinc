/*******************************************************************************
FILE : help_window.c

LAST MODIFIED : 24 June 1996

DESCRIPTION :
Code for opening and closing and working a CMISS help window.
==============================================================================*/
#include <stdio.h>
#include <stddef.h>

#include <X11/Intrinsic.h>
#include <X11/cursorfont.h>
#include <X11/Xlib.h>
#include <Xm/Protocols.h>
#include <Xm/Xm.h>
#include <Xm/AtomMgr.h>
#include <Xm/DialogS.h>
#include <Xm/TextF.h>
#include <Xm/MwmUtil.h>
#include <Mrm/MrmPublic.h>
#include <Mrm/MrmDecls.h>

#include "general/debug.h"
#include "help/help_window.h"
#include "help/help_window.uid64"
#include "general/mystring.h"
#include "help/help_work.h"
#include "user_interface/message.h"

/*
Module types
------------
*/
enum Help_id
{
	HELP_TEXT,
	HELP_TOPIC,
	HELP_FIND_BUTTON,
	HELP_DO_BUTTON,
	HELP_SELECT_BUTTON,
	HELP_COPY_BUTTON
}; /* enum Help_id */

/*
Module variables
----------------
*/
static int help_window_hierarchy_open=0;
static MrmHierarchy help_window_hierarchy;

/*
Module functions
----------------
*/
static void help_identify(Widget caller,XtPointer part_id,
	XmAnyCallbackStruct  *caller_data)
/*******************************************************************************
LAST MODIFIED : 19 February 1995

DESCRIPTION :
Identify a part of the help window widget.
==============================================================================*/
{
	struct Help_window *the_window;

	ENTER(help_identify);
	XtVaGetValues(caller,XmNuserData,&the_window,NULL);
	switch ((enum Help_id)part_id)
	{
		case HELP_TEXT:
		{
			the_window->help_text=caller;
		} break;
		case HELP_TOPIC:
		{
			the_window->help_topic=caller;
		} break;
		case HELP_FIND_BUTTON:
		{
			the_window->help_find_button=caller;
		} break;
		case HELP_DO_BUTTON:
		{
			the_window->help_do_button=caller;
		} break;
		case HELP_SELECT_BUTTON:
		{
			the_window->help_select_button=caller;
		} break;
		case HELP_COPY_BUTTON:
		{
			the_window->help_copy_button=caller;
		} break;
	}
	LEAVE;
}/* help_identify */

static void close_help_window_callback(Widget caller,XtPointer  help_window,
	XtPointer caller_data)
/*******************************************************************************
LAST MODIFIED : 19 February 1995

DESCRIPTION :
Callback for when the window is closed and destroyed.  Calls the destroy_func
if necessary and disposes of the window description structure.
==============================================================================*/
{
	struct Help_window *the_window;

	ENTER(close_help_window_callback);
	if (the_window=(struct Help_window *)help_window)
	{
		if (the_window->destroy_func)
		{
			the_window->destroy_func(the_window->data_ptr);
		}
		else
		{
			if (the_window->data_ptr)
			{
				*((struct Help_window **)the_window->data_ptr) =
					(struct Help_window *)0;
			}
		}
		DEALLOCATE(the_window);
	}
	LEAVE;
} /* close_help_window_callback */

static Boolean select_thing_wp(XtPointer to_select)
/*******************************************************************************
LAST MODIFIED : 19 February 1995

DESCRIPTION :
Selects (sets to current tab group thing) the widget specified by to_select.
This is a WorkProc because XmProcessTraversal doesn't seem to work too well
if you pop-up the window and then call XmProcessTraversak before things get
back to the event loop.
==============================================================================*/
{
	ENTER(select_thing_wp);
	XmProcessTraversal((Widget)to_select,XmTRAVERSE_CURRENT);
	LEAVE;

	return (TRUE);
} /* select_thing_wp */

static void changed_help_topic(Widget caller,struct Help_window *the_window,
	XmAnyCallbackStruct  *caller_data)
/*******************************************************************************
LAST MODIFIED : 19 February 1995

DESCRIPTION :
This is called whenever a character is entered in the help topic edit field,
or the text is changed in some other way.  This is so that the "Find"
button can be made sensitive (it becomes insensitive after help has been
found).
==============================================================================*/
{
	ENTER(changed_help_topic);
	XtVaSetValues(the_window->help_find_button,XmNsensitive,True,NULL);
	LEAVE;
} /* identify_help_topic */

static void do_help_find_button(Widget caller,struct Help_window *the_window,
	XmAnyCallbackStruct  *caller_data)
/*******************************************************************************
LAST MODIFIED : 19 February 1995

DESCRIPTION :
Callback for when the find button is pressed.  Calls do_topic_text_help.
==============================================================================*/
{
	ENTER(do_help_find_button);
	do_topic_text_help(the_window);
	LEAVE;
} /* do_help_find_button */

static void return_in_help_topic(Widget caller,struct Help_window *the_window,
	XmAnyCallbackStruct  *caller_data)
/*******************************************************************************
LAST MODIFIED : 19 February 1995

DESCRIPTION :
Called when the return key is pressed in the help topic edit text.  This is
treated the same as pressing the find button.
==============================================================================*/
{
	/* whether or not the find button is sensitive */
	Boolean find_state;

	ENTER(return_in_help_topic);
	if (caller_data->event->type == KeyPress)
	{
		/*Check whether the find button is actually sensitive first.    */
		XtVaGetValues(the_window->help_find_button,
			XmNsensitive,&find_state,
			NULL);
		if (find_state)
		{
			do_topic_text_help(the_window);
		}
	}
	LEAVE;
} /* return_in_help_topic */

static void do_help_other_button(Widget caller,XtPointer part_id,
	XmAnyCallbackStruct  *caller_data)
/*******************************************************************************
LAST MODIFIED : 19 February 1995

DESCRIPTION :
Action when the user presses the "do", "select" or "copy" button.  Currently
nothing.
==============================================================================*/
{
	struct Help_window  *the_window;

	ENTER(do_help_other_button);
	XtVaGetValues(caller,XmNuserData,&the_window,NULL);
	switch((int)part_id)
	{
		case HELP_DO_BUTTON:
		{
		} break;
		case HELP_SELECT_BUTTON:
		{
		} break;
		case HELP_COPY_BUTTON:
		{
		} break;
	}
	LEAVE;
} /* do_help_other_button */

static void set_other_buttons(struct Help_window *the_window,char set)
/*******************************************************************************
LAST MODIFIED : 19 February 1995

DESCRIPTION :
Makes the do, select and copy buttons either sensitive or insensitive.
==============================================================================*/
{
	ENTER(set_other_buttons)
	XtVaSetValues(the_window->help_select_button,XmNsensitive,set,NULL);
	XtVaSetValues(the_window->help_do_button,XmNsensitive,set,NULL);
	XtVaSetValues(the_window->help_copy_button,XmNsensitive,set,NULL);
	LEAVE;
} /* set_other_buttons */

static void do_help_close_button(Widget caller,XtPointer  help_window,
	XtPointer  caller_data)
/*******************************************************************************
LAST MODIFIED : 19 February 1995

DESCRIPTION :
Action for when the close button is pressed.  Actually just pops the window
down, so it can bes used again later.
==============================================================================*/
{
	struct Help_window *the_window;

	ENTER(do_help_close_button);
	if (the_window=(struct Help_window *)help_window)
	{
		pop_down_help_window(the_window);
	}
	LEAVE;
} /* do_help_close_button */

/*
Global functions
----------------
*/
struct Help_window *create_help_window(D_FUNC destroy_func,void *data_ptr)
/*******************************************************************************
LAST MODIFIED : 29 January 1996

DESCRIPTION :
Creates a help window, and returns a pointer to the structure describing the
window.  If <destroy_func> is not NULL, it will be called when the window is
destroyed, with <data_ptr> as a parameter.  If destroy_func is NULL, but
<data_ptr> is not, then when the window is created, a pointer to the window
structure will be stored at the location pointed to by <data_ptr>, and when the
window is destroyed, that location will be cleared.
==============================================================================*/
{
	/* pointer to the help_window structure */
	struct Help_window *help_window;
	/* delete callback thinggy */
		/*???DB.  Get rid of ? */
	Atom WM_DELETE_WINDOW;
	/* dummy variable for Mrm Calls */
	MrmType my_class;
	static MrmRegisterArg  callbacks[]=
	{
		{"help_identify",(XtPointer)help_identify},
		{"changed_help_topic",(XtPointer)changed_help_topic},
		{"return_in_help_topic",(XtPointer)return_in_help_topic},
		{"do_help_find_button",(XtPointer)do_help_find_button},
		{"do_help_other_button",(XtPointer)do_help_other_button},
		{"do_help_close_button",(XtPointer)do_help_close_button}
	};
	static MrmRegisterArg identifiers[]=
	{
		{"help_window_pointer",(XtPointer)NULL},
		{"HELP_TEXT",(XtPointer)HELP_TEXT},
		{"HELP_TOPIC",(XtPointer)HELP_TOPIC},
		{"HELP_FIND_BUTTON",(XtPointer)HELP_FIND_BUTTON},
		{"HELP_DO_BUTTON",(XtPointer)HELP_DO_BUTTON},
		{"HELP_SELECT_BUTTON",(XtPointer)HELP_SELECT_BUTTON},
		{"HELP_COPY_BUTTON",(XtPointer)HELP_COPY_BUTTON}
	};

	ENTER(create_help_window);
	if (MrmOpenHierarchy_base64_string(help_window_uid64,
		&help_window_hierarchy, &help_window_hierarchy_open))
	{
		/* try to allocate space for the help window */
		if (ALLOCATE(help_window,struct Help_window,1))
		{
			/* initialize the fields of the window structure */
			help_window->popped_up=0;
			help_window->window_shell=(Widget)NULL;
			help_window->hierarchy=help_window_hierarchy;
			help_window->display=display;
			help_window->app_shell=application_shell;
				/*???DB.  application_shell should be passed ? */
			help_window->main_window=(Widget)NULL;
			help_window->help_text=(Widget)NULL;
			help_window->help_topic=(Widget)NULL;
			help_window->help_find_button=(Widget)NULL;
			help_window->help_do_button=(Widget)NULL;
			help_window->help_select_button=(Widget)NULL;
			help_window->help_copy_button=(Widget)NULL;
			help_window->destroy_func=destroy_func;
			help_window->data_ptr=data_ptr;
			/* create the help window shell */
			if (help_window->window_shell=XtVaCreatePopupShell("help_shell",
				xmDialogShellWidgetClass,application_shell,
						/*???DB.  application_shell should be passed ? */
				XmNdeleteResponse,XmDO_NOTHING,
				XmNmwmDecorations,MWM_DECOR_ALL|MWM_DECOR_MAXIMIZE,
				XmNmwmFunctions,MWM_FUNC_ALL|MWM_FUNC_MAXIMIZE,
				XmNtransient,FALSE,
				NULL))
			{
				/* register callbacks and identifiers */
				if (MrmSUCCESS==MrmRegisterNamesInHierarchy(help_window_hierarchy,
					callbacks,XtNumber(callbacks)))
				{
					identifiers[0].value=(XtPointer)help_window;
					if (MrmSUCCESS==MrmRegisterNamesInHierarchy(help_window_hierarchy,
						identifiers,XtNumber(identifiers)))
					{
						/* retrieve the help window widget */
						if (MrmSUCCESS==MrmFetchWidget(help_window_hierarchy,"help_window",
							help_window->window_shell,&(help_window->main_window),&my_class))
						{
							/* add a destroy callback to the window shell */
							XtAddCallback(help_window->window_shell,XmNdestroyCallback,
								close_help_window_callback,help_window);
							WM_DELETE_WINDOW=XmInternAtom(
								XtDisplay(help_window->window_shell),"WM_DELETE_WINDOW",FALSE);
							XmAddWMProtocolCallback(help_window->window_shell,
								WM_DELETE_WINDOW,do_help_close_button,(XtPointer)help_window);
							/* manage the widget */
							XtManageChild(help_window->main_window);
							XtRealizeWidget(help_window->window_shell);
							/* set the pointer to us,if required */
							if (!destroy_func && data_ptr)
							{
								*((struct Help_window **)data_ptr)=help_window;
							}
							XmProcessTraversal(help_window->help_topic,XmTRAVERSE_CURRENT);
						}
						else
						{
							XtDestroyWidget(help_window->window_shell);
							DEALLOCATE(help_window);
							display_message(ERROR_MESSAGE,
								"create_help_window.  Couldn't find the help window widget");
						}
					}
					else
					{
						XtDestroyWidget(help_window->window_shell);
						DEALLOCATE(help_window);
						display_message(ERROR_MESSAGE,
							"create_help_window.  Couldn't register a pointer with Motif");
					}
				}
				else
				{
					XtDestroyWidget(help_window->window_shell);
					DEALLOCATE(help_window);
					display_message(ERROR_MESSAGE,
						"create_help_window.  Couldn't register callbacks with Motif");
				}
			}
			else
			{
				DEALLOCATE(help_window);
				display_message(ERROR_MESSAGE,
					"create_help_window.  Couldn't open the window shell");
			}
		}
		else
		{
			help_window=NULL;
			display_message(CM_MEM_NOT_ALLOC);
		}
	}
	else
	{
		help_window=NULL;
		display_message(ERROR_MESSAGE,
			"create_help_window.  Could not open hierarchy");
		display_message(CM_MEM_NOT_ALLOC);
	}
	LEAVE;

	return (help_window);
} /* create_help_window */

void pop_up_help_window(struct Help_window *the_window)
/*******************************************************************************
LAST MODIFIED : 19 February 1995

DESCRIPTION :
Pops up the (already created) help window described in the structure the_window.
==============================================================================*/
{
	ENTER(pop_up_help_window);
	XtPopup(the_window->window_shell,XtGrabNone);
	the_window->popped_up=1;
	/* set up a whole bunch of WorkProcs to select the text.  Hopefully one of
		them will be called after the window appears (when XmProcessTraversal seems
		to start working (I'm not sure why I have to do it this way */
	XtAppAddWorkProc(application_context,select_thing_wp,the_window->help_topic);
	XtAppAddWorkProc(application_context,select_thing_wp,the_window->help_topic);
	XtAppAddWorkProc(application_context,select_thing_wp,the_window->help_topic);
	XtAppAddWorkProc(application_context,select_thing_wp,the_window->help_topic);
	XtAppAddWorkProc(application_context,select_thing_wp,the_window->help_topic);
	LEAVE;
} /* pop_up_help_window */

void pop_down_help_window(struct Help_window *the_window)
/*******************************************************************************
LAST MODIFIED : 19 February 1995

DESCRIPTION :
Pops down (but doesn't destroy) the window described in the_window.
==============================================================================*/

{
	ENTER(pop_down_help_window);
	XtPopdown(the_window->window_shell);
	the_window->popped_up=0;
	LEAVE;
} /* pop_down_help_window */

void close_help_window(XtPointer help_window)
/*******************************************************************************
LAST MODIFIED : 19 February 1995

DESCRIPTION :
Closes and destroys the window pointed to by the window.  After this the
window must be recreated.  The routine relies on close_help_window_callback
to actually dispose of the window description structure and call the
destroy function (if necessary).
==============================================================================*/
{
	struct Help_window *the_window;

	ENTER(close_help_window);
	if (the_window=(struct Help_window *)help_window)
	{
		XtPopdown(the_window->window_shell);
		the_window->popped_up=0;
		XtDestroyWidget(the_window->window_shell);
	}
	LEAVE;
} /* close_help_window */

void do_topic_text_help(struct Help_window *the_window)
/*******************************************************************************
LAST MODIFIED : 19 February 1995

DESCRIPTION :
Gets help on the string in the help topic edit field.
==============================================================================*/
{
	/* the edit field string */
	char *the_string;
	/* pointers to the words in the string */
	char **the_strings;
	/* the number of words in the string */
	short num_strings;

	ENTER(do_topic_text_help);
	/* get the topic text, and convert it to upper case */
	the_string=(char *)XmTextFieldGetString(the_window->help_topic);
	/* break the string into words */
	num_strings=0;
	the_strings=break_string(the_string,&num_strings);
	/* get help on the words */
	do_strings_help(the_window,the_strings,num_strings);
	DEALLOCATE(the_strings);
	XtFree(the_string);
	LEAVE;
} /* do_topic_text_help */

void do_strings_help(struct Help_window *the_window,char **the_strings,
	short num_strings)
/*******************************************************************************
LAST MODIFIED : 19 February 1995

DESCRIPTION :
Gets help on the keywords in the_strings, and displays the help in the_window.
==============================================================================*/
{
	char *the_text;

	ENTER(do_strings_help);
	/* make the find button insensitive */
	XtVaSetValues(the_window->help_find_button,
		XmNsensitive,False,
		NULL);
	if (!num_strings)
	{
		char error_string[128];

		/* write a message saying there was nothing to find */
		strcpy(error_string,"Sorry, there's nothing to find.");
		XmTextSetString(the_window->help_text,error_string);
		set_other_buttons(the_window,False);
	}
	else
	{
		/* set the watch cursor */
		XDefineCursor(the_window->display,XtWindow(the_window->main_window),
			XCreateFontCursor(the_window->display,XC_watch));
		XmUpdateDisplay(the_window->main_window);
		/* call parse_help_strings to get the help text */
		if ((the_text=parse_help_strs(the_strings,num_strings,
			&(the_window->help_from_file),the_window->help_file_name))!=(char *)-1)
		{
			/* write the text in the window */
			XmTextSetString(the_window->help_text,the_text);
			DEALLOCATE(the_text);
			/* if the help came from a single file, set the do, copy and select
				buttons */
			if (the_window->help_from_file)
			{
				set_other_buttons(the_window,True);
			}
			else
			{
				set_other_buttons(the_window,False);
			}
		}
		else
		{
			char error_string[128];

			/* write a message saying we couldn't find help */
			strcpy(error_string,"Sorry, no help for ");
			reassemble_string(error_string,the_strings,num_strings);
			XmTextSetString(the_window->help_text,error_string);
			set_other_buttons(the_window,False);
		}
		XUndefineCursor(the_window->display,XtWindow(the_window->main_window));
	}
	LEAVE;
}/* do_strings_help */
