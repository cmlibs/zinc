/*******************************************************************************
FILE : relative.c

LAST MODIFIED : 21 June 1995

DESCRIPTION :
This module creates a free relative input device, using two dof3, two control
and one input widget.  The position is given relative to some coordinate system,
and the returned value is a global one.
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
#include <math.h>
#include <stdio.h>
#include "dof3/dof3.h"
#include "general/debug.h"
#include "user_interface/message.h"
#include "view/coord.h"
#include "view/coord_trans.h"
#include "view/relative.h"


/*
Local Functions
---------------
*/
void relative_update(struct Relative_struct *temp_relative)
/*******************************************************************************
LAST MODIFIED : 8 September 1994

DESCRIPTION :
Tells CMGUI about the current values.
==============================================================================*/
{
	ENTER(relative_update);
	if(temp_relative->callback_array[RELATIVE_UPDATE_CB].procedure)
	{
		/* now call the procedure with the user data and the position data */
		(temp_relative->callback_array[RELATIVE_UPDATE_CB].procedure)
			(temp_relative->widget,
			temp_relative->callback_array[RELATIVE_UPDATE_CB].data,
				&temp_relative->current_value);
	}
	LEAVE;
} /* relative_update */

void relative_update_coord(Widget coord_widget,void *user_data,
	void *temp_coordinate)
/*******************************************************************************
LAST MODIFIED : 8 September 1994

DESCRIPTION :
Tells CMGUI about the current values.
==============================================================================*/
{
	struct Relative_struct *temp_relative = user_data;
	struct Cmgui_coordinate *coordinate = temp_coordinate;
	struct Dof3_data new_dof3;

	ENTER(relative_update_coord);
	temp_relative->current_coordinate = coordinate;
	get_local_position(&(temp_relative->current_value.position),
		coordinate,&new_dof3);
	dof3_set_data(temp_relative->position_widget,DOF3_DATA,&new_dof3);
	get_local_direction(&(temp_relative->current_value.direction),
		coordinate,&new_dof3);
	dof3_set_data(temp_relative->direction_widget,DOF3_DATA,&new_dof3);
	LEAVE;
} /* relative_update_coord */

void relative_update_position(Widget position_widget,void *user_data,
	void *temp_dof3)
/*******************************************************************************
LAST MODIFIED : 8 September 1994

DESCRIPTION :
Tells CMGUI about the current values.
==============================================================================*/
{
	struct Relative_struct *temp_relative = user_data;
	struct Dof3_data new_position,*temp_position = temp_dof3;

	ENTER(relative_update_position);
	get_global_position(temp_position,temp_relative->current_coordinate,
		&(temp_relative->current_value.position));
	relative_update(temp_relative);
	LEAVE;
} /* relative_update_position */

void relative_update_direction(Widget direction_widget,void *user_data,
	void *temp_dof3)
/*******************************************************************************
LAST MODIFIED : 8 September 1994

DESCRIPTION :
Tells CMGUI about the current values.
==============================================================================*/
{
	struct Relative_struct *temp_relative = user_data;
	struct Dof3_data new_direction,*temp_direction = temp_dof3;

	ENTER(relative_update_direction);
	get_global_direction(temp_direction,temp_relative->current_coordinate,
		&(temp_relative->current_value.direction));
	relative_update(temp_relative);
	LEAVE;
} /* relative_update_direction */

