/*******************************************************************************
FILE : help.c

LAST MODIFIED : 24 June 1996

DESCRIPTION :
Code for opening and closing and working a CMISS help window.
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
#include <stdio.h>
#include <stddef.h>
#include <X11/Intrinsic.h>
#include <X11/cursorfont.h>
#include <X11/Xlib.h>
#include <Xm/Xm.h>
#include <Xm/DialogS.h>
#include <Xm/MwmUtil.h>
#include <Mrm/MrmPublic.h>
#include <Mrm/MrmDecls.h>

#include "general/debug.h"
#include "help/help_work.h"
#include "help/help.h"
#include "user_interface/message.h"




/**********The Code**********/


/**********Creating and closing a window**********/

struct Help_window *create_help(
	D_FUNC    destroy_func,
	void      *data_ptr
	)
{
static MrmRegisterArg  callbacks[] =
	{
		{"close_help",(XtPointer)close_help},

		{"identify_help_text",(XtPointer)identify_help_text},

		{"identify_help_topic",(XtPointer)identify_help_topic},
		{"changed_help_topic",(XtPointer)changed_help_topic},
		{"return_in_help_topic",(XtPointer)return_in_help_topic},

		{"identify_help_find_button",(XtPointer)identify_help_find_button},
		{"do_help_find_button",(XtPointer)do_help_find_button},

		{"identify_help_do_button",(XtPointer)identify_help_do_button},
		{"do_help_do_button",(XtPointer)do_help_do_button},

		{"identify_help_select_button",(XtPointer)identify_help_select_button},
		{"do_help_select_button",(XtPointer)do_help_select_button},

		{"identify_help_copy_button",(XtPointer)identify_help_copy_button},
		{"do_help_copy_button",(XtPointer)do_help_copy_button},

		{"do_help_close_button",(XtPointer)do_help_close_button}
	};
	static MrmRegisterArg      identifiers[] =
	{
		{"help_window_pointer",(XtPointer)0x00000000}
	};
	struct Help_window  *help_window;
	MrmType    my_class;


	ENTER(create_help);

	if (!(ALLOCATE(help_window,struct Help_window,1)))
	{
		help_window = NULL;
		display_message(CM_MEM_NOT_ALLOC);
	}
	else
	{
		help_window->app_shell = application_shell;
			/*???DB.  application_shell should be passed ? */
		help_window->destroy_func = destroy_func;
		help_window->data_ptr = data_ptr;
		help_window->hierarchy = hierarchy;
		help_window->display = display;

		if (!(help_window->window_shell = XtVaCreatePopupShell("help_shell",
				xmDialogShellWidgetClass,application_shell,
					/*???DB.  application_shell should be passed ? */
				XmNallowShellResize,TRUE,
				XmNmwmDecorations,MWM_DECOR_ALL|MWM_DECOR_MAXIMIZE,
				XmNmwmFunctions,MWM_FUNC_RESIZE|MWM_FUNC_MOVE|MWM_FUNC_MINIMIZE,
				XmNtransient,FALSE,
				NULL)))
		{
			DEALLOCATE(help_window);
			display_message(ERROR_MESSAGE,"Couldn't open the window shell");
		}
		else
		{
			if (MrmRegisterNamesInHierarchy(hierarchy,callbacks,XtNumber(callbacks))
					!= MrmSUCCESS)
			{
				DEALLOCATE(help_window);
				display_message(ERROR_MESSAGE,"Couldn't register callbacks with Motif");
			}
			else
			{
				identifiers[0].value = (XtPointer)help_window;

				if (MrmRegisterNamesInHierarchy(hierarchy,identifiers,
																				XtNumber(identifiers)) !=
						MrmSUCCESS)
				{
					DEALLOCATE(help_window);
			display_message(ERROR_MESSAGE,"Couldn't register a pointer with Motif");
				}
				else
				{
					if (MrmFetchWidget(hierarchy,"cm_help_window",
														help_window->window_shell,&help_window->main_window,
														&my_class) != MrmSUCCESS)
					{
						DEALLOCATE(help_window);
				display_message(ERROR_MESSAGE,"Couldn't find the help window widget");
					}
					else
					{
						if (!destroy_func && data_ptr)
							*((struct Help_window **)data_ptr) = help_window;

						XtManageChild(help_window->main_window);
						XtRealizeWidget(help_window->window_shell);
						XtPopup(help_window->window_shell,XtGrabNone);
					}
				}
			}
		}
	}

	LEAVE;

	return(help_window);
}/*create_help_wind*/


void close_help(
	Widget    caller,
	struct Help_window  *the_window,
	void      *caller_data
	)
{
	ENTER(close_help);

	/*Close me somehow*/

	if (the_window->destroy_func)
		the_window->destroy_func(the_window->data_ptr);
	else if (the_window->data_ptr);
		*((struct Help_window **)the_window->data_ptr) =
			(struct Help_window *)0x00000000;

	DEALLOCATE(the_window);

	LEAVE;
}/*close_help*/


void identify_help_text(
	Widget    caller,
	struct Help_window  *the_window,
	void      *caller_data
	)
{
	ENTER(identify_help_text);

	the_window->help_text = caller;

	LEAVE;
}/*identify_help_text*/


