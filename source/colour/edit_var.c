/*******************************************************************************
FILE : edit_var.c

LAST MODIFIED : 29 November 1997

DESCRIPTION :
Creates a window that allows the user to create a edit_var.
Each edit_var component ranges between 0-1.
NOTE :
==============================================================================*/
#include <stddef.h>
#include <stdio.h>
#include <math.h>
#if defined (MOTIF)
#include <Xm/Xm.h>
#include <Xm/Text.h>
#include <X11/Xlib.h>
#include <Mrm/MrmPublic.h>
#endif /* defined (MOTIF) */
#include "colour/edit_var.h"
#include "colour/edit_var.uidh"
#include "general/debug.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module constants
----------------
*/
/* UIL Identifiers */
#define edit_var_text_ID     1
#define edit_var_slider_ID   2
#define edit_var_label_ID    3

/*
Module variables
----------------
*/
static int edit_var_hierarchy_open=0;
static MrmHierarchy edit_var_hierarchy;

struct Edit_var_struct
/*******************************************************************************
LAST MODIFIED : 29 November 1997

DESCRIPTION :
Contains information required by the edit_var control dialog.
==============================================================================*/
{
	Widget widget_parent,widget;
	EDIT_VAR_PRECISION current_value,low_limit,high_limit;
	Widget slider,text,label;
	struct Callback_data update_callback;
}; /* Edit_var_struct */

/*
Module functions
----------------
*/
static void edit_var_update(struct Edit_var_struct *temp_edit_var)
/*******************************************************************************
LAST MODIFIED : 8 September 1994

DESCRIPTION :
Tells CMGUI about the current values. Returns a pointer to the material.
==============================================================================*/
{
	ENTER(edit_var_update);
	/* Now send an update to the client if requested */
	if (temp_edit_var->update_callback.procedure)
	{
		/* now call the procedure with the user data and the colour data */
		(temp_edit_var->update_callback.procedure)(temp_edit_var->widget,
			temp_edit_var->update_callback.data,&temp_edit_var->current_value);
	}
	LEAVE;
} /* edit_var_update */

