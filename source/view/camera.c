/*******************************************************************************
FILE : camera.c

LAST MODIFIED : 26 November 2001

DESCRIPTION :
This module creates a free camera input device,using two dof3,two control and
one input widget.  The position is given relative to some coordinate system,
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
#include "dof3/dof3_control.h"
#if defined (EXT_INPUT)
#include "dof3/dof3_input.h"
#endif
#include "general/debug.h"
#include "view/camera.h"
static char camera_uidh[] =
#include "view/camera.uidh"
	;
#include "view/coord.h"
#include "view/coord_trans.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module variables
----------------
*/
#if defined (MOTIF)
static int camera_hierarchy_open=0;
static MrmHierarchy camera_hierarchy;
#endif /* defined (MOTIF) */

/*
Module functions
----------------
*/
void camera_update(struct Camera_struct *temp_camera)
/*******************************************************************************
LAST MODIFIED : 8 September 1994

DESCRIPTION :
Tells CMGUI about the current values.
==============================================================================*/
{
	ENTER(camera_update);
	if (temp_camera->callback_array[CAMERA_UPDATE_CB].procedure)
	{
		/* now call the procedure with the user data and the position data */
		(temp_camera->callback_array[CAMERA_UPDATE_CB].procedure)
			(temp_camera->widget,temp_camera->callback_array[CAMERA_UPDATE_CB].data,
			&temp_camera->current_value);
	}
	LEAVE;
} /* camera_update */

void camera_update_coord(Widget coord_widget,void *user_data,
	void *temp_coordinate)
/*******************************************************************************
LAST MODIFIED : 8 September 1994

DESCRIPTION :
Tells CMGUI about the current values.
==============================================================================*/
{
	struct Camera_struct *temp_camera=user_data;
	struct Cmgui_coordinate *coordinate=temp_coordinate;
	struct Dof3_data new_dof3;

	ENTER(camera_update_coord);
	USE_PARAMETER(coord_widget);
	temp_camera->current_coordinate=coordinate;
	get_local_position(&(temp_camera->current_value.position),
		coordinate,&new_dof3);
	dof3_set_data(temp_camera->position_widget,DOF3_DATA,&new_dof3);
	get_local_direction(&(temp_camera->current_value.direction),
		coordinate,&new_dof3);
	dof3_set_data(temp_camera->direction_widget,DOF3_DATA,&new_dof3);
	LEAVE;
} /* camera_update_coord */

void camera_update_position(Widget position_widget,void *user_data,
	void *temp_dof3)
/*******************************************************************************
LAST MODIFIED : 4 November 1995

DESCRIPTION :
Tells CMGUI about the current values.
==============================================================================*/
{
	struct Camera_struct *temp_camera=user_data;
	struct Dof3_data *temp_position=temp_dof3;

	ENTER(camera_update_position);
	USE_PARAMETER(position_widget);
	get_global_position(temp_position,temp_camera->current_coordinate,
		&(temp_camera->current_value.position));
	camera_update(temp_camera);
	LEAVE;
} /* camera_update_position */

void camera_update_direction(Widget direction_widget,void *user_data,
	void *temp_dof3)
/*******************************************************************************
LAST MODIFIED : 4 November 1995

DESCRIPTION :
Tells CMGUI about the current values.
==============================================================================*/
{
	struct Camera_struct *temp_camera=user_data;
	struct Dof3_data *temp_direction=temp_dof3;

	ENTER(camera_update_direction);
	USE_PARAMETER(direction_widget);
	get_global_direction(temp_direction,temp_camera->current_coordinate,
		&(temp_camera->current_value.direction));
	camera_update(temp_camera);
	LEAVE;
} /* camera_update_direction */

