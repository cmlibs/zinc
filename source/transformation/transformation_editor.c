/*******************************************************************************
FILE : transformation_editor.c

LAST MODIFIED : 26 November 2001

DESCRIPTION :
This module creates a free transformation_editor input device, using two dof3,
two control and one input widget.
==============================================================================*/
#include <math.h>
#include <stdio.h>
#include "dof3/dof3.h"
#include "dof3/dof3_control.h"
#if defined (EXT_INPUT)
#include "dof3/dof3_input.h"
#endif
#include "general/debug.h"
#include "transformation/transformation_editor.h"
#include "transformation/transformation_editor.uidh"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"
#include "view/coord.h"
#include "view/coord_trans.h"

/*
Module variables
----------------
*/
#if defined (MOTIF)
static int transformation_editor_hierarchy_open=0;
static MrmHierarchy transformation_editor_hierarchy;
#endif /* defined (MOTIF) */

/*
Module functions
----------------
*/
static void transformation_editor_update(
	struct Transformation_editor_struct *temp_transformation_editor)
/*******************************************************************************
LAST MODIFIED : 26 September 1995

DESCRIPTION :
Tells CMGUI about the current values.
==============================================================================*/
{
	ENTER(transformation_editor_update);
	if ((temp_transformation_editor->callback_array)
		[TRANSFORMATION_EDITOR_UPDATE_CB]. procedure)
	{
		/* now call the procedure with the user data and the transformation_editor
			data */
		((temp_transformation_editor->callback_array)
			[TRANSFORMATION_EDITOR_UPDATE_CB].procedure)(
			temp_transformation_editor->widget,
			(temp_transformation_editor->callback_array)
			[TRANSFORMATION_EDITOR_UPDATE_CB].data,
			&temp_transformation_editor->current_value);
	}
	LEAVE;
} /* transformation_editor_update */

static void transformation_editor_update_coord(Widget coord_widget,
	void *user_data,void *temp_coordinate)
/*******************************************************************************
LAST MODIFIED : 26 September 1995

DESCRIPTION :
Tells CMGUI about the current values.
==============================================================================*/
{
	struct Transformation_editor_struct *temp_transformation_editor = user_data;
	struct Cmgui_coordinate *coordinate = temp_coordinate;
	struct Dof3_data new_dof3;

	ENTER(transformation_editor_update_coord);
	USE_PARAMETER(coord_widget);
	temp_transformation_editor->parent_coordinate = coordinate;
	get_local_position(&(temp_transformation_editor->current_value.origin.
		position),coordinate,&new_dof3);
	dof3_set_data(temp_transformation_editor->position_widget,
		DOF3_DATA,&new_dof3);
	get_local_direction(&(temp_transformation_editor->current_value.origin.
		direction),coordinate,&new_dof3);
	dof3_set_data(temp_transformation_editor->direction_widget,DOF3_DATA,
		&new_dof3);
	LEAVE;
} /* transformation_editor_update_coord */

static void transformation_editor_update_position(
	Widget transformation_editor_widget,void *user_data,void *temp_dof3)
/*******************************************************************************
LAST MODIFIED : 4 November 1995

DESCRIPTION :
Tells CMGUI about the current values.
==============================================================================*/
{
	struct Transformation_editor_struct *temp_transformation_editor = user_data;
	struct Dof3_data *temp_pos = temp_dof3;

	ENTER(transformation_editor_update_position);
	USE_PARAMETER(transformation_editor_widget);
	get_global_position(temp_pos,temp_transformation_editor->parent_coordinate,
		&(temp_transformation_editor->current_value.origin.position));
	transformation_editor_update(temp_transformation_editor);
	LEAVE;
} /* transformation_editor_update_position */

static void transformation_editor_update_direction(Widget direction_widget,
	void *user_data,void *temp_dof3)
/*******************************************************************************
LAST MODIFIED : 4 November 1995

DESCRIPTION :
Tells CMGUI about the current values.
==============================================================================*/
{
	struct Transformation_editor_struct *temp_transformation_editor = user_data;
	struct Dof3_data *temp_direction = temp_dof3;

	ENTER(transformation_editor_update_direction);
	USE_PARAMETER(direction_widget);
	get_global_direction(temp_direction,
		temp_transformation_editor->parent_coordinate,
		&(temp_transformation_editor->current_value.origin.direction));
	transformation_editor_update(temp_transformation_editor);
	LEAVE;
} /* transformation_editor_update_direction */

