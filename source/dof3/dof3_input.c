/*******************************************************************************
FILE : dof3_input.c

LAST MODIFIED : 9 April 1997

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
and input to the whole set is controlled by the input widget.  Note that in this
case, the <bounding> widget passed to create_input_widget would encompass all of
the widgets.  This is to allow the input_module to pass the correct events to
this 'structure'.

Links are created between the widgets by using the set_data routines.  The
dominant widget (ie the one highest above) should be told who its subwidgets
are.
ie input_set data would be called with
	INPUT_POSITION_WIDGET      DOF31
	INPUT_DIRECTION_WIDGET      DOF32
	INPUT_POSCTRL_WIDGET        CONTROL1
	INPUT_DIRCTRL_WIDGET        CONTROL2
input_set_data will ensure that the control and dof3 widgets are informed as
to who is providing their input.

When any of the widgets are destroyed, they ensure that all links with other
widgets are terminated, and so the whole structure does not need to be destroyed
- widgets may be mixed and matched as the situation changes.

???DB.  All the EXT_INPUT #if defined's should be moved inside this module.
==============================================================================*/
#include <math.h>
#include <stdio.h>
#include <Xm/ToggleB.h>
#include <Xm/ToggleBG.h>
#include "dof3/dof3_input.h"
#include "dof3/dof3_input.uid64"
#include "dof3/dof3_control.h"
#include "dof3/dof3.h"
#include "general/debug.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module variables
----------------
*/
static int dof3_input_hierarchy_open=0;
static MrmHierarchy dof3_input_hierarchy;

/*
Module functions
----------------
*/
#if defined (EXT_INPUT)
static int input_input_module_CB(void *identifier,Input_module_message message)
/*******************************************************************************
LAST MODIFIED : 7 September 1994

DESCRIPTION :
Finds the id of the buttons on the inputment dialog box.
==============================================================================*/
{
	int return_code;
	struct Input_struct *temp_input=identifier;

	ENTER(input_input_module_CB);
	switch (message->type)
	{
		case IM_TYPE_MOTION:
		{
			if (temp_input->position)
			{ /* pass the message to the dof3 widget */
				dof3_input_module_CB(temp_input->position,message);
			}
			if (temp_input->direction)
			{ /* pass the message to the dof3 widget */
				dof3_input_module_CB(temp_input->direction,message);
			}
		} break;
		/* These are here so we can interpret any external input -
			ie spaceball buttons to link resolution etc */
		case IM_TYPE_BUTTON_PRESS:
		{
		} break;
		case IM_TYPE_BUTTON_RELEASE:
		{
			if (temp_input->posctrl)
			{
				control_input_module_CB(temp_input->posctrl,message);
			}
			if (temp_input->dirctrl)
			{
				control_input_module_CB(temp_input->dirctrl,message);
			}
		} break;
		default:
		{
			display_message(WARNING_MESSAGE,
				"input_input_module_CB.  Invalid message type.");
		} break;
	}
	return_code=1;
	LEAVE;

	return (return_code);
} /* input_input_module_CB */