static void relative_identify_button(Widget w, int *tag, unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 18 April 1994

DESCRIPTION :
Finds the id of the buttons on the relative widget.
==============================================================================*/
{
	int button_num = *tag,combo_num;
	struct Relative_struct *temp_relative;

	ENTER(relative_identify_button);
	/* find out which relative widget we are in */
	XtVaGetValues(w,XmNuserData,&temp_relative,NULL);
	switch (button_num)
	{
		case relative_menu_ID:
		{
			temp_relative->menu = w;
		}; break;
		case relative_pos_form_ID:
		{
			temp_relative->pos_form = w;
		}; break;
		case relative_dir_form_ID:
		{
			temp_relative->dir_form = w;
		}; break;
		default:
		{
		display_message(WARNING_MESSAGE,
			"relative_identify_button.  Invalid button number");
		}; break;
	}
	LEAVE;
} /* relative_identify_button */

static void relative_destroy_CB(Widget w, int *tag, unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 7 May 1994

DESCRIPTION :
Callback for the relativement dialog - tidies up all details - mem etc
==============================================================================*/
{
	int i;
	struct Relative_struct *temp_relative;

	ENTER(relative_destroy_CB);
	/* Get the pointer to the data for the relative widget */
	XtVaGetValues(w,XmNuserData,&temp_relative,NULL);
	/* deallocate the memory for the user data */
	printf("De-allocating memory.\n");
	DEALLOCATE(temp_relative);
	LEAVE;
} /* relative_destroy_CB */


/*
Global Functions
----------------
*/
Widget create_relative_widget(Widget parent,struct Relative_data *init_data)
/*******************************************************************************
LAST MODIFIED : 8 April 1995

DESCRIPTION :
Creates a relative widget that gets a position and orientation from the user.
==============================================================================*/
{
	int i,init_widgets,n;
	MrmType relative_dialog_class;
	struct Callback_data callback;
	struct Relative_struct *temp_relative = NULL;
	static MrmRegisterArg callback_list[]=
	{
		{"relative_identify_button",(XtPointer)relative_identify_button},
		{"relative_destroy_CB",(XtPointer)relative_destroy_CB}
	};
#define NUMBER_IDENTIFIERS 1
	static MrmRegisterArg identifier_list[NUMBER_IDENTIFIERS];
	Widget return_widget;

	ENTER(create_relative_widget);
	return_widget = (Widget)NULL;
	/* allocate memory */
	if (ALLOCATE(temp_relative,struct Relative_struct,1))
	{
		/* initialise the structure */
		temp_relative->widget_parent = parent;
		temp_relative->widget = (Widget)NULL;
		temp_relative->menu = (Widget)NULL;
		temp_relative->pos_form = (Widget)NULL;
		temp_relative->dir_form = (Widget)NULL;
		temp_relative->position_widget = (Widget)NULL;
		temp_relative->direction_widget = (Widget)NULL;
		temp_relative->posctrl_widget = (Widget)NULL;
		temp_relative->dirctrl_widget = (Widget)NULL;
		temp_relative->input_widget = (Widget)NULL;
		temp_relative->current_coordinate = (struct Cmgui_coordinate *)NULL;
		for(i=0;i<3;i++)
		{
			temp_relative->current_value.position.data[i] = init_data->position.data[i];
			temp_relative->current_value.direction.data[i] = init_data->direction.data[i];
		}
		for(i=0;i<RELATIVE_NUM_CALLBACKS;i++)
		{
			temp_relative->callback_array[i].procedure = (Callback_procedure *)NULL;
			temp_relative->callback_array[i].data = NULL;
		}
		/* register the callbacks */
		if (MrmRegisterNamesInHierarchy(hierarchy,callback_list,
			XtNumber(callback_list))==MrmSUCCESS)
		{
			/* assign and register the identifiers */
			n = 0;
			identifier_list[n].name="Relative_structure";
			identifier_list[n].value=(XtPointer)temp_relative;
			n++;
			if (MrmRegisterNamesInHierarchy(hierarchy,identifier_list,n)==
				MrmSUCCESS)
			{
				/* fetch position window widget */
				if (MrmFetchWidget(hierarchy,"relative_widget",temp_relative->widget_parent,
					&(temp_relative->widget),&relative_dialog_class)==MrmSUCCESS)
				{
					XtManageChild(temp_relative->widget);
					/* set the mode toggle to the correct position */
					init_widgets = 1;
					if (!create_dof3_widget(&(temp_relative->position_widget),
						temp_relative->pos_form,DOF3_POSITION,
						CONV_RECTANGULAR_CARTESIAN,&temp_relative->current_value.position))
					{
						display_message(ERROR_MESSAGE,
							"create_relative_widget.  Could not create position dof3 widget.");
						init_widgets = 0;
					}
					if (!create_dof3_widget(&(temp_relative->direction_widget),
						temp_relative->dir_form,DOF3_DIRECTION,
						CONV_RECTANGULAR_CARTESIAN,&temp_relative->current_value.direction))
					{
						display_message(ERROR_MESSAGE,
							"create_relative_widget.  Could not create direction dof3 widget.");
						init_widgets = 0;
					}
					if(!(temp_relative->input_widget = create_input_widget(
						temp_relative->menu,temp_relative->widget)))
					{
						display_message(ERROR_MESSAGE,
							"create_relative_widget.  Could not create input widget.");
						init_widgets = 0;
					}
					if(!(temp_relative->posctrl_widget = create_control_widget(
						temp_relative->menu,"Position")))
					{
						display_message(ERROR_MESSAGE,
							"create_relative_widget.  Could not create position control widget.");
						init_widgets = 0;
					}
					if(!(temp_relative->dirctrl_widget = create_control_widget(
						temp_relative->menu,"Direction")))
					{
						display_message(ERROR_MESSAGE,
							"create_relative_widget.  Could not create direction control widget.");
						init_widgets = 0;
					}
					if(!(temp_relative->coord_widget = create_coord_widget(
						temp_relative->menu)))
					{
						display_message(ERROR_MESSAGE,
							"create_relative_widget.  Could not create coord widget.");
						init_widgets = 0;
					}
					if(init_widgets)
					{
						/* now link all the widgets together */
						input_set_data(temp_relative->input_widget,INPUT_POSITION_WIDGET,
							temp_relative->position_widget);
						input_set_data(temp_relative->input_widget,INPUT_DIRECTION_WIDGET,
							temp_relative->direction_widget);
						input_set_data(temp_relative->input_widget,INPUT_POSCTRL_WIDGET,
							temp_relative->posctrl_widget);
						input_set_data(temp_relative->input_widget,INPUT_DIRCTRL_WIDGET,
							temp_relative->dirctrl_widget);
						control_set_data(temp_relative->posctrl_widget,CONTROL_DOF3_WIDGET,
							temp_relative->position_widget);
						control_set_data(temp_relative->dirctrl_widget,CONTROL_DOF3_WIDGET,
							temp_relative->direction_widget);
						/* tell the direction that it has a position sibling */
						dof3_set_data(temp_relative->direction_widget,DOF3_POSITION_WIDGET,
							temp_relative->position_widget);
						/* get the global coordinate system - a bit of a hack at the moment */
						temp_relative->current_coordinate = coord_get_data(
							temp_relative->coord_widget,COORD_COORD_DATA);
						callback.procedure = relative_update_coord;
						callback.data = temp_relative;
						coord_set_data(temp_relative->coord_widget,COORD_UPDATE_CB,
							&callback);
						callback.procedure = relative_update_position;
						callback.data = temp_relative;
						dof3_set_data(temp_relative->position_widget,DOF3_UPDATE_CB,
							&callback);
						callback.procedure = relative_update_direction;
						callback.data = temp_relative;
						dof3_set_data(temp_relative->direction_widget,DOF3_UPDATE_CB,
							&callback);
						return_widget = temp_relative->widget;
					}
					else
					{
						DEALLOCATE(temp_relative);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_relative_widget.  Could not fetch relative dialog");
					DEALLOCATE(temp_relative);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_relative_widget.  Could not register identifiers");
				DEALLOCATE(temp_relative);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_relative_widget.  Could not register callbacks");
			DEALLOCATE(temp_relative);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_relative_widget.  Could not allocate relative widget structure");
	}
	LEAVE;

	return (return_widget);
} /* create_relative_widget */

int relative_set_data(Widget relative_widget,
	enum Relative_data_type data_type,void *data)
/*******************************************************************************
LAST MODIFIED : 4 January 1995

DESCRIPTION :
Changes a data item of the relative widget.
==============================================================================*/
{
	char temp_str[RELATIVE_STRING_SIZE];
	int coord_sys,i,return_code;
	struct Relative_struct *temp_relative;
	struct Relative_data temp_values;

	ENTER(relative_register_callback);
	/* Get the pointer to the data for the relative dialog */
	XtVaGetValues(relative_widget,XmNuserData,&temp_relative,NULL);
	switch(data_type)
	{
		case RELATIVE_UPDATE_CB:
		{
			temp_relative->callback_array[RELATIVE_UPDATE_CB].procedure =
				((struct Callback_data *)data)->procedure;
			temp_relative->callback_array[RELATIVE_UPDATE_CB].data =
				((struct Callback_data *)data)->data;
			return_code = CM_SUCCESS;
		}; break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"relative_set_data.  Invalid data type.");
			return_code = CM_FAILURE;
		}; break;
	}
	LEAVE;

	return (return_code);
} /* relative_set_data */

void *relative_get_data(Widget relative_widget,
	enum Relative_data_type data_type)
/*******************************************************************************
LAST MODIFIED : 4 January 1995

DESCRIPTION :
Changes a data item of the relative widget.
==============================================================================*/
{
	int i;
	void *return_code;
	struct Relative_struct *temp_relative;
	static struct Callback_data dat_callback;
	static Widget dat_widget;
	static struct Relative_data dat_data;

	ENTER(relative_register_callback);
	/* Get the pointer to the data for the relative dialog */
	XtVaGetValues(relative_widget,XmNuserData,&temp_relative,NULL);
	switch(data_type)
	{
		case RELATIVE_UPDATE_CB:
		{
			dat_callback.procedure =
				temp_relative->callback_array[RELATIVE_UPDATE_CB].procedure;
			dat_callback.data = temp_relative->callback_array[RELATIVE_UPDATE_CB].data;
			return_code = &dat_callback;
		}; break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"relative_get_data.  Invalid data type.");
			return_code = NULL;
		}; break;
	}
	LEAVE;

	return (return_code);
} /* relative_get_data */
