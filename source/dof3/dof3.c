/*******************************************************************************
FILE : dof3.c

LAST MODIFIED : 20 May 2002

DESCRIPTION :
This module allows for the creation of three types of widgets -
	dof3 (three degree of freedom)
	input (input router to dof3 and control widgets)
	control (controlling widget for each of the dof3 widgets)

The rough 'control structure' is as follows
											---------
											| INPUT |
											---------
													|
	------------------------------------------------
	|              |button messages|                | movement messages
	|         -----------     -----------          |
	|         | CONTROL1|     | CONTROL2|          |
	|         -----------     -----------          |
	|              |               |                |
	------------   |               |   -------------
							|   |               |   |
						----------         ----------
						|  DOF31 |         |  DOF32 |
						----------         ----------
This is for the full case (6DOF), where two dof3 widgets each have a controller,
and input to the whole set is controlled by the input widget.  Note that in
this case, the <bounding> widget passed to create_input_widget would encompass
all of the widgets.  This is to allow the input_module to pass the correct
events to this 'structure'.

Links are created between the widgets by using the set_data routines.  The
dominant widget (ie the one highest above) should be told who its subwidgets
are, ie input_set data would be called with
	INPUT_POSITION_WIDGET      DOF31
	INPUT_DIRECTION_WIDGET      DOF32
	INPUT_POSCTRL_WIDGET        CONTROL1
	INPUT_DIRCTRL_WIDGET        CONTROL2
input_set_data will ensure that the control and dof3 widgets are informed as
to who is providing their input.

When any of the widgets are destroyed, they ensure that all links with other
widgets are terminated, and so the whole structure does not need to be destroyed
- widgets may be mixed and matched as the situation changes.


The update callback for the dof3 widget returns -
	Widget dof3_widget
	void *user data    (sent in Callback_data record)
	struct Dof3_data *dof3_data (the current values of the dof3 widget)
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
#include <Xm/Text.h>
#include <Xm/ToggleB.h>
#include <Xm/ToggleBG.h>
#include "dof3/dof3.h"
#include "dof3/dof3.uidh"
#include "dof3/dof3_control.h"
#include "dof3/dof3_input.h"
#include "general/debug.h"
#include "io_devices/conversion.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"
#include "view/coord_trans.h"

/*
Module variables
----------------
*/
static int dof3_hierarchy_open=0;
static MrmHierarchy dof3_hierarchy;

struct Dof3_coord_limits
/*******************************************************************************
LAST MODIFIED : 14 May 1998

DESCRIPTION :
The following structures are used to give maximum and minimum limits for
	the coordinate systems.
==============================================================================*/
{
	DOF3_PRECISION minimum[3];    /* min val of coord */
	DOF3_PRECISION maximum[3];
	int initialexp[3];    /* init val of exponent (resolution) */
	int minexp[3];      /* smallest resolution */
	int maxexp[3];      /* biggest resolution */
}; /* Dof3_coord_limits */

struct Dof3_coord_limits dof3_coord_limits[DOF3_NUM_COORD][DOF3_NUM_MAX_COORD]=
{
	{
		{
			/* rectangular cartesian */
			{-1e10,-1e10,-1e10},
			{1e10,1e10,1e10},
			{0,0,0},
			{-10,-10,-10},
			{10,10,10}
		},
		{
			/* cylindrical polar */
			{0.0,-720.0,-100000.0},
			{10000.0,720.0,100000.0},
			{0,0,0},
			{-7,-7,-7},
			{6,2,6}
		},
		{
			/* spherical polar */
			{0.0,-720.0,-720.0},
			{10000.0,720.0,720.0},
			{0,0,0},
			{-7,-7,-7},
			{6,2,2}
		}
	},
	{
		{
			/* euler */
			{-180.0,-180.0,-180.0},
			{180.0,180.0,180.0},
			{0,0,0},
			{-7,-7,-7},
			{2,2,2}
		}
	},
	{
		{
			/* cartesian component */
			{-100000.0,-100000.0,-100000.0},
			{100000.0,100000.0,100000.0},
			{0,0,0},
			{-7,-7,-7},
			{6,6,6}
		},
		{
			/* spherical polar */
			{0.0,-720.0,-720.0},
			{10000.0,720.0,720.0},
			{0,0,0},
			{-7,-7,-7},
			{6,2,2}
		}
	}
};
/* ensure the maximum size of the labels is less than 9, else change size */
char dof3_axis_names[DOF3_NUM_COORD][DOF3_NUM_MAX_COORD][3][10]=
{
	{
		{"X Axis","Y Axis","Z Axis"},
		{"Radius","Theta","Z Axis"},
		{"Radius","Theta","Phi"}
	},
	{
		{"Azimuth","Elevation","Roll"}
	},
	{
		{"X Compo","Y Compo","Z Compo"},
		{"Radius","Theta","Phi"}
	}
};

char dof3_coord_names[DOF3_NUM_COORD][DOF3_NUM_MAX_COORD][22]=
{
	{
		"Rectangular Cartesian",
		"Cylindrical Polar",
		"Spherical Polar"
	},
	{
		"Euler"
	},
	{
		"Projection",
		"Spherical Polar"
	}
};

/*
Module functions
----------------
*/
void dof3_update(struct Dof3_struct *temp_dof3)
/*******************************************************************************
LAST MODIFIED : 2 November 1995

DESCRIPTION :
Tells CMGUI about the current values.
==============================================================================*/
{
	struct Dof3_data data_temp;

	ENTER(dof3_update);
	if (temp_dof3->callback_array[DOF3_UPDATE_CB].procedure)
	{
		/* convert the data to the format that the client requires */
		switch (temp_dof3->type)
		{
			case DOF3_POSITION:
			{
				(*(conversion_position[temp_dof3->current_coord]
					[temp_dof3->return_coord]))(&(temp_dof3->current_value),
						&data_temp);
			} break;
			case DOF3_DIRECTION:
			{
				(*(conversion_direction[temp_dof3->current_coord]
					[temp_dof3->return_coord]))(&(temp_dof3->current_value),
						&data_temp);
			} break;
			case DOF3_VECTOR:
			{
				(*(conversion_vector[temp_dof3->current_coord]
					[temp_dof3->return_coord]))(&(temp_dof3->current_value),
						&data_temp);
			} break;
		}
		/* now call the procedure with the user data and the position data */
		(temp_dof3->callback_array[DOF3_UPDATE_CB].procedure)
			(temp_dof3->widget,
				temp_dof3->callback_array[DOF3_UPDATE_CB].data,&data_temp);
	}
	LEAVE;
} /* dof3_update */

#if defined (EXT_INPUT)
void dof3_update_ext_origin(struct Dof3_struct *temp_dof3,
	struct Dof3_data *rel_euler)