static void input_identify_button(Widget w,int button_type,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 18 April 1994

DESCRIPTION :
Finds the id of the buttons on the input widget.
==============================================================================*/
{
	int button_num;
	struct Input_struct *temp_input;

	ENTER(input_identify_button);
	USE_PARAMETER(reason);
	/* find out which input widget we are in */
	XtVaGetValues(XtParent(w),XmNuserData,&temp_input,NULL);
	XtVaGetValues(w,XmNuserData,&button_num,NULL);
	switch (button_type)
	{
		case input_device_button_ID:
		{
#if defined (EXT_INPUT)
			temp_input->input[button_num]=w;
#endif
		} break;
		case input_polhemus_button_ID:
		{
			temp_input->polhemus_control[button_num]=w;
		} break;
		default:
		{
		display_message(WARNING_MESSAGE,
			"input_identify_button.  Invalid button number");
		} break;
	}
	LEAVE;
} /* input_identify_button */

static void input_destroy_CB(Widget w,XtPointer user_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 9 April 1997

DESCRIPTION :
Callback for the input widget - tidies up all details - mem etc
==============================================================================*/
{
	int i;
	struct Input_struct *temp_input;

	ENTER(input_destroy_CB);
	USE_PARAMETER(user_data);
	USE_PARAMETER(call_data);
	/* Get the pointer to the data for the input widget */
	XtVaGetValues(w,XmNuserData,&temp_input,NULL);
/*???debug */
printf("input_destroy_CB %p %p %p %p\n",w,temp_input,
	temp_input->position_widget,temp_input->direction_widget);
#if defined (EXT_INPUT)
	/* Deregister the input module callbacks */
	for (i=0;i<INPUT_MODULE_NUM_DEVICES;i++)
	{
		if (temp_input->input_device[i])
		{
			input_module_deregister((enum Input_module_device)i,temp_input);
		}
	}
#endif
	if (temp_input->position_widget)
	{
		dof3_set_data(temp_input->position_widget,DOF3_INPUT_WIDGET,NULL);
	}
	if (temp_input->direction_widget)
	{
		dof3_set_data(temp_input->direction_widget,DOF3_INPUT_WIDGET,NULL);
	}
	if (temp_input->posctrl_widget)
	{
		control_set_data(temp_input->posctrl_widget,CONTROL_INPUT_WIDGET,NULL);
	}
	if (temp_input->dirctrl_widget)
	{
		control_set_data(temp_input->dirctrl_widget,CONTROL_INPUT_WIDGET,NULL);
	}
	/* deallocate the memory for the user data */
	printf("De-allocating memory.\n");
	DEALLOCATE(temp_input);
/*??? this will have to tie in with a controller for any external input */
	LEAVE;
} /* input_destroy_CB */

static void input_device_CB(Widget w, int *tag,
	XmToggleButtonCallbackStruct *reason)
/*******************************************************************************
LAST MODIFIED : 9 April 1997

DESCRIPTION :
Callback for the input widget.  Processes which device has been selected or
unselected.
==============================================================================*/
{
	int button_num;
	struct Input_struct *temp_input;

	ENTER(input_device_CB);
	USE_PARAMETER(tag);
	/* Get the pointer to the data for the input widget */
	XtVaGetValues(XtParent(w),XmNuserData,&temp_input,NULL);
	XtVaGetValues(w,XmNuserData,&button_num,NULL);
#if defined (EXT_INPUT)
	if ((button_num>=0)&&(button_num<INPUT_MODULE_NUM_DEVICES))
	{
		temp_input->input_device[button_num]=reason->set;
		if (temp_input->input_device[button_num])
		{
			input_module_register((enum Input_module_device)button_num,temp_input,
				temp_input->bounding,(Input_module_callback_proc)input_input_module_CB);
		}
		else
		{
			input_module_deregister((enum Input_module_device)button_num,temp_input);
		}
	}
#endif
	LEAVE;
} /* input_device_CB */

static void input_polhemus_CB(Widget w, int *tag,
	XmToggleButtonCallbackStruct *reason)
/*******************************************************************************
LAST MODIFIED : 2 November 1995

DESCRIPTION :
Callback for the input widget.  Processes which device has been selected or
unselected.
==============================================================================*/
{
	int button_num;
	struct Input_struct *temp_input;

	ENTER(input_polhemus_CB);
	USE_PARAMETER(tag);
	USE_PARAMETER(reason);
	/* Get the pointer to the data for the input widget */
	XtVaGetValues(XtParent(w),XmNuserData,&temp_input,NULL);
	XtVaGetValues(w,XmNuserData,&button_num,NULL);
#if defined (EXT_INPUT)
#if defined (POLHEMUS)
	if (temp_input->position)
	{
		if (button_num==0)
		{
			input_module_polhemus_reset_pos(
				&(temp_input->position->ext_origin.position.data[0]));
		}
		else
		{
			input_module_polhemus_revert_pos(
				&(temp_input->position->ext_origin.position.data[0]));
		}
	}
	if (temp_input->direction)
	{
#if defined (CODE_FRAGMENTS)
		if (button_num==0)
		{
			input_module_polhemus_reset_dir(
				&(temp_input->direction->ext_origin.last));
		}
		else
		{
			input_module_polhemus_revert_dir(
				&(temp_input->direction->ext_origin.last));
		}
		/* now reset the actual displayed value */
		matrix_copy(&(temp_input->direction->ext_origin.origin),
			&(temp_input->direction->ext_origin.last));
#endif
		if (button_num==0)
		{
			input_module_polhemus_reset_dir(
				&(temp_input->direction->ext_origin.last));
			matrix_copy(&(temp_input->direction->ext_origin.origin),
				&(temp_input->direction->ext_origin.last));
		}
		else
		{
			input_module_polhemus_revert_dir(
				&(temp_input->direction->ext_origin.origin));
		}
		/* does it have a position sibling to update? */
		if (temp_input->direction->position)
		{
			matrix_copy(&temp_input->direction->position->ext_origin.origin,
				&temp_input->direction->ext_origin.origin);
		}
		/* dont actually update the value, as ext_origin.last may not actually
			reflect current_values. */
	}
#endif
#endif
	LEAVE;
} /* input_polhemus_CB */
#endif


/*
Global Functions
----------------
*/
Widget create_input_widget(Widget parent,Widget bounding)
/*******************************************************************************
LAST MODIFIED : 13 December 1996

DESCRIPTION :
Creates a widget that will handle the redirection of external input to dof3
widgets.  <position/direction> may be NULL.  <bounding> is the widget that
encloses all dof3 and input and control widgets that are to be regarded as one
item.
==============================================================================*/
{
	Widget return_widget;
#if defined (EXT_INPUT)
	int n;
	MrmType input_dialog_class;
	struct Input_struct *temp_input=NULL;
	static MrmRegisterArg callback_list[]=
	{
		{"input_identify_button",(XtPointer)input_identify_button},
		{"input_destroy_CB",(XtPointer)input_destroy_CB},
		{"input_device_CB",(XtPointer)input_device_CB},
		{"input_polhemus_CB",(XtPointer)input_polhemus_CB}
	};
	static MrmRegisterArg identifier_list[]=
	{
		{"Input_structure",(XtPointer)NULL},
		{"input_device_button_ID",(XtPointer)input_device_button_ID},
		{"input_polhemus_button_ID",(XtPointer)input_polhemus_button_ID}
	};
#endif

	ENTER(create_input_widget);
#if defined (EXT_INPUT)
	return_widget=(Widget)NULL;
	if (MrmOpenHierarchy_base64_string(dof3_input_uid64,
		&dof3_input_hierarchy,&dof3_input_hierarchy_open))
	{
		/* allocate memory */
		if (ALLOCATE(temp_input,struct Input_struct,1))
		{
			/* initialise the structure */
			temp_input->widget_parent=parent;
			temp_input->widget=(Widget)NULL;
			temp_input->bounding=bounding;
			temp_input->position=(struct Dof3_struct *)NULL;
			temp_input->direction=(struct Dof3_struct *)NULL;
			temp_input->posctrl=(struct Control_struct *)NULL;
			temp_input->dirctrl=(struct Control_struct *)NULL;
			temp_input->position_widget=(Widget)NULL;
			temp_input->direction_widget=(Widget)NULL;
			temp_input->posctrl_widget=(Widget)NULL;
			temp_input->dirctrl_widget=(Widget)NULL;
#if defined (EXT_INPUT)
			for (n=0;n<INPUT_MODULE_NUM_DEVICES;n++)
			{
				temp_input->input[n]=(Widget)NULL;
				temp_input->input_device[n]=0;
			}
#endif
			for (n=0;n<2;n++)
			{
				temp_input->polhemus_control[n]=(Widget)NULL;
			}
			/* register the callbacks */
			if (MrmSUCCESS==MrmRegisterNamesInHierarchy(dof3_input_hierarchy,
				callback_list,XtNumber(callback_list)))
			{
				/* assign and register the identifiers */
				identifier_list[0].value=(XtPointer)temp_input;
				if (MrmSUCCESS==MrmRegisterNamesInHierarchy(dof3_input_hierarchy,
					identifier_list,XtNumber(identifier_list)))
				{
					/* fetch input window widget */
					if (MrmSUCCESS==MrmFetchWidget(dof3_input_hierarchy,"input_widget",
						temp_input->widget_parent,&(temp_input->widget),
						&input_dialog_class))
					{
/*???debug */
printf("create_input_widget %p %p %p\n",temp_input,temp_input->widget,
	temp_input->position_widget);
						XtManageChild(temp_input->widget);
						return_widget=temp_input->widget;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_input_widget.  Could not fetch input dialog");
						DEALLOCATE(temp_input);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_input_widget.  Could not register identifiers");
					DEALLOCATE(temp_input);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_input_widget.  Could not register callbacks");
				DEALLOCATE(temp_input);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_input_widget.  Could not allocate input widget structure");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_input_widget.  Could not open hierarchy");
	}
#else
	return_widget=(Widget)NULL;
#endif
	LEAVE;

	return (return_widget);
} /* create_input_widget */

int input_set_data(Widget input_widget,enum Input_data_type data_type,
	void *data)
/*******************************************************************************
LAST MODIFIED : 9 April 1997

DESCRIPTION :
Changes a data item of the input widget.
==============================================================================*/
{
	int return_code;
#if defined (EXT_INPUT)
	struct Input_struct *temp_input;
#endif

	ENTER(input_set_data);
#if defined (EXT_INPUT)
	/* Get the pointer to the data for the input dialog */
	XtVaGetValues(input_widget,XmNuserData,&temp_input,NULL);
	switch (data_type)
	{
		case INPUT_POSITION_WIDGET:
		{
			temp_input->position_widget=(Widget)data;
/*???debug */
printf("input_set_data %p %p\n",temp_input,temp_input->position_widget);
			if (data)
			{
				XtVaGetValues((Widget)data,XmNuserData,&(temp_input->position),NULL);
				/* now inform the dof3 widget who its gets its input from */
				dof3_set_data((Widget)data,DOF3_INPUT_WIDGET,temp_input->widget);
			}
			else
			{
				temp_input->position=(struct Dof3_struct *)NULL;
			}
			return_code=1;
		} break;
		case INPUT_DIRECTION_WIDGET:
		{
			temp_input->direction_widget=(Widget)data;
			if (data)
			{
				XtVaGetValues((Widget)data,XmNuserData,&temp_input->direction,NULL);
				/* now inform the dof3 widget who its gets its input from */
				dof3_set_data((Widget)data,DOF3_INPUT_WIDGET,temp_input->widget);
			}
			else
			{
				temp_input->direction=(struct Dof3_struct *)NULL;
			}
			return_code=1;
		} break;
		case INPUT_POSCTRL_WIDGET:
		{
			temp_input->posctrl_widget=(Widget)data;
			if (data)
			{
				XtVaGetValues((Widget)data,XmNuserData,&temp_input->posctrl,NULL);
				/* now inform the controller widget who its gets its input from */
				control_set_data((Widget)data,CONTROL_INPUT_WIDGET,temp_input->widget);
			}
			else
			{
				temp_input->posctrl=(struct Control_struct *)NULL;
			}
			return_code=1;
		} break;
		case INPUT_DIRCTRL_WIDGET:
		{
			temp_input->dirctrl_widget=(Widget)data;
			if (data)
			{
				XtVaGetValues((Widget)data,XmNuserData,&temp_input->dirctrl,NULL);
				/* now inform the controller widget who its gets its input from */
				control_set_data((Widget)data,CONTROL_INPUT_WIDGET,temp_input->widget);
			}
			else
			{
				temp_input->dirctrl=(struct Control_struct *)NULL;
			}
			return_code=1;
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,"input_set_data.  Invalid data type.");
			return_code=0;
		} break;
	}
#else
	return_code=0;
#endif
	LEAVE;

	return (return_code);
} /* input_register_callback */
