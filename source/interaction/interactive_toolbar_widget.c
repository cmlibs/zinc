/*******************************************************************************
FILE : interactive_toolbar_widget.c

LAST MODIFIED : 30 May 2001

DESCRIPTION :
Widget for choosing the Interactive_tool currently in-use in a dialog from a
series of icons presented as radio buttons.
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
#include <Xm/LabelG.h>
#include <Xm/RowColumn.h>
#include <Xm/ToggleB.h>
#include <stdlib.h>
#include "general/debug.h"
#include "interaction/interactive_toolbar_widget.h"
#include "motif/image_utilities.h"
#include "user_interface/message.h"

/*
Module types
------------
*/

struct Interactive_toolbar_widget_struct
/*******************************************************************************
LAST MODIFIED : 8 May 2000

DESCRIPTION :
Contains all the information carried by the interactive_toolbar widget.
Note that we just hold a pointer to the interactive_toolbar, and must access and
deaccess it.
==============================================================================*/
{
	struct MANAGER(Interactive_tool) *interactive_tool_manager;
	void *interactive_tool_manager_callback_id;
	struct Interactive_tool *current_interactive_tool,
		*last_updated_interactive_tool;
	struct Callback_data update_callback;
	Widget widget,widget_parent;
}; /* Interactive_toolbar_widget_struct */

/*
Module functions
----------------
*/

static Widget interactive_toolbar_get_widget_for_interactive_tool(
	struct Interactive_toolbar_widget_struct *interactive_toolbar,
	struct Interactive_tool *interactive_tool)