/*******************************************************************************
LAST MODIFIED : 18 April 1994

DESCRIPTION :
We are given the angles that we wish to achieve, and from that must change
the origin matrix so that abs*origin=desired.
==============================================================================*/
{
	ENTER(dof3_update_ext_origin);
	euler_matrix(rel_euler,&(temp_dof3->orientation));
	matrix_mult(&(temp_dof3->ext_origin.last),&(temp_dof3->orientation),
		&(temp_dof3->ext_origin.origin));
	/* if we have a position sibling, we must tell them that the ext_origin has been
	changed */
	if (temp_dof3->position)
	{
		matrix_copy(&temp_dof3->position->ext_origin.origin,
			&temp_dof3->ext_origin.origin);
		if (temp_dof3->mode==DOF3_RELATIVE)
		{
			matrix_copy(&(temp_dof3->position->orientation),
				&(temp_dof3->orientation));
		}
	}
	LEAVE;
} /* dof3_update_ext_origin */
#endif

#if defined (EXT_INPUT)
void dof3_convert_dir(struct Dof3_struct *temp_dof3,Gmatrix *abs_direction,
	struct Dof3_data *rel_euler)
/*******************************************************************************
LAST MODIFIED : 18 April 1994

DESCRIPTION :
Takes the absolute direction matrix and returns the directions relative
to the origin.  The directions are returned as euler angles.
==============================================================================*/
{
	ENTER(dof3_convert_dir);
	matrix_mult(abs_direction,
		&(temp_dof3->ext_origin.origin),&(temp_dof3->orientation));
	/* this is our current orientation, so must be stored in orientation */
	/* if we are not a direction_relative widget, then it just serves as
	working space */
	matrix_euler(&(temp_dof3->orientation),rel_euler);
	/* if we are a direction_rel widget, and we have a position sibling, then
	we need to give it our orientation matrix */
	if ((temp_dof3->mode==DOF3_RELATIVE)&&(temp_dof3->position))
	{
		matrix_copy(&(temp_dof3->position->orientation),&(temp_dof3->orientation));
	}
	LEAVE;
} /* dof3_convert_dir */
#endif

#if defined (EXT_INPUT)
void dof3_convert_pos(struct Dof3_struct *temp_dof3,
	struct Dof3_data *new_change,struct Dof3_data *old_change)
/*******************************************************************************
LAST MODIFIED : 18 April 1994

DESCRIPTION :
Based on the origin, it takes absolute position values, and converts them to
relative values about the new origin orientation.
==============================================================================*/
{
	int i,j;

	ENTER(dof3_convert_pos);
	for (i=0;i<3;i++)
	{
		new_change->data[i]=0.0;
		for (j=0;j<3;j++)
		{
			new_change->data[i] += temp_dof3->ext_origin.origin.data[j][i]*
				old_change->data[j];
		}
	}
	LEAVE;
} /* dof3_convert_pos */
#endif

void dof3_refresh(struct Dof3_struct *temp_dof3,struct Dof3_data *temp_values)
/*******************************************************************************
LAST MODIFIED : 18 April 1994

DESCRIPTION :
Refreshes all three numbers.
==============================================================================*/
{
	char temp_str[DOF3_STRING_SIZE];
	int i,coord_sys,type;

	ENTER(dof3_refresh);
	coord_sys=temp_dof3->current_coord;
	type=temp_dof3->type;
	for (i=0;i<3;i++)
	{
		if (!temp_dof3->editing[i])
		{
			if (temp_values->data[i]>dof3_coord_limits[type][coord_sys].
				maximum[i])
			{
				temp_values->data[i]=dof3_coord_limits[type][coord_sys].
					maximum[i];
			}
			if (temp_values->data[i]<dof3_coord_limits[type][coord_sys].
				minimum[i])
			{
				temp_values->data[i]=dof3_coord_limits[type][coord_sys].
					minimum[i];
			}
			if (temp_values->data[i]!=temp_dof3->current_value.data[i])
			{
				sprintf(temp_str,DOF3_NUM_FORMAT,temp_values->data[i]);
				XmTextSetString(temp_dof3->axis[i].data,temp_str);
				temp_dof3->current_value.data[i]=temp_values->data[i];
			}
		}
	}
	LEAVE;
} /* dof3_refresh */

void dof3_convert_relative_pos(struct Dof3_struct *temp_dof3,
	struct Dof3_data *relative,struct Dof3_data *global)
/*******************************************************************************
LAST MODIFIED : 18 April 1994

DESCRIPTION :
Takes a position change relative to our current orientation, and returns a
global (local) absolute value.
==============================================================================*/
{
	int i;
	struct Dof3_data cart_global;

	ENTER(dof3_convert_relative_pos);
	/* change current value to cartesian */
	(*(conversion_position[temp_dof3->current_coord]
		[CONV_RECTANGULAR_CARTESIAN]))
		(&temp_dof3->current_value,&cart_global);
	/* we want the x movements to be changes along the y axis */
	/* we want the y movements to be changes along the -z axis */
	/* we want the z movements to be changes along the -x axis */
	for (i=0;i<3;i++)
	{
		/* x compo * y vector */
		cart_global.data[i] += relative->data[0]*temp_dof3->orientation.data[1][i];
		/* y compo * -z vector */
		cart_global.data[i] -= relative->data[1]*temp_dof3->orientation.data[2][i];
		/* z compo * -x vector */
		cart_global.data[i] -= relative->data[2]*temp_dof3->orientation.data[0][i];
	}
	/* change the coordinate system back */
	(*(conversion_position[CONV_RECTANGULAR_CARTESIAN]
		[temp_dof3->current_coord]))
		(&cart_global,global);
	LEAVE;
} /* dof3_convert_relative_pos */

void dof3_convert_relative_dir(struct Dof3_struct *temp_dof3,
	struct Dof3_data *relative,struct Dof3_data *global)
/*******************************************************************************
LAST MODIFIED : 2 November 1995

DESCRIPTION :
Takes a  relative orientation change, and returns the global (local) euler
values for this new orientation.
==============================================================================*/
{
	ENTER(dof3_convert_relative_dir);
	if (relative->data[0])
	{
		/* need to rotate about the -ve z axis */
		matrix_rotate(&(temp_dof3->orientation),-relative->data[0],'z');
	}
	if (relative->data[1])
	{
		matrix_rotate(&(temp_dof3->orientation),-relative->data[1],'y');
	}
	if (relative->data[2])
	{
		matrix_rotate(&(temp_dof3->orientation),-relative->data[2],'x');
	}
	matrix_euler(&(temp_dof3->orientation),global);
	if (temp_dof3->position)
	{
		matrix_copy(&(temp_dof3->position->orientation),&(temp_dof3->orientation));
	}
	LEAVE;
} /* dof3_convert_relative_dir */

