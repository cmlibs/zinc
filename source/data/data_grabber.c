/*******************************************************************************
FILE : data_grabber.c

LAST MODIFIED : 14 May 1998

DESCRIPTION :
Creates a scrolled list of objects based upon their name.  Allows the user
to add, delete and rename the objects.  Interfaces with the global manager
for each type it supports.
NOTE:
Contains multipletype definitions of essentially the same variables.  At some
stage, this should be changed to OOP, using this as a base class.
==============================================================================*/
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <Xm/Xm.h>
#include <X11/Xlib.h>
#include <Mrm/MrmPublic.h>
#include <Xm/ScrollBar.h>
#include <Xm/List.h>
#include "data/data_grabber.h"
#include "data/data_grabber.uidh"
#include "general/debug.h"
#include "general/compare.h"
#include "general/list_private.h"
#include "io_devices/input_module_widget.h"
#include "user_interface/gui_dialog_macros.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module types
------------
*/
struct DG_calib_sum_position_struct
{
	struct DG_struct *data_grabber;
	int component;
	struct Dof3_data sum;
};

/*
Module variables
----------------
*/
static int data_grabber_hierarchy_open=0;
static MrmHierarchy data_grabber_hierarchy;

/*
Module functions
----------------
*/
DECLARE_SIMPLE_LIST_OBJECT_FUNCTIONS(DG_calib_data)
FULL_DECLARE_LIST_TYPE(DG_calib_data);
DECLARE_LIST_FUNCTIONS(DG_calib_data)
DECLARE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(DG_calib_data,list_number,int,
	compare_int)

DECLARE_DIALOG_IDENTIFY_FUNCTION(data_grabber, \
	DG_struct, stream_distance_entry)
DECLARE_DIALOG_IDENTIFY_FUNCTION(data_grabber, \
	DG_struct, stream_distance_text)

static void dg_create_calib_dialog(struct DG_struct *data_grabber,
	int component)
/*******************************************************************************
LAST MODIFIED : 25 September 1995

DESCRIPTION :
Creates a new calibration dialog.  If one exists, bring it up to the front.
==============================================================================*/
{
	Arg override_arg;
	MrmType data_grabber_calibration_class;

	ENTER(dg_create_calib_dialog);
	if (data_grabber_hierarchy_open)
	{
		if (data_grabber->calib_struct[component].dialog=XtVaCreatePopupShell(
			"Calibrate",topLevelShellWidgetClass,data_grabber->widget,
			/* XmNallowShellResize,TRUE, */NULL))
		{
			XtSetArg(override_arg,XmNuserData,component);
			if (MrmSUCCESS==MrmFetchWidgetOverride(data_grabber_hierarchy,
				"dg_calib_widget",data_grabber->calib_struct[component].dialog,
				NULL,&override_arg,1,&data_grabber->calib_struct[component].widget,
				&data_grabber_calibration_class))
			{
				XtManageChild(data_grabber->calib_struct[component].widget);
				XtRealizeWidget(data_grabber->calib_struct[component].dialog);
				XtPopup(data_grabber->calib_struct[component].dialog, XtGrabNone);
			}
			else
			{
				display_message(WARNING_MESSAGE,
					"dg_create_calib_dialog.  Could not fetch calibration widget");
			}
		}
		else
		{
			display_message(WARNING_MESSAGE,
				"dg_create_calib_dialog.  Could not create shell widget");
		}
	}
	else
	{
		display_message(WARNING_MESSAGE,
			"dg_create_calib_dialog.  Hierarchy not open");
	}
	LEAVE;
} /* dg_create_calib_dialog */

static void dg_bring_up_calib(struct DG_struct *data_grabber,int component)
/*******************************************************************************
LAST MODIFIED : 25 September 1995

DESCRIPTION :
Creates a new calibration dialog.  If one exists, bring it up to the front.
==============================================================================*/
{
	ENTER(dg_bring_up_calib);
	if (data_grabber->calib_struct[component].dialog)
	{
		XtPopup(data_grabber->calib_struct[component].dialog,XtGrabNone);
	}
	else
	{
		dg_create_calib_dialog(data_grabber,component);
	}
	LEAVE;
} /* dg_bring_up_calib */

static void dg_update(struct DG_struct *data_grabber)
/*******************************************************************************
LAST MODIFIED : 25 September 1994

DESCRIPTION :
Tells CMGUI about the current values. Returns a pointer to the currently
data_grabbered object.
==============================================================================*/
{
	ENTER(dg_update);
	if (data_grabber->callback_array[DATA_GRABBER_UPDATE_CB].procedure)
	{
		(data_grabber->callback_array[DATA_GRABBER_UPDATE_CB].procedure)
			(data_grabber->widget,
				data_grabber->callback_array[DATA_GRABBER_UPDATE_CB].data,
				&data_grabber->current_value);
	}
	LEAVE;
} /* dg_update */

static void dg_stream_mode_CB(Widget w,int *tag,
	XmToggleButtonCallbackStruct *reason)
/*******************************************************************************
LAST MODIFIED : 18 August 2000

DESCRIPTION :
==============================================================================*/
{
	struct DG_struct *data_grabber;

	ENTER(dg_stream_mode_CB);
	USE_PARAMETER(tag);
	/* find out which data_grabber_dialog widget we are in */
	XtVaGetValues(w,XmNuserData,&data_grabber,NULL);
	if (reason->set)
	{
		data_grabber->streaming_mode = 1;
		if (data_grabber->stream_distance_entry)
		{
			XtSetSensitive(data_grabber->stream_distance_entry,
				True );
		}
	}
	else
	{
		data_grabber->streaming_mode = 0;
		if (data_grabber->stream_distance_entry)
		{
			XtSetSensitive(data_grabber->stream_distance_entry,
				False );
		}
	}
	LEAVE;
} /* dg_stream_mode_CB */

