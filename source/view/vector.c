/*******************************************************************************
FILE : vector.c

LAST MODIFIED : 26 November 2001

DESCRIPTION :
This module creates a free vector input device, using two dof3, two control and
one input widget.  The position is given relative to some vectorinate system,
and the returned value is a global one.
==============================================================================*/
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "general/debug.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"
#include "view/vector.h"
#include "view/vector.uidh"

/*
Module variables
----------------
*/
#if defined (MOTIF)
static int vector_hierarchy_open=0;
static MrmHierarchy vector_hierarchy;
#endif /* defined (MOTIF) */
/* initialisation data for the menu choices and their values */
struct Dof3_data vector_values[VECTOR_NUM_CHOICES] =
{
	{{1.0,0.0,0.0}},
	{{0.0,1.0,0.0}},
	{{0.0,0.0,1.0}},
	{{-1.0,0.0,0.0}},
	{{0.0,-1.0,0.0}},
	{{0.0,0.0,-1.0}}
};
char vector_names[VECTOR_NUM_CHOICES][3] =
{
	"+X","+Y","+Z","-X","-Y","-Z"
};

/*
Module functions
----------------
*/
void vector_update(struct Vector_struct *temp_vector)
/*******************************************************************************
LAST MODIFIED : 8 September 1994

DESCRIPTION :
Tells CMGUI about the current values.
==============================================================================*/
{
	ENTER(vector_update);
	if(temp_vector->callback_array[VECTOR_UPDATE_CB].procedure)
	{
		/* now call the procedure with the user data and the position data */
		(temp_vector->callback_array[VECTOR_UPDATE_CB].procedure)
			(temp_vector->widget,
			temp_vector->callback_array[VECTOR_UPDATE_CB].data,
			&temp_vector->current_value);
	}
	LEAVE;
} /* vector_update */

static void vector_identify_button(Widget w,int button_num,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 18 April 1994

DESCRIPTION :
Finds the id of the buttons on the vector widget.
==============================================================================*/
{
	int toggle_num;
	struct Vector_struct *temp_vector;

	ENTER(vector_identify_button);
	USE_PARAMETER(reason);
	switch (button_num)
	{
		case vector_menu_ID:
		{
			/* find out which vector widget we are in */
			XtVaGetValues(w,XmNuserData,&temp_vector,NULL);
			temp_vector->menu = w;
		}; break;
		case vector_toggle_ID:
		{
			/* find out which vector widget we are in */
			XtVaGetValues(XtParent(w),XmNuserData,&temp_vector,NULL);
			XtVaGetValues(w,XmNuserData,&toggle_num,NULL);
			if((toggle_num>=0)&&(toggle_num<VECTOR_NUM_CHOICES))
			{
				temp_vector->toggle[toggle_num] = w;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"vector_identify_button.  Invalid toggle number.");
			}
		}; break;
		default:
		{
		display_message(WARNING_MESSAGE,
			"vector_identify_button.  Invalid button number");
		}; break;
	}
	LEAVE;
} /* vector_identify_button */

static void vector_destroy_CB(Widget w, int *tag, unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 4 November 1995

DESCRIPTION :
Callback for the vectorment dialog - tidies up all details - mem etc
==============================================================================*/
{
	struct Vector_struct *temp_vector;

	ENTER(vector_destroy_CB);
	USE_PARAMETER(tag);
	USE_PARAMETER(reason);
	/* Get the pointer to the data for the vector widget */
	XtVaGetValues(w,XmNuserData,&temp_vector,NULL);
	/* deallocate the memory for the user data */
	printf("De-allocating memory.\n");
	DEALLOCATE(temp_vector);
	LEAVE;
} /* vector_destroy_CB */

static void vector_button_CB(Widget w, int *tag, XmAnyCallbackStruct *reason)
/*******************************************************************************
LAST MODIFIED : 12 January 1995

DESCRIPTION :
Callback for the vector widget - changes the current vector to be one of the
menu choices.
==============================================================================*/
{
	int i,toggle_num;
	struct Vector_struct *temp_vector;

	ENTER(vector_button_CB);
	USE_PARAMETER(tag);
	USE_PARAMETER(reason);
	XtVaGetValues(XtParent(w),XmNuserData,&temp_vector,NULL);
	XtVaGetValues(w,XmNuserData,&toggle_num,NULL);
	if ((toggle_num>=0)&&(toggle_num<VECTOR_NUM_CHOICES))
	{
		for(i=0;i<3;i++)
		{
			temp_vector->current_value.data[i] = vector_values[toggle_num].data[i];
		}
		vector_update(temp_vector);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"vector_button_CB.  Invalid toggle number.");
	}
	LEAVE;
} /* vector_button_CB */