static void dof3_identify_button(Widget w,int button_num,unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 20 May 2002

DESCRIPTION :
Finds the id of the buttons on the dof3 widget.
==============================================================================*/
{
	long int combo_num;
	struct Dof3_struct *temp_dof3;
	Widget parent;
	XtPointer combo_addr;

	ENTER(dof3_identify_button);
	USE_PARAMETER(reason);
	/* find out which dof3 widget we are in */
	XtVaGetValues(w,XmNuserData,&temp_dof3,NULL);
	if (button_num==dof3_slider_ID)
	{
		temp_dof3->slider=w;
		XtVaSetValues(w, XmNminimum, -100, NULL);
		XtVaSetValues(w, XmNmaximum, 125, NULL);
		XtVaSetValues(w, XmNsliderSize, 25, NULL);
	}
	else
	{
		parent = XtParent(w);
		XtVaGetValues(parent,XmNuserData,&combo_addr,NULL);
		combo_num = (long int)combo_addr;
		switch (button_num)
		{
			case dof3_combo_toggle_ID:
			{
				temp_dof3->axis[combo_num].toggle=w;
			} break;
			case dof3_combo_data_ID:
			{
				temp_dof3->axis[combo_num].data=w;
			} break;
			case dof3_combo_label_ID:
			{
				temp_dof3->axis[combo_num].label=w;
			} break;
			case dof3_combo_dataexp_ID:
			{
				temp_dof3->axis[combo_num].dataexp=w;
			} break;
			case dof3_combo_arrpos_ID:
			{
				temp_dof3->axis[combo_num].arrow_pos=w;
			} break;
			case dof3_combo_arrneg_ID:
			{
				temp_dof3->axis[combo_num].arrow_neg=w;
			} break;
			default:
			{
				display_message(WARNING_MESSAGE,
					"dof3_identify_button.  Invalid button number");
			} break;
		}
	}
	LEAVE;
} /* dof3_identify_button */

static void dof3_destroy_CB(Widget w, int *tag, unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 2 November 1995

DESCRIPTION :
Callback for the dof3ment dialog - tidies up all details - mem etc
==============================================================================*/
{
	struct Dof3_struct *temp_dof3;

	ENTER(dof3_destroy_CB);
	USE_PARAMETER(tag);
	USE_PARAMETER(reason);
	/* Get the pointer to the data for the dof3 widget */
	XtVaGetValues(w,XmNuserData,&temp_dof3,NULL);
	/* terminate any links with this widget */
#if defined (EXT_INPUT)
	if (temp_dof3->input_widget)
	{
		switch (temp_dof3->type)
		{
			case DOF3_POSITION:
			case DOF3_VECTOR:
			{
				input_set_data(temp_dof3->input_widget,INPUT_POSITION_WIDGET,NULL);
			} break;
			case DOF3_DIRECTION:
			{
				input_set_data(temp_dof3->input_widget,INPUT_DIRECTION_WIDGET,NULL);
			} break;
		}
	}
#endif
	if (temp_dof3->control_widget)
	{
		control_set_data(temp_dof3->control_widget,CONTROL_DOF3_WIDGET,NULL);
	}
	*(temp_dof3->widget_address)=(Widget)NULL;
	/* deallocate the memory for the user data */
	DEALLOCATE(temp_dof3);
	LEAVE;
} /* dof3_destroy_CB */

static void dof3_slider_CB(Widget w, int *tag, XmAnyCallbackStruct *reason)
/*******************************************************************************
LAST MODIFIED : 2 November 1995

DESCRIPTION :
Callback for the dof3 widget.  Processes any changes that have occurred to the
slider.
==============================================================================*/
{
	char temp_str[DOF3_STRING_SIZE];
	int i,axis_num;
	struct Dof3_struct *temp_dof3;
	struct Dof3_data temp_values,temp_increment;
	XmScrollBarCallbackStruct *temp_data;

	ENTER(dof3_slider_CB);
	USE_PARAMETER(tag);
	/* reset temp_increment */
	for (i=0;i<3;i++)
	{
		temp_increment.data[i]=0.0;
	}
	/* Get the pointer to the data for the dof3 widget */
	XtVaGetValues(w,XmNuserData,&temp_dof3,NULL);
	if (!temp_dof3->lock_data)
	{ /* can we change it */
		axis_num=temp_dof3->current_axis;
		temp_data=(XmScrollBarCallbackStruct *)reason;
		/* update the data value */
		sprintf(temp_str,"%i.0E%i",(temp_data->value-temp_dof3->slider_ofs),temp_dof3->resolution[axis_num]);
		sscanf(temp_str,"%"DOF3_PRECISION_STRING,&temp_increment.data[axis_num]);
		temp_dof3->slider_ofs=temp_data->value;
		if (temp_dof3->mode==DOF3_RELATIVE)
		{
			if (temp_dof3->type==DOF3_DIRECTION)
			{
				dof3_convert_relative_dir(temp_dof3,&temp_increment,&temp_values);
			}
			else
			{
				dof3_convert_relative_pos(temp_dof3,&temp_increment,&temp_values);
			}
		}
		else
		{
			for (i=0;i<3;i++)
			{
				temp_values.data[i]=temp_increment.data[i]+
					temp_dof3->current_value.data[i];
			}
		}
		/* now reflect the change */
		dof3_refresh(temp_dof3,&temp_values);
		if (reason->reason!=XmCR_DRAG)
		{
			/* reset the slider */
			temp_dof3->slider_ofs=0;
		}
#if defined (EXT_INPUT)
		/* update the origin */
		if (temp_dof3->type==DOF3_DIRECTION)
		{
			dof3_update_ext_origin(temp_dof3,&temp_dof3->current_value);
		}
#endif
		dof3_update(temp_dof3);
	}
	if (reason->reason!=XmCR_DRAG)
	{
		/* reset the slider */
		XtVaSetValues(w,XmNvalue,0,NULL);
	}
	LEAVE;
} /* dof3_slider_CB */

static void dof3_change_axis(struct Dof3_struct *temp_dof3,
	int old_axis,int new_axis)
/*******************************************************************************
LAST MODIFIED : 28 March 1994

DESCRIPTION :
Changes the current toggle button and slider setting, also changes slider limits
==============================================================================*/
{
	ENTER(dof3_change_axis);
	if (old_axis!=new_axis)
	{
		XmToggleButtonSetState(temp_dof3->axis[old_axis].toggle,FALSE,FALSE);
	}
	XmToggleButtonSetState(temp_dof3->axis[new_axis].toggle,TRUE,FALSE);
	XtVaSetValues(temp_dof3->slider,XmNvalue,0,NULL);
	temp_dof3->current_axis=new_axis;
	LEAVE;
} /* dof3_change_axis */

static void dof3_toggle_CB(Widget w, int *tag, unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 20 March 1994

DESCRIPTION :
Callback for the dof3 widget.  Processes any changes that have occurred to the
toggle.
==============================================================================*/
{
	int combo_num,axis_num;
	struct Dof3_struct *temp_dof3;

	ENTER(dof3_slider_CB);
	USE_PARAMETER(tag);
	USE_PARAMETER(reason);
	/* Get the pointer to the data for the dof3 widget */
	XtVaGetValues(w,XmNuserData,&temp_dof3,NULL);
	/* first, get the user data of the parent */
	XtVaGetValues(XtParent(w),XmNuserData,&combo_num,NULL);
	axis_num=temp_dof3->current_axis;
	dof3_change_axis(temp_dof3,axis_num,combo_num);
	LEAVE;
} /* dof3_slider_CB */

static void dof3_data_CB(Widget w, int *tag, XmAnyCallbackStruct *reason)
/*******************************************************************************
LAST MODIFIED : 2 November 1995

DESCRIPTION :
Callback for the dof3 widget.  Processes any changes that have occurred to the
data.
==============================================================================*/
{
	char *temp_str_ptr;
	int axis_num,combo_num,i;
	struct Dof3_struct *temp_dof3;
	struct Dof3_data temp_increment;

	ENTER(dof3_data_CB);
	USE_PARAMETER(tag);
	/* Get the pointer to the data for the dof3 widget */
	XtVaGetValues(w,XmNuserData,&temp_dof3,NULL);
	/* reset temp_increment */
	for (i=0;i<3;i++)
	{
		temp_increment.data[i]=temp_dof3->current_value.data[i];
	}
	/* first, get the user data of the parent */
	XtVaGetValues(XtParent(w),XmNuserData,&combo_num,NULL);
	axis_num=temp_dof3->current_axis;
	/* why are we being called - gain focus or lose it */
	if (reason->reason==XmCR_FOCUS)
	{
		dof3_change_axis(temp_dof3,axis_num,combo_num);
		temp_dof3->editing[combo_num]=1;
	}
	else
	{ /* change it */
		XtVaGetValues(w,XmNvalue,&temp_str_ptr,NULL);
		sscanf(temp_str_ptr,"%"DOF3_PRECISION_STRING,
			&temp_increment.data[axis_num]);
		XtFree(temp_str_ptr);
		/* are we allowed to change it */
		if (temp_dof3->lock_data)
		{
			temp_increment.data[axis_num]=temp_dof3->current_value.data[axis_num];
		}
		/* now reflect the change */
		temp_dof3->editing[axis_num]=0;
		dof3_refresh(temp_dof3,&temp_increment);
		/* update the origin */
		if (temp_dof3->type==DOF3_DIRECTION)
		{
#if defined (EXT_INPUT)
			dof3_update_ext_origin(temp_dof3,&temp_dof3->current_value);
#endif
			if (temp_dof3->mode==DOF3_RELATIVE)
			{
				euler_matrix(&temp_dof3->current_value,&(temp_dof3->orientation));
				if (temp_dof3->position)
				{
					matrix_copy(&(temp_dof3->position->orientation),
						&(temp_dof3->orientation));
				}
			}
		}
		/* now reflect the change */
		dof3_update(temp_dof3);
	}
	LEAVE;
} /* dof3_data_CB */

static void dof3_dataexp_CB(Widget w, int *tag, XmAnyCallbackStruct *reason)
/*******************************************************************************
LAST MODIFIED : 20 March 1994

DESCRIPTION :
Callback for the dof3 widget.  Processes any changes that have occurred to the
exponent data.
==============================================================================*/
{
	char temp_str[DOF3_STRING_SIZE],*temp_str_ptr;
	int axis_num,coord_sys,combo_num,i,temp_value,temp_value2;
	struct Dof3_struct *temp_dof3;

	ENTER(dof3_dataexp_CB);
	USE_PARAMETER(tag);
	/* Get the pointer to the data for the dof3 widget */
	XtVaGetValues(w,XmNuserData,&temp_dof3,NULL);
	/* first, get the user data of the parent */
	XtVaGetValues(XtParent(w),XmNuserData,&combo_num,NULL);
	axis_num=temp_dof3->current_axis;
	/* why are we being called - gain focus or lose it */
	if (reason->reason==XmCR_FOCUS)
	{
		dof3_change_axis(temp_dof3,axis_num,combo_num);
	}
	else
	{
		/* make any changes */
		XtVaGetValues(w,XmNvalue,&temp_str_ptr,NULL);
		sscanf(temp_str_ptr,"%i",&temp_value);
		XtFree(temp_str_ptr);
		coord_sys=temp_dof3->current_coord;
		/* do we have to update one or all three resolutions */
		if (temp_dof3->link_res)
		{
			for (i=0;i<3;i++)
			{
				/* update all three resolution bits */
				temp_value2=temp_value;
				if ((temp_value>=
					dof3_coord_limits[temp_dof3->type][coord_sys].maxexp[i])||
					(temp_value<=dof3_coord_limits[temp_dof3->type][coord_sys].minexp[i]))
				{
					temp_value2=temp_dof3->resolution[i];
				}
				else
				{
					/* standard update of value */
					temp_dof3->resolution[i]=temp_value;
				}
				sprintf(temp_str,"%2i",temp_value2);
				XmTextSetString(temp_dof3->axis[i].dataexp,temp_str);
			}
		}
		else
		{
			if ((temp_value>=
				dof3_coord_limits[temp_dof3->type][coord_sys].maxexp[combo_num])||
				(temp_value<=
				dof3_coord_limits[temp_dof3->type][coord_sys].minexp[combo_num]))
			{
				temp_value=temp_dof3->resolution[axis_num];
			}
			else
			{
				/* standard update of value */
				temp_dof3->resolution[axis_num]=temp_value;
			}
			sprintf(temp_str,"%2i",temp_value);
			XmTextSetString(w,temp_str);
		}
	}
	LEAVE;
} /* dof3_dataexp_CB */

static void dof3_arrow_CB(Widget w, int button_num, unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 2 November 1995

DESCRIPTION :
Callback for the dof3 widget.  Processes any changes that have occurred to the
exponent arrows.
==============================================================================*/
{
	char temp_str[DOF3_STRING_SIZE];
	int axis_num,change,coord_sys,combo_num,i;
	struct Dof3_struct *temp_dof3;

	ENTER(dof3_arrow_CB);
	USE_PARAMETER(reason);
	/* Get the pointer to the data for the dof3 widget */
	XtVaGetValues(w,XmNuserData,&temp_dof3,NULL);
	/* first, get the user data of the parent */
	XtVaGetValues(XtParent(w),XmNuserData,&combo_num,NULL);
	axis_num=temp_dof3->current_axis;
	/* first highlight the correct combo */
	dof3_change_axis(temp_dof3,axis_num,combo_num);
	coord_sys=temp_dof3->current_coord;
	/* are we incrementing or decrementing? */
	change=0;
	switch (button_num)
	{
		case dof3_combo_arrpos_ID:
		{
			change=1;
		} break;
		case dof3_combo_arrneg_ID:
		{
			change= -1;
		} break;
	}
	/* do we have to update one or all three resolutions */
	if (temp_dof3->link_res)
	{
		for (i=0;i<3;i++)
		{
			/* can we increment the counter */
			if ((temp_dof3->resolution[i]+change<=
				dof3_coord_limits[temp_dof3->type][coord_sys].maxexp[i])&&
				(temp_dof3->resolution[i]+change>=
					dof3_coord_limits[temp_dof3->type][coord_sys].minexp[i]))
			{
				temp_dof3->resolution[i] += change;
			}
			sprintf(temp_str,"%2i",temp_dof3->resolution[i]);
			XmTextSetString(temp_dof3->axis[i].dataexp,temp_str);
		}
	}
	else
	{
		/* can we increment the counter */
		if ((temp_dof3->resolution[combo_num]+change<=
			dof3_coord_limits[temp_dof3->type][coord_sys].maxexp[combo_num])&&
			(temp_dof3->resolution[combo_num]+change>=
				dof3_coord_limits[temp_dof3->type][coord_sys].minexp[combo_num]))
		{
			temp_dof3->resolution[combo_num] += change;
		}
		sprintf(temp_str,"%2i",temp_dof3->resolution[combo_num]);
		XmTextSetString(temp_dof3->axis[combo_num].dataexp,temp_str);
	}
	LEAVE;
} /* dof3_arrow_CB */


/*
Global Functions
----------------
*/
Widget create_dof3_widget(Widget *dof3_widget,Widget parent,
	enum Dof3_widget_type type,enum Dof3_widget_mode mode,int coord_sys,
	struct Dof3_data *init_data)
/*******************************************************************************
LAST MODIFIED : 13 December 1996

DESCRIPTION :
Creates a widget that will allow for the acquisition of three degrees of
freedom.  This may either be in position form or direction form.
==============================================================================*/
{
	int i,n;
	MrmType dof3_dialog_class;
	struct Dof3_struct *temp_dof3=NULL;
	static MrmRegisterArg callback_list[]=
	{
		{"dof3_identify_button",(XtPointer)dof3_identify_button},
		{"dof3_destroy_CB",(XtPointer)dof3_destroy_CB},
		{"dof3_slider_CB",(XtPointer)dof3_slider_CB},
		{"dof3_toggle_CB",(XtPointer)dof3_toggle_CB},
		{"dof3_data_CB",(XtPointer)dof3_data_CB},
		{"dof3_dataexp_CB",(XtPointer)dof3_dataexp_CB},
		{"dof3_arrow_CB",(XtPointer)dof3_arrow_CB}
	};
	static MrmRegisterArg identifier_list[]=
	{
		{"Dof3_structure",(XtPointer)NULL},
		{"dof3_slider_ID",(void *)dof3_slider_ID},
		{"dof3_combo_label_ID",(XtPointer)dof3_combo_label_ID},
		{"dof3_combo_data_ID",(XtPointer)dof3_combo_data_ID},
		{"dof3_combo_arrneg_ID",(XtPointer)dof3_combo_arrneg_ID},
		{"dof3_combo_dataexp_ID",(XtPointer)dof3_combo_dataexp_ID},
		{"dof3_combo_arrpos_ID",(XtPointer)dof3_combo_arrpos_ID},
		{"dof3_combo_toggle_ID",(XtPointer)dof3_combo_toggle_ID}
	};
	Widget return_widget;

	ENTER(create_dof3_widget);
	return_widget=(Widget)NULL;
	if (MrmOpenHierarchy_base64_string(dof3_uidh,
		&dof3_hierarchy,&dof3_hierarchy_open))
	{
		/* allocate memory */
		if (ALLOCATE(temp_dof3,struct Dof3_struct,1))
		{
			/* initialise the structure */
			temp_dof3->widget_parent=parent;
			temp_dof3->widget=(Widget)NULL;
			/* only used if it is a direction widget */
			temp_dof3->position_widget=(Widget)NULL;
			temp_dof3->widget_address=dof3_widget;
			temp_dof3->position=(struct Dof3_struct *)NULL;
			temp_dof3->type=type;
			temp_dof3->mode=mode;
			temp_dof3->slider_ofs=0;
			temp_dof3->control_widget=(Widget)NULL;
			temp_dof3->input_widget=(Widget)NULL;
			for (i=0;i<DOF3_NUM_CALLBACKS;i++)
			{
				temp_dof3->callback_array[i].procedure=(Callback_procedure *)NULL;
				temp_dof3->callback_array[i].data=NULL;
			}
			switch (temp_dof3->type)
			{
				case DOF3_POSITION:
				{
					if ((coord_sys<0)||(coord_sys>=DOF3_NUM_POSITION_COORD))
					display_message(ERROR_MESSAGE,
						"create_dof3_widget.  Invalid coordinate system.");
				} break;
				case DOF3_DIRECTION:
				{
					if ((coord_sys<0)||(coord_sys>=DOF3_NUM_DIRECTION_COORD))
					display_message(ERROR_MESSAGE,
						"create_dof3_widget.  Invalid coordinate system.");
				} break;
				case DOF3_VECTOR:
				{
					if ((coord_sys<0)||(coord_sys>=DOF3_NUM_VECTOR_COORD))
					display_message(ERROR_MESSAGE,
						"create_dof3_widget.  Invalid coordinate system.");
				} break;
			}
			if ((temp_dof3->mode==DOF3_RELATIVE)&&(coord_sys!=0))
			{
				display_message(ERROR_MESSAGE,
					"create_dof3_widget.  Invalid coordinate system for relative widget");
			}
			temp_dof3->current_coord=coord_sys;
			temp_dof3->return_coord=coord_sys;
			for (n=0;n<3;n++)
			{
				temp_dof3->axis[n].toggle=(Widget)NULL;
				temp_dof3->axis[n].data=(Widget)NULL;
				temp_dof3->axis[n].label=(Widget)NULL;
				temp_dof3->axis[n].dataexp=(Widget)NULL;
				temp_dof3->axis[n].labelexp=(Widget)NULL;
				temp_dof3->axis[n].arrow_pos=(Widget)NULL;
				temp_dof3->axis[n].arrow_neg=(Widget)NULL;
				temp_dof3->current_value.data[n]=init_data->data[n];
				temp_dof3->save_value.data[n]=init_data->data[n];
				temp_dof3->editing[n]=0;
				temp_dof3->resolution[n]=
					dof3_coord_limits[temp_dof3->type][temp_dof3->current_coord].
					initialexp[n];
			}
			temp_dof3->current_axis=0;
			temp_dof3->lock_data=0;
			/* keep all the resolution scales the same */
			temp_dof3->link_res=1;
			if ((DOF3_DIRECTION==temp_dof3->type)&&(DOF3_RELATIVE==temp_dof3->mode))
			{
				euler_matrix(&temp_dof3->current_value,&temp_dof3->orientation);
			}
			else
			{
				matrix_I(&temp_dof3->orientation);
			}
#if defined (EXT_INPUT)
			matrix_I(&(temp_dof3->ext_origin.origin));
			matrix_I(&(temp_dof3->ext_origin.last));
			for (i=0;i<3;i++)
			{
				temp_dof3->ext_origin.position.data[i]=0;
			}
#endif
			/* register the callbacks */
			if (MrmSUCCESS==MrmRegisterNamesInHierarchy(dof3_hierarchy,callback_list,
				XtNumber(callback_list)))
			{
				/* assign and register the identifiers */
				identifier_list[0].value=(XtPointer)temp_dof3;
				if (MrmSUCCESS==MrmRegisterNamesInHierarchy(dof3_hierarchy,
					identifier_list,XtNumber(identifier_list)))
				{
					/* fetch position window widget */
					if (MrmSUCCESS==MrmFetchWidget(dof3_hierarchy,"dof3_dialog",
						temp_dof3->widget_parent,&(temp_dof3->widget),&dof3_dialog_class))
					{
						XtManageChild(temp_dof3->widget);
						/* initialise with the correct images */
						dof3_init(temp_dof3,temp_dof3->current_coord);
						/* now select the first toggle button */
						dof3_change_axis(temp_dof3,0,0);
						return_widget=temp_dof3->widget;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_dof3_widget.  Could not fetch dof3 dialog");
						DEALLOCATE(temp_dof3);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_dof3_widget.  Could not register identifiers");
					DEALLOCATE(temp_dof3);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_dof3_widget.  Could not register callbacks");
				DEALLOCATE(temp_dof3);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_dof3_widget.  Could not allocate dof3 widget structure");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_dof3_widget.  Could not open hierarchy");
	}
	*dof3_widget=return_widget;
	LEAVE;

	return (return_widget);
} /* create_dof3_widget */

int dof3_set_data(Widget dof3_widget,
	enum Dof3_data_type data_type,void *data)
/*******************************************************************************
LAST MODIFIED : 21 January 1995

DESCRIPTION :
Changes a data item of the dof3 widget.
==============================================================================*/
{
	char temp_str[DOF3_STRING_SIZE];
	int coord_sys,i,return_code;
	struct Dof3_struct *temp_dof3;
	struct Dof3_data temp_values,new_values;

	ENTER(dof3_set_data);
	/* Get the pointer to the data for the dof3 dialog */
	XtVaGetValues(dof3_widget,XmNuserData,&temp_dof3,NULL);
	switch (data_type)
	{
		case DOF3_UPDATE_CB:
		{
			temp_dof3->callback_array[DOF3_UPDATE_CB].procedure=
				((struct Callback_data *)data)->procedure;
			temp_dof3->callback_array[DOF3_UPDATE_CB].data=
				((struct Callback_data *)data)->data;
			return_code=1;
		} break;
		case DOF3_INPUT_WIDGET:
		{
			temp_dof3->input_widget=(Widget)data;
			return_code=1;
		} break;
		case DOF3_CONTROL_WIDGET:
		{
			temp_dof3->control_widget=(Widget)data;
			return_code=1;
		} break;
		case DOF3_DATA:
		{
			/* convert the data from the clients format to our format */
			switch (temp_dof3->type)
			{
				case DOF3_POSITION:
				{
					(*(conversion_position[temp_dof3->return_coord]
						[temp_dof3->current_coord]))((struct Dof3_data *)data,
							&new_values);
				} break;
				case DOF3_DIRECTION:
				{
					(*(conversion_direction[temp_dof3->return_coord]
						[temp_dof3->current_coord]))((struct Dof3_data *)data,
							&new_values);
				} break;
				case DOF3_VECTOR:
				{
					(*(conversion_vector[temp_dof3->return_coord]
						[temp_dof3->current_coord]))((struct Dof3_data *)data,
							&new_values);
				} break;
			}
			coord_sys=temp_dof3->current_coord;
			for (i=0;i<3;i++)
			{
				temp_values.data[i]=new_values.data[i];
				if (temp_values.data[i]>dof3_coord_limits[temp_dof3->type][coord_sys].
					maximum[i])
				{
					temp_values.data[i]=dof3_coord_limits[temp_dof3->type][coord_sys].
						maximum[i];
				}
				if (temp_values.data[i]<dof3_coord_limits[temp_dof3->type][coord_sys].
					minimum[i])
				{
					temp_values.data[i]=dof3_coord_limits[temp_dof3->type][coord_sys].
						minimum[i];
				}
				if (temp_values.data[i]!=temp_dof3->current_value.data[i])
				{
					sprintf(temp_str,DOF3_NUM_FORMAT,temp_values.data[i]);
					XmTextSetString(temp_dof3->axis[i].data,temp_str);
					temp_dof3->current_value.data[i]=temp_values.data[i];
				}
			}
			/* update the origin */
			if (temp_dof3->type==DOF3_DIRECTION)
			{
#if defined (EXT_INPUT)
				dof3_update_ext_origin(temp_dof3,&(temp_dof3->current_value));
#endif
				if (DOF3_RELATIVE==temp_dof3->mode)
				{
					euler_matrix(&(temp_dof3->current_value),&(temp_dof3->orientation));
					if (temp_dof3->position)
					{
						matrix_copy(&(temp_dof3->position->orientation),
							&(temp_dof3->orientation));
					}
				}
			}
			return_code=1;
		} break;
		case DOF3_POSITION_WIDGET:
		{
			if (DOF3_DIRECTION==temp_dof3->type)
			{
				temp_dof3->position_widget=(Widget)data;
				if (data)
				{
					XtVaGetValues((Widget)data,XmNuserData,&(temp_dof3->position),NULL);
					if (DOF3_RELATIVE==temp_dof3->mode)
					{
						matrix_copy(&(temp_dof3->position->orientation),
							&(temp_dof3->orientation));
					}
				}
				else
				{
					temp_dof3->position=(struct Dof3_struct *)NULL;
				}
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"dof3_set_data.  Cannot set position widget for a position widget.");
				return_code=0;
			}
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"dof3_set_data.  Invalid data type.");
			return_code=0;
		} break;
	}
	LEAVE;

	return (return_code);
} /* dof3_set_data */

void *dof3_get_data(Widget dof3_widget,
	enum Dof3_data_type data_type)
/*******************************************************************************
LAST MODIFIED : 2 November 1995

DESCRIPTION :
Changes a data item of the dof3 widget.
==============================================================================*/
{
	void *return_code;
	struct Dof3_struct *temp_dof3;
	static struct Callback_data dat_callback;
	static Widget dat_widget;
	static struct Dof3_data dat_data;

	ENTER(dof3_get_data);
	/* Get the pointer to the data for the dof3 dialog */
	XtVaGetValues(dof3_widget,XmNuserData,&temp_dof3,NULL);
	switch (data_type)
	{
		case DOF3_UPDATE_CB:
		{
			dat_callback.procedure=
				temp_dof3->callback_array[DOF3_UPDATE_CB].procedure;
			dat_callback.data=temp_dof3->callback_array[DOF3_UPDATE_CB].data;
			return_code= &dat_callback;
		} break;
		case DOF3_INPUT_WIDGET:
		{
			dat_widget=temp_dof3->input_widget;
			return_code=dat_widget;
		} break;
		case DOF3_CONTROL_WIDGET:
		{
			dat_widget=temp_dof3->control_widget;
			return_code=dat_widget;
		} break;
		case DOF3_DATA:
		{
			/* convert the data to the format that the client requires */
			switch (temp_dof3->type)
			{
				case DOF3_POSITION:
				{
					(*(conversion_position[temp_dof3->current_coord]
						[temp_dof3->return_coord]))(&(temp_dof3->current_value),
							&dat_data);
				} break;
				case DOF3_DIRECTION:
				{
					(*(conversion_direction[temp_dof3->current_coord]
						[temp_dof3->return_coord]))(&(temp_dof3->current_value),
							&dat_data);
				} break;
				case DOF3_VECTOR:
				{
					(*(conversion_vector[temp_dof3->current_coord]
						[temp_dof3->return_coord]))(&(temp_dof3->current_value),
							&dat_data);
				} break;
			}
			return_code= &dat_data;
		} break;
		case DOF3_POSITION_WIDGET:
		{
			dat_widget=temp_dof3->position_widget;
			return_code=dat_widget;
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"dof3_get_data.  Invalid data type.");
			return_code=NULL;
		} break;
	}
	LEAVE;

	return (return_code);
} /* dof3_get_data */

void dof3_init(struct Dof3_struct *temp_dof3,int old_coord)
/*******************************************************************************
LAST MODIFIED : 29 January 1996

DESCRIPTION :
Changes the icons and labels to reflect the new coordinate system.
==============================================================================*/
{
	char pixmap_name[DOF3_STRING_SIZE],temp_str[DOF3_STRING_SIZE];
	/* space to create the name of the pixmap */
	int n;
	Pixel fg,bg,yellow_pix,red_pix;
	Pixmap new,old;
	struct Dof3_data old_values;
	XmString temp_label;

	ENTER(dof3_init);
	if (dof3_hierarchy_open)
	{
		/* get the old values */
		for (n=0;n<3;n++)
		{
			old_values.data[n]=temp_dof3->current_value.data[n];
		}
		/* change the values to make the actual position stay constant */
		switch (temp_dof3->type)
		{
			case DOF3_POSITION:
			{
				(*(conversion_position[old_coord][temp_dof3->current_coord]))
					(&old_values,&(temp_dof3->current_value));
			} break;
			case DOF3_DIRECTION:
			{
				(*(conversion_direction[old_coord][temp_dof3->current_coord]))
					(&old_values,&(temp_dof3->current_value));
			} break;
			case DOF3_VECTOR:
			{
				(*(conversion_vector[old_coord][temp_dof3->current_coord]))
					(&old_values,&(temp_dof3->current_value));
			} break;
		}
		/* get the foreground and background colors for the pixmaps */
		XtVaGetValues(temp_dof3->widget,XmNforeground,&fg,
			XmNbackground,&bg,NULL);

		/*???GMH.  Temporarily, get the same pixmap with a yellow background for the
			selected button - also change the fill color to yellow when selected */
		if ((MrmSUCCESS==MrmFetchColorLiteral(dof3_hierarchy,"yellow",
			XtDisplayOfObject(temp_dof3->widget),(Colormap)NULL,&yellow_pix))&&
			(MrmSUCCESS==MrmFetchColorLiteral(dof3_hierarchy,"red",
			XtDisplayOfObject(temp_dof3->widget),(Colormap)NULL,&red_pix)))
		{
			for (n=0;n<3;n++)
			{
				/* work out the name of the icon */
				switch (temp_dof3->type)
				{
					case DOF3_POSITION:
					{
						if (temp_dof3->mode==DOF3_RELATIVE)
						{
							sprintf(pixmap_name,"dof3_pos_r_axis%d_coord0_icon",n);
						}
						else
						{
							sprintf(pixmap_name,"dof3_pos_axis%d_coord%d_icon",n,
								temp_dof3->current_coord);
						}
					} break;
					case DOF3_DIRECTION:
					{
						if (temp_dof3->mode==DOF3_RELATIVE)
						{
							sprintf(pixmap_name,"dof3_dir_r_axis%d_coord0_icon",n);
						}
						else
						{
							sprintf(pixmap_name,"dof3_dir_axis%d_coord%d_icon",n,
								temp_dof3->current_coord);
						}
					} break;
					case DOF3_VECTOR:
					{
						sprintf(pixmap_name,"dof3_vec_axis%d_coord%d_icon",n,
							temp_dof3->current_coord);
					} break;
				}
				/* get the normal icon */
				if (MrmSUCCESS==MrmFetchIconLiteral(dof3_hierarchy,pixmap_name,
					XtScreen(temp_dof3->widget),XtDisplayOfObject(temp_dof3->widget),
					red_pix,bg,&new))
				{
					XtVaGetValues(temp_dof3->axis[n].toggle,XmNlabelPixmap,&old,NULL);
					XtVaSetValues(temp_dof3->axis[n].toggle,XmNlabelPixmap,new,NULL);
					XmDestroyPixmap(XtScreen(temp_dof3->widget),old);
				}
				/* now get the selected icon */
				if (MrmSUCCESS==MrmFetchIconLiteral(dof3_hierarchy,pixmap_name,
					XtScreen(temp_dof3->widget),XtDisplayOfObject(temp_dof3->widget),
					yellow_pix,bg,&new))
				{
					XtVaGetValues(temp_dof3->axis[n].toggle,XmNselectPixmap,&old,NULL);
					XtVaSetValues(temp_dof3->axis[n].toggle,XmNselectPixmap,new,NULL);
					XmDestroyPixmap(XtScreen(temp_dof3->widget),old);
				}
				/* set up the names of the axes */
				temp_label=XmStringCreateSimple(
					dof3_axis_names[temp_dof3->type][temp_dof3->current_coord][n]);
				XtVaSetValues(temp_dof3->axis[n].label,XmNlabelString,temp_label,NULL);
				XmStringFree(temp_label);
				/* and now display the initial values of the data fields */
				sprintf(temp_str,DOF3_NUM_FORMAT,temp_dof3->current_value.data[n]);
				XmTextSetString(temp_dof3->axis[n].data,temp_str);
				/* set the resolution to be correct for the new coordinate system */
				temp_dof3->resolution[n]=
					dof3_coord_limits[temp_dof3->type][temp_dof3->current_coord].
					initialexp[n];
				sprintf(temp_str,"%2i",temp_dof3->resolution[n]);
				XmTextSetString(temp_dof3->axis[n].dataexp,temp_str);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"dof3_init.  Dof3 hierarchy is not open");
	}
	LEAVE;
} /* dof3_init */

#if defined (EXT_INPUT)
int dof3_input_module_CB(void *identifier,Input_module_message message)
/*******************************************************************************
LAST MODIFIED : 7 September 1994

DESCRIPTION :
Finds the id of the buttons on the dof3ment dialog box.
==============================================================================*/
{
	char temp_str[DOF3_STRING_SIZE];
	struct Dof3_data temp_increment,temp_values;
	int coord_sys,i,return_code;
	struct Dof3_struct *temp_dof3=identifier;

	ENTER(dof3_input_module_CB);
	switch (message->type)
	{
		case IM_TYPE_MOTION:
		{
			if (!temp_dof3->lock_data)
			{
				coord_sys=temp_dof3->current_coord;
				switch (temp_dof3->type)
				{
					case DOF3_POSITION:
					case DOF3_VECTOR:
					{
						for (i=0;i<3;i++)
						{
							if (message->source==IM_SOURCE_POLHEMUS)
							{
								message->data[i] -= temp_dof3->ext_origin.position.data[i];
							}
							/* check which coord sys we are in so we know which values
							to scale */
							if (message->source==IM_SOURCE_DIALS)
							{
								/* dials do not correspond to fixed directions, so take them
								as just changing the raw values */
								sprintf(temp_str,"%"DOF3_PRECISION_STRING"E%i",message->data[i],
									temp_dof3->resolution[i]);
							}
							else
							{
								switch (coord_sys)
								{
									case CONV_RECTANGULAR_CARTESIAN:
									{
										sprintf(temp_str,"%"DOF3_PRECISION_STRING"E%i",
											message->data[i],temp_dof3->resolution[i]);
									} break;
									case CONV_CYLINDRICAL_POLAR:
									{
										/* the x and y cartesian compos should be scaled by the
										radius scale factor */
										if (1==i)
										{
											sprintf(temp_str,"%"DOF3_PRECISION_STRING"E%i",
												message->data[i],temp_dof3->resolution[0]);
										}
										else
										{
											sprintf(temp_str,"%"DOF3_PRECISION_STRING"E%i",
												message->data[i],temp_dof3->resolution[i]);
										}
									} break;
									case CONV_SPHERICAL_POLAR:
									{
										/* the x and y and z cartesian compos should be scaled by
										the radius scale factor */
										sprintf(temp_str,"%"DOF3_PRECISION_STRING"E%i",
											message->data[i],temp_dof3->resolution[0]);
									} break;
									default:
									{
										display_message(WARNING_MESSAGE,
											"dof3_input_module_CB.  Invalid coordinate system");
										coord_sys=CONV_RECTANGULAR_CARTESIAN;
									} break;
								}
							} /* !dials */
							if (message->source==IM_SOURCE_POLHEMUS)
							{
								/* undo the previous operation */
								message->data[i] += temp_dof3->ext_origin.position.data[i];
								temp_dof3->ext_origin.position.data[i]=message->data[i];
							}
							sscanf(temp_str,"%"DOF3_PRECISION_STRING,&temp_increment.data[i]);
						}
						if (message->source==IM_SOURCE_POLHEMUS)
						{
							/* copy the data to temp_values from temp_increment */
							for (i=0;i<3;i++)
							{
								temp_values.data[i]=temp_increment.data[i];
							}
							/* make the change relative to the new origin, not the world */
							dof3_convert_pos(temp_dof3,&temp_increment,&temp_values);
						}
						if ((temp_dof3->mode==DOF3_RELATIVE)&&(message->source!=
							IM_SOURCE_POLHEMUS))
						{
							/* we need to convert the values in temp_increment to values
							that are relative to our current orientation */
							dof3_convert_relative_pos(temp_dof3,&temp_increment,&temp_values);
							/* the result is now in temp_values */
						}
						else
						{
							/* must be position or vector */
							if (message->source!=IM_SOURCE_DIALS)
							{
								if (temp_dof3->type==DOF3_POSITION)
								{
									(*(conversion_increment_position[CONV_RECTANGULAR_CARTESIAN]
										[coord_sys]))(&temp_increment,&temp_dof3->current_value,
											&temp_values);
								}
								else /* vector */
								{
									(*(conversion_increment_vector[CONV_RECTANGULAR_CARTESIAN]
										[coord_sys]))(&temp_increment,&temp_dof3->current_value,
											&temp_values);
								}
							}
							else
							{ /* copy the dial changes directly to temp_values */
								for (i=0;i<3;i++)
								{
									temp_values.data[i]=temp_dof3->current_value.data[i]+
										temp_increment.data[i];
								}
							}
						}
					} break;
					case DOF3_DIRECTION:
					{
						if (message->source==IM_SOURCE_POLHEMUS)
						{
							/* remember this orientation */
							matrix_copy_transpose(&(temp_dof3->ext_origin.last),
								(Gmatrix *)&message->data[3]);
							/* convert the absolute euler to euler wrt origin */
							dof3_convert_dir(temp_dof3,(Gmatrix *)&message->data[3],
								&temp_values);
						}
						else
						{
							/* copy the dial/spaceball changes directly to temp_values */
							for (i=0;i<3;i++)
							{
								sprintf(temp_str,"%"DOF3_PRECISION_STRING"E%i",
									message->data[i+3],temp_dof3->resolution[i]);
								sscanf(temp_str,"%"DOF3_PRECISION_STRING,
									&temp_increment.data[i]);
							}
							if (temp_dof3->mode==DOF3_RELATIVE)
							{
								/* we need to rotate our orientation matrix by what ever
								changes have been made */
								dof3_convert_relative_dir(temp_dof3,&temp_increment,
									&temp_values);
							}
							else
							{
								/* we just add the current value to temp_increment */
								for (i=0;i<3;i++)
								{
									temp_values.data[i]=
										temp_increment.data[i]+temp_dof3->current_value.data[i];
								}
							}
						}
					} break;
				} /* case of type */
				dof3_refresh(temp_dof3,&temp_values);
			}
		} break;
		case IM_TYPE_BUTTON_PRESS:
		{
		} break;
		case IM_TYPE_BUTTON_RELEASE:
		{
		} break;
		default:
		{
			display_message(WARNING_MESSAGE,
				"dof3_input_module_CB.  Invalid message type.");
		} break;
	}
	dof3_update(temp_dof3);
	return_code=1;
	LEAVE;

	return (return_code);
} /* dof3_input_module_CB */
#endif

void dof3_reset(struct Dof3_struct *temp_dof3)
/*******************************************************************************
LAST MODIFIED : 18 April 1994

DESCRIPTION :
Set the values to whatever has been saved.
==============================================================================*/
{
	int i;
	struct Dof3_data temp_values;

	ENTER(dof3_reset);
	for (i=0;i<3;i++)
	{
		temp_values.data[i]=temp_dof3->save_value.data[i];
	}
	dof3_refresh(temp_dof3,&temp_values);
#if defined (EXT_INPUT)
	/* update the origin */
	if (temp_dof3->type==DOF3_DIRECTION)
	{
		dof3_update_ext_origin(temp_dof3,&temp_dof3->current_value);
	}
#endif
	/* now reflect the change */
	dof3_update(temp_dof3);
	LEAVE;
} /* dof3_reset */

void dof3_save(struct Dof3_struct *temp_dof3)
/*******************************************************************************
LAST MODIFIED : 18 April 1994

DESCRIPTION :
Set the values to whatever has been saved.
==============================================================================*/
{
	int i;

	ENTER(dof3_save);
	for (i=0;i<3;i++)
	{
		temp_dof3->save_value.data[i]=temp_dof3->current_value.data[i];
	}
	LEAVE;
} /* dof3_save */

void dof3_change_lock(struct Dof3_struct *temp_dof3,int state)
/*******************************************************************************
LAST MODIFIED : 18 April 1994

DESCRIPTION :
Set the values to whatever has been saved.
==============================================================================*/
{
	ENTER(dof3_change_lock);
	temp_dof3->lock_data=state;
	LEAVE;
} /* dof3_change_lock */


int dof3_get_lock(struct Dof3_struct *temp_dof3)
/*******************************************************************************
LAST MODIFIED : 2 November 1995

DESCRIPTION :
returns the value of lock_data.
==============================================================================*/
{
	ENTER(dof3_get_lock);
	LEAVE;

	return (temp_dof3->lock_data);
} /* dof3_get_lock */

void dof3_change_link(struct Dof3_struct *temp_dof3,int state)
/*******************************************************************************
LAST MODIFIED : 18 April 1994

DESCRIPTION :
sets link_res to state.
==============================================================================*/
{
	ENTER(dof3_change_link);
	temp_dof3->link_res=state;
	LEAVE;
} /* dof3_change_link */

int dof3_get_link(struct Dof3_struct *temp_dof3)
/*******************************************************************************
LAST MODIFIED : 2 November 1995

DESCRIPTION :
returns the value of link_res.
==============================================================================*/
{
	ENTER(dof3_get_link);
	LEAVE;

	return (temp_dof3->link_res);
} /* dof3_get_link */