static void camera_identify_button(Widget w,int button_num,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 4 November 1995

DESCRIPTION :
Finds the id of the buttons on the camera widget.
==============================================================================*/
{
	struct Camera_struct *temp_camera;

	ENTER(camera_identify_button);
	USE_PARAMETER(reason);
	/* find out which camera widget we are in */
	XtVaGetValues(w,XmNuserData,&temp_camera,NULL);
	switch (button_num)
	{
		case camera_menu_ID:
		{
			temp_camera->menu=w;
		}; break;
		case camera_pos_form_ID:
		{
			temp_camera->pos_form=w;
		}; break;
		case camera_dir_form_ID:
		{
			temp_camera->dir_form=w;
		}; break;
		case camera_coord_ID:
		{
			temp_camera->coord_form=w;
		}; break;
		case camera_pos_menu_ID:
		{
			temp_camera->pos_menu=w;
		}; break;
		case camera_dir_menu_ID:
		{
			temp_camera->dir_menu=w;
		}; break;
		default:
		{
			display_message(WARNING_MESSAGE,
				"camera_identify_button.  Invalid button number");
		}; break;
	}
	LEAVE;
} /* camera_identify_button */

static void camera_destroy_CB(Widget w,int *tag,unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 4 November 1995

DESCRIPTION :
Callback for the camerament dialog - tidies up all details - mem etc
==============================================================================*/
{
	struct Camera_struct *temp_camera;

	ENTER(camera_destroy_CB);
	USE_PARAMETER(tag);
	USE_PARAMETER(reason);
	/* Get the pointer to the data for the camera widget */
	XtVaGetValues(w,XmNuserData,&temp_camera,NULL);
	*(temp_camera->widget_address)=(Widget)NULL;
	/* deallocate the memory for the user data */
	DEALLOCATE(temp_camera);
	LEAVE;
} /* camera_destroy_CB */


/*
Global functions
----------------
*/
Widget create_camera_widget(Widget *camera_widget,Widget parent,
	struct Camera_data *init_data,enum Camera_mode mode)
/*******************************************************************************
LAST MODIFIED : 13 December 1996

DESCRIPTION :
Creates a camera widget that gets a position and orientation from the user.
==============================================================================*/
{
	int i,init_widgets;
	MrmType camera_dialog_class;
	struct Callback_data callback;
	struct Camera_struct *temp_camera=NULL;
	static MrmRegisterArg callback_list[]=
	{
		{"camera_identify_button",(XtPointer)camera_identify_button},
		{"camera_destroy_CB",(XtPointer)camera_destroy_CB}
	};
	static MrmRegisterArg identifier_list[]=
	{
		{"Camera_structure",(XtPointer)NULL},
		{"camera_menu_ID",(XtPointer)camera_menu_ID},
		{"camera_pos_form_ID",(XtPointer)camera_pos_form_ID},
		{"camera_dir_form_ID",(XtPointer)camera_dir_form_ID},
		{"camera_coord_ID",(XtPointer)camera_coord_ID},
		{"camera_pos_menu_ID",(XtPointer)camera_pos_menu_ID},
		{"camera_dir_menu_ID",(XtPointer)camera_dir_menu_ID}
	};
	Widget return_widget;

	ENTER(create_camera_widget);
	return_widget=(Widget)NULL;
	if (MrmOpenHierarchy_binary_string(camera_uidh,sizeof(camera_uidh),
		&camera_hierarchy,&camera_hierarchy_open))
	{
		/* allocate memory */
		if (ALLOCATE(temp_camera,struct Camera_struct,1))
		{
			/* initialise the structure */
			temp_camera->widget_parent=parent;
			temp_camera->widget=(Widget)NULL;
			temp_camera->menu=(Widget)NULL;
			temp_camera->widget_address=camera_widget;
			temp_camera->pos_form=(Widget)NULL;
			temp_camera->dir_form=(Widget)NULL;
			temp_camera->pos_menu=(Widget)NULL;
			temp_camera->dir_menu=(Widget)NULL;
			temp_camera->coord_form=(Widget)NULL;
			temp_camera->position_widget=(Widget)NULL;
			temp_camera->direction_widget=(Widget)NULL;
			temp_camera->posctrl_widget=(Widget)NULL;
			temp_camera->dirctrl_widget=(Widget)NULL;
			temp_camera->input_widget=(Widget)NULL;
			temp_camera->current_coordinate=(struct Cmgui_coordinate *)NULL;
			for (i=0;i<3;i++)
			{
				temp_camera->current_value.position.data[i]=init_data->position.data[i];
				temp_camera->current_value.direction.data[i]=
					init_data->direction.data[i];
			}
			for (i=0;i<CAMERA_NUM_CALLBACKS;i++)
			{
				temp_camera->callback_array[i].procedure=(Callback_procedure *)NULL;
				temp_camera->callback_array[i].data=NULL;
			}
			/* register the callbacks */
			if (MrmRegisterNamesInHierarchy(camera_hierarchy,callback_list,
				XtNumber(callback_list))==MrmSUCCESS)
			{
				/* assign and register the identifiers */
				identifier_list[0].value=(XtPointer)temp_camera;
				if (MrmRegisterNamesInHierarchy(camera_hierarchy,identifier_list,
					XtNumber(identifier_list))==MrmSUCCESS)
				{
					/* fetch position window widget */
					if (MrmSUCCESS==MrmFetchWidget(camera_hierarchy,"camera_widget",
						temp_camera->widget_parent,&(temp_camera->widget),
						&camera_dialog_class))
					{
						XtManageChild(temp_camera->widget);
						/* set the mode toggle to the correct position */
						init_widgets=1;
						/* do we want the dof3 widgets to be relative or absolute */
						if (mode==CAMERA_ABSOLUTE)
						{
							if (!create_dof3_widget(&temp_camera->position_widget,
								temp_camera->pos_form,DOF3_POSITION,DOF3_ABSOLUTE,
								CONV_RECTANGULAR_CARTESIAN,
								&temp_camera->current_value.position))
							{
								display_message(ERROR_MESSAGE,
							"create_camera_widget.  Could not create position dof3 widget.");
								init_widgets=0;
							}
							if (!create_dof3_widget(&temp_camera->direction_widget,
								temp_camera->dir_form,DOF3_DIRECTION,DOF3_ABSOLUTE,
								CONV_DIR_EULER,&temp_camera->current_value.direction))
							{
								display_message(ERROR_MESSAGE,
							"create_camera_widget.  Could not create direction dof3 widget.");
								init_widgets=0;
							}
						}
						else
						{
							if (!create_dof3_widget(&temp_camera->position_widget,
								temp_camera->pos_form,DOF3_POSITION,DOF3_RELATIVE,
								CONV_RECTANGULAR_CARTESIAN,
								&temp_camera->current_value.position))
							{
								display_message(ERROR_MESSAGE,
							"create_camera_widget.  Could not create position dof3 widget.");
								init_widgets=0;
							}
							if (!create_dof3_widget(&temp_camera->direction_widget,
								temp_camera->dir_form,DOF3_DIRECTION,DOF3_RELATIVE,
								CONV_DIR_EULER,&temp_camera->current_value.direction))
							{
								display_message(ERROR_MESSAGE,
							"create_camera_widget.  Could not create direction dof3 widget.");
								init_widgets=0;
							}
						}
#if defined (EXT_INPUT)
						if (!(temp_camera->input_widget=create_input_widget(
							temp_camera->menu,temp_camera->widget)))
						{
							display_message(ERROR_MESSAGE,
								"create_camera_widget.  Could not create input widget.");
							init_widgets=0;
						}
#endif
						if (!(temp_camera->posctrl_widget=create_control_widget(
							temp_camera->pos_menu,"Control")))
						{
							display_message(ERROR_MESSAGE,
						"create_camera_widget.  Could not create position control widget.");
							init_widgets=0;
						}
						if (!(temp_camera->dirctrl_widget=create_control_widget(
							temp_camera->dir_menu,"Control")))
						{
							display_message(ERROR_MESSAGE,
					"create_camera_widget.  Could not create direction control widget.");
							init_widgets=0;
						}
						if (!(temp_camera->coord_widget=create_coord_widget(
							temp_camera->coord_form)))
						{
							display_message(ERROR_MESSAGE,
								"create_camera_widget.  Could not create coord widget.");
							init_widgets=0;
						}
						if (init_widgets)
						{
#if defined (EXT_INPUT)
							/* now link all the widgets together */
							input_set_data(temp_camera->input_widget,INPUT_POSITION_WIDGET,
								temp_camera->position_widget);
							input_set_data(temp_camera->input_widget,INPUT_DIRECTION_WIDGET,
								temp_camera->direction_widget);
							input_set_data(temp_camera->input_widget,INPUT_POSCTRL_WIDGET,
								temp_camera->posctrl_widget);
							input_set_data(temp_camera->input_widget,INPUT_DIRCTRL_WIDGET,
								temp_camera->dirctrl_widget);
#endif
							control_set_data(temp_camera->posctrl_widget,CONTROL_DOF3_WIDGET,
								temp_camera->position_widget);
							control_set_data(temp_camera->dirctrl_widget,CONTROL_DOF3_WIDGET,
								temp_camera->direction_widget);
							/* tell the direction that it has a position sibling */
							dof3_set_data(temp_camera->direction_widget,DOF3_POSITION_WIDGET,
								temp_camera->position_widget);
							/* get the global coordinate system */
								/*???GMH.  A bit of a hack at the moment */
							temp_camera->current_coordinate=global_coordinate_ptr;
							coord_set_data(temp_camera->coord_widget,COORD_COORD_DATA,
								global_coordinate_ptr);
							callback.procedure=camera_update_coord;
							callback.data=temp_camera;
							coord_set_data(temp_camera->coord_widget,COORD_UPDATE_CB,
								&callback);
							callback.procedure=camera_update_position;
							callback.data=temp_camera;
							dof3_set_data(temp_camera->position_widget,DOF3_UPDATE_CB,
								&callback);
							callback.procedure=camera_update_direction;
							callback.data=temp_camera;
							dof3_set_data(temp_camera->direction_widget,DOF3_UPDATE_CB,
								&callback);
							return_widget=temp_camera->widget;
						}
						else
						{
							DEALLOCATE(temp_camera);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_camera_widget.  Could not fetch camera dialog");
						DEALLOCATE(temp_camera);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_camera_widget.  Could not register identifiers");
					DEALLOCATE(temp_camera);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_camera_widget.  Could not register callbacks");
				DEALLOCATE(temp_camera);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_camera_widget.  Could not allocate camera widget structure");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_camera_widget.  Could not open hierarchy");
	}
	*camera_widget=return_widget;
	LEAVE;

	return (return_widget);
} /* create_camera_widget */

int camera_set_data(Widget camera_widget,
	enum Camera_data_type data_type,void *data)
/*******************************************************************************
LAST MODIFIED : 4 November 1995

DESCRIPTION :
Changes a data item of the camera widget.
==============================================================================*/
{
	int i,return_code;
	struct Camera_struct *temp_camera;

	ENTER(camera_set_data);
	/* Get the pointer to the data for the camera dialog */
	XtVaGetValues(camera_widget,XmNuserData,&temp_camera,NULL);
	switch (data_type)
	{
		case CAMERA_UPDATE_CB:
		{
			temp_camera->callback_array[CAMERA_UPDATE_CB].procedure=
				((struct Callback_data *)data)->procedure;
			temp_camera->callback_array[CAMERA_UPDATE_CB].data=
				((struct Callback_data *)data)->data;
			return_code=1;
		}; break;
		case CAMERA_DATA:
		{
			for (i=0;i<3;i++)
			{
				temp_camera->current_value.position.data[i]=
					((struct Camera_data *)data)->position.data[i];
				temp_camera->current_value.direction.data[i]=
					((struct Camera_data *)data)->direction.data[i];
			}
			dof3_set_data(temp_camera->position_widget,DOF3_DATA,
				&temp_camera->current_value.position);
			dof3_set_data(temp_camera->direction_widget,DOF3_DATA,
				&temp_camera->current_value.direction);
			/* we must change to the global coord system */
			temp_camera->current_coordinate=global_coordinate_ptr;
			coord_set_data(temp_camera->coord_widget,COORD_COORD_DATA,
				temp_camera->current_coordinate);
			return_code=1;
		}; break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"camera_set_data.  Invalid data type.");
			return_code=0;
		}; break;
	}
	LEAVE;

	return (return_code);
} /* camera_set_data */

void *camera_get_data(Widget camera_widget,enum Camera_data_type data_type)
/*******************************************************************************
LAST MODIFIED : 4 November 1995

DESCRIPTION :
Changes a data item of the camera widget.
==============================================================================*/
{
	void *return_code;
	struct Camera_struct *temp_camera;
	static struct Callback_data dat_callback;

	ENTER(camera_get_data);
	/* Get the pointer to the data for the camera dialog */
	XtVaGetValues(camera_widget,XmNuserData,&temp_camera,NULL);
	switch (data_type)
	{
		case CAMERA_UPDATE_CB:
		{
			dat_callback.procedure=
				temp_camera->callback_array[CAMERA_UPDATE_CB].procedure;
			dat_callback.data=temp_camera->callback_array[CAMERA_UPDATE_CB].data;
			return_code= &dat_callback;
		}; break;
		case CAMERA_DATA:
		{
			return_code= &temp_camera->current_value;
		}; break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"camera_get_data.  Invalid data type.");
			return_code=NULL;
		}; break;
	}
	LEAVE;

	return (return_code);
} /* camera_get_data */
