/*******************************************************************************
FILE : dof3_control.c

LAST MODIFIED : 6 January 1998

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
are.  ie input_set data would be called with
	INPUT_POSITION_WIDGET      DOF31
	INPUT_DIRECTION_WIDGET      DOF32
	INPUT_POSCTRL_WIDGET        CONTROL1
	INPUT_DIRCTRL_WIDGET        CONTROL2
input_set_data will ensure that the control and dof3 widgets are informed as
to who is providing their input.

When any of the widgets are destroyed, they ensure that all links with other
widgets are terminated, and so the whole structure does not need to be destroyed
- widgets may be mixed and matched as the situation changes.
==============================================================================*/
#include <math.h>
#include <stdio.h>
#include <Xm/ToggleB.h>
#include <Xm/ToggleBG.h>
#include "dof3/dof3_control.h"
#include "dof3/dof3_control.uid64"
#include "dof3/dof3_input.h"
#include "general/debug.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module variables
----------------
*/
#if defined (MOTIF)
static int dof3_control_hierarchy_open=0;
static MrmHierarchy dof3_control_hierarchy;
#endif /* defined (MOTIF) */

/*
Module functions
----------------
*/
static void control_identify_button(Widget w, int button_type,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 18 April 1994

DESCRIPTION :
Finds the id of the buttons on the control widget.
==============================================================================*/
{
	int button_num;
	struct Control_struct *temp_control;

	ENTER(control_identify_button);
	USE_PARAMETER(reason);
	/* find out which control widget we are in */
	if (button_type==control_coord_item_button_ID)
	{
		XtVaGetValues(XtParent(w),XmNuserData,&temp_control,NULL);
		XtVaGetValues(w,XmNuserData,&button_num,NULL);
		temp_control->coord[button_num]=w;
	}
	else
	{
		XtVaGetValues(w,XmNuserData,&temp_control,NULL);
		switch (button_type)
		{
			case control_save_button_ID:
			{
				temp_control->save=w;
			} break;
			case control_reset_button_ID:
			{
				temp_control->reset=w;
			} break;
			case control_lockval_button_ID:
			{
				temp_control->lockval=w;
			} break;
			case control_linkres_button_ID:
			{
				temp_control->linkres=w;
			} break;
			case control_coord_menu_button_ID:
			{
				temp_control->coord_menu=w;
			} break;
			default:
			{
				display_message(WARNING_MESSAGE,
					"control_identify_button.  Invalid button number");
			} break;
		}
	}
	LEAVE;
} /* control_identify_button */

static void control_destroy_CB(Widget w, int *tag, unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 2 November 1995

DESCRIPTION :
Callback for the control widget - tidies up all details - mem etc
==============================================================================*/
{
	struct Control_struct *temp_control;

	ENTER(control_destroy_CB);
	USE_PARAMETER(tag);
	USE_PARAMETER(reason);
	/* Get the pointer to the data for the control widget */
	XtVaGetValues(w,XmNuserData,&temp_control,NULL);
	/*???debug */
	if (temp_control->dof3_widget)
	{
		dof3_set_data(temp_control->dof3_widget,DOF3_CONTROL_WIDGET,NULL);
	}
#if defined (EXT_INPUT)
	if (temp_control->input_widget)
	{
		if (temp_control->type==DOF3_POSITION)
		{
			input_set_data(temp_control->input_widget,INPUT_POSCTRL_WIDGET,NULL);
		}
		else
		{
			input_set_data(temp_control->input_widget,INPUT_DIRCTRL_WIDGET,NULL);
		}
	}
#endif
	/* deallocate the memory for the user data */
	DEALLOCATE(temp_control);
	/*??? this will have to tie in with a controller for any external control */
	LEAVE;
} /* control_destroy_CB */

static void control_save_CB(Widget w, int *tag,
	XmAnyCallbackStruct *reason)
/*******************************************************************************
LAST MODIFIED : 2 November 1995

DESCRIPTION :
Callback for the control widget.  Saves a set of values into the dof3
widget.
==============================================================================*/
{
	struct Control_struct *temp_control;

	ENTER(control_save_CB);
	USE_PARAMETER(tag);
	USE_PARAMETER(reason);
	/* Get the pointer to the data for the control widget */
	XtVaGetValues(w,XmNuserData,&temp_control,NULL);
	if (temp_control->dof3)
	{
		dof3_save(temp_control->dof3);
	}
	LEAVE;
} /* control_save_CB */

static void control_reset_CB(Widget w, int *tag,
	XmAnyCallbackStruct *reason)
/*******************************************************************************
LAST MODIFIED : 2 November 1995

DESCRIPTION :
Callback for the control widget.  Sets the values to zero.
==============================================================================*/
{
	struct Control_struct *temp_control;

	ENTER(control_reset_CB);
	USE_PARAMETER(tag);
	USE_PARAMETER(reason);
	/* Get the pointer to the data for the control widget */
	XtVaGetValues(w,XmNuserData,&temp_control,NULL);
	if (temp_control->dof3)
	{
		dof3_reset(temp_control->dof3);
	}
	LEAVE;
} /* control_reset_CB */

static void control_lockval_CB(Widget w, int *tag,
	XmToggleButtonCallbackStruct *reason)
/*******************************************************************************
LAST MODIFIED : 20 March 1994

DESCRIPTION :
Callback for the control widget.  Loads a saved set of values into the dof3
widget.
==============================================================================*/
{
	struct Control_struct *temp_control;

	ENTER(control_lockval_CB);
	USE_PARAMETER(tag);
	/* Get the pointer to the data for the control widget */
	XtVaGetValues(w,XmNuserData,&temp_control,NULL);
	if (temp_control->dof3)
	{
		dof3_change_lock(temp_control->dof3,
			((XmToggleButtonCallbackStruct *)reason)->set);
	}
	LEAVE;
} /* control_lockval_CB */

static void control_linkres_CB(Widget w, int *tag,
	XmToggleButtonCallbackStruct *reason)
/*******************************************************************************
LAST MODIFIED : 20 March 1994

DESCRIPTION :
Callback for the control widget.  Loads a saved set of values into the dof3
widget.
==============================================================================*/
{
	struct Control_struct *temp_control;

	ENTER(control_linkres_CB);
	USE_PARAMETER(tag);
	/* Get the pointer to the data for the control widget */
	XtVaGetValues(w,XmNuserData,&temp_control,NULL);
	if (temp_control->dof3)
	{
		dof3_change_link(temp_control->dof3,
			((XmToggleButtonCallbackStruct *)reason)->set);
	}
	LEAVE;
} /* control_linkres_CB */

static void control_coord_CB(Widget w, int *tag,
	XmAnyCallbackStruct *reason)
/*******************************************************************************
LAST MODIFIED : 20 March 1994

DESCRIPTION :
Callback for the control widget.  Sets the values to zero.
==============================================================================*/
{
	int  coord_num,i,num_coords,temp;
	struct Control_struct *temp_control;

	ENTER(control_coord_CB);
	USE_PARAMETER(tag);
	USE_PARAMETER(reason);
	/* Get the pointer to the data for the control widget */
	XtVaGetValues(XtParent(w),XmNuserData,&temp_control,NULL);
	XtVaGetValues(w,XmNuserData,&coord_num,NULL);
	if (temp_control->dof3)
	{
		/* keep a record of the old coord system */
		temp=temp_control->dof3->current_coord;
		temp_control->dof3->current_coord=coord_num;
		switch (temp_control->type)
		{
			case DOF3_POSITION:
			{
				num_coords=DOF3_NUM_POSITION_COORD;
			} break;
			case DOF3_DIRECTION:
			{
				num_coords=DOF3_NUM_DIRECTION_COORD;
			} break;
			case DOF3_VECTOR:
			{
				num_coords=DOF3_NUM_VECTOR_COORD;
			} break;
		}
		for (i=0;i<num_coords;i++)
		{
			if (temp_control->coord[i])
			{
				if (i!=coord_num)
				{
					XmToggleButtonGadgetSetState(temp_control->coord[i],FALSE,FALSE);
				}
				else
				{
					XmToggleButtonGadgetSetState(temp_control->coord[i],TRUE,FALSE);
				}
			}
		}
		dof3_init(temp_control->dof3,temp);
	}
	LEAVE;
} /* control_coord_CB */

static void control_reflect_dof3(struct Control_struct *temp_control)
/*******************************************************************************
LAST MODIFIED : 10 January 1995

DESCRIPTION :
Makes sure that the widgets in the control widget show the correct state of
lockval and linkres and current_coord.
==============================================================================*/
{
	ENTER(control_reflect_dof3);
	if (temp_control->dof3)
	{
		XmToggleButtonGadgetSetState(temp_control->lockval,
			dof3_get_lock(temp_control->dof3),FALSE);
		XmToggleButtonGadgetSetState(temp_control->linkres,
			dof3_get_link(temp_control->dof3),FALSE);
		if (temp_control->coord[temp_control->dof3->current_coord])
		{
			XmToggleButtonGadgetSetState(
				temp_control->coord[temp_control->dof3->current_coord],
				TRUE,FALSE);
		}
	}
	else
	{ /* just set the buttons off */
		XmToggleButtonGadgetSetState(temp_control->lockval,FALSE,FALSE);
		XmToggleButtonGadgetSetState(temp_control->lockval,FALSE,FALSE);
	}
	LEAVE;
} /* control_reflect_dof3 */

static void control_set_coord(struct Control_struct *temp_control)
/*******************************************************************************
LAST MODIFIED : 29 January 1996

DESCRIPTION :
Frees any widgets in the coord submenu if they exist, then gets menu widgets
from the uil file to place in the submenu.
==============================================================================*/
{
	Arg override_arg;
	int i,num_coords;
	MrmType coord_submenu_class;
	XmString temp_label;

	ENTER(control_set_coord);
	if (dof3_control_hierarchy_open)
	{
		for (i=0;i<DOF3_NUM_MAX_COORD;i++)
		{
			if (temp_control->coord[i])
			{
				XtDestroyWidget(temp_control->coord[i]);
				temp_control->coord[i]=(Widget)NULL;
			}
		}
		if ((temp_control->dof3)&&(temp_control->coord_menu))
		{
			switch (temp_control->type)
			{
				case DOF3_POSITION:
				{
					num_coords=DOF3_NUM_POSITION_COORD;
				} break;
				case DOF3_DIRECTION:
				{
					num_coords=DOF3_NUM_DIRECTION_COORD;
				} break;
				case DOF3_VECTOR:
				{
					num_coords=DOF3_NUM_VECTOR_COORD;
				} break;
			}
			for (i=0;i<num_coords;i++)
			{
				XtSetArg(override_arg,XmNuserData,i);
				if (MrmSUCCESS==MrmFetchWidgetOverride(dof3_control_hierarchy,
					"control_menu_coord_menu_item",temp_control->coord_menu,NULL,
					&override_arg,1,&(temp_control->coord[i]),&coord_submenu_class))
				{
					XtManageChild(temp_control->coord[i]);
					/* now we have to set the name */
					temp_label=XmStringCreateSimple(
						dof3_coord_names[temp_control->type][i]);
					XtVaSetValues(temp_control->coord[i],XmNlabelString,temp_label,NULL);
					XmStringFree(temp_label);
				}
				else
				{
					display_message(WARNING_MESSAGE,
						"control_set_submenu.  Could not fetch submenu widget");
				}
			}
		}
	}
	else
	{
		display_message(WARNING_MESSAGE,
			"control_set_submenu.  Dof3 control hierarchy not open");
	}
	LEAVE;
} /* control_set_coord */


/*
Global Functions
----------------
*/
Widget create_control_widget(Widget parent,char *description)
/*******************************************************************************
LAST MODIFIED : 13 December 1996

DESCRIPTION :
Creates a widget that will handle the redirection of external control to dof3
widgets.  <position/direction> may be NULL.  <bounding> is the widget that
encloses all dof3 and control and control widgets that wis to be regarded as one
item.
==============================================================================*/
{
	XmString temp_label;
	int n;
	MrmType control_dialog_class;
	struct Control_struct *temp_control=NULL;
	static MrmRegisterArg callback_list[]=
	{
		{"control_identify_button",(XtPointer)control_identify_button},
		{"control_destroy_CB",(XtPointer)control_destroy_CB},
		{"control_save_CB",(XtPointer)control_save_CB},
		{"control_reset_CB",(XtPointer)control_reset_CB},
		{"control_lockval_CB",(XtPointer)control_lockval_CB},
		{"control_linkres_CB",(XtPointer)control_linkres_CB},
		{"control_coord_CB",(XtPointer)control_coord_CB}
	};
	static MrmRegisterArg identifier_list[]=
	{
		{"Control_structure",(XtPointer)(NULL)},
		{"control_save_button_ID",(XtPointer)control_save_button_ID},
		{"control_reset_button_ID",(XtPointer)control_reset_button_ID},
		{"control_lockval_button_ID",(XtPointer)control_lockval_button_ID},
		{"control_linkres_button_ID",(XtPointer)control_linkres_button_ID},
		{"control_coord_menu_button_ID",(XtPointer)control_coord_menu_button_ID},
		{"control_coord_item_button_ID",(XtPointer)control_coord_item_button_ID}
	};
	Widget return_widget;

	ENTER(create_control_widget);
	return_widget=(Widget)NULL;
	if (MrmOpenHierarchy_base64_string(dof3_control_uid64,
		&dof3_control_hierarchy,&dof3_control_hierarchy_open))
	{
		/* allocate memory */
		if (ALLOCATE(temp_control,struct Control_struct,1))
		{
			/* initialise the structure */
			temp_control->widget_parent=parent;
			temp_control->widget=(Widget)NULL;
			temp_control->dof3=(struct Dof3_struct *)NULL;
			temp_control->type=DOF3_POSITION; /*initialisation value only */
			temp_control->dof3_widget=(Widget)NULL;
			temp_control->input_widget=(Widget)NULL;
			temp_control->load=(Widget)NULL;
			temp_control->save=(Widget)NULL;
			temp_control->reset=(Widget)NULL;
			temp_control->lockval=(Widget)NULL;
			temp_control->linkres=(Widget)NULL;
			temp_control->coord_menu=(Widget)NULL;
			for (n=0;n<DOF3_NUM_MAX_COORD;n++)
			{
				temp_control->coord[n]=(Widget)NULL;
			}
			/* register the callbacks */
			if (MrmSUCCESS==MrmRegisterNamesInHierarchy(dof3_control_hierarchy,
				callback_list,XtNumber(callback_list)))
			{
				/* assign and register the identifiers */
				identifier_list[0].value=(XtPointer)temp_control;
				if (MrmSUCCESS==MrmRegisterNamesInHierarchy(dof3_control_hierarchy,
					identifier_list,XtNumber(identifier_list)))
				{
					/* fetch control window widget */
					if (MrmSUCCESS==MrmFetchWidget(dof3_control_hierarchy,
						"control_widget",temp_control->widget_parent,
						&(temp_control->widget),&control_dialog_class))
					{
						XtManageChild(temp_control->widget);
						/* Show the correct name */
						temp_label=XmStringCreateSimple(description);
						XtVaSetValues(temp_control->widget,XmNlabelString,temp_label,NULL);
						XmStringFree(temp_label);
						/* set up the coordinate submenu */
						control_set_coord(temp_control);
						/* ensure that lockval and linkres reflect the state of the dof3
							widget */
						control_reflect_dof3(temp_control);
						return_widget=temp_control->widget;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_control_widget.  Could not fetch control dialog");
						DEALLOCATE(temp_control);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_control_widget.  Could not register identifiers");
					DEALLOCATE(temp_control);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_control_widget.  Could not register callbacks");
				DEALLOCATE(temp_control);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_control_widget.  Could not allocate control widget structure");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_control_widget.  Could not open hierarchy");
	}
	LEAVE;

	return (return_widget);
} /* create_control_widget */

int control_set_data(Widget control_widget,
	enum Control_data_type data_type,void *data)
/*******************************************************************************
LAST MODIFIED : 4 January 1995

DESCRIPTION :
Changes a data item of the control widget.
==============================================================================*/
{
	int return_code;
	struct Control_struct *temp_control;

	ENTER(control_set_data);
	/* Get the pointer to the data for the control dialog */
	XtVaGetValues(control_widget,XmNuserData,&temp_control,NULL);
	switch (data_type)
	{
		case CONTROL_DOF3_WIDGET:
		{
			temp_control->dof3_widget=(Widget)data;
			if (data)
			{
				XtVaGetValues((Widget)data,XmNuserData,&temp_control->dof3,NULL);
				temp_control->type=temp_control->dof3->type;
				/* now inform the dof3 widget who its controller is */
				dof3_set_data((Widget)data,DOF3_CONTROL_WIDGET,temp_control->widget);
			}
			else
			{
				temp_control->dof3=(struct Dof3_struct *)NULL;
			}
			/* set up the coordinate submenu */
			control_set_coord(temp_control);
			/* ensure that lockval and linkres reflect the state of the dof3 widget */
			control_reflect_dof3(temp_control);
			return_code=1;
		} break;
		case CONTROL_INPUT_WIDGET:
		{
			temp_control->input_widget=(Widget)data;
			return_code=1;
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"control_set_data.  Invalid data type.");
			return_code=0;
		} break;
	}
	LEAVE;

	return (return_code);
} /* control_register_callback */

#if defined (EXT_INPUT)
int control_input_module_CB(void *identifier,Input_module_message message)
/*******************************************************************************
LAST MODIFIED : 2 November 1995

DESCRIPTION :
Finds the id of the buttons on the control dialog box.
==============================================================================*/
{
	int lock_data,return_code;
	struct Control_struct *temp_control=identifier;

	ENTER(control_input_module_CB);
	switch (message->type)
	{
		case IM_TYPE_MOTION:
		{
		} break;
		case IM_TYPE_BUTTON_PRESS:
		{
		} break;
		case IM_TYPE_BUTTON_RELEASE:
		{
			switch (message->source)
			{
				case IM_SOURCE_SPACEBALL:
				{
					if (temp_control->dof3)
					{
						if (message->data[0]==9)
						{
							dof3_reset(temp_control->dof3);
						}
						else
						{
							if (((message->data[0]==1)&&(temp_control->type==DOF3_POSITION))
								||((message->data[0]==2)&&(temp_control->type==DOF3_DIRECTION)))
							{
								lock_data= !dof3_get_lock(temp_control->dof3);
								dof3_change_lock(temp_control->dof3,lock_data);
								XmToggleButtonGadgetSetState(temp_control->lockval,
									lock_data,FALSE);
							}
						}
					}
				} break;
				default:
				{
					display_message(WARNING_MESSAGE,
						"control_input_module_CB.  Invalid device source.");
				} break;
			}
		} break;
		default:
		{
			display_message(WARNING_MESSAGE,
				"control_input_module_CB.  Invalid message type.");
		} break;
	}
	return_code=1;
	LEAVE;

	return (return_code);
} /* control_input_module_CB */
#endif