static void vector_add_buttons(struct Vector_struct *temp_vector)
/*******************************************************************************
LAST MODIFIED : 12 February 1996

DESCRIPTION :
Adds VECTOR_NUM_CHOICES buttons to the vector widget.
==============================================================================*/
{
	Arg override_arg;
	int i;
	MrmType vector_submenu_class;
	Widget new_button;
	XmString temp_label;

	ENTER(vector_add_buttons);
	if (vector_hierarchy_open)
	{
		for (i=0;i<VECTOR_NUM_CHOICES;i++)
		{
			/* fetch the new button, and give it user data of its button number */
			new_button=(Widget)NULL;
			XtSetArg(override_arg,XmNuserData,i);
			if (MrmSUCCESS==MrmFetchWidgetOverride(vector_hierarchy,
				"vector_menu_item",temp_vector->menu,NULL,&override_arg,1,
				&new_button,&vector_submenu_class))
			{
				XtManageChild(new_button);
				/* now we have to set the name */
				temp_label = XmStringCreateSimple(vector_names[i]);
				XtVaSetValues(new_button,XmNlabelString,temp_label,NULL);
				XmStringFree(temp_label);
			}
			else
			{
				display_message(WARNING_MESSAGE,
					"vector_add_button.  Could not fetch submenu widget");
			}
		}
	}
	else
	{
		display_message(WARNING_MESSAGE,"vector_add_button.  Hierarchy not open");
	}
	LEAVE;
} /* vector_add_buttons */