void identify_help_topic(
	Widget    caller,
	struct Help_window  *the_window,
	void      *caller_data
	)
{
	ENTER(identify_help_topic);

	the_window->help_topic = caller;

	LEAVE;
}/*identify_help_topic*/


void changed_help_topic(
	Widget    caller,
	struct Help_window  *the_window,
	void      *caller_data
	)
{
	ENTER(changed_help_topic);

	XtVaSetValues(the_window->help_find_button,
		XmNsensitive,True,
		NULL);

	LEAVE;
}/*identify_help_topic*/


void return_in_help_topic(
	Widget    caller,
	struct Help_window  *the_window,
	XmAnyCallbackStruct  *caller_data
	)
{
	int      find_state;


	ENTER(return_in_help_topic);


	if (caller_data->event->type == KeyPress)
	{
		XtVaGetValues(the_window->help_find_button,
			XmNsensitive,&find_state,
			NULL);

		if (find_state)
			do_help_find_button(the_window->help_find_button,the_window,caller_data);
	}

	LEAVE;
}/*return_in_help_topic*/


void identify_help_find_button(
	Widget    caller,
	struct Help_window  *the_window,
	void      *caller_data
	)
{
	ENTER(identify_help_find_button);

	the_window->help_find_button = caller;

	LEAVE;
}/*identify_help_find_button*/


void do_help_find_button(
	Widget    caller,
	struct Help_window  *the_window,
	void      *caller_data
	)
{
	char        *the_string,
				*str_ptr,
				*the_text;
	char        **the_strs;
	short        num_strs;


	ENTER(do_help_find_button);

	XtVaSetValues(caller,
		XmNsensitive,False,
		NULL);

	XDefineCursor(the_window->display,XtWindow(the_window->main_window),
								XCreateFontCursor(the_window->display,XC_watch));
	XmUpdateDisplay(the_window->main_window);

	str_ptr = the_string = XmTextFieldGetString(the_window->help_topic);
	for ( ; *str_ptr != 0x00 ; str_ptr++)
		*str_ptr = toupper(*str_ptr);

	num_strs = 0;
	the_strs = break_string(the_string,&num_strs);

	if ((the_text = find_help(the_strs,num_strs,
														&the_window->help_from_file,
														the_window->help_file_name)) != -1)
	{
		XmTextSetString(the_window->help_text,the_text);
		DEALLOCATE(the_text);

		if (the_window->help_from_file)
			set_other_buttons(the_window,True);
		else
			set_other_buttons(the_window,False);
	}
	else
	{
		char      error_string[256];

		XtFree(the_string);
		the_string = XmTextFieldGetString(the_window->help_topic);

		strcpy(error_string,"Sorry, no help for ");
		strcat(error_string,the_string);
		XmTextSetString(the_window->help_text,error_string);

		set_other_buttons(the_window,False);
	}

	DEALLOCATE(the_strs);
	XtFree(the_string);

	XUndefineCursor(the_window->display,XtWindow(the_window->main_window));

	LEAVE;
}/*do_help_find_button*/


void identify_help_do_button(
	Widget    caller,
	struct Help_window  *the_window,
	void      *caller_data
	)
{
	ENTER(identify_help_do_button);

	the_window->help_do_button = caller;

	LEAVE;
}/*identify_help_do_button*/


void do_help_do_button(
	Widget    caller,
	struct Help_window  *the_window,
	void      *caller_data
	)
{
	ENTER(do_help_do_button);
	LEAVE;
}/*do_help_do_button*/


void identify_help_select_button(
	Widget    caller,
	struct Help_window  *the_window,
	void      *caller_data
	)
{
	ENTER(identify_help_select_button);

	the_window->help_select_button = caller;

	LEAVE;
}/*identify_help_select_button*/


void do_help_select_button(
	Widget    caller,
	struct Help_window  *the_window,
	void      *caller_data
	)
{
	ENTER(do_help_select_button);
	LEAVE;
}/*do_help_select_button*/


void identify_help_copy_button(
	Widget    caller,
	struct Help_window  *the_window,
	void      *caller_data
	)
{
	ENTER(identify_help_copy_button);

	the_window->help_copy_button = caller;

	LEAVE;
}/*identify_help_copy_button*/


void do_help_copy_button(
	Widget    caller,
	struct Help_window  *the_window,
	void      *caller_data
	)
{
	ENTER(do_help_copy_button);
	LEAVE;
}/*do_help_copy_button*/


void set_other_buttons(
	struct Help_window  *the_window,
	char      set
	)
{
	XtVaSetValues(the_window->help_select_button,
		XmNsensitive,set,
		NULL);

	XtVaSetValues(the_window->help_do_button,
		XmNsensitive,set,
		NULL);

	XtVaSetValues(the_window->help_copy_button,
		XmNsensitive,set,
		NULL);
}/*set_other_buttons*/


void do_help_close_button(
	Widget    caller,
	struct Help_window  *the_window,
	void      *caller_data
	)
{
	XtDestroyWidget(the_window->window_shell);
}/*do_help_close_button*/