static void transformation_editor_identify_button(Widget w,int button_num,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 4 November 1994

DESCRIPTION :
Finds the id of the buttons on the transformation_editor widget.
==============================================================================*/
{
	struct Transformation_editor_struct *temp_transformation_editor;

	ENTER(transformation_editor_identify_button);
	USE_PARAMETER(reason);
	/* find out which transformation_editor widget we are in */
	XtVaGetValues(w,XmNuserData,&temp_transformation_editor,NULL);
	switch (button_num)
	{
		case transformation_editor_menu_ID:
		{
			temp_transformation_editor->menu = w;
		}; break;
		case transformation_editor_pos_form_ID:
		{
			temp_transformation_editor->pos_form = w;
		}; break;
		case transformation_editor_dir_form_ID:
		{
			temp_transformation_editor->dir_form = w;
		}; break;
		case transformation_editor_coord_ID:
		{
			temp_transformation_editor->coord_form = w;
		}; break;
		case transformation_editor_pos_menu_ID:
		{
			temp_transformation_editor->pos_menu = w;
		}; break;
		case transformation_editor_dir_menu_ID:
		{
			temp_transformation_editor->dir_menu = w;
		}; break;
		case transformation_editor_name_ID:
		{
			temp_transformation_editor->name = w;
		}; break;
		default:
		{
			display_message(WARNING_MESSAGE,
				"transformation_editor_identify_button.  Invalid button number");
		}; break;
	}
	LEAVE;
} /* transformation_editor_identify_button */

static void transformation_editor_destroy_CB(Widget w,int *tag,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 4 November 1995

DESCRIPTION :
Callback for the transformation_editorment dialog - tidies up all details -
mem etc
==============================================================================*/
{
	struct Transformation_editor_struct *temp_transformation_editor;

	ENTER(transformation_editor_destroy_CB);
	USE_PARAMETER(reason);
	USE_PARAMETER(tag);
	/* Get the pointer to the data for the transformation_editor widget */
	XtVaGetValues(w,XmNuserData,&temp_transformation_editor,NULL);
	/* deallocate the memory for the user data */
	printf("De-allocating memory.\n");
	*(temp_transformation_editor->widget_address) = (Widget)NULL;
	DEALLOCATE(temp_transformation_editor);
	LEAVE;
} /* transformation_editor_destroy_CB */

/*
Global Functions
----------------
*/
Widget create_transformation_editor_widget(Widget *transformation_editor_widget,
	Widget parent,struct Cmgui_coordinate *init_data)
/*******************************************************************************
LAST MODIFIED : 13 December 1996

DESCRIPTION :
Creates a transformation_editor widget that gets a position and orientation
from the user.
==============================================================================*/
{
	XmString temp_label;
	int i,init_widgets;
	MrmType transformation_editor_dialog_class;
	struct Callback_data callback;
	struct Transformation_editor_struct *temp_transformation_editor = NULL;
	static MrmRegisterArg callback_list[]=
	{
		{"trans_editor_identify_button",
		(XtPointer)transformation_editor_identify_button},
		{"trans_editor_destroy_CB",
		(XtPointer)transformation_editor_destroy_CB}
	};
	static MrmRegisterArg identifier_list[] =
	{
		{"trans_editor_structure",(XtPointer)NULL},
		{"trans_editor_menu_ID",(XtPointer)transformation_editor_menu_ID},
		{"trans_editor_pos_form_ID",(XtPointer)transformation_editor_pos_form_ID},
		{"trans_editor_dir_form_ID",(XtPointer)transformation_editor_dir_form_ID},
		{"trans_editor_coord_ID",(XtPointer)transformation_editor_coord_ID},
		{"trans_editor_pos_menu_ID",(XtPointer)transformation_editor_pos_menu_ID},
		{"trans_editor_dir_menu_ID",(XtPointer)transformation_editor_dir_menu_ID},
		{"trans_editor_name_ID",(XtPointer)transformation_editor_name_ID}
	};
	Widget return_widget;

	ENTER(create_transformation_editor_widget);
	return_widget = (Widget)NULL;
	if (MrmOpenHierarchy_base64_string(transformation_editor_uidh,
		&transformation_editor_hierarchy,&transformation_editor_hierarchy_open))
	{
		/* allocate memory */
		if (ALLOCATE(temp_transformation_editor,struct Transformation_editor_struct,
			1))
		{
			/* initialise the structure */
			temp_transformation_editor->widget_parent = parent;
			temp_transformation_editor->widget = (Widget)NULL;
			temp_transformation_editor->widget_address = transformation_editor_widget;
			temp_transformation_editor->name = (Widget)NULL;
			temp_transformation_editor->menu = (Widget)NULL;
			temp_transformation_editor->pos_form = (Widget)NULL;
			temp_transformation_editor->dir_form = (Widget)NULL;
			temp_transformation_editor->pos_menu = (Widget)NULL;
			temp_transformation_editor->dir_menu = (Widget)NULL;
			temp_transformation_editor->coord_form = (Widget)NULL;
			temp_transformation_editor->position_widget = (Widget)NULL;
			temp_transformation_editor->direction_widget = (Widget)NULL;
			temp_transformation_editor->posctrl_widget = (Widget)NULL;
			temp_transformation_editor->dirctrl_widget = (Widget)NULL;
			temp_transformation_editor->input_widget = (Widget)NULL;
			temp_transformation_editor->parent_coordinate=
				(struct Cmgui_coordinate *)NULL;
			if (init_data)
			{
				for (i=0;i<3;i++)
				{
					temp_transformation_editor->current_value.origin.position.data[i] =
						init_data->origin.position.data[i];
					temp_transformation_editor->current_value.origin.direction.data[i] =
						init_data->origin.direction.data[i];
				}
				temp_transformation_editor->current_value.name=init_data->name;
			}
			else
			{
				for (i=0;i<3;i++)
				{
					temp_transformation_editor->current_value.origin.position.data[i]=0;
					temp_transformation_editor->current_value.origin.direction.data[i]=0;
				}
				temp_transformation_editor->current_value.name=(char *)NULL;
			}
			temp_transformation_editor->current_value.access_count = 0;
			for(i=0;i<TRANSFORMATION_EDITOR_NUM_CALLBACKS;i++)
			{
				temp_transformation_editor->callback_array[i].procedure =
					(Callback_procedure *)NULL;
				temp_transformation_editor->callback_array[i].data = NULL;
			}
			/* register the callbacks */
			if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
				transformation_editor_hierarchy,callback_list,XtNumber(callback_list)))
			{
				/* assign and register the identifiers */
				identifier_list[0].value=(XtPointer)temp_transformation_editor;
				if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
					transformation_editor_hierarchy,identifier_list,
					XtNumber(identifier_list)))
				{
					/* fetch transformation_editor window widget */
					if (MrmSUCCESS==MrmFetchWidget(transformation_editor_hierarchy,
						"trans_editor_widget",temp_transformation_editor->widget_parent,
						&(temp_transformation_editor->widget),
						&transformation_editor_dialog_class))
					{
						XtManageChild(temp_transformation_editor->widget);
						/* set the mode toggle to the correct transformation_editor */
						init_widgets = 1;
						/* do we want the dof3 widgets to be relative or absolute */
						if (!create_dof3_widget(
							&temp_transformation_editor->position_widget,
							temp_transformation_editor->pos_form,DOF3_POSITION,
							DOF3_ABSOLUTE,CONV_RECTANGULAR_CARTESIAN,
							&temp_transformation_editor->current_value.origin.position))
						{
							display_message(ERROR_MESSAGE,
"create_transformation_editor_widget.  Could not create position dof3 widget.");
							init_widgets = 0;
						}
						if (!create_dof3_widget(
							&(temp_transformation_editor->direction_widget),
							temp_transformation_editor->dir_form,DOF3_DIRECTION,DOF3_ABSOLUTE,
							CONV_DIR_EULER,
							&(temp_transformation_editor->current_value.origin.direction)))
						{
							display_message(ERROR_MESSAGE,
"create_transformation_editor_widget.  Could not create direction dof3 widget.");
							init_widgets = 0;
						}
#if defined (EXT_INPUT)
						if (!(temp_transformation_editor->input_widget=create_input_widget(
							temp_transformation_editor->menu,
							temp_transformation_editor->widget)))
						{
							display_message(ERROR_MESSAGE,
								"create_transformation_editor_widget.  Could not create input widget.");
							init_widgets = 0;
						}
#endif
						if (!(temp_transformation_editor->posctrl_widget=
							create_control_widget(temp_transformation_editor->pos_menu,
							"Control")))
						{
							display_message(ERROR_MESSAGE,
"create_transformation_editor_widget.  Could not create position control widget.");
							init_widgets = 0;
						}
						if (!(temp_transformation_editor->dirctrl_widget=
							create_control_widget(temp_transformation_editor->dir_menu,
							"Control")))
						{
							display_message(ERROR_MESSAGE,
"create_transformation_editor_widget.  Could not create direction control widget.");
							init_widgets = 0;
						}
						if (!(temp_transformation_editor->coord_widget=create_coord_widget(
							temp_transformation_editor->coord_form)))
						{
							display_message(ERROR_MESSAGE,
				"create_transformation_editor_widget.  Could not create coord widget.");
							init_widgets = 0;
						}
						if (init_widgets)
						{
#if defined (EXT_INPUT)
							/* now link all the widgets together */
							input_set_data(temp_transformation_editor->input_widget,
								INPUT_POSITION_WIDGET,
								temp_transformation_editor->position_widget);
							input_set_data(temp_transformation_editor->input_widget,
								INPUT_DIRECTION_WIDGET,
								temp_transformation_editor->direction_widget);
							input_set_data(temp_transformation_editor->input_widget,
								INPUT_POSCTRL_WIDGET,
								temp_transformation_editor->posctrl_widget);
							input_set_data(temp_transformation_editor->input_widget,
								INPUT_DIRCTRL_WIDGET,
								temp_transformation_editor->dirctrl_widget);
#endif
							control_set_data(temp_transformation_editor->posctrl_widget,
								CONTROL_DOF3_WIDGET,
								temp_transformation_editor->position_widget);
							control_set_data(temp_transformation_editor->dirctrl_widget,
								CONTROL_DOF3_WIDGET,
								temp_transformation_editor->direction_widget);
							/* tell the direction that it has a transformation_editor
								sibling */
							dof3_set_data(temp_transformation_editor->direction_widget,
								DOF3_POSITION_WIDGET,
								temp_transformation_editor->position_widget);
							/* get the global coordinate system */
								/*???GMH.  a bit of a hack at the moment */
							temp_transformation_editor->parent_coordinate=
								global_coordinate_ptr;
							coord_set_data(temp_transformation_editor->coord_widget,
								COORD_COORD_DATA,global_coordinate_ptr);
							callback.procedure = transformation_editor_update_coord;
							callback.data = temp_transformation_editor;
							coord_set_data(temp_transformation_editor->coord_widget,
								COORD_UPDATE_CB,&callback);
							callback.procedure = transformation_editor_update_position;
							callback.data = temp_transformation_editor;
							dof3_set_data(temp_transformation_editor->position_widget,
								DOF3_UPDATE_CB,
								&callback);
							callback.procedure = transformation_editor_update_direction;
							callback.data = temp_transformation_editor;
							dof3_set_data(temp_transformation_editor->direction_widget,
								DOF3_UPDATE_CB,&callback);
							if (temp_transformation_editor->current_value.name)
							{
								temp_label=XmStringCreateSimple(
									temp_transformation_editor->current_value.name);
							}
							else
							{
								temp_label=XmStringCreateSimple(" ");
							}
							XtVaSetValues(temp_transformation_editor->name,XmNlabelString,
								temp_label,NULL);
							XmStringFree(temp_label);
							return_widget = temp_transformation_editor->widget;
						}
						else
						{
							DEALLOCATE(temp_transformation_editor);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
"create_transformation_editor_widget.  Could not fetch transformation_editor dialog");
						DEALLOCATE(temp_transformation_editor);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
				"create_transformation_editor_widget.  Could not register identifiers");
					DEALLOCATE(temp_transformation_editor);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_transformation_editor_widget.  Could not register callbacks");
				DEALLOCATE(temp_transformation_editor);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
"create_transformation_editor_widget.  Could not allocate transformation_editor widget structure");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_transformation_editor_widget.  Could not open hierarchy");
	}
	*transformation_editor_widget = return_widget;
	LEAVE;

	return (return_widget);
} /* create_transformation_editor_widget */