static void edit_var_identify_button(Widget w,int button_num,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 12 June 1994

DESCRIPTION :
Finds the id of the buttons on the edit_var control dialog box.
==============================================================================*/
{
	struct Edit_var_struct *temp_edit_var;

	ENTER(edit_var_identify_button);
	USE_PARAMETER(reason);
	/* find out which edit_var dialog we are in */
	XtVaGetValues(w,XmNuserData,&temp_edit_var,NULL);
	switch (button_num)
	{
		case edit_var_slider_ID:
		{
			temp_edit_var->slider=w;
		} break;
		case edit_var_label_ID:
		{
			temp_edit_var->label=w;
		} break;
		case edit_var_text_ID:
		{
			temp_edit_var->text=w;
		} break;
		default:
		{
			display_message(WARNING_MESSAGE,
				"edit_var_identify_button.  Invalid button number");
		} break;
	};
	LEAVE;
} /* edit_var_identify_button */

static void edit_var_destroy_CB(Widget w, int *tag, unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 2 November 1995

DESCRIPTION :
Callback for the edit_var dialog - tidies up all memory allocation
==============================================================================*/
{
	struct Edit_var_struct *temp_edit_var;

	ENTER(edit_var_destroy_CB);
	USE_PARAMETER(tag);
	USE_PARAMETER(reason);
	/* Get the pointer to the data for the edit_var dialog */
	XtVaGetValues(w,XmNuserData,&temp_edit_var,NULL);
	/* deallocate the memory for the user data */
	DEALLOCATE(temp_edit_var);
	LEAVE;
} /* edit_var_destroy_CB */

static void edit_var_update_value(struct Edit_var_struct *temp_edit_var)
/*******************************************************************************
LAST MODIFIED : 16 March 2001

DESCRIPTION :
Makes the numeric value and the slider agree.
==============================================================================*/
{
	char temp_str[EDIT_VAR_STRING_SIZE];
	EDIT_VAR_PRECISION temp;
	int slider_value;

	ENTER(edit_var_update_value);
	sprintf(temp_str,EDIT_VAR_NUM_FORMAT,temp_edit_var->current_value);
	XmTextSetString(temp_edit_var->text,temp_str);
	temp=temp_edit_var->high_limit-temp_edit_var->low_limit;
	if (temp != 0.0)
	{
		temp=100.0 * (temp_edit_var->current_value-temp_edit_var->low_limit)/
			temp;
		slider_value=temp;
		if(slider_value > 100)
		{
			slider_value = 100;
		}
		if(slider_value < 0)
		{
			slider_value = 0;
		}
	}
	else
	{
		slider_value=0;
	}
	XtVaSetValues(temp_edit_var->slider,XmNvalue,slider_value,NULL);
	LEAVE;
} /* edit_var_update_value */

static void edit_var_set_name(struct Edit_var_struct *temp_edit_var,
	char *description)
/*******************************************************************************
LAST MODIFIED : 2 November 1995

DESCRIPTION :
Writes the correct name on the labels for each of the sliders.
==============================================================================*/
{
	XmString temp_label;

	ENTER(edit_var_set_name);
	temp_label=XmStringCreateSimple(description);
	XtVaSetValues(temp_edit_var->label,XmNlabelString,temp_label,NULL);
	XmStringFree(temp_label);
	LEAVE;
} /* edit_var_set_name */

static void edit_var_number_CB(Widget w,int *tag,XmAnyCallbackStruct *reason)
/*******************************************************************************
LAST MODIFIED : 2 November 1995

DESCRIPTION :
Callback for the text portion of the edit_var dialog.  Determines which button
has been pressed, and then exits.
==============================================================================*/
{
	char *temp_str_ptr;
	EDIT_VAR_PRECISION temp_value;
	struct Edit_var_struct *temp_edit_var;

	ENTER(edit_var_number_CB);
	USE_PARAMETER(tag);
	USE_PARAMETER(reason);
	/* Get the pointer to the data for the edit_var dialog */
	XtVaGetValues(w,XmNuserData,&temp_edit_var,NULL);
	/* get the string */
	XtVaGetValues(w,XmNvalue,&temp_str_ptr,NULL);
	sscanf(temp_str_ptr,"%"EDIT_VAR_PRECISION_STRING,&temp_value);
	XtFree(temp_str_ptr);
	if ((temp_value>=temp_edit_var->low_limit)&&
		(temp_value<=temp_edit_var->high_limit))
	{
		temp_edit_var->current_value=temp_value;
	}
	/* now print it in the correct format */
	edit_var_update_value(temp_edit_var);
	edit_var_update(temp_edit_var);
	LEAVE;
} /* edit_var_number_CB */

static void edit_var_slider_CB(Widget w, int *tag, unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 2 November 1995

DESCRIPTION :
Callback for the slider portion of the edit_var dialog.  Determines which button
has been pressed, and then exits.
==============================================================================*/
{
	EDIT_VAR_PRECISION temp_value;
	int temp_int;
	struct Edit_var_struct *temp_edit_var;

	ENTER(edit_var_slider_CB);
	USE_PARAMETER(tag);
	USE_PARAMETER(reason);
	/* Get the pointer to the data for the edit_var dialog */
	XtVaGetValues(w,XmNuserData,&temp_edit_var,NULL);
	/* get the string */
	XtVaGetValues(w,XmNvalue,&temp_int,NULL);
	temp_value=((EDIT_VAR_PRECISION)temp_int)/100.0*
		(temp_edit_var->high_limit-temp_edit_var->low_limit)+
		temp_edit_var->low_limit;
	temp_edit_var->current_value=temp_value;
	/* now print it in the correct format */
	edit_var_update_value(temp_edit_var);
	edit_var_update(temp_edit_var);
	LEAVE;
} /* edit_var_slider_CB */


/*
Global Functions
----------------
*/
Widget create_edit_var_widget(Widget parent,char *description,
	EDIT_VAR_PRECISION init_data,EDIT_VAR_PRECISION low_limit,
	EDIT_VAR_PRECISION high_limit)
/*******************************************************************************
LAST MODIFIED : 13 December 1996

DESCRIPTION :
Allows the user to set the value of a variable between the limits low_limit
and high_limit.
==============================================================================*/
{
	MrmType edit_var_dialog_class;
	struct Edit_var_struct *temp_edit_var=NULL;
	static MrmRegisterArg callback_list[]=
	{
		{"edit_var_identify_button",(XtPointer)edit_var_identify_button},
		{"edit_var_destroy_CB",(XtPointer)edit_var_destroy_CB},
		{"edit_var_slider_CB",(XtPointer)edit_var_slider_CB},
		{"edit_var_number_CB",(XtPointer)edit_var_number_CB},
	};
	static MrmRegisterArg identifier_list[]=
	{
		{"Edit_var_structure",(XtPointer)NULL},
		{"edit_var_text_ID",(XtPointer)edit_var_text_ID},
		{"edit_var_slider_ID",(XtPointer)edit_var_slider_ID},
		{"edit_var_label_ID",(XtPointer)edit_var_label_ID}
	};
	Widget return_widget;

	ENTER(create_edit_var_widget);
	return_widget=(Widget)NULL;
	if (MrmOpenHierarchy_base64_string(edit_var_uidh,
		&edit_var_hierarchy,&edit_var_hierarchy_open))
	{
		/* allocate memory */
		if (ALLOCATE(temp_edit_var,struct Edit_var_struct,1))
		{
			/* initialise the structure */
			temp_edit_var->widget_parent=parent;
			temp_edit_var->widget=(Widget)NULL;
			temp_edit_var->current_value=init_data;
			temp_edit_var->low_limit=low_limit;
			temp_edit_var->high_limit=high_limit;
			temp_edit_var->slider=(Widget)NULL;
			temp_edit_var->label=(Widget)NULL;
			temp_edit_var->text=(Widget)NULL;
			temp_edit_var->update_callback.procedure=(Callback_procedure *)NULL;
			temp_edit_var->update_callback.data=NULL;
			/* register the callbacks */
			if (MrmSUCCESS==MrmRegisterNamesInHierarchy(edit_var_hierarchy,
				callback_list,XtNumber(callback_list)))
			{
				/* assign and register the identifiers */
				identifier_list[0].value=(XtPointer)temp_edit_var;
				if (MrmSUCCESS==MrmRegisterNamesInHierarchy(edit_var_hierarchy,
					identifier_list,XtNumber(identifier_list)))
				{
					/* fetch edit_var control widget */
					if (MrmSUCCESS==MrmFetchWidget(edit_var_hierarchy,"edit_var_widget",
						temp_edit_var->widget_parent,&(temp_edit_var->widget),
						&edit_var_dialog_class))
					{
						XtManageChild(temp_edit_var->widget);
						/* display the correct names and values */
						edit_var_set_name(temp_edit_var,description);
						edit_var_update_value(temp_edit_var);
						return_widget=temp_edit_var->widget;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_edit_var_dialog.  Could not fetch edit_var dialog");
						DEALLOCATE(temp_edit_var);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_edit_var_dialog.  Could not register identifiers");
					DEALLOCATE(temp_edit_var);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_edit_var_dialog.  Could not register callbacks");
				DEALLOCATE(temp_edit_var);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_edit_var_dialog.  Could not allocate control window structure");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_edit_var_dialog.  Could not open hierarchy");
	}
	LEAVE;

	return (return_widget);
} /* create_edit_var_widget */

int edit_var_get_callback(Widget edit_var_widget,
	struct Callback_data *callback)
/*******************************************************************************
LAST MODIFIED : 29 November 1997

DESCRIPTION :
Returns the update_callback for the edit_var widget.
==============================================================================*/
{
	int return_code;
	struct Edit_var_struct *temp_edit_var;

	ENTER(edit_var_get_callback);
	/* check arguments */
	if (edit_var_widget&&callback)
	{
		/* Get the pointer to the data for the dialog */
		XtVaGetValues(edit_var_widget,XmNuserData,&temp_edit_var,NULL);
		if (temp_edit_var)
		{
			callback->procedure=temp_edit_var->update_callback.procedure;
			callback->data=temp_edit_var->update_callback.data;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"edit_var_get_callback.  Missing widget data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"edit_var_get_callback.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* edit_var_get_callback */

int edit_var_set_callback(Widget edit_var_widget,
	struct Callback_data *callback)
/*******************************************************************************
LAST MODIFIED : 29 November 1997

DESCRIPTION :
Changes the update_callback for the edit_var widget.
==============================================================================*/
{
	int return_code;
	struct Edit_var_struct *temp_edit_var=NULL;

	ENTER(edit_var_set_callback);
	if (edit_var_widget&&callback)
	{
		/* Get the pointer to the data for the edit_var dialog */
		XtVaGetValues(edit_var_widget,XmNuserData,&temp_edit_var,NULL);
		if (temp_edit_var)
		{
			temp_edit_var->update_callback.procedure=callback->procedure;
			temp_edit_var->update_callback.data=callback->data;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"edit_var_set_callback.  Missing widget data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"edit_var_set_callback.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* edit_var_set_callback */

int edit_var_get_data(Widget edit_var_widget,
	enum Edit_var_data_type data_type,EDIT_VAR_PRECISION *data)
/*******************************************************************************
LAST MODIFIED : 29 November 1997

DESCRIPTION :
Returns a data item of the edit_var widget.
==============================================================================*/
{
	int return_code;
	struct Edit_var_struct *temp_edit_var;

	ENTER(edit_var_get_data);
	if (edit_var_widget)
	{
		/* Get the pointer to the data for the edit_var dialog */
		XtVaGetValues(edit_var_widget,XmNuserData,&temp_edit_var,NULL);
		if (temp_edit_var)
		{
			return_code=1;
			switch (data_type)
			{
				case EDIT_VAR_VALUE:
				{
					*data=temp_edit_var->current_value;
				} break;
				case EDIT_VAR_LOW_LIMIT:
				{
					*data=temp_edit_var->low_limit;
				} break;
				case EDIT_VAR_HIGH_LIMIT:
				{
					*data=temp_edit_var->high_limit;
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"edit_var_set_data.  Invalid data type");
					return_code=0;
				} break;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"edit_var_set_data.  Missing widget data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"edit_var_set_data.  Missing widget");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* edit_var_get_data */

int edit_var_set_data(Widget edit_var_widget,
	enum Edit_var_data_type data_type,EDIT_VAR_PRECISION data)
/*******************************************************************************
LAST MODIFIED : 29 November 1997

DESCRIPTION :
Changes a data item of the edit_var widget.
==============================================================================*/
{
	int return_code;
	struct Edit_var_struct *temp_edit_var;

	ENTER(edit_var_set_data);
	if (edit_var_widget)
	{
		/* Get the pointer to the data for the edit_var dialog */
		XtVaGetValues(edit_var_widget,XmNuserData,&temp_edit_var,NULL);
		if (temp_edit_var)
		{
			return_code=1;
			switch (data_type)
			{
				case EDIT_VAR_VALUE:
				{
					temp_edit_var->current_value=data;
					edit_var_update_value(temp_edit_var);
				} break;
				case EDIT_VAR_LOW_LIMIT:
				{
					temp_edit_var->low_limit=data;
					edit_var_update_value(temp_edit_var);
				} break;
				case EDIT_VAR_HIGH_LIMIT:
				{
					temp_edit_var->high_limit=data;
					edit_var_update_value(temp_edit_var);
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"edit_var_set_data.  Invalid data type");
					return_code=0;
				} break;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"edit_var_set_data.  Missing widget data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"edit_var_set_data.  Missing widget");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* edit_var_set_data */