/*******************************************************************************
LAST MODIFIED : 8 May 2000

DESCRIPTION :
Returns the widget representing <interactive_tool> in the <interactive_toolbar>.
Returns NULL if the tool is not in the toolbar.
==============================================================================*/
{
	int i,num_children;
	struct Interactive_tool *this_interactive_tool;
	Widget *child_list,widget;

	ENTER(interactive_toolbar_get_widget_for_interactive_tool);
	widget=(Widget)NULL;
	if (interactive_toolbar&&interactive_tool)
	{
		XtVaGetValues(interactive_toolbar->widget,
			XmNnumChildren,&num_children,
			XmNchildren,&child_list,
			NULL);
		for (i=0;(i<num_children)&&(!widget);i++)
		{
			this_interactive_tool=(struct Interactive_tool *)NULL;
			XtVaGetValues(child_list[i],XmNuserData,&this_interactive_tool,NULL);
			if (this_interactive_tool == interactive_tool)
			{
				widget=child_list[i];
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"interactive_toolbar_get_widget_for_interactive_tool.  "
			"Invalid argument(s)");
	}
	LEAVE;

	return (widget);
} /* interactive_toolbar_get_widget_for_interactive_tool */

static void interactive_toolbar_widget_update(
	struct Interactive_toolbar_widget_struct *interactive_toolbar)
/*******************************************************************************
LAST MODIFIED : 8 May 2000

DESCRIPTION :
Tells the parent dialog the current_node has changed; sends a pointer to the
current node.
==============================================================================*/
{
	ENTER(interactive_toolbar_widget_update);
	if (interactive_toolbar)
	{
		if (interactive_toolbar->current_interactive_tool !=
			interactive_toolbar->last_updated_interactive_tool)
		{
			interactive_toolbar->last_updated_interactive_tool =
				interactive_toolbar->current_interactive_tool;
			if (interactive_toolbar->update_callback.procedure)
			{
				/* now call the procedure with the user data and selected tool */
				(interactive_toolbar->update_callback.procedure)(
					interactive_toolbar->widget,
					interactive_toolbar->update_callback.data,
					interactive_toolbar->current_interactive_tool);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"interactive_toolbar_widget_update.  Invalid argument(s)");
	}
	LEAVE;
} /* interactive_toolbar_widget_update */

static void interactive_toolbar_widget_destroy_CB(Widget widget,
	void *interactive_toolbar_void,void *call_data)
/*******************************************************************************
LAST MODIFIED : 8 May 2000

DESCRIPTION :
Callback for when the interactive_toolbar widget is destroyed. Tidies up all
dynamic memory allocations, pointers and callbacks.
==============================================================================*/
{
	struct Interactive_toolbar_widget_struct *interactive_toolbar;

	ENTER(interactive_toolbar_widget_destroy_CB);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (interactive_toolbar=
		(struct Interactive_toolbar_widget_struct *)interactive_toolbar_void)
	{
		if (interactive_toolbar->interactive_tool_manager_callback_id)
		{
			MANAGER_DEREGISTER(Interactive_tool)(
				interactive_toolbar->interactive_tool_manager_callback_id,
				interactive_toolbar->interactive_tool_manager);
		}
		DEALLOCATE(interactive_toolbar);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"interactive_toolbar_widget_destroy_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* interactive_toolbar_widget_destroy_CB */

static void interactive_toolbar_widget_entry_CB(Widget widget,
	void *interactive_toolbar_void,void *call_data)
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
Callback from interactive tool buttons - change of tool.
==============================================================================*/
{
	struct Interactive_tool *interactive_tool;
	struct Interactive_toolbar_widget_struct *interactive_toolbar;
	Widget toggle_button;

	ENTER(interactive_toolbar_widget_entry_CB);
	USE_PARAMETER(widget);
	if (call_data&&(interactive_toolbar=
		(struct Interactive_toolbar_widget_struct *)interactive_toolbar_void))
	{
		/* get the toggle_button from the call data */
		if (toggle_button=((XmRowColumnCallbackStruct *)call_data)->widget)
		{
			if (XmToggleButtonGetState(toggle_button))
			{
				interactive_tool=(struct Interactive_tool *)NULL;
				XtVaGetValues(toggle_button,XmNuserData,&interactive_tool,NULL);
				if (interactive_tool)
				{
					if (interactive_tool != interactive_toolbar->current_interactive_tool)
					{
						interactive_toolbar->current_interactive_tool=interactive_tool;
						interactive_toolbar_widget_update(interactive_toolbar);
					}
					else
					{
						Interactive_tool_bring_up_dialog(interactive_tool);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"interactive_toolbar_widget_entry_CB.  Missing interactive_tool");
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"interactive_toolbar_widget_entry_CB.  Missing toggle_button");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"interactive_toolbar_widget_entry_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* interactive_toolbar_widget_entry_CB */

int Interactive_tool_remove_from_interactive_toolbar_widget(
	struct Interactive_tool *interactive_tool, void *interactive_toolbar_void)
/*******************************************************************************
LAST MODIFIED : 30 May 2001

DESCRIPTION :
Makes sure there is no widget for <interactive_tool> in the
<interactive_toolbar>.
==============================================================================*/
{
	int return_code;
	struct Interactive_toolbar_widget_struct *interactive_toolbar;
	Widget widget;

	ENTER(Interactive_tool_remove_from_interactive_toolbar_widget);
	if (interactive_tool && (interactive_toolbar =
		(struct Interactive_toolbar_widget_struct *)interactive_toolbar_void))
	{
		if (widget = interactive_toolbar_get_widget_for_interactive_tool(
			interactive_toolbar, interactive_tool))
		{
			XtDestroyWidget(widget);
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Interactive_tool_remove_from_interactive_toolbar_widget.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Interactive_tool_remove_from_interactive_toolbar_widget */

static void interactive_toolbar_widget_interactive_tool_change(
	struct MANAGER_MESSAGE(Interactive_tool) *message,
	void *interactive_toolbar_void)
/*******************************************************************************
LAST MODIFIED : 30 May 2001

DESCRIPTION :
One or more of the interactive_tools have changed in the manager. Removes
managed tools from the toolbar in response to MANAGER_CHANGE_REMOVE messages.
???RC May want to respond to change object functions, eg. to swap icons as
functions of tool changes.
==============================================================================*/
{
	ENTER(interactive_toolbar_widget_interactive_tool_change);
	if (message && interactive_toolbar_void)
	{
		switch (message->change)
		{
			case MANAGER_CHANGE_REMOVE(Interactive_tool):
			{
				FOR_EACH_OBJECT_IN_LIST(Interactive_tool)(
					Interactive_tool_remove_from_interactive_toolbar_widget,
					interactive_toolbar_void, message->changed_object_list);
			} break;
			case MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Interactive_tool):
			case MANAGER_CHANGE_OBJECT(Interactive_tool):
			case MANAGER_CHANGE_ADD(Interactive_tool):
			case MANAGER_CHANGE_IDENTIFIER(Interactive_tool):
			{
				/* do nothing */
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"interactive_toolbar_widget_interactive_tool_change.  "
			"Invalid argument(s)");
	}
	LEAVE;
} /* interactive_toolbar_widget_interactive_tool_change */

/*
Global functions
----------------
*/

Widget create_interactive_toolbar_widget(Widget parent,
	struct MANAGER(Interactive_tool) *interactive_tool_manager,
	enum Interactive_toolbar_orientation orientation)
/*******************************************************************************
LAST MODIFIED : 28 August 2000

DESCRIPTION :
Creates a RowColumn widget for storing a set of radio buttons for selecting the
current Interactive_tool. Tools must be added with the
interactive_toolbar_widget_add_tool function; the <interactive_tool_manager> is
supplied to allow tools to be automatically removed from the tool bar if they
are removed from the manager - will have no effect on any unmanaged tools added
to the toolbar.
==============================================================================*/
{
	Arg args[14];
	struct Interactive_toolbar_widget_struct *interactive_toolbar;
	Widget return_widget;

	ENTER(create_interactive_toolbar_widget);
	return_widget=(Widget)NULL;
	if (parent&&interactive_tool_manager)
	{
		/* allocate memory */
		if (ALLOCATE(interactive_toolbar,
			struct Interactive_toolbar_widget_struct,1))
		{
			XtSetArg(args[0],XmNleftAttachment,XmATTACH_FORM);
			XtSetArg(args[1],XmNrightAttachment,XmATTACH_FORM);
			XtSetArg(args[2],XmNtopAttachment,XmATTACH_FORM);
			XtSetArg(args[3],XmNbottomAttachment,XmATTACH_FORM);
			if (INTERACTIVE_TOOLBAR_HORIZONTAL == orientation)
			{
				XtSetArg(args[4],XmNorientation,XmHORIZONTAL);
			}
			else
			{
				XtSetArg(args[4],XmNorientation,XmVERTICAL);
			}
			XtSetArg(args[5],XmNpacking,XmPACK_TIGHT);
			XtSetArg(args[6],XmNradioBehavior,True);
			XtSetArg(args[7],XmNuserData,interactive_toolbar);
			XtSetArg(args[8],XmNmarginHeight,0);
			XtSetArg(args[9],XmNmarginWidth,0);
			XtSetArg(args[10],XmNspacing,0);
			XtSetArg(args[11],XmNadjustLast,False);
			XtSetArg(args[12],XmNradioAlwaysOne,True);
			if (interactive_toolbar->widget =
				XmCreateRowColumn(parent, "interactive_toolbar", args, 13))
			{
				/* add destroy callback for widget */
				XtAddCallback(interactive_toolbar->widget,XmNdestroyCallback,
					interactive_toolbar_widget_destroy_CB,(XtPointer)interactive_toolbar);
				/* add entry callback for the widget */
				XtAddCallback(interactive_toolbar->widget,XmNentryCallback,
					interactive_toolbar_widget_entry_CB,(XtPointer)interactive_toolbar);
				interactive_toolbar->interactive_tool_manager=interactive_tool_manager;
				interactive_toolbar->interactive_tool_manager_callback_id=(void *)NULL;
				interactive_toolbar->current_interactive_tool=
					(struct Interactive_tool *)NULL;
				interactive_toolbar->last_updated_interactive_tool=
					(struct Interactive_tool *)NULL;
				interactive_toolbar->update_callback.procedure=
					(Callback_procedure *)NULL;
				interactive_toolbar->update_callback.data=NULL;
				/* initialise the widgets */
				interactive_toolbar->widget_parent=parent;
				/* register for Interactive_tool manager callbacks */
				interactive_toolbar->interactive_tool_manager_callback_id=
					MANAGER_REGISTER(Interactive_tool)(
						interactive_toolbar_widget_interactive_tool_change,
						(void *)interactive_toolbar,interactive_tool_manager);
				XtManageChild(interactive_toolbar->widget);
				return_widget=interactive_toolbar->widget;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_interactive_toolbar_widget.  Could not create widget");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_interactive_toolbar_widget.  Not enough memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_interactive_toolbar_widget.  Invalid argument(s)");
	}
	LEAVE;

	return (return_widget);
} /* create_interactive_toolbar_widget */

int interactive_toolbar_widget_set_callback(Widget interactive_toolbar_widget,
	struct Callback_data *callback)
/*******************************************************************************
LAST MODIFIED : 8 May 2000

DESCRIPTION :
Sets callback for when a different interactive_tool is chosen in the toolbar.
==============================================================================*/
{
	int return_code;
	struct Interactive_toolbar_widget_struct *interactive_toolbar;

	ENTER(interactive_toolbar_widget_set_callback);
	if (interactive_toolbar_widget&&callback)
	{
		interactive_toolbar=(struct Interactive_toolbar_widget_struct *)NULL;
		/* get the interactive_toolbar structure from the widget */
		XtVaGetValues(interactive_toolbar_widget,
			XmNuserData,&interactive_toolbar,NULL);
		if (interactive_toolbar)
		{
			interactive_toolbar->update_callback.procedure=callback->procedure;
			interactive_toolbar->update_callback.data=callback->data;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"interactive_toolbar_widget_set_callback.  Missing widget data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"interactive_toolbar_widget_set_callback.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* interactive_toolbar_widget_set_callback */

struct
Interactive_tool *interactive_toolbar_widget_get_current_interactive_tool(
	Widget interactive_toolbar_widget)
/*******************************************************************************
LAST MODIFIED : 8 May 2000

DESCRIPTION :
Returns the currently chosen interactive_tool in <interactive_toolbar_widget>.
==============================================================================*/
{
	struct Interactive_tool *interactive_tool;
	struct Interactive_toolbar_widget_struct *interactive_toolbar;

	ENTER(interactive_toolbar_widget_get_current_interactive_tool);
	if (interactive_toolbar_widget)
	{
		interactive_toolbar=(struct Interactive_toolbar_widget_struct *)NULL;
		/* get the interactive_toolbar structure from the widget */
		XtVaGetValues(interactive_toolbar_widget,
			XmNuserData,&interactive_toolbar,NULL);
		if (interactive_toolbar)
		{
			interactive_tool=interactive_toolbar->current_interactive_tool;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"interactive_toolbar_widget_get_current_interactive_tool.  "
				"Missing widget data");
			interactive_tool=(struct Interactive_tool *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"interactive_toolbar_widget_get_current_interactive_tool.  "
			"Invalid argument(s)");
		interactive_tool=(struct Interactive_tool *)NULL;
	}
	LEAVE;

	return (interactive_tool);
} /* interactive_toolbar_widget_get_current_interactive_tool */

int interactive_toolbar_widget_set_current_interactive_tool(
	Widget interactive_toolbar_widget,struct Interactive_tool *interactive_tool)
/*******************************************************************************
LAST MODIFIED : 13 June 2000

DESCRIPTION :
Sets the current interactive_tool in the <interactive_toolbar_widget>.
==============================================================================*/
{
	int return_code;
	struct Interactive_toolbar_widget_struct *interactive_toolbar;
	Widget toggle_button;

	ENTER(interactive_toolbar_widget_set_current_interactive_tool);
	if (interactive_toolbar_widget&&interactive_tool)
	{
		interactive_toolbar=(struct Interactive_toolbar_widget_struct *)NULL;
		/* get the interactive_toolbar structure from the widget */
		XtVaGetValues(interactive_toolbar_widget,
			XmNuserData,&interactive_toolbar,NULL);
		if (interactive_toolbar)
		{
			if (interactive_tool != interactive_toolbar->current_interactive_tool)
			{
				interactive_toolbar->last_updated_interactive_tool=interactive_tool;
				if (toggle_button=interactive_toolbar_get_widget_for_interactive_tool(
					interactive_toolbar,interactive_tool))
				{
					XmToggleButtonSetState(toggle_button,/*state*/True,/*notify*/False);
					/* uncheck the last tool */
					if (toggle_button=interactive_toolbar_get_widget_for_interactive_tool(
						interactive_toolbar,interactive_toolbar->current_interactive_tool))
					{
						XmToggleButtonSetState(toggle_button,/*state*/False,
							/*notify*/False);
					}
					interactive_toolbar->current_interactive_tool=interactive_tool;
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"interactive_toolbar_widget_set_current_interactive_tool.  "
						"Tool is not in toolbar");
					return_code=0;
					/* tell the client the tool is left as is was */
					interactive_toolbar_widget_update(interactive_toolbar);
				}
			}
			else
			{
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"interactive_toolbar_widget_set_current_interactive_tool.  "
				"Missing widget data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"interactive_toolbar_widget_set_current_interactive_tool.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* interactive_toolbar_widget_set_current_interactive_tool */

int interactive_toolbar_widget_add_interactive_tool(
	Widget interactive_toolbar_widget,struct Interactive_tool *interactive_tool)
/*******************************************************************************
LAST MODIFIED : 21 November 2001

DESCRIPTION :
Adds <interactive_tool> to the toolbar widget. Tool is represented by a button
with the tool's icon on it. If this is the first tool added to the toolbar then
it is automatically chosen as the current tool.
==============================================================================*/
{
	Arg args[8];
	Display *display;
	int depth, return_code;
	struct Cmgui_image *image;
	struct Interactive_toolbar_widget_struct *interactive_toolbar;
	Pixel pixel;
	Pixmap pixmap;
	struct Colour background, foreground;
	Widget widget;

	ENTER(interactive_toolbar_widget_add_interactive_tool);
	if (interactive_toolbar_widget&&interactive_tool)
	{
		interactive_toolbar=(struct Interactive_toolbar_widget_struct *)NULL;
		/* get the interactive_toolbar structure from the widget */
		XtVaGetValues(interactive_toolbar_widget,
			XmNuserData,&interactive_toolbar,NULL);
		if (interactive_toolbar)
		{
			if (widget=interactive_toolbar_get_widget_for_interactive_tool(
				interactive_toolbar,interactive_tool))
			{
				display_message(WARNING_MESSAGE,
					"interactive_toolbar_widget_add_interactive_tool.  "
					"Interactive tool already in toolbar");
				return_code=1;
			}
			else
			{
				display = XtDisplay(interactive_toolbar_widget);
				XtVaGetValues(interactive_toolbar_widget, XmNforeground,
					&pixel, NULL);
				convert_Pixel_to_Colour(display, pixel, &foreground);
				XtVaGetValues(interactive_toolbar_widget, XmNbackground,
					&pixel, NULL);
				convert_Pixel_to_Colour(display, pixel, &background);
				XtVaGetValues(interactive_toolbar_widget, XmNdepth,
					&depth, NULL);
				if (image=Interactive_tool_get_icon(&foreground,
					&background, interactive_tool))
				{
					if (pixmap = create_Pixmap_from_Cmgui_image(display, image,
						depth))
					{
						XtSetArg(args[0], XmNlabelType, XmPIXMAP);
						XtSetArg(args[1], XmNlabelPixmap, pixmap);
						XtSetArg(args[2], XmNindicatorOn, False);
						XtSetArg(args[3], XmNhighlightThickness, 0);
						XtSetArg(args[4], XmNset, False);
						XtSetArg(args[5], XmNspacing, 0);
						XtSetArg(args[6], XmNshadowThickness, 2);
						XtSetArg(args[7], XmNuserData, (void *)interactive_tool);
						if (widget=XmCreateToggleButton(interactive_toolbar_widget,
							"interactive_button",args, 8))
						{
							/* work-around since there is an open motif bug that makes
								toggle buttons unreleasable with XmONE_OF_MANY */
							XtVaSetValues(widget, XmNindicatorType, XmN_OF_MANY, NULL);
							if (!(interactive_toolbar->current_interactive_tool))
							{
								XmToggleButtonSetState(widget,True,False);
								interactive_toolbar->current_interactive_tool=interactive_tool;
								interactive_toolbar_widget_update(interactive_toolbar);
							}
							XtManageChild(widget);
						}
						return_code=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"interactive_toolbar_widget_add_interactive_tool.  "
							"Could not create pixmap from Cmgui_image.");
						return_code=0;
					}
					DESTROY(Cmgui_image)(&image);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"interactive_toolbar_widget_add_interactive_tool.  "
						"Could not get icon for interactive tool.");
					return_code=0;
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"interactive_toolbar_widget_add_interactive_tool.  "
				"Missing widget data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"interactive_toolbar_widget_add_interactive_tool.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* interactive_toolbar_widget_add_interactive_tool */

int add_interactive_tool_to_interactive_toolbar_widget(
	struct Interactive_tool *interactive_tool,
	void *interactive_toolbar_widget_void)
/*******************************************************************************
LAST MODIFIED : 8 May 2000

DESCRIPTION :
Iterator version of interactive_toolbar_widget_add_interactive_tool.
==============================================================================*/
{
	int return_code;

	ENTER(add_interactive_tool_to_interactive_toolbar_widget);
	return_code=interactive_toolbar_widget_add_interactive_tool(
		(Widget)interactive_toolbar_widget_void,interactive_tool);
	LEAVE;

	return (return_code);
} /* add_interactive_tool_to_interactive_toolbar_widget */

#if defined (OLD_CODE)
char **interactive_toolbar_get_tool_names(Widget interactive_toolbar_widget,
	int *number_of_tools,struct Interactive_tool *current_interactive_tool,
	char **current_interactive_tool_string)
/*******************************************************************************
LAST MODIFIED : 12 June 2000

DESCRIPTION :
Returns an array of strings containing the names of the tools - suitable for
choosing in a text command. On success, also returns <*number_of_tools>. Also
returns in <current_interactive_tool_string> the pointer to the tool_names
string for <current_interactive_tool>, or the first one if it is not found.
Up to calling function to deallocate the returned array AND the strings in it.
==============================================================================*/
{
	char **tool_names;
	int i,j,num_children;
	struct Interactive_tool *interactive_tool;
	struct Interactive_toolbar_widget_struct *interactive_toolbar;
	Widget *child_list;

	ENTER(interactive_toolbar_get_tool_names);
	tool_names=(char **)NULL;
	if (interactive_toolbar_widget&&number_of_tools&&
		current_interactive_tool_string)
	{
		*interactive_tool_string=(char *)NULL;
		interactive_toolbar=(struct Interactive_toolbar_widget_struct *)NULL;
		/* get the interactive_toolbar structure from the widget */
		XtVaGetValues(interactive_toolbar_widget,
			XmNuserData,&interactive_toolbar,NULL);
		if (interactive_toolbar)
		{
			num_children=0;
			XtVaGetValues(interactive_toolbar_widget,
				XmNnumChildren,&num_children,
				XmNchildren,&child_list,NULL);
			if ((0==num_children)||ALLOCATE(tool_names,char *,num_children))
			{
				for (i=0;(i<num_children)&&tool_names;i++)
				{
					interactive_tool=(struct Interactive_tool *)NULL;
					XtVaGetValues(child_list[i],XmNuserData,&interactive_tool,NULL);
					if (interactive_tool&&
						GET_NAME(Interactive_tool)(interactive_tool,&(tool_names[i])))
					{
						if (interactive_tool == current_interactive_tool)
						{
							*interactive_tool_string = tool_names[i];
						}
					}
					else
					{
						display_message(WARNING_MESSAGE,
							"interactive_toolbar_get_tool_names.  Could not get tool name");
						for (j=0;j<i;j++)
						{
							DEALLOCATE(tool_names[i]);
						}
						DEALLOCATE(tool_names);
					}
				}
				if (tool_names&&((char *)NULL == *interactive_tool_string))
				{
					*interactive_tool_string = tool_names[0];
				}
			}
			else
			{
				display_message(WARNING_MESSAGE,
					"interactive_toolbar_get_tool_names.  Not enough memory");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"interactive_toolbar_get_tool_names.  Missing widget data");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"interactive_toolbar_get_tool_names.  Invalid argument(s)");
	}
	LEAVE;

	return (tool_names);
} /* interactive_toolbar_get_tool_names */
#endif /* defined (OLD_CODE) */
