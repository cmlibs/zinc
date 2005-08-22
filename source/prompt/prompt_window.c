/*******************************************************************************
FILE : prompt_window.c

LAST MODIFIED : 23 November 2001

DESCRIPTION :
Management routines for the prompt window.
???DB.  Needs tidying
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
#include <Xm/DialogS.h>
#include <Xm/MwmUtil.h>
#include <Xm/Text.h>
#include "general/debug.h"
#include "prompt/prompt_window.h"
#include "prompt/prompt_window.uidh"
#if defined (OLD_CODE)
#include "socket/socket.h"
#endif /* defined (OLD_CODE) */
#include "user_interface/message.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"
#if defined (LINK_CMISS)
#include "link/cmiss.h"
#endif /* defined (LINK_CMISS) */

#if defined (LINK_CMISS)
extern struct CMISS_connection *CMISS;
#endif /* defined (LINK_CMISS) */
/*
Module variables
----------------
*/
#if defined (MOTIF)
static int prompt_window_hierarchy_open=0;
static MrmHierarchy prompt_window_hierarchy;
#endif /* defined (MOTIF) */

/*
Global functions
----------------
*/
struct Prompt_window *create_Prompt_window(struct Prompt_window **address,
	Widget parent)
/*******************************************************************************
LAST MODIFIED : 13 December 1996

DESCRIPTION :
Create the structures and retrieve the prompt window from the uil file.
==============================================================================*/
{
	MrmType prompt_window_class;
	static MrmRegisterArg callback_list[]=
	{
		{ "get_reply",(XtPointer)get_reply },
		{ "identify_prompt_history",(XtPointer)identify_prompt_history },
		{ "identify_prompt_reply",(XtPointer)identify_prompt_reply }
	};
	static MrmRegisterArg identifier_list[1];
	struct Prompt_window *prompt_window;

	ENTER(create_Prompt_window);
	/* check argument */
	if (parent)
	{
		if (MrmOpenHierarchy_base64_string(prompt_window_uidh,
			&prompt_window_hierarchy,&prompt_window_hierarchy_open))
		{
			if (ALLOCATE(prompt_window,struct Prompt_window,1))
			{
				prompt_window->address=address;
				prompt_window->prompt_history=(Widget)NULL;
				prompt_window->prompt_reply=(Widget)NULL;
				prompt_window->window=(Widget)NULL;
				prompt_window->shell=(Widget)NULL;
				prompt_window->popup=0;
				prompt_window->replied = 0;
				if (prompt_window->shell=XtVaCreatePopupShell("prompt_window_shell",
					xmDialogShellWidgetClass,parent,
					XmNmwmDecorations,MWM_DECOR_ALL|MWM_DECOR_MAXIMIZE,
					XmNmwmFunctions,MWM_FUNC_ALL|MWM_FUNC_MAXIMIZE,
					XmNtransient,FALSE,
					XmNkeyboardFocusPolicy,XmEXPLICIT,
					NULL))
				{
					/* Add destroy callback */
					XtAddCallback(prompt_window->shell,XmNdestroyCallback,
						destroy_Prompt_window,(XtPointer)prompt_window);
					/* Register callbacks in UIL */
					if (MrmSUCCESS==MrmRegisterNamesInHierarchy(prompt_window_hierarchy,
						callback_list,XtNumber(callback_list)))
					{
						identifier_list[0].name="prompt_window_structure";
						identifier_list[0].value=(XtPointer)prompt_window;
						if (MrmSUCCESS==MrmRegisterNamesInHierarchy(prompt_window_hierarchy,
							identifier_list,XtNumber(identifier_list)))
						{
							if (MrmSUCCESS==MrmFetchWidget(prompt_window_hierarchy,
								"prompt_window",prompt_window->shell,&(prompt_window->window),
								&prompt_window_class))
							{
								XtManageChild(prompt_window->window);
								XtRealizeWidget(prompt_window->shell);
								if (address)
								{
									*address=prompt_window;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,"Could not retrieve widget");
								DEALLOCATE(prompt_window);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,"Could not register identifiers");
							DEALLOCATE(prompt_window);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"Could not register callbacks");
						DEALLOCATE(prompt_window);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"Could not create shell");
					DEALLOCATE(prompt_window);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_Prompt_window.  Insufficient memory for prompt_window");
				prompt_window=(struct Prompt_window *)NULL;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_Prompt_window.  Could not open hierarchy");
			prompt_window=(struct Prompt_window *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"create_Prompt_window.  Missing parent");
		prompt_window=(struct Prompt_window *)NULL;
	}
	LEAVE;

	return (prompt_window);
} /* create_Prompt_window */

void destroy_Prompt_window(Widget widget, XtPointer client_data,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 18 February 1997

DESCRIPTION :
Destroy the prompt_window structure and remove the window
==============================================================================*/
{
	struct Prompt_window *prompt_window;

	ENTER(destroy_Prompt_window);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (prompt_window = (struct Prompt_window *)client_data)
	{
		if (prompt_window->address)
		{
			*(prompt_window->address)=(struct Prompt_window *)NULL;
		}
		if (!prompt_window->replied)
		{
#if defined (LINK_CMISS)
			if (CMISS)
			{
				CMISS_connection_process_prompt_reply(CMISS,"r");
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"destroy_Prompt_window.  No CMISS connection");
			}
#else /* LINK_CMISS */
			display_message(ERROR_MESSAGE,
				"destroy_Prompt_window.  Define LINK_CMISS");
#if defined (OLD_CODE)
			write_socket("r",CONN_ID2);
#endif /* defined (OLD_CODE) */
#endif /* LINK_CMISS */
		}
		DEALLOCATE(prompt_window);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"destroy_Prompt_window.  Prompt window structure missing");
	}

	LEAVE;
} /* destroy_Prompt_window */

void get_reply(Widget widget,XtPointer prompt_window_structure,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 4 March 1997

DESCRIPTION :
Called when a reply is entered in the prompt reply area.  Also pops down the
prompt widget.
==============================================================================*/
{
	char *reply;
	struct Prompt_window *prompt_window;

	ENTER(get_reply);
	USE_PARAMETER(call_data);
	if (prompt_window=(struct Prompt_window *)prompt_window_structure)
	{
#if defined (OLD_CODE)
		if (prompt_window->replied)
		{
			display_message(WARNING_MESSAGE,"get_reply.  Prompt already replied to.");
		}
#endif /* defined (OLD_CODE) */
		prompt_window->replied = 1;
		reply=XmTextGetString(widget);
#if defined (LINK_CMISS)
		if (CMISS)
		{
			CMISS_connection_process_prompt_reply(CMISS,reply);
		}
		else
		{
			display_message(ERROR_MESSAGE, "get_reply.  No CMISS connection");
		}
#else /* LINK_CMISS */
		display_message(ERROR_MESSAGE,"get_reply.  Define LINK_CMISS");
#if defined (OLD_CODE)
		write_socket(reply,CONN_ID2);
#endif /* defined (OLD_CODE) */
#endif /* LINK_CMISS */
		XmTextSetString(widget,NULL);
		sprintf(global_temp_string,"%s\n",reply);
		XmTextInsert(prompt_window->prompt_history,
			XmTextGetLastPosition(prompt_window->prompt_history),
			global_temp_string);
		XmTextShowPosition(prompt_window->prompt_history,
			XmTextGetLastPosition(prompt_window->prompt_history));
		XtFree(reply);
	}
	else
	{
		display_message(ERROR_MESSAGE, "Prompt window structure not available.");
	}
	LEAVE;
} /* get_reply */

void identify_prompt_history(Widget widget, XtPointer client_data,
	XtPointer call_data)
/*******************************************************************************
DESCRIPTION:
	Stores the id of the prompt history text area.
==============================================================================*/
{
	struct Prompt_window *prompt_window;

	ENTER(identify_prompt_history);
	USE_PARAMETER(call_data);
	if (prompt_window = (struct Prompt_window *)client_data)
	{
		prompt_window->prompt_history = widget;
	}
	else
	{
		display_message(ERROR_MESSAGE, "Prompt window structure missing");
	}

	LEAVE;
} /* identify_prompt_history */

void identify_prompt_reply(Widget widget, XtPointer client_data,
	XtPointer call_data)
/*******************************************************************************
DESCRIPTION:
	Stores the id of the prompt reply text area.
==============================================================================*/
{
	struct Prompt_window *prompt_window;

	ENTER(identify_prompt_reply);
	USE_PARAMETER(call_data);
	if (prompt_window = (struct Prompt_window *)client_data)
	{
		prompt_window->prompt_reply = widget;
	}
	else
	{
		display_message(ERROR_MESSAGE, "Missing prompt_window");
	}
	LEAVE;
} /* identify_prompt_reply */

int popdown_prompt_window(struct Prompt_window *prompt_window)
/*******************************************************************************
LAST MODIFIED : 23 June 1996

DESCRIPTION :
Pops down the prompt window if it is up.
==============================================================================*/
{
	int return_code;

	ENTER(popdown_prompt_window);
	if (prompt_window)
	{
		if (prompt_window->popup)
		{
			XtRemoveGrab(prompt_window->shell);
			XtPopdown(prompt_window->shell);
			prompt_window->popup = 0;
			XtVaSetValues(prompt_window->prompt_history,
				XmNvalue,"",
				NULL);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE, "Missing prompt_window");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* popdown_prompt_window */

int popup_prompt_window(struct Prompt_window *prompt_window)
/*******************************************************************************
LAST MODIFIED : 23 June 1996

DESCRIPTION :
Pops up the prompt window.
==============================================================================*/
{
	int return_code;

	ENTER(popup_prompt_window);
	if (prompt_window)
	{
		if (!prompt_window->popup)
		{
			XtPopup(prompt_window->shell, XtGrabNone);
			XtAddGrab(prompt_window->shell,True,False);
			prompt_window->popup = 1;
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE, "Missing prompt_window");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* popup_prompt_window */

int write_question(char *question,struct Prompt_window **prompt_window_address,
	Widget parent)
/*******************************************************************************
LAST MODIFIED : 23 June 1996

DESCRIPTION :
Writes a string to the prompt question label, and pops up the window.
==============================================================================*/
{
	int return_code;
	struct Prompt_window *prompt_window;

	ENTER(write_question);
	if (prompt_window_address)
	{
		if (!(prompt_window= *prompt_window_address))
		{
			if (prompt_window=create_Prompt_window(prompt_window_address,parent))
			{
				XmProcessTraversal(prompt_window->prompt_reply,
					XmTRAVERSE_CURRENT);
			}
		}
		if (prompt_window)
		{
			prompt_window->replied = 0; /* ie not replied to yet */
			XtSetKeyboardFocus(prompt_window->shell,prompt_window->prompt_reply);
			XmTextInsert(prompt_window->prompt_history,
				XmTextGetLastPosition(prompt_window->prompt_history),
				question);
			XmTextShowPosition(prompt_window->prompt_history,
				XmTextGetLastPosition(prompt_window->prompt_history));
			popup_prompt_window(prompt_window);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE, "Prompt window cannot be created");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "Missing prompt_window_address");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* write_question */