int transformation_editor_set_data(Widget transformation_editor_widget,
	enum Transformation_editor_data_type data_type,void *data)
/*******************************************************************************
LAST MODIFIED : 4 November 1995

DESCRIPTION :
Changes a data item of the transformation_editor widget.
==============================================================================*/
{
	int i,return_code;
	struct Transformation_editor_struct *temp_transformation_editor;
	XmString temp_label;

	ENTER(transformation_editor_set_data);
	/* Get the pointer to the data for the transformation_editor dialog */
	XtVaGetValues(transformation_editor_widget,
		XmNuserData,&temp_transformation_editor,
		NULL);
	switch(data_type)
	{
		case TRANSFORMATION_EDITOR_UPDATE_CB:
		{
			temp_transformation_editor->callback_array[TRANSFORMATION_EDITOR_UPDATE_CB].
				procedure = ((struct Callback_data *)data)->procedure;
			temp_transformation_editor->callback_array[TRANSFORMATION_EDITOR_UPDATE_CB].
				data = ((struct Callback_data *)data)->data;
			return_code = 1;
		}; break;
		case TRANSFORMATION_EDITOR_DATA:
		{
			for(i=0;i<3;i++)
			{
				temp_transformation_editor->current_value.origin.position.data[i] =
					((struct Cmgui_coordinate *)data)->origin.position.data[i];
				temp_transformation_editor->current_value.origin.direction.data[i] =
					((struct Cmgui_coordinate *)data)->origin.direction.data[i];
			}
			temp_transformation_editor->current_value.name=
				((struct Cmgui_coordinate *)data)->name;
			if (temp_transformation_editor->current_value.name)
			{
				temp_label=XmStringCreateSimple(
					temp_transformation_editor->current_value.name);
			}
			else
			{
				temp_label=XmStringCreateSimple(" ");
			}
			XtVaSetValues(temp_transformation_editor->name,XmNlabelString,
				temp_label,NULL);
			XmStringFree(temp_label);
			dof3_set_data(temp_transformation_editor->position_widget,DOF3_DATA,
				&temp_transformation_editor->current_value.origin.position);
			dof3_set_data(temp_transformation_editor->direction_widget,DOF3_DATA,
				&temp_transformation_editor->current_value.origin.direction);
			/* we must change to the global coord system */
			temp_transformation_editor->parent_coordinate = global_coordinate_ptr;
			coord_set_data(temp_transformation_editor->coord_widget,COORD_COORD_DATA,
				temp_transformation_editor->parent_coordinate);
			return_code = 1;
		}; break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"transformation_editor_set_data.  Invalid data type.");
			return_code = 0;
		}; break;
	}
	LEAVE;

	return (return_code);
} /* transformation_editor_set_data */