static void dg_stream_distance_CB(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 18 August 2000

DESCRIPTION :
==============================================================================*/
{
	char temp_string[50];
	char *text;
	float new_stream_distance;
	struct DG_struct *data_grabber;

	ENTER(dg_stream_distance_CB);
	USE_PARAMETER(call_data);
	USE_PARAMETER(widget);
	/* check arguments */
	if (data_grabber = (struct DG_struct *)client_data)
	{
		new_stream_distance = 1;
		if (data_grabber->stream_distance_text)
		{
			XtVaGetValues(data_grabber->stream_distance_text,
				XmNvalue,&text,NULL);
			if (text)
			{
				sscanf(text,"%f",&new_stream_distance);
				XtFree(text);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"dg_stream_distance_CB.  Missing widget text");
			}
			if(new_stream_distance != data_grabber->streaming_distance)
			{
				data_grabber->streaming_distance = new_stream_distance;
				sprintf(temp_string,"%g",data_grabber->streaming_distance);
				XtVaSetValues(data_grabber->stream_distance_text,
					XmNvalue,temp_string,
					NULL);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"dg_stream_distance_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* dg_stream_distance_CB */

static void dg_update_select(struct DG_struct *data_grabber)
/*******************************************************************************
LAST MODIFIED : 25 September 1995

DESCRIPTION :
A button has been pressed, so tell any clients.
==============================================================================*/
{
	ENTER(dg_update_select);
	if (data_grabber->callback_array[DATA_GRABBER_SELECT_CB].procedure)
	{
		(data_grabber->callback_array[DATA_GRABBER_SELECT_CB].procedure)
			(data_grabber->widget,
			data_grabber->callback_array[DATA_GRABBER_SELECT_CB].data,
			&data_grabber->current_value);
	}
	LEAVE;
} /* dg_update_select */

static void dg_dof3_update(Widget dof3_widget,void *user_data,
	void *new_dof3_data)
/*******************************************************************************
LAST MODIFIED : 25 September 1995

DESCRIPTION :
Receives a pointer to a dof3_struct, and the new data for it.
==============================================================================*/
{
	int i;
	struct Dof3_data *new=new_dof3_data,*dest=user_data;

	ENTER(dg_dof3_update);
	USE_PARAMETER(dof3_widget);
	for (i=0;i<3;i++)
	{
		dest->data[i]=new->data[i];
	}
	LEAVE;
} /* dg_dof3_update */

static void dg_dof3_data_position_update(Widget dof3_widget,void *user_data,
	void *new_dof3_data)
/*******************************************************************************
LAST MODIFIED : 25 September 1995

DESCRIPTION :
Receives a pointer to a dof3_struct, and the new data for it.
==============================================================================*/
{
	int i;
	struct DG_struct *data_grabber=user_data;
	struct Dof3_data *new=new_dof3_data;

	ENTER(dg_dof3_data_position_update);
	USE_PARAMETER(dof3_widget);
	for (i=0;i<3;i++)
	{
		data_grabber->current_value.position.data[i]=new->data[i];
	}
	dg_update(data_grabber);
	LEAVE;
} /* dg_dof3_position_update */

static void dg_dof3_data_tangent_update(Widget dof3_widget,void *user_data,
	void *new_dof3_data)
/*******************************************************************************
LAST MODIFIED : 25 September 1995

DESCRIPTION :
Receives a pointer to a dof3_struct, and the new data for it.
==============================================================================*/
{
	int i;
	struct DG_struct *data_grabber=user_data;
	struct Dof3_data *new=new_dof3_data;

	ENTER(dg_dof3_data_tangent_update);
	USE_PARAMETER(dof3_widget);
	for (i=0;i<3;i++)
	{
		data_grabber->current_value.tangent.data[i]=new->data[i];
	}
	dg_update(data_grabber);
	LEAVE;
} /* dg_dof3_tangent_update */

static void dg_dof3_data_normal_update(Widget dof3_widget,void *user_data,
	void *new_dof3_data)
/*******************************************************************************
LAST MODIFIED : 25 September 1995

DESCRIPTION :
Receives a pointer to a dof3_struct, and the new data for it.
==============================================================================*/
{
	int i;
	struct DG_struct *data_grabber=user_data;
	struct Dof3_data *new=new_dof3_data;

	ENTER(dg_dof3_data_normal_update);
	USE_PARAMETER(dof3_widget);
	for (i=0;i<3;i++)
	{
		data_grabber->current_value.normal.data[i]=new->data[i];
	}
	dg_update(data_grabber);
	LEAVE;
} /* dg_dof3_normal_update */

#if defined (EXT_INPUT)
static int dg_input_module_CB(void *identifier,Input_module_message message)
/*******************************************************************************
LAST MODIFIED : 17 February 1998

DESCRIPTION :
Accepts input from one of the devices.
==============================================================================*/
{
	float distance;
	int i,return_code,rotate_axis;
	DATA_GRABBER_PRECISION max_rotation;
	struct DG_struct *data_grabber=identifier;

	ENTER(dg_input_module_CB);
	switch (message->type)
	{
		case IM_TYPE_MOTION:
		{
			switch (message->source)
			{
				case IM_SOURCE_SPACEBALL:
				case IM_SOURCE_DIALS:
				{
					for (i=0;i<3;i++)
					{
						data_grabber->current_value.position.data[i] +=
							message->data[i];
						data_grabber->last_position.data[i]=
							data_grabber->current_value.position.data[i];
					}
					/* see if there is a rotation>0 */
					rotate_axis=-1;
					max_rotation=0.0;
					for (i=0;i<3;i++)
					{
						if (message->data[i+3]>max_rotation)
						{
							rotate_axis=i;
							max_rotation=message->data[i+3];
						}
					}
					if (rotate_axis>=0)
					{
						for (i=0;i<3;i++)
						{
							data_grabber->current_value.tangent.data[i]=
								data_grabber->calib_value.tangent.data[i];
							data_grabber->current_value.normal.data[i]=
								data_grabber->calib_value.normal.data[i];
						}
						matrix_rotate(&data_grabber->last_direction,max_rotation,
							'x'+rotate_axis);
							/*???DB.  Is this a good idea ?  Enumerated type ? */
						matrix_postmult_vector(
							data_grabber->current_value.tangent.data,
							&data_grabber->last_direction);
						matrix_postmult_vector(data_grabber->current_value.normal.data,
							&data_grabber->last_direction);
					}
				} break;
				case IM_SOURCE_HAPTIC:
				{
					for (i=0;i<3;i++)
					{
						data_grabber->last_position.data[i]=
							data_grabber->current_value.position.data[i];
						data_grabber->current_value.position.data[i]=message->data[i];
					}
				} break;
				case IM_SOURCE_FARO:
				{
					for (i=0;i<3;i++)
					{
						data_grabber->last_position.data[i]=
							data_grabber->current_value.position.data[i];
						data_grabber->current_value.position.data[i]=
							message->data[i];
						data_grabber->current_value.tangent.data[i]=
							message->data[i+3];
						data_grabber->current_value.normal.data[i]=
							message->data[i+6];
					}
				} break;
				case IM_SOURCE_POLHEMUS:
				{
					for (i=0;i<3;i++)
					{
						data_grabber->last_position.data[i]=
							message->data[i];
						data_grabber->current_value.position.data[i]=
							data_grabber->calib_value.position.data[i];
						data_grabber->current_value.tangent.data[i]=
							data_grabber->calib_value.tangent.data[i];
						data_grabber->current_value.normal.data[i]=
							data_grabber->calib_value.normal.data[i];
					}
					matrix_copy(&data_grabber->last_direction,
						(Gmatrix *)&message->data[3]);
					matrix_postmult_vector(data_grabber->current_value.position.data,
						&data_grabber->last_direction);
					matrix_postmult_vector(data_grabber->current_value.tangent.data,
						&data_grabber->last_direction);
					matrix_postmult_vector(data_grabber->current_value.normal.data,
						&data_grabber->last_direction);
					for (i=0;i<3;i++)
					{
						data_grabber->current_value.position.data[i] +=
							data_grabber->last_position.data[i];
					}
				} break;
				default:
				{
					display_message(WARNING_MESSAGE,
						"dg_input_module_CB.  Invalid message source");
				} break;
			}
			if (data_grabber->mode&DATA_GRABBER_POSITION)
			{
				dof3_set_data(data_grabber->data_widget[0],
					DOF3_DATA,&data_grabber->current_value.position);
			}
			if (data_grabber->mode&DATA_GRABBER_TANGENT)
			{
				dof3_set_data(data_grabber->data_widget[1],
					DOF3_DATA,&data_grabber->current_value.tangent);
			}
			if (data_grabber->mode&DATA_GRABBER_NORMAL)
			{
				dof3_set_data(data_grabber->data_widget[2],
					DOF3_DATA,&data_grabber->current_value.normal);
			}
			if (data_grabber->streaming_mode && data_grabber->button_state)
			{
				distance = 
					(data_grabber->current_value.position.data[0] - 
						data_grabber->previous_streaming.data[0]) *
					(data_grabber->current_value.position.data[0] - 
						data_grabber->previous_streaming.data[0]) +
					(data_grabber->current_value.position.data[1] - 
						data_grabber->previous_streaming.data[1]) *
					(data_grabber->current_value.position.data[1] - 
						data_grabber->previous_streaming.data[1]) +
					(data_grabber->current_value.position.data[2] - 
						data_grabber->previous_streaming.data[2]) *
					(data_grabber->current_value.position.data[2] - 
						data_grabber->previous_streaming.data[2]);
				if (distance > 4 * data_grabber->streaming_distance * 
				  data_grabber->streaming_distance)
				{
					display_message(WARNING_MESSAGE,
						"The input is moving too fast for streaming with this distance");
				}
				if (distance > data_grabber->streaming_distance *
				  data_grabber->streaming_distance)
				{
					dg_update_select(data_grabber);
					data_grabber->previous_streaming.data[0] =
						data_grabber->current_value.position.data[0];
					data_grabber->previous_streaming.data[1] =
						data_grabber->current_value.position.data[1];
					data_grabber->previous_streaming.data[2] =
						data_grabber->current_value.position.data[2];
				}
			}
		} break;
		case IM_TYPE_BUTTON_PRESS:
		{
			switch (message->source)
			{
				case IM_SOURCE_POLHEMUS:
				{
					dg_update_select(data_grabber);
				} break;
				case IM_SOURCE_FARO:
				{
					for (i=0;i<3;i++)
					{
						data_grabber->last_position.data[i]=
							data_grabber->current_value.position.data[i];
						data_grabber->current_value.position.data[i]=
							message->data[i];
						data_grabber->current_value.tangent.data[i]=
							message->data[i+3];
						data_grabber->current_value.normal.data[i]=
							message->data[i+6];
					}
					dg_update_select(data_grabber);
				} break;
				default:
				{
					display_message(WARNING_MESSAGE,
						"dg_input_module_CB.  Invalid message source");
				} break;
			}
			data_grabber->button_state = 1;
			for (i=0;i<3;i++)
			{
				data_grabber->previous_streaming.data[i] =
					data_grabber->current_value.position.data[i];
			}
		} break;
		case IM_TYPE_BUTTON_RELEASE:
		{
			data_grabber->button_state = 0;
		} break;
		default:
		{
			display_message(WARNING_MESSAGE,
				"dg_input_module_CB.  Invalid message type");
		} break;
	}
	return_code=1;
	LEAVE;

	return (return_code);
} /* dg_input_module_CB */
#endif /* defined (EXT_INPUT) */

#if defined (EXT_INPUT)
static void dg_change_device(Widget input_module_widget,
	struct Input_module_widget_data *device_change,void *user_data)
/*******************************************************************************
LAST MODIFIED : 20 March 2000

DESCRIPTION :
Receives information from the input_module_widget to say which device is now
being received by this client.
==============================================================================*/
{
	struct DG_struct *data_grabber=user_data;

	ENTER(dg_change_device);
	USE_PARAMETER(input_module_widget);
	if (device_change->status)
	{
		input_module_register(device_change->device,data_grabber,
			data_grabber->widget,dg_input_module_CB);
	}
	else
	{
		input_module_deregister(device_change->device,data_grabber);
	}
	LEAVE;
} /* dg_change_device */
#endif /* defined (EXT_INPUT) */

static void dg_identify_button(Widget w,int button_num,unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 25 September 1995

DESCRIPTION :
Finds the id of the buttons on the data_grabber widget.
==============================================================================*/
{
	int form_number;
	struct DG_struct *data_grabber;
	XmString new_string;

	ENTER(dg_identify_button);
	USE_PARAMETER(reason);
	XtVaGetValues(w,XmNuserData,&data_grabber,NULL);
	if ((button_num==data_grabber_calib_form_ID)||
		(button_num==data_grabber_calib_label_ID)||
		(button_num==data_grabber_data_form_ID)||
		(button_num==data_grabber_data_label_ID)||
		(button_num==data_grabber_calib_button_ID))
	{
		XtVaGetValues(XtParent(w),XmNuserData,&form_number,NULL);
		switch (button_num)
		{
			case data_grabber_calib_form_ID:
			{
				data_grabber->calib_form[form_number]=w;
			} break;
			case data_grabber_calib_label_ID:
			{
				data_grabber->calib_label[form_number]=w;
				/* change the name */
				new_string=NULL;
				switch (form_number)
				{
					case 0:
					{
						new_string=XmStringCreateSimple("Position");
					} break;
					case 1:
					{
						new_string=XmStringCreateSimple("Tangent");
					} break;
					case 2:
					{
						new_string=XmStringCreateSimple("Normal");
					} break;
				}
				XtVaSetValues(w,XmNlabelString,new_string,NULL);
				XmStringFree(new_string);
			} break;
			case data_grabber_calib_button_ID:
			{
				data_grabber->calib_button[form_number]=w;
			} break;
			case data_grabber_data_form_ID:
			{
				data_grabber->data_form[form_number]=w;
			} break;
			case data_grabber_data_label_ID:
			{
				data_grabber->data_label[form_number]=w;
				/* change the name */
				new_string=NULL;
				switch (form_number)
				{
					case 0:
					{
						new_string=XmStringCreateSimple("Position");
					} break;
					case 1:
					{
						new_string=XmStringCreateSimple("Tangent");
					} break;
					case 2:
					{
						new_string=XmStringCreateSimple("Normal");
					} break;
				}
				XtVaSetValues(w,XmNlabelString,new_string,NULL);
				XmStringFree(new_string);
			} break;
			default:
			{
				display_message(WARNING_MESSAGE,
					"dg_identify_button.  Invalid button number");
			} break;
		}
	}
	else
	{
		switch (button_num)
		{
			case data_grabber_menu_bar_ID:
			{
				data_grabber->menu_bar=w;
			} break;
			case data_grabber_calib_frame_ID:
			{
				data_grabber->calib_frame=w;
			} break;
			case data_grabber_calib_rowcol_ID:
			{
				data_grabber->calib_rowcol=w;
			} break;
			case data_grabber_data_rowcol_ID:
			{
				data_grabber->data_rowcol=w;
			} break;
			case data_grabber_calib_menu_button_ID:
			{
				data_grabber->calib_menu_button=w;
			} break;
			default:
			{
				display_message(WARNING_MESSAGE,
					"dg_identify_button.  Invalid button number");
			} break;
		}
	}
	LEAVE;
} /* dg_identify_button */

static void dg_add_forms(struct DG_struct *data_grabber,int new_mode)
/*******************************************************************************
LAST MODIFIED : 2 November 1995

DESCRIPTION :
Adds however many forms are required to the calibration and data rowcol's.
Deletes any forms that are now not required, and adds any new forms to the
bottom of the rowcol.
==============================================================================*/
{
	int new_forms,old_forms;
	Arg override_arg;
	MrmType data_grabber_calib_form_class,data_grabber_data_form_class;
	struct Callback_data callback;

	ENTER(dg_add_forms);
	if (data_grabber_hierarchy_open)
	{
#if defined (DEBUG)
		printf("old_mode %i new_mode %i\n",data_grabber->mode,new_mode);
#endif /* defined (DEBUG) */
		/* kill any widgets that are there already */
		new_forms=new_mode&(~data_grabber->mode);
#if defined (DEBUG)
		printf("new forms %i\n",new_forms);
#endif /* defined (DEBUG) */
		old_forms=data_grabber->mode&(~new_mode);
#if defined (DEBUG)
		printf("old forms %i\n",old_forms);
#endif /* defined (DEBUG) */
		if (old_forms&DATA_GRABBER_POSITION)
		{
			XtDestroyWidget(data_grabber->calib_rc_form[0]);
			XtDestroyWidget(data_grabber->data_rc_form[0]);
		}
		if (old_forms&DATA_GRABBER_TANGENT)
		{
			XtDestroyWidget(data_grabber->calib_rc_form[1]);
			XtDestroyWidget(data_grabber->data_rc_form[1]);
		}
		if (old_forms&DATA_GRABBER_NORMAL)
		{
			XtDestroyWidget(data_grabber->calib_rc_form[2]);
			XtDestroyWidget(data_grabber->data_rc_form[2]);
		}
		if (new_forms&DATA_GRABBER_POSITION)
		{
			XtSetArg(override_arg,XmNuserData,0);
			if (MrmSUCCESS==MrmFetchWidgetOverride(data_grabber_hierarchy,
				"dg_calib_form",data_grabber->calib_rowcol,NULL,&override_arg,1,
				&data_grabber->calib_rc_form[0],&data_grabber_calib_form_class))
			{
				XtManageChild(data_grabber->calib_rc_form[0]);
				if (!create_dof3_widget(&data_grabber->calib_widget[0],
					data_grabber->calib_form[0],DOF3_VECTOR,DOF3_ABSOLUTE,
					CONV_VEC_COMPONENT,&data_grabber->calib_value.position))
				{
					display_message(ERROR_MESSAGE,
						"dg_add_forms.  Could not create calib position widget");
				}
				callback.procedure=dg_dof3_update;
				callback.data= &(data_grabber->calib_value.position);
				dof3_set_data(data_grabber->calib_widget[0],
					DOF3_UPDATE_CB,&callback);
			}
			else
			{
				display_message(WARNING_MESSAGE,
					"dg_add_forms.  Could not fetch calib form widget");
			}
			if (MrmSUCCESS==MrmFetchWidgetOverride(data_grabber_hierarchy,
				"dg_data_form",data_grabber->data_rowcol,NULL,&override_arg,1,
				&data_grabber->data_rc_form[0],&data_grabber_data_form_class))
			{
				XtManageChild(data_grabber->data_rc_form[0]);
				if (!create_dof3_widget(&data_grabber->data_widget[0],
					data_grabber->data_form[0],DOF3_POSITION,DOF3_ABSOLUTE,
					CONV_RECTANGULAR_CARTESIAN,
					&data_grabber->current_value.position))
				{
					display_message(ERROR_MESSAGE,
						"dg_add_forms.  Could not create data position widget");
				}
				callback.procedure=dg_dof3_data_position_update;
				callback.data=data_grabber;
				dof3_set_data(data_grabber->data_widget[0],
					DOF3_UPDATE_CB,&callback);
			}
			else
			{
				display_message(WARNING_MESSAGE,
					"dg_add_forms.  Could not fetch calib subform widget");
			}
		}
		if (new_forms&DATA_GRABBER_TANGENT)
		{
			XtSetArg(override_arg,XmNuserData,1);
			if (MrmSUCCESS==MrmFetchWidgetOverride(data_grabber_hierarchy,
				"dg_calib_form",data_grabber->calib_rowcol,NULL,&override_arg,1,
				&data_grabber->calib_rc_form[1],&data_grabber_calib_form_class))
			{
				XtManageChild(data_grabber->calib_rc_form[1]);
				if (!create_dof3_widget(&data_grabber->calib_widget[1],
					data_grabber->calib_form[1],DOF3_VECTOR,DOF3_ABSOLUTE,
					CONV_VEC_COMPONENT,&data_grabber->calib_value.tangent))
				{
					display_message(ERROR_MESSAGE,
						"dg_add_forms.  Could not create calib tangent widget");
				}
				callback.procedure=dg_dof3_update;
				callback.data= &(data_grabber->calib_value.tangent);
				dof3_set_data(data_grabber->calib_widget[1],
					DOF3_UPDATE_CB,&callback);
			}
			else
			{
				display_message(WARNING_MESSAGE,
					"dg_add_forms.  Could not fetch calib form widget");
			}
			if (MrmSUCCESS==MrmFetchWidgetOverride(data_grabber_hierarchy,
				"dg_data_form",data_grabber->data_rowcol,NULL,&override_arg,1,
				&data_grabber->data_rc_form[1],&data_grabber_data_form_class))
			{
				XtManageChild(data_grabber->data_rc_form[1]);
				if (!create_dof3_widget(&data_grabber->data_widget[1],
					data_grabber->data_form[1],DOF3_VECTOR,DOF3_ABSOLUTE,
					CONV_VEC_COMPONENT,&data_grabber->current_value.tangent))
				{
					display_message(ERROR_MESSAGE,
						"dg_add_forms.  Could not create data tangent widget");
				}
				callback.procedure=dg_dof3_data_tangent_update;
				callback.data=data_grabber;
				dof3_set_data(data_grabber->data_widget[1],
					DOF3_UPDATE_CB,&callback);
			}
			else
			{
				display_message(WARNING_MESSAGE,
					"dg_add_forms.  Could not fetch calib subform widget");
			}
		}
		if (new_forms&DATA_GRABBER_NORMAL)
		{
			XtSetArg(override_arg,XmNuserData,2);
			if (MrmSUCCESS==MrmFetchWidgetOverride(data_grabber_hierarchy,
				"dg_calib_form",data_grabber->calib_rowcol,NULL,&override_arg,1,
				&data_grabber->calib_rc_form[2],&data_grabber_calib_form_class))
			{
				XtManageChild(data_grabber->calib_rc_form[2]);
				if (!create_dof3_widget(&data_grabber->calib_widget[2],
					data_grabber->calib_form[2],DOF3_VECTOR,DOF3_ABSOLUTE,
					CONV_VEC_COMPONENT,&data_grabber->calib_value.normal))
				{
					display_message(ERROR_MESSAGE,
						"dg_add_forms.  Could not create calib normal widget");
				}
				callback.procedure=dg_dof3_update;
				callback.data= &(data_grabber->calib_value.normal);
				dof3_set_data(data_grabber->calib_widget[2],
					DOF3_UPDATE_CB,&callback);
			}
			else
			{
				display_message(WARNING_MESSAGE,
					"dg_add_forms.  Could not fetch calib form widget");
			}
			if (MrmSUCCESS==MrmFetchWidgetOverride(data_grabber_hierarchy,
				"dg_data_form",data_grabber->data_rowcol,NULL,&override_arg,1,
				&data_grabber->data_rc_form[2],&data_grabber_data_form_class))
			{
				XtManageChild(data_grabber->data_rc_form[2]);
				if (!create_dof3_widget(&data_grabber->data_widget[2],
					data_grabber->data_form[2],DOF3_VECTOR,DOF3_ABSOLUTE,
					CONV_VEC_COMPONENT,&data_grabber->current_value.normal))
				{
					display_message(ERROR_MESSAGE,
						"dg_add_forms.  Could not create data normal widget");
				}
				callback.procedure=dg_dof3_data_normal_update;
				callback.data=data_grabber;
				dof3_set_data(data_grabber->data_widget[2],
					DOF3_UPDATE_CB,&callback);
			}
			else
			{
				display_message(WARNING_MESSAGE,
					"dg_add_forms.  Could not fetch calib subform widget");
			}
		}
		if (!data_grabber->calib_visible)
		{
			XtUnmanageChild(data_grabber->calib_frame);
		}
		data_grabber->mode=new_mode;
	}
	else
	{
		display_message(WARNING_MESSAGE,"dg_add_forms.  Hierarchy not open");
	}
	LEAVE;
} /* dg_add_forms */

static void dg_add_menu(struct DG_struct *data_grabber)
/*******************************************************************************
LAST MODIFIED : 2 November 1995

DESCRIPTION :
Adds some menu items to the menubar at the top of the widget.
==============================================================================*/
{
	MrmType data_grabber_calib_menu_button_class;
	Widget widget;

	ENTER(dg_add_menu);
	if (data_grabber_hierarchy_open)
	{
		if (MrmSUCCESS==MrmFetchWidget(data_grabber_hierarchy,
			"dg_calib_menu_button",data_grabber->menu_bar,&widget,
			&data_grabber_calib_menu_button_class))
		{
			XtManageChild(widget);
		}
		else
		{
			display_message(WARNING_MESSAGE,
				"dg_add_menu.  Could not fetch calib menu button widget");
		}
	}
	else
	{
		display_message(WARNING_MESSAGE,"dg_add_menu.  Hierarchy not open");
	}
	LEAVE;
} /* dg_add_menu */

static void dg_destroy_CB(Widget w,int *tag,unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 25 September 1995

DESCRIPTION :
Callback for the data_grabber dialog - tidies up all memory allocation
==============================================================================*/
{
	struct DG_struct *data_grabber;

	ENTER(dg_destroy_CB);
	USE_PARAMETER(tag);
	USE_PARAMETER(reason);
	/* Get the pointer to the data for the data_grabber dialog */
	XtVaGetValues(w,XmNuserData,&data_grabber,NULL);
	*(data_grabber->widget_address)=(Widget)NULL;
	/* deallocate the memory for the user data */
	DEALLOCATE(data_grabber);
	LEAVE;
} /* dg_destroy_CB */

static void dg_control_CB(Widget w,int button_num,XmAnyCallbackStruct *reason)
/*******************************************************************************
LAST MODIFIED : 25 September 1995

DESCRIPTION :
Performs actions depending upon the button pressed.
==============================================================================*/
{
	int component;
	struct DG_struct *data_grabber;

	ENTER(dg_identify_button);
	USE_PARAMETER(reason);
	XtVaGetValues(w,XmNuserData,&data_grabber,NULL);
	switch (button_num)
	{
		case data_grabber_calib_button_ID:
		{
			XtVaGetValues(XtParent(w),XmNuserData,&component,NULL);
			dg_bring_up_calib(data_grabber,component);
		} break;
		default:
		{
			display_message(WARNING_MESSAGE,"dg_control_CB.  Invalid button number");
		} break;
	}
	LEAVE;
} /* dg_control_CB */

static void dg_menu_CB(Widget w,int button_num,XmAnyCallbackStruct *reason)
/*******************************************************************************
LAST MODIFIED : 25 September 1995

DESCRIPTION :
Checks which menu button was pressed, then actions it.
==============================================================================*/
{
	int i;
	struct DG_struct *data_grabber;
	XmString new_string;

	ENTER(dg_identify_button);
	USE_PARAMETER(reason);
	XtVaGetValues(w,XmNuserData,&data_grabber,NULL);
	switch (button_num)
	{
		case data_grabber_calib_menu_button_ID:
		{
			/* the user wants to change the visibility of the calb window */
			/* ??? GMH. What about calibration sub windows? */
			if (data_grabber->calib_visible)
			{
				XtUnmanageChild(data_grabber->calib_frame);
				new_string=XmStringCreateSimple("Show calib.");
				XtVaSetValues(data_grabber->calib_menu_button,XmNlabelString,
					new_string,NULL);
				XmStringFree(new_string);
				data_grabber->calib_visible=0;
				for (i=0;i<3;i++)
				{
					if (data_grabber->calib_struct[i].dialog)
					{
						XtDestroyWidget(data_grabber->calib_struct[i].dialog);
					}
				}
			}
			else
			{
				XtManageChild(data_grabber->calib_frame);
				new_string=XmStringCreateSimple("Hide calib.");
				XtVaSetValues(data_grabber->calib_menu_button,XmNlabelString,
					new_string,NULL);
				XmStringFree(new_string);
				data_grabber->calib_visible=1;
			}
		} break;
		default:
		{
			display_message(WARNING_MESSAGE,"dg_menu_CB.  Invalid button number");
		} break;
	}
	LEAVE;
} /* dg_menu_CB */

static void dg_get_calib_data(struct DG_struct *data_grabber)
/*******************************************************************************
LAST MODIFIED : 25 September 1995

DESCRIPTION :
Initialises the calibration values.
==============================================================================*/
{
	int i;

	ENTER(dg_get_calib_data);
	/* this should get the calibration data from a file. */
	for (i=0;i<3;i++)
	{
		data_grabber->calib_value.position.data[i]=0.0;
		data_grabber->calib_value.tangent.data[i]=0.0;
		data_grabber->calib_value.normal.data[i]=0.0;
	}
	data_grabber->calib_value.tangent.data[0]=1.0;
	data_grabber->calib_value.normal.data[0]=1.0;
	LEAVE;
} /* dg_get_calib_data */

static DATA_GRABBER_PRECISION dg_calib_get_error(
	struct DG_struct *data_grabber,int component,
	struct DG_calib_data *new_data)
/*******************************************************************************
LAST MODIFIED : 25 September 1995

DESCRIPTION :
Gets the L2 norm of the distance between the global and the estimate coords.
==============================================================================*/
{
	int i;
	DATA_GRABBER_PRECISION return_value;
	struct Dof3_data temp;

	ENTER(dg_calib_get_error);
	for (i=0;i<3;i++)
	{
		temp.data[i]=data_grabber->calib_struct[component].offset.data[i];
	}
	matrix_postmult_vector(temp.data,&new_data->direction);
	return_value=0.0;
	for (i=0;i<3;i++)
	{
		return_value += (temp.data[i]+new_data->position.data[i]-
			data_grabber->calib_struct[component].global.data[i])*
			(temp.data[i]+new_data->position.data[i]-
			data_grabber->calib_struct[component].global.data[i]);
	}
	return_value=sqrt(return_value);
	LEAVE;

	return return_value;
} /* dg_calib_get_error */

static int dg_calib_sum_positions(struct DG_calib_data *object,void *user_data)
/*******************************************************************************
LAST MODIFIED : 25 September 1995

DESCRIPTION :
Calculates the average position of all the data points according to the current
transformation.
==============================================================================*/
{
	struct DG_calib_sum_position_struct *sum_position=user_data;
	int i,return_code;
	struct Dof3_data temp;

	ENTER(dg_calib_sum_positions);
	return_code=1;
	for (i=0;i<3;i++)
	{
		temp.data[i]=sum_position->data_grabber->
			calib_struct[sum_position->component].offset.data[i];
	}
	matrix_postmult_vector(temp.data,&object->direction);
	for (i=0;i<3;i++)
	{
		temp.data[i] += object->position.data[i];
		sum_position->sum.data[i] += temp.data[i];
	}
	LEAVE;

	return (return_code);
} /* dg_calib_sum_positions */

static int dg_calib_sum_difference(struct DG_calib_data *object,void *user_data)
/*******************************************************************************
LAST MODIFIED : 25 September 1995

DESCRIPTION :
Calculates the average difference between the global position and the data
point positions.
==============================================================================*/
{
	struct DG_calib_sum_position_struct *sum_position=user_data;
	int i,return_code;
	struct Dof3_data temp;

	ENTER(dg_calib_sum_difference);
	return_code=1;
	for (i=0;i<3;i++)
	{
		temp.data[i]=sum_position->data_grabber->
			calib_struct[sum_position->component].global.data[i]-object->
			position.data[i];
	}
	/* this may be in any direction from the data point, now transform it */
	matrix_premult_vector(temp.data,
		&object->direction);
	for (i=0;i<3;i++)
	{
		sum_position->sum.data[i] += temp.data[i];
	}
	LEAVE;

	return (return_code);
} /* dg_calib_sum_difference */

static int dg_calib_display_error(struct DG_calib_data *object,void *user_data)
/*******************************************************************************
LAST MODIFIED : 2 November 1995

DESCRIPTION :
Calculates the average difference between the global position and the data
point positions.
==============================================================================*/
{
	char new_string[DATA_GRABBER_STRING_SIZE];
	int return_code;
	struct DG_calib_sum_position_struct *sum_position=user_data;
	XmString list_item;

	ENTER(dg_calib_display_error);
	return_code=1;
	object->list_number=sum_position->data_grabber->
		calib_struct[sum_position->component].last_number++;
	sprintf(new_string,"Calib point %2i.  Error %6.3"
		DATA_GRABBER_PRECISION_STRING,object->list_number,
		dg_calib_get_error(sum_position->data_grabber,
			sum_position->component,object));
	list_item=XmStringCreateSimple(new_string);
	XmListAddItemUnselected(sum_position->data_grabber->
		calib_struct[sum_position->component].scroll,list_item,0);
	XmStringFree(list_item);
	LEAVE;

	return (return_code);
} /* dg_calib_display_error */

static int dg_calib_add_display(struct DG_calib_data *object,void *user_data)
/*******************************************************************************
LAST MODIFIED : 2 November 1995

DESCRIPTION :
Calculates the average difference between the global position and the data
point positions.
==============================================================================*/
{
	char new_string[DATA_GRABBER_STRING_SIZE];
	int return_code;
	struct DG_calib_sum_position_struct *sum_position=user_data;
	XmString list_item;

	ENTER(dg_calib_add_display);
	return_code=1;
	object->list_number=(sum_position->data_grabber->calib_struct)
		[sum_position->component].last_number++;
	sprintf(new_string,"Calib point %2i.  Error %6.3"
		DATA_GRABBER_PRECISION_STRING,object->list_number,
		dg_calib_get_error(sum_position->data_grabber,
			sum_position->component,object));
	list_item=XmStringCreateSimple(new_string);
	XmListAddItemUnselected((sum_position->data_grabber->calib_struct)
		[sum_position->component].scroll,list_item,0);
	XmStringFree(list_item);
	LEAVE;

	return (return_code);
} /* dg_calib_add_display */

static void dg_calib_calibrate(struct DG_struct *data_grabber,
	int component)
/*******************************************************************************
LAST MODIFIED : 25 September 1995

DESCRIPTION :
Minimises the position of the global coordinates so that the difference between
the estimates and the global posn is minimised.  Must be passed with at least
one data point.
==============================================================================*/
{
	int i,num_items;
	struct DG_calib_sum_position_struct sum_position;

	ENTER(dg_calib_calibrate);
	num_items=NUMBER_IN_LIST(DG_calib_data)(
		data_grabber->calib_struct[component].data_list);
	sum_position.component=component;
	sum_position.data_grabber=data_grabber;
	/* get the average position of the data points */
	for (i=0;i<3;i++)
	{
		sum_position.sum.data[i]=0.0;
	}
	FOR_EACH_OBJECT_IN_LIST(DG_calib_data)(dg_calib_sum_positions,&sum_position,
		data_grabber->calib_struct[component].data_list);
	/* we have the sum, now average */
	for (i=0;i<3;i++)
	{
		data_grabber->calib_struct[component].global.data[i]=
			sum_position.sum.data[i]/num_items;
	}
	/* get the average difference (corrected by transformation) */
	for (i=0;i<3;i++)
	{
		sum_position.sum.data[i]=0.0;
	}
	FOR_EACH_OBJECT_IN_LIST(DG_calib_data)(dg_calib_sum_difference,&sum_position,
		data_grabber->calib_struct[component].data_list);
	/* we have the sum, now average */
	for (i=0;i<3;i++)
	{
		data_grabber->calib_struct[component].offset.data[i]=
			sum_position.sum.data[i]/num_items;
	}
	/* now update the tangent and normal vectors */
	switch (component)
	{
		case 0:
		{
			for (i=0;i<3;i++)
			{
				data_grabber->calib_value.position.data[i]=
					data_grabber->calib_struct[0].offset.data[i];
			}
			dof3_set_data(data_grabber->calib_widget[0],DOF3_DATA,
				&data_grabber->calib_value.position);
		} break;
		case 1:
		{
			for (i=0;i<3;i++)
			{
				data_grabber->calib_value.tangent.data[i]=
					data_grabber->calib_struct[1].offset.data[i]-
					data_grabber->calib_struct[0].offset.data[i];
			}
			/* make it a unit vector */
			matrix_vector_unit(&data_grabber->calib_value.tangent.data[0]);
			dof3_set_data(data_grabber->calib_widget[1],DOF3_DATA,
				&data_grabber->calib_value.tangent);
		} break;
		case 2:
		{
			for (i=0;i<3;i++)
			{
				data_grabber->calib_value.normal.data[i]=
					data_grabber->calib_struct[2].offset.data[i]-
					data_grabber->calib_struct[0].offset.data[i];
			}
			/* make it a unit vector */
			matrix_vector_unit(&data_grabber->calib_value.normal.data[0]);
			dof3_set_data(data_grabber->calib_widget[2],DOF3_DATA,
				&data_grabber->calib_value.normal);
		} break;
	}
	/* Update the view */
	XmListDeleteAllItems(data_grabber->calib_struct[component].scroll);
	data_grabber->calib_struct[component].last_number=0;
	/* go through and display all the errors for the data points */
	FOR_EACH_OBJECT_IN_LIST(DG_calib_data)(dg_calib_display_error,&sum_position,
		data_grabber->calib_struct[component].data_list);
	LEAVE;
#if defined (OLD_CODE)
	/* start off with an average */
	for (i=0;i<3;i++)
	{
		sum.data[i]=0.0;
	}
	current_item=data_grabber->calib_struct[component].data_list;
	num_items=0;
	while (current_item)
	{
		for (i=0;i<3;i++)
		{
			temp.data[i]=data_grabber->calib_struct[component].offset.data[i];
		}
		matrix_postmult_vector(temp.data,&current_item->object->direction);
		for (i=0;i<3;i++)
		{
			temp.data[i] += current_item->object->position.data[i];
			sum.data[i] += temp.data[i];
		}
		current_item=current_item->next;
		num_items++;
	}
	for (i=0;i<3;i++)
	{
		data_grabber->calib_struct[component].global.data[i]=
			sum.data[i]/num_items;
	}
	current_item=data_grabber->calib_struct[component].data_list;
	num_items=0;
		for (i=0;i<3;i++)
		{
			sum.data[i]=0.0;
		}
			while (current_item)
	{
		for (i=0;i<3;i++)
		{
			temp.data[i]=data_grabber->calib_struct[component].global.data[i]-
				current_item->object->position.data[i];
		}
		matrix_premult_vector(temp.data,
			&current_item->object->direction);
		for (i=0;i<3;i++)
		{
			sum.data[i] += temp.data[i];
		}
		current_item=current_item->next;
		num_items++;
	}
	for (i=0;i<3;i++)
	{
		data_grabber->calib_struct[component].offset.data[i]=sum.data[i]/
			num_items;
	}
	/* now update the tangent and normal vectors */
	switch (component)
	{
		case 0:
		{
			for (i=0;i<3;i++)
			{
				data_grabber->calib_value.position.data[i]=
					data_grabber->calib_struct[0].offset.data[i];
			}
			dof3_set_data(data_grabber->calib_widget[0],DOF3_DATA,
				&data_grabber->calib_value.position);
		} break;
		case 1:
		{
			for (i=0;i<3;i++)
			{
				data_grabber->calib_value.tangent.data[i]=
					data_grabber->calib_struct[1].offset.data[i]-
					data_grabber->calib_struct[0].offset.data[i];
			}
			/* make it a unit vector */
			matrix_vector_unit(&data_grabber->calib_value.tangent.data[0]);
			dof3_set_data(data_grabber->calib_widget[1],DOF3_DATA,
				&data_grabber->calib_value.tangent);
		} break;
		case 2:
		{
			for (i=0;i<3;i++)
			{
				data_grabber->calib_value.normal.data[i]=
					data_grabber->calib_struct[2].offset.data[i]-
					data_grabber->calib_struct[0].offset.data[i];
			}
			/* make it a unit vector */
			matrix_vector_unit(&data_grabber->calib_value.normal.data[0]);
			dof3_set_data(data_grabber->calib_widget[2],DOF3_DATA,
				&data_grabber->calib_value.normal);
		} break;
	}
	/* Update the view */
	XmListDeleteAllItems(data_grabber->calib_struct[component].scroll);
	data_grabber->calib_struct[component].last_number=0;
	current_item=data_grabber->calib_struct[component].data_list;
	while (current_item)
	{
		current_item->object->list_number=
			data_grabber->calib_struct[component].last_number++;
		sprintf(new_string,"Calib point %2i.  Error %6.3"
			DATA_GRABBER_PRECISION_STRING,current_item->object->list_number,
			dg_calib_get_error(data_grabber,component,current_item->object));
		list_item=XmStringCreateSimple(new_string);
		XmListAddItemUnselected(data_grabber->calib_struct[component].scroll,
			list_item,0);
		XmStringFree(list_item);
		current_item=current_item->next;
	}
#endif
} /* dg_calib_calibrate */

static void dg_calib_identify_button(Widget w,int button_num,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 25 September 1995

DESCRIPTION :
Finds the id of the buttons on the data_grabber_calib widget.
==============================================================================*/
{
	int component;
	struct DG_struct *data_grabber;

	ENTER(dg_calib_identify_button);
	USE_PARAMETER(reason);
	XtVaGetValues(w,XmNuserData,&data_grabber,NULL);
	if (button_num==data_grabber_calib_control_ID)
	{
		XtVaGetValues(XtParent(w),XmNuserData,&component,NULL);
		switch (button_num)
		{
			case data_grabber_calib_control_ID:
			{
				data_grabber->calib_struct[component].control=w;
			} break;
			default:
			{
				display_message(WARNING_MESSAGE,
					"dg_calib_identify_button.  Invalid button number");
			} break;
		}
	}
	else
	{
		XtVaGetValues(XtParent(XtParent(w)),XmNuserData,&component,NULL);
		switch (button_num)
		{
			case data_grabber_calib_scroll_ID:
			{
				data_grabber->calib_struct[component].scroll=w;
			} break;
			case data_grabber_calib_cont_record_ID:
			{
				data_grabber->calib_struct[component].control_record=w;
			} break;
			case data_grabber_calib_cont_clear_ID:
			{
				data_grabber->calib_struct[component].control_clear=w;
			} break;
			case data_grabber_calib_cont_solve_ID:
			{
				data_grabber->calib_struct[component].control_solve=w;
			} break;
			case data_grabber_calib_cont_delete_ID:
			{
				data_grabber->calib_struct[component].control_delete=w;
			} break;
			default:
			{
				display_message(WARNING_MESSAGE,
					"dg_calib_identify_button.  Invalid button number");
			} break;
		}
	}
	LEAVE;
} /* dg_identify_button */

static void dg_calib_control_CB(Widget w,int button_num,
	XmAnyCallbackStruct *reason)
/*******************************************************************************
LAST MODIFIED : 18 February 1997

DESCRIPTION :
Performs actions depending upon the button pressed.
==============================================================================*/
{
	char new_string[DATA_GRABBER_STRING_SIZE];
	int i,component,*select_list,num_selected;
	struct DG_struct *data_grabber;
	struct DG_calib_data *new_data;
	struct DG_calib_sum_position_struct sum_position;
	XmString list_item;

	ENTER(dg_calib_identify_button);
	USE_PARAMETER(reason);
	XtVaGetValues(w,XmNuserData,&data_grabber,NULL);
	XtVaGetValues(XtParent(XtParent(w)),XmNuserData,&component,NULL);
	switch (button_num)
	{
		case data_grabber_calib_cont_record_ID:
		{
			if (ALLOCATE(new_data,struct DG_calib_data,1))
			{
				new_data->list_number=
					data_grabber->calib_struct[component].last_number++;
				for (i=0;i<3;i++)
				{
					new_data->position.data[i]=data_grabber->last_position.data[i];
				}
				matrix_copy(&new_data->direction,&data_grabber->last_direction);
				ADD_OBJECT_TO_LIST(DG_calib_data)(new_data,
					data_grabber->calib_struct[component].data_list);
				sprintf(new_string,"Calib point %2i.  Error %6.3"
					DATA_GRABBER_PRECISION_STRING,new_data->list_number,
					dg_calib_get_error(data_grabber,component,new_data));
				list_item=XmStringCreateSimple(new_string);
				XmListAddItemUnselected(data_grabber->calib_struct[component].
					scroll,list_item,0);
				XmStringFree(list_item);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"dg_calib_control_CB.  Could not allocate memory");
			}
		} break;
		case data_grabber_calib_cont_solve_ID:
		{
			if (0<NUMBER_IN_LIST(DG_calib_data)(
				data_grabber->calib_struct[component].data_list))
			{
				dg_calib_calibrate(data_grabber,component);
			}
			else
			{
				display_message(WARNING_MESSAGE,
			"dg_calib_control_CB.  Must have at least one data point before solving");
			}
		} break;
		case data_grabber_calib_cont_delete_ID:
		{
			if (XmListGetSelectedPos(
				data_grabber->calib_struct[component].scroll,&select_list,
				&num_selected))
			{
				for (i=0;i<num_selected;i++)
				{
					if (new_data=FIND_BY_IDENTIFIER_IN_LIST(
						DG_calib_data,list_number)(select_list[i]-1,
						data_grabber->calib_struct[component].data_list))
					{
						REMOVE_OBJECT_FROM_LIST(DG_calib_data)(new_data,
							data_grabber->calib_struct[component].data_list);
					}
					else
					{
						display_message(WARNING_MESSAGE,
							"dg_calib_control_CB.  Could not find calibration point");
					}
				}
				XmListDeleteAllItems(data_grabber->calib_struct[component].scroll);
				data_grabber->calib_struct[component].last_number=0;
				sum_position.component=component;
				sum_position.data_grabber=data_grabber;
				FOR_EACH_OBJECT_IN_LIST(DG_calib_data)(dg_calib_add_display,
					&sum_position,data_grabber->calib_struct[component].data_list);
#if defined (OLD_CODE)
				current_item=data_grabber->calib_struct[component].data_list;
				while (current_item)
				{
					current_item->object->list_number=
						data_grabber->calib_struct[component].last_number++;
					sprintf(new_string,"Calib point %2i.  Error %6.3"
						DATA_GRABBER_PRECISION_STRING,current_item->object->list_number,
						dg_calib_get_error(data_grabber,component,
						current_item->object));
					list_item=XmStringCreateSimple(new_string);
					XmListAddItemUnselected(
						data_grabber->calib_struct[component].scroll,list_item,0);
					XmStringFree(list_item);
					current_item=current_item->next;
				}
#endif
			}
		} break;
		case data_grabber_calib_cont_clear_ID:
		{
			for (i=0;i<3;i++)
			{
				data_grabber->calib_struct[component].offset.data[i]=0.0;
			}
			REMOVE_ALL_OBJECTS_FROM_LIST(DG_calib_data)(
				data_grabber->calib_struct[component].data_list);
			XmListDeleteAllItems(data_grabber->calib_struct[component].scroll);
			data_grabber->calib_struct[component].last_number=0;
		} break;
		default:
		{
			display_message(WARNING_MESSAGE,
				"dg_calib_control_CB.  Invalid button number");
		} break;
	}
	LEAVE;
} /* dg_calib_control_CB */

static void dg_calib_destroy_CB(Widget w,int *tag,unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 25 September 1995

DESCRIPTION :
Callback for the data_grabber dialog - tidies up all memory allocation
==============================================================================*/
{
	int component,num_children;
	struct DG_struct *data_grabber;
	Widget *child_list;

	ENTER(dg_calib_destroy_CB);
	USE_PARAMETER(tag);
	USE_PARAMETER(reason);
	/* Get the pointer to the data for the data_grabber dialog */
	XtVaGetValues(w,
		XmNuserData,&component,
		XmNnumChildren,&num_children,
		XmNchildren,&child_list,
		NULL);
	/* get a child */
	if (2==num_children)
	{
		XtVaGetValues(child_list[0],XmNuserData,&data_grabber,NULL);
		data_grabber->calib_struct[component].dialog=(Widget)NULL;
		DESTROY_LIST(DG_calib_data)(
			&data_grabber->calib_struct[component].data_list);
		data_grabber->calib_struct[component].last_number=0;
	}
	else
	{
		display_message(WARNING_MESSAGE,"dg_calib_destroy_CB.  Invalid widget");
	}
	LEAVE;
} /* dg_calib_destroy_CB */

/*
Global Functions
---------------
*/
Widget create_data_grabber_widget(Widget *data_grabber_widget,Widget parent,
	int mode)
/*******************************************************************************
LAST MODIFIED : 20 March 2000

DESCRIPTION :
Creates a data_grabber widget that will allow the user to choose an object based
upon its name.
==============================================================================*/
{
	char temp_string[50];
	int i,init_widgets,n;
	MrmType data_grabber_dialog_class;
	struct DG_struct *data_grabber=NULL;
	static MrmRegisterArg callback_list[]=
	{
		{"dg_identify_button",(XtPointer)dg_identify_button},
		{"dg_destroy_CB",(XtPointer)dg_destroy_CB},
		{"dg_control_CB",(XtPointer)dg_control_CB},
		{"dg_menu_CB",(XtPointer)dg_menu_CB},
		{"dg_calib_identify_button",(XtPointer)dg_calib_identify_button},
		{"dg_calib_destroy_CB",(XtPointer)dg_calib_destroy_CB},
		{"dg_calib_control_CB",(XtPointer)dg_calib_control_CB},
		{"dg_stream_mode_CB",(XtPointer)dg_stream_mode_CB},
		{"dg_stream_distance_CB",(XtPointer)dg_stream_distance_CB},
		{"dg_id_stream_distance_entry",(XtPointer)
			DIALOG_IDENTIFY(data_grabber, stream_distance_entry)},
		{"dg_id_stream_distance_text",(XtPointer)
			DIALOG_IDENTIFY(data_grabber, stream_distance_text)},
	};
	static MrmRegisterArg identifier_list[]=
	{
		{"dg_structure",(XtPointer)NULL},
		{"dg_menu_bar_ID",(XtPointer)data_grabber_menu_bar_ID},
		{"dg_calib_frame_ID",(XtPointer)data_grabber_calib_frame_ID},
		{"dg_calib_rowcol_ID",(XtPointer)data_grabber_calib_rowcol_ID},
		{"dg_calib_form_ID",(XtPointer)data_grabber_calib_form_ID},
		{"dg_calib_button_ID",(XtPointer)data_grabber_calib_button_ID},
		{"dg_calib_label_ID",(XtPointer)data_grabber_calib_label_ID},
		{"dg_data_rowcol_ID",(XtPointer)data_grabber_data_rowcol_ID},
		{"dg_data_form_ID",(XtPointer)data_grabber_data_form_ID},
		{"dg_data_label_ID",(XtPointer)data_grabber_data_label_ID},
		{"dg_calib_menu_button_ID",(XtPointer)data_grabber_calib_menu_button_ID},
		{"dg_calib_control_ID",(XtPointer)data_grabber_calib_control_ID},
		{"dg_calib_scroll_ID",(XtPointer)data_grabber_calib_scroll_ID},
		{"dg_calib_cont_record_ID",(XtPointer)data_grabber_calib_cont_record_ID},
		{"dg_calib_cont_solve_ID",(XtPointer)data_grabber_calib_cont_solve_ID},
		{"dg_calib_cont_delete_ID",(XtPointer)data_grabber_calib_cont_delete_ID},
		{"dg_calib_cont_clear_ID",(XtPointer)data_grabber_calib_cont_clear_ID},
	};
	Widget return_widget;

	ENTER(create_data_grabber_widget);
	return_widget=(Widget)NULL;
	if (MrmOpenHierarchy_base64_string(data_grabber_uidh,
		&data_grabber_hierarchy,&data_grabber_hierarchy_open))
	{
		/* allocate memory */
		if (ALLOCATE(data_grabber,struct DG_struct,1))
		{
			/* initialise the structure */
			data_grabber->widget_parent=parent;
			data_grabber->widget=(Widget)NULL;
			data_grabber->widget_address=data_grabber_widget;
			data_grabber->calib_visible=0;
			data_grabber->mode=0;
			data_grabber->button_state=0;
			data_grabber->streaming_distance=1.0;
			data_grabber->streaming_mode=0;
			/* initialisation */
			for (i=0;i<3;i++)
			{
				data_grabber->current_value.position.data[i]=0.0;
				data_grabber->current_value.tangent.data[i]=0.0;
				data_grabber->current_value.normal.data[i]=0.0;
				data_grabber->last_position.data[i]=0.0;
				data_grabber->previous_streaming.data[i]=0.0;
			}
			matrix_I(&data_grabber->last_direction);
			dg_get_calib_data(data_grabber);
			data_grabber->menu_bar=(Widget)NULL;
			data_grabber->input_module_widget=(Widget)NULL;
			data_grabber->calib_frame=(Widget)NULL;
			data_grabber->calib_rowcol=(Widget)NULL;
			data_grabber->data_rowcol=(Widget)NULL;
			for (i=0;i<3;i++)
			{
				data_grabber->calib_rc_form[i]=(Widget)NULL;
				data_grabber->calib_widget[i]=(Widget)NULL;
				data_grabber->calib_form[i]=(Widget)NULL;
				data_grabber->calib_button[i]=(Widget)NULL;
				data_grabber->calib_label[i]=(Widget)NULL;
				data_grabber->data_rc_form[i]=(Widget)NULL;
				data_grabber->data_widget[i]=(Widget)NULL;
				data_grabber->data_form[i]=(Widget)NULL;
				data_grabber->data_label[i]=(Widget)NULL;
				data_grabber->calib_struct[i].dialog=(Widget)NULL;
				data_grabber->calib_struct[i].widget=(Widget)NULL;
				data_grabber->calib_struct[i].control=(Widget)NULL;
				data_grabber->calib_struct[i].scroll=(Widget)NULL;
				data_grabber->calib_struct[i].control_record=(Widget)NULL;
				data_grabber->calib_struct[i].control_solve=(Widget)NULL;
				data_grabber->calib_struct[i].control_delete=(Widget)NULL;
				data_grabber->calib_struct[i].control_clear=(Widget)NULL;
				data_grabber->calib_struct[i].last_number=0;
				data_grabber->calib_struct[i].data_list=
					CREATE_LIST(DG_calib_data)();
				for (n=0;n<3;n++)
				{
					data_grabber->calib_struct[i].global.data[n]=0.0;
					data_grabber->calib_struct[i].offset.data[n]=0.0;
				}
			}
			data_grabber->calib_menu_button=(Widget)NULL;
			for (i=0;i<DATA_GRABBER_NUM_CALLBACKS;i++)
			{
				data_grabber->callback_array[i].procedure=
					(Callback_procedure *)NULL;
				data_grabber->callback_array[i].data=NULL;
			}
			/* register the callbacks */
			if (MrmSUCCESS==MrmRegisterNamesInHierarchy(data_grabber_hierarchy,
				callback_list,XtNumber(callback_list)))
			{
				/* assign and register the identifiers */
				identifier_list[0].value=(XtPointer)data_grabber;
				if (MrmSUCCESS==MrmRegisterNamesInHierarchy(data_grabber_hierarchy,
					identifier_list,XtNumber(identifier_list)))
				{
					/* fetch data_grabber control widget */
					if (MrmSUCCESS==MrmFetchWidget(data_grabber_hierarchy,
						"dg_widget",data_grabber->widget_parent,
						&(data_grabber->widget),&data_grabber_dialog_class))
					{
						dg_add_forms(data_grabber,mode);
						init_widgets=1;
#if defined (EXT_INPUT)
						/* add the input menu */
						if (!create_input_module_widget(
							&(data_grabber->input_module_widget),
							data_grabber->menu_bar))
						{
							display_message(ERROR_MESSAGE,
						"create_data_grabber_widget.  Could not create input menu widget");
							init_widgets=0;
						}
#endif /* defined (EXT_INPUT) */
						if (init_widgets)
						{
#if defined (EXT_INPUT)
							Input_module_add_device_change_callback(
								data_grabber->input_module_widget,
								dg_change_device,(void *)data_grabber);
#endif /* defined (EXT_INPUT) */
							/* all of these use the same procedure, different data */
							XtManageChild(data_grabber->widget);
							dg_add_menu(data_grabber);
							XtUnmanageChild(data_grabber->calib_frame);
							/* Set the text in the stream_distance text */
							if (data_grabber->stream_distance_text)
							{
								sprintf(temp_string,"%g",data_grabber->streaming_distance);
								XtVaSetValues(data_grabber->stream_distance_text,
									XmNvalue,temp_string,
									NULL);
							}
							if (data_grabber->stream_distance_entry)
							{
								XtSetSensitive(data_grabber->stream_distance_entry,
									False );
							}
							
							return_widget=data_grabber->widget;
						}
						else
						{
							DEALLOCATE(data_grabber);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
						"create_data_grabber_widget.  Could not fetch data_grabber widget");
						free(data_grabber);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_data_grabber_widget.  Could not register identifiers");
					free(data_grabber);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_data_grabber_widget.  Could not register callbacks");
				free(data_grabber);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
		"create_data_grabber_widget.  Could not allocate control window structure");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_data_grabber_widget.  Could not open hierarchy");
	}
	*data_grabber_widget=return_widget;
	LEAVE;

	return (return_widget);
} /* create_data_grabber_widget */

int data_grabber_set_data(Widget data_grabber_widget,
	enum DG_data_type data_type,void *data)
/*******************************************************************************
LAST MODIFIED : 25 September 1995

DESCRIPTION :
Changes a data item of the input widget.
==============================================================================*/
{
	int i,return_code;
	struct DG_struct *data_grabber;

	ENTER(data_grabber_set_data);
	/* Get the pointer to the data for the data_grabber dialog */
	XtVaGetValues(data_grabber_widget,XmNuserData,&data_grabber,NULL);
	switch (data_type)
	{
		case DATA_GRABBER_UPDATE_CB:
		{
			data_grabber->callback_array[DATA_GRABBER_UPDATE_CB].procedure=
				((struct Callback_data *)data)->procedure;
			data_grabber->callback_array[DATA_GRABBER_UPDATE_CB].data=
				((struct Callback_data *)data)->data;
			return_code=1;
		} break;
		case DATA_GRABBER_SELECT_CB:
		{
			data_grabber->callback_array[DATA_GRABBER_SELECT_CB].procedure=
				((struct Callback_data *)data)->procedure;
			data_grabber->callback_array[DATA_GRABBER_SELECT_CB].data=
				((struct Callback_data *)data)->data;
			return_code=1;
		} break;
		case DATA_GRABBER_DATA:
		{
			for (i=0;i<3;i++)
			{
				data_grabber->current_value.position.data[i]=
					((struct DG_data *)data)->position.data[i];
				data_grabber->current_value.tangent.data[i]=
					((struct DG_data *)data)->tangent.data[i];
				data_grabber->current_value.normal.data[i]=
					((struct DG_data *)data)->normal.data[i];
			}
			if (data_grabber->mode&DATA_GRABBER_POSITION)
			{
				dof3_set_data(data_grabber->data_widget[0],DOF3_DATA,
					&data_grabber->current_value.position);
			}
			if (data_grabber->mode&DATA_GRABBER_TANGENT)
			{
				dof3_set_data(data_grabber->data_widget[1],DOF3_DATA,
					&data_grabber->current_value.tangent);
			}
			if (data_grabber->mode&DATA_GRABBER_NORMAL)
			{
				dof3_set_data(data_grabber->data_widget[2],DOF3_DATA,
					&data_grabber->current_value.normal);
			}
			return_code=1;
		} break;
		case DATA_GRABBER_MODE:
		{
			dg_add_forms(data_grabber,*((int *)data));
			return_code=1;
		} break;
		default:
		{
			return_code=0;
		} break;
	}
	LEAVE;
	return (return_code);
} /* data_grabber_set_data */

void *data_grabber_get_data(Widget data_grabber_widget,
	enum DG_data_type data_type)
/*******************************************************************************
LAST MODIFIED : 25 September 1995

DESCRIPTION :
Returns a pointer to a data item of the input widget.
==============================================================================*/
{
	int i;
	void *return_code;
	static struct Callback_data dat_callback;
	static int dat_mode;
	struct DG_struct *data_grabber;
	static struct DG_data dat_data;

	ENTER(data_grabber_get_data);
	/* Get the pointer to the data for the data_grabber dialog */
	XtVaGetValues(data_grabber_widget,XmNuserData,&data_grabber,NULL);
	switch (data_type)
	{
		case DATA_GRABBER_UPDATE_CB:
		{
			dat_callback.procedure=
				(data_grabber->callback_array)[DATA_GRABBER_UPDATE_CB].procedure;
			dat_callback.data=
				(data_grabber->callback_array)[DATA_GRABBER_UPDATE_CB].data;
			return_code= &dat_callback;
		} break;
		case DATA_GRABBER_SELECT_CB:
		{
			dat_callback.procedure=
				data_grabber->callback_array[DATA_GRABBER_SELECT_CB].procedure;
			dat_callback.data=
				(data_grabber->callback_array)[DATA_GRABBER_SELECT_CB].data;
			return_code= &dat_callback;
		} break;
		case DATA_GRABBER_DATA:
		{
			for (i=0;i<3;i++)
			{
				dat_data.position.data[i]=
					data_grabber->current_value.position.data[i];
				dat_data.tangent.data[i]=
					data_grabber->current_value.tangent.data[i];
				dat_data.normal.data[i]=
					data_grabber->current_value.normal.data[i];
			}
			return_code= &dat_data;
		} break;
		case DATA_GRABBER_MODE:
		{
			dat_mode=data_grabber->mode;
			return_code= &dat_mode;
		} break;
		default:
		{
			return_code=NULL;
		} break;
	}
	LEAVE;
	return (return_code);
} /* data_grabber_get_data */