/*
Global functions
----------------
*/
Widget create_vector_widget(Widget parent)
/*******************************************************************************
LAST MODIFIED : 13 December 1996

DESCRIPTION :
Creates a vector widget that gets a position and orientation from the user.
==============================================================================*/
{
	int i;
	MrmType vector_dialog_class;
	struct Vector_struct *temp_vector = NULL;
	static MrmRegisterArg callback_list[]=
	{
		{"vector_identify_button",(XtPointer)vector_identify_button},
		{"vector_destroy_CB",(XtPointer)vector_destroy_CB},
		{"vector_button_CB",(XtPointer)vector_button_CB}
	};
	static MrmRegisterArg identifier_list[] =
	{
		{"Vector_structure",(XtPointer)NULL},
		{"vector_menu_ID",(XtPointer)vector_menu_ID},
		{"vector_toggle_ID",(XtPointer)vector_toggle_ID}
	};
	Widget return_widget;

	ENTER(create_vector_widget);
	return_widget = (Widget)NULL;
	if (MrmOpenHierarchy_base64_string(vector_uidh,
		&vector_hierarchy,&vector_hierarchy_open))
	{
		/* allocate memory */
		if (ALLOCATE(temp_vector,struct Vector_struct,1))
		{
			/* initialise the structure */
			temp_vector->widget_parent = parent;
			temp_vector->widget = (Widget)NULL;
			temp_vector->menu = (Widget)NULL;
			for(i=0;i<2;i++)
			{
				temp_vector->current_value.data[i] = 0.0;
			}
			temp_vector->current_value.data[2] = 1.0;
			for(i=0;i<VECTOR_NUM_CHOICES;i++)
			{
				temp_vector->toggle[i] = (Widget)NULL;
			}
			for(i=0;i<VECTOR_NUM_CALLBACKS;i++)
			{
				temp_vector->callback_array[i].procedure = (Callback_procedure *)NULL;
				temp_vector->callback_array[i].data = NULL;
			}
			/* register the callbacks */
			if (MrmSUCCESS==MrmRegisterNamesInHierarchy(vector_hierarchy,
				callback_list,XtNumber(callback_list)))
			{
				/* assign and register the identifiers */
				identifier_list[0].value=(XtPointer)temp_vector;
				if (MrmSUCCESS==MrmRegisterNamesInHierarchy(vector_hierarchy,
					identifier_list,XtNumber(identifier_list)))
				{
					/* fetch vector widget */
					if (MrmSUCCESS==MrmFetchWidget(vector_hierarchy,"vector_widget",
						temp_vector->widget_parent,&(temp_vector->widget),
						&vector_dialog_class))
					{
						XtManageChild(temp_vector->widget);
						/* add ourself to the list of vector_widgets */
						vector_add_buttons(temp_vector);
						return_widget = temp_vector->widget;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_vector_widget.  Could not fetch vector dialog");
						DEALLOCATE(temp_vector);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_vector_widget.  Could not register identifiers");
					DEALLOCATE(temp_vector);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_vector_widget.  Could not register callbacks");
				DEALLOCATE(temp_vector);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_vector_widget.  Could not allocate vector widget structure");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_vector_widget.  Could not open hierarchy");
	}
	LEAVE;

	return (return_widget);
} /* create_vector_widget */

int vector_set_data(Widget vector_widget,
	enum Vector_data_type data_type,void *data)
/*******************************************************************************
LAST MODIFIED : 4 January 1995

DESCRIPTION :
Changes a data item of the vector widget.
==============================================================================*/
{
	int found,i,j,match,return_code;
	struct Vector_struct *temp_vector;

	ENTER(vector_set_data);
	/* Get the pointer to the data for the vector dialog */
	XtVaGetValues(vector_widget,XmNuserData,&temp_vector,NULL);
	switch(data_type)
	{
		case VECTOR_UPDATE_CB:
		{
			temp_vector->callback_array[VECTOR_UPDATE_CB].procedure =
				((struct Callback_data *)data)->procedure;
			temp_vector->callback_array[VECTOR_UPDATE_CB].data =
				((struct Callback_data *)data)->data;
			return_code = 1;
		}; break;
		case VECTOR_DATA:
		{
			found = 0;
			for(i=0;i<VECTOR_NUM_CHOICES;i++)
			{
				match = 1;
				for(j=0;j<3;j++)
				{
					if(((struct Dof3_data *)data)->data[j]!=vector_values[i].data[j])
					{
						match = 0;
					}
				}
				if (match)
				{
					for(j=0;j<3;j++)
					{
						temp_vector->current_value.data[j] = vector_values[i].data[j];
					}
					found = 1;
					/* now make sure that that widget is displayed */
					XtVaSetValues(temp_vector->widget,XmNmenuHistory,
						temp_vector->toggle[i],NULL);
				}
			}
			if (!found)
			{
				display_message(ERROR_MESSAGE,
					"vector_button_CB.  Could not find vector.");
			}
			return_code = 1;
		}; break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"vector_set_data.  Invalid data type.");
			return_code = 0;
		}; break;
	}
	LEAVE;

	return (return_code);
} /* vector_set_data */

void *vector_get_data(Widget vector_widget,
	enum Vector_data_type data_type)
/*******************************************************************************
LAST MODIFIED : 4 November 1995

DESCRIPTION :
Changes a data item of the vector widget.
==============================================================================*/
{
	void *return_code;
	struct Vector_struct *temp_vector;
	static struct Callback_data dat_callback;

	ENTER(vector_get_data);
	/* Get the pointer to the data for the vector dialog */
	XtVaGetValues(vector_widget,XmNuserData,&temp_vector,NULL);
	switch(data_type)
	{
		case VECTOR_UPDATE_CB:
		{
			dat_callback.procedure =
				temp_vector->callback_array[VECTOR_UPDATE_CB].procedure;
			dat_callback.data = temp_vector->callback_array[VECTOR_UPDATE_CB].data;
			return_code = &dat_callback;
		}; break;
		case VECTOR_DATA:
		{
			return_code = &temp_vector->current_value;
		}; break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"vector_get_data.  Invalid data type.");
			return_code = NULL;
		}; break;
	}
	LEAVE;

	return (return_code);
} /* vector_get_data */