void *transformation_editor_get_data(Widget transformation_editor_widget,
	enum Transformation_editor_data_type data_type)
/*******************************************************************************
LAST MODIFIED : 4 November 1995

DESCRIPTION :
Changes a data item of the transformation_editor widget.
==============================================================================*/
{
	void *return_code;
	struct Transformation_editor_struct *temp_transformation_editor;
	static struct Callback_data dat_callback;

	ENTER(transformation_editor_get_data);
	/* Get the pointer to the data for the transformation_editor dialog */
	XtVaGetValues(transformation_editor_widget,XmNuserData,&temp_transformation_editor,
		NULL);
	switch(data_type)
	{
		case TRANSFORMATION_EDITOR_UPDATE_CB:
		{
			dat_callback.procedure = temp_transformation_editor->
				callback_array[TRANSFORMATION_EDITOR_UPDATE_CB].procedure;
			dat_callback.data = temp_transformation_editor->
				callback_array[TRANSFORMATION_EDITOR_UPDATE_CB].data;
			return_code = &dat_callback;
		}; break;
		case TRANSFORMATION_EDITOR_DATA:
		{
			return_code = &temp_transformation_editor->current_value;
		}; break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"transformation_editor_get_data.  Invalid data type.");
			return_code = NULL;
		}; break;
	}
	LEAVE;

	return (return_code);
} /* transformation_editor_get_data */
