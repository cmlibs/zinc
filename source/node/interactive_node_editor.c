/*******************************************************************************
FILE : interactive_node_editor.c

LAST MODIFIED : 2 November 1998

DESCRIPTION :
This module creates a free interactive_node_editor input device, using two dof3,
two control and one input widget.  The position is given relative to some
coordinate system, and the returned value is a global one.
==============================================================================*/
#include <math.h>
#include <stdio.h>
#include "dof3/dof3.h"
#include "dof3/dof3_control.h"
#if defined (EXT_INPUT)
#include "dof3/dof3_input.h"
#endif
#include "general/debug.h"
#include "node/interactive_node_editor.h"
#include "node/interactive_node_editor.uid64"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"
#include "view/coord.h"
#include "view/coord_trans.h"

/*
Module variables
----------------
*/
#if defined (MOTIF)
static int interactive_node_editor_hierarchy_open=0;
static MrmHierarchy interactive_node_editor_hierarchy;
#endif /* defined (MOTIF) */

/*
Module functions
----------------
*/
#if defined (OLD_CODE)
void get_node_position(struct FE_node *node,struct Dof3_data *position,
	enum Conv_coordinate_system coord_sys)
/*******************************************************************************
LAST MODIFIED : 26 September 1995

DESCRIPTION :
Returns the global position of the node in the specified coordinate system
==============================================================================*/
{
	struct FE_node_field_component *coordinate_node_field_component;
	enum Coordinate_system_type coordinate_system_type;
	int i,number_of_coordinate_components;
	struct FE_node_field *coordinate_node_field;
	struct FE_node_field_info *node_field_information;

	ENTER(get_node_position);
	if(position)
	{
		for(i=0;i<3;i++)
		{
			position->data[i] = 0.0;
		}
		if(node&&(node_field_information=node->fields))
		{
			coordinate_node_field=FIRST_OBJECT_IN_LIST_THAT(FE_node_field)(
				(LIST_CONDITIONAL_FUNCTION(FE_node_field) *)NULL,(void *)NULL,
				node_field_information->coordinate_node_field_list);
			if (coordinate_node_field&&node->values)
			{
				coordinate_system=coordinate_node_field->field->coordinate_system;
				if((coordinate_system==RECTANGULAR_CARTESIAN)&&
					(coord_sys==CONV_RECTANGULAR_CARTESIAN))
				{
					number_of_coordinate_components=
						coordinate_node_field->field->number_of_components;
					coordinate_node_field_component=coordinate_node_field->components;
					position->data[0]=node->values[coordinate_node_field_component->value];
					if (1<number_of_coordinate_components)
					{
						coordinate_node_field_component++;
						position->data[1]=node->values[coordinate_node_field_component->value];
						if (2<number_of_coordinate_components)
						{
							coordinate_node_field_component++;
							position->data[2]=node->values[coordinate_node_field_component->value];
						}
						else
						{
							position->data[2]=0.0;
						}
					}
					else
					{
						position->data[1]=0.0;
						position->data[2]=0.0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"get_node_position.  Coordinate system not implemented");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"get_node_position.  Invalid coordinate node field");
			}

		}
		else
		{
			display_message(ERROR_MESSAGE,
				"get_node_position.  Invalid node field information");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_node_position.  Invalid arguments");
	}
	LEAVE;
}
void set_node_position(struct FE_node *node,struct Dof3_data *position,
	enum Conv_coordinate_system coord_sys)
/*******************************************************************************
LAST MODIFIED : 26 September 1995

DESCRIPTION :
Returns the global position of the node in the specified coordinate system
==============================================================================*/
{
	struct FE_node_field_component *coordinate_node_field_component;
	enum Coordinate_system_type coordinate_system_type;
	int number_of_coordinate_components;
	struct FE_node_field *coordinate_node_field;
	struct FE_node_field_info *node_field_information;

	ENTER(set_node_position);
	if(position)
	{
		if(node&&(node_field_information=node->fields))
		{
			coordinate_node_field=FIRST_OBJECT_IN_LIST_THAT(FE_node_field)(
				(LIST_CONDITIONAL_FUNCTION(FE_node_field) *)NULL,(void *)NULL,
				node_field_information->coordinate_node_field_list);
			if (coordinate_node_field&&node->values)
			{
				coordinate_system=coordinate_node_field->field->coordinate_system;
				if((coordinate_system==RECTANGULAR_CARTESIAN)&&
					(coord_sys==CONV_RECTANGULAR_CARTESIAN))
				{
					number_of_coordinate_components=
						coordinate_node_field->field->number_of_components;
					coordinate_node_field_component=coordinate_node_field->components;
					node->values[coordinate_node_field_component->value]=position->data[0];
					if (1<number_of_coordinate_components)
					{
						coordinate_node_field_component++;
						node->values[coordinate_node_field_component->value]=position->data[1];
						if (2<number_of_coordinate_components)
						{
							coordinate_node_field_component++;
							node->values[coordinate_node_field_component->value]=position->data[2];
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_node_position.  Coordinate system not implemented");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"set_node_position.  Invalid coordinate node field");
			}

		}
		else
		{
			display_message(ERROR_MESSAGE,
				"set_node_position.  Invalid node field information");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_node_position.  Invalid arguments");
	}
	LEAVE;
}
#endif /* defined (OLD_CODE) */

static void interactive_node_editor_update(struct Interactive_node_editor_struct *temp_interactive_node_editor)
/*******************************************************************************
LAST MODIFIED : 26 September 1995

DESCRIPTION :
Tells CMGUI about the current values.
==============================================================================*/
{
	ENTER(interactive_node_editor_update);
	if (temp_interactive_node_editor->
		callback_array[INTERACTIVE_NODE_EDITOR_UPDATE_CB].procedure)
	{
		if (temp_interactive_node_editor->global_value==
			temp_interactive_node_editor->default_value)
		{
			/* now call the procedure with the user data and a NULL pointer */
			(temp_interactive_node_editor->
				callback_array[INTERACTIVE_NODE_EDITOR_UPDATE_CB].procedure)
				(temp_interactive_node_editor->widget,
				temp_interactive_node_editor->
					callback_array[INTERACTIVE_NODE_EDITOR_UPDATE_CB].data,
				NULL);
		}
		else
		{
			/* now call the procedure with the user data and a pointer to the node */
			(((temp_interactive_node_editor->callback_array)[INTERACTIVE_NODE_EDITOR_UPDATE_CB]).
				procedure)(temp_interactive_node_editor->widget,
				temp_interactive_node_editor->
					callback_array[INTERACTIVE_NODE_EDITOR_UPDATE_CB].data,
				temp_interactive_node_editor->current_value);
		}
	}
	LEAVE;
} /* interactive_node_editor_update */

static void interactive_node_editor_global_object_change(
	struct MANAGER_MESSAGE(FE_node) *message,void *data)
/*******************************************************************************
LAST MODIFIED : 16 February 1997

DESCRIPTION :
Something has changed globally about the objects this widget uses, so refresh.
==============================================================================*/
{
	struct Interactive_node_editor_struct *temp_interactive_node_editor=data;

	ENTER(interactive_node_editor_global_object_change);
	if (message)
	{
		switch (message->change)
		{
			case MANAGER_CHANGE_ALL(FE_node):
			{
				if (IS_MANAGED(FE_node)(temp_interactive_node_editor->global_value,
					temp_interactive_node_editor->manager))
				{
					interactive_node_editor_set_data(temp_interactive_node_editor->widget,
						INTERACTIVE_NODE_EDITOR_DATA,
						temp_interactive_node_editor->global_value);
				}
				else
				{
					interactive_node_editor_set_data(temp_interactive_node_editor->widget,
						INTERACTIVE_NODE_EDITOR_DATA,FIRST_OBJECT_IN_MANAGER_THAT(FE_node)(
						(MANAGER_CONDITIONAL_FUNCTION(FE_node) *)NULL,(void *)NULL,
						temp_interactive_node_editor->manager));
				}
			}; break;
			case MANAGER_CHANGE_DELETE(FE_node):
			{
				if (message->object_changed==temp_interactive_node_editor->global_value)
				{
					interactive_node_editor_set_data(temp_interactive_node_editor->widget,
						INTERACTIVE_NODE_EDITOR_DATA,FIRST_OBJECT_IN_MANAGER_THAT(FE_node)(
						(MANAGER_CONDITIONAL_FUNCTION(FE_node) *)NULL,(void *)NULL,
						temp_interactive_node_editor->manager));
				}
			}; break;
			case MANAGER_CHANGE_ADD(FE_node):
			{
				if (NULL==temp_interactive_node_editor->global_value)
				{
					interactive_node_editor_set_data(temp_interactive_node_editor->widget,
						INTERACTIVE_NODE_EDITOR_DATA,message->object_changed);
				}
			}; break;
			case MANAGER_CHANGE_IDENTIFIER(FE_node):
			case MANAGER_CHANGE_OBJECT(FE_node):
			case MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(FE_node):
			{
				if (message->object_changed==temp_interactive_node_editor->global_value)
				{
					interactive_node_editor_set_data(temp_interactive_node_editor->widget,
						INTERACTIVE_NODE_EDITOR_DATA,message->object_changed);
				}
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"interactive_node_editor_global_object_change.  Missing message");
	}
	LEAVE;
} /* interactive_node_editor_global_object_change */

static void interactive_node_editor_update_coord(Widget coord_widget,
	void *user_data,void *temp_coordinate)
/*******************************************************************************
LAST MODIFIED : 2 November 1998

DESCRIPTION :
Tells CMGUI about the current values.
==============================================================================*/
{
	FE_value node_x,node_y,node_z;
	struct Interactive_node_editor_struct
		*temp_interactive_node_editor = user_data;
	struct Cmgui_coordinate *coordinate = temp_coordinate;
	struct Dof3_data new_dof3,node_position;

	ENTER(interactive_node_editor_update_coord);
	temp_interactive_node_editor->current_coordinate = coordinate;
	/* get the position of the node */
	FE_node_get_position_cartesian(temp_interactive_node_editor->current_value,
		(struct FE_field *)NULL,&node_x,&node_y,&node_z,(FE_value *)NULL);
	(node_position.data)[0]=node_x;
	(node_position.data)[1]=node_y;
	(node_position.data)[2]=node_z;
	get_local_position(&node_position,
		coordinate,&new_dof3);
	dof3_set_data(temp_interactive_node_editor->position_widget,DOF3_DATA,
		&new_dof3);
	dof3_set_data(temp_interactive_node_editor->position_widget,DOF3_DATA,&new_dof3);
	LEAVE;
} /* interactive_node_editor_update_coord */

static void interactive_node_editor_update_position(Widget position_widget,
	void *user_data,
	void *temp_dof3)
/*******************************************************************************
LAST MODIFIED : 2 November 1998

DESCRIPTION :
Receives an update from the dof3 position widget.
==============================================================================*/
{
	struct Interactive_node_editor_struct *temp_interactive_node_editor = user_data;
	struct Dof3_data *temp_position = temp_dof3,node_position;

	ENTER(interactive_node_editor_update_position);
	/* put the information into the current position */
	get_global_position(temp_position,
		temp_interactive_node_editor->current_coordinate,
		&node_position);
	FE_node_set_position_cartesian(temp_interactive_node_editor->current_value,
		(struct FE_field *)NULL,(FE_value)(node_position.data)[0],
		(FE_value)(node_position.data)[1],(FE_value)(node_position.data)[2]);
	interactive_node_editor_update(temp_interactive_node_editor);
	LEAVE;
} /* interactive_node_editor_update_position */

static void interactive_node_editor_identify_button(Widget w,int button_num,unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 4 November 1995

DESCRIPTION :
Finds the id of the buttons on the interactive_node_editor widget.
==============================================================================*/
{
	struct Interactive_node_editor_struct *temp_interactive_node_editor;

	ENTER(interactive_node_editor_identify_button);
	/* find out which interactive_node_editor widget we are in */
	XtVaGetValues(w,XmNuserData,&temp_interactive_node_editor,NULL);
	switch (button_num)
	{
		case interactive_node_editor_menu_ID:
		{
			temp_interactive_node_editor->menu = w;
		} break;
		case interactive_node_editor_position_form_ID:
		{
			temp_interactive_node_editor->position_form = w;
		} break;
		case interactive_node_editor_coord_form_ID:
		{
			temp_interactive_node_editor->coord_form = w;
		} break;
		default:
		{
			display_message(WARNING_MESSAGE,
				"interactive_node_editor_identify_button.  Invalid button number");
		} break;
	}
	LEAVE;
} /* interactive_node_editor_identify_button */

static void interactive_node_editor_destroy_CB(Widget widget,
	int *tag,unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 4 November 1995

DESCRIPTION :
Callback for the interactive_node_editorment dialog - tidies up all details - mem etc
==============================================================================*/
{
	struct Interactive_node_editor_struct *temp_interactive_node_editor;

	ENTER(interactive_node_editor_destroy_CB);
	if (widget)
	{
		/* Get the pointer to the data for the interactive_node_editor widget */
		XtVaGetValues(widget,XmNuserData,&temp_interactive_node_editor,NULL);
		if (temp_interactive_node_editor)
		{
			*(temp_interactive_node_editor->widget_address) = (Widget)NULL;
			MANAGER_DEREGISTER(FE_node)(temp_interactive_node_editor->manager_callback_id,
				temp_interactive_node_editor->manager);
			/* deallocate the memory for the user data */
			/* deallocate the memory for the user data */
			if (temp_interactive_node_editor->current_value!=temp_interactive_node_editor->default_value)
			{
				DESTROY(FE_node)(&(temp_interactive_node_editor->current_value));
			}
			DESTROY(FE_node)(&(temp_interactive_node_editor->default_value));
			DEALLOCATE(temp_interactive_node_editor);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"interactive_node_editor_destroy_CB.  Missing node_editor");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"interactive_node_editor_destroy_CB.  Missing widget");
	}
	LEAVE;
} /* interactive_node_editor_destroy_CB */

/*
Global Functions
----------------
*/
Widget create_interactive_node_editor_widget(
	Widget *interactive_node_editor_widget,Widget parent,
	struct MANAGER(FE_node) *manager,struct FE_node *init_data)
/*******************************************************************************
LAST MODIFIED : 2 November 1998

DESCRIPTION :
Creates a interactive_node_editor widget that gets a position from the user.
==============================================================================*/
{
	FE_value node_x,node_y,node_z;
	int i,init_widgets;
	MrmType interactive_node_editor_dialog_class;
	struct Callback_data callback;
	struct Interactive_node_editor_struct *temp_interactive_node_editor = NULL;
	static MrmRegisterArg callback_list[]=
	{
		{"i_node_editor_identify_button",
		(XtPointer)interactive_node_editor_identify_button},
		{"i_node_editor_destroy_CB",
		(XtPointer)interactive_node_editor_destroy_CB}
	};
	static MrmRegisterArg identifier_list[] =
	{
		{"I_node_editor_structure",(XtPointer)NULL},
		{"i_node_editor_menu_ID",
		(XtPointer)interactive_node_editor_menu_ID},
		{"i_node_editor_position_form_ID",
		(XtPointer)interactive_node_editor_position_form_ID},
		{"i_node_editor_coord_form_ID",
		(XtPointer)interactive_node_editor_coord_form_ID}
	};
	struct Dof3_data node_position;
	Widget return_widget;

	ENTER(create_interactive_node_editor_widget);
	return_widget = (Widget)NULL;
	if (manager&&interactive_node_editor_widget&&parent)
	{
	if (MrmOpenHierarchy_base64_string(interactive_node_editor_uid64,
		&interactive_node_editor_hierarchy,
		&interactive_node_editor_hierarchy_open))
	{
		/* allocate memory */
		if (ALLOCATE(temp_interactive_node_editor,struct Interactive_node_editor_struct,1)&&
				(temp_interactive_node_editor->default_value=
				CREATE(FE_node)(0,(struct FE_node *)NULL))&&
				(temp_interactive_node_editor->current_value=
				CREATE(FE_node)(0,(struct FE_node *)NULL)))
		{
			/* initialise the structure */
			temp_interactive_node_editor->manager = manager;
			temp_interactive_node_editor->manager_callback_id = (void *)NULL;
			temp_interactive_node_editor->widget_parent = parent;
			temp_interactive_node_editor->widget = (Widget)NULL;
			temp_interactive_node_editor->widget_address = interactive_node_editor_widget;
			temp_interactive_node_editor->menu = (Widget)NULL;
			temp_interactive_node_editor->position_form = (Widget)NULL;
			temp_interactive_node_editor->coord_form = (Widget)NULL;
			temp_interactive_node_editor->current_coordinate = (struct Cmgui_coordinate *)NULL;
			if (init_data)
			{
				temp_interactive_node_editor->global_value=init_data;
				if (!IS_MANAGED(FE_node)(temp_interactive_node_editor->global_value,
					temp_interactive_node_editor->manager))
				{
					display_message(ERROR_MESSAGE,
						"create_interactive_node_editor_widget.  Invalid FE_node.");
				}
			}
			else
			{
				temp_interactive_node_editor->global_value=FIRST_OBJECT_IN_MANAGER_THAT(FE_node)(
					(MANAGER_CONDITIONAL_FUNCTION(FE_node) *)NULL,(void *)NULL,
					temp_interactive_node_editor->manager);
				if (!temp_interactive_node_editor->global_value)
				{
					temp_interactive_node_editor->global_value=
						temp_interactive_node_editor->default_value;
				}
			}
			/* copy to the working node */
			overwrite_FE_node_with_cm_node_identifier(temp_interactive_node_editor->current_value,
				temp_interactive_node_editor->global_value);
			/* get the position of the node */
			FE_node_get_position_cartesian(
				temp_interactive_node_editor->current_value,(struct FE_field *)NULL,
				&node_x,&node_y,&node_z,(FE_value *)NULL);
			(node_position.data)[0]=node_x;
			(node_position.data)[1]=node_y;
			(node_position.data)[2]=node_z;
			for (i=0;i<INTERACTIVE_NODE_EDITOR_NUM_CALLBACKS;i++)
			{
				temp_interactive_node_editor->callback_array[i].procedure =
					(Callback_procedure *)NULL;
				temp_interactive_node_editor->callback_array[i].data = NULL;
			}
			/* register the callbacks */
			if (MrmSUCCESS==MrmRegisterNamesInHierarchy(interactive_node_editor_hierarchy,
				callback_list,
				XtNumber(callback_list)))
			{
				/* assign and register the identifiers */
				identifier_list[0].value=(XtPointer)temp_interactive_node_editor;
				if (MrmSUCCESS==MrmRegisterNamesInHierarchy(interactive_node_editor_hierarchy,
					identifier_list,XtNumber(identifier_list)))
				{
					/* fetch position window widget */
					if (MrmSUCCESS==MrmFetchWidget(interactive_node_editor_hierarchy,
						"i_node_editor_widget",
						temp_interactive_node_editor->widget_parent,
						&(temp_interactive_node_editor->widget),
						&interactive_node_editor_dialog_class))
					{
						XtManageChild(temp_interactive_node_editor->widget);
						/* set the mode toggle to the correct position */
						init_widgets = 1;
						if (!create_dof3_widget(&(temp_interactive_node_editor->position_widget),
							temp_interactive_node_editor->position_form,DOF3_POSITION,DOF3_ABSOLUTE,
							CONV_RECTANGULAR_CARTESIAN,&node_position))
						{
							display_message(ERROR_MESSAGE,
								"create_interactive_node_editor_widget.  Could not create position dof3 widget.");
							init_widgets = 0;
						}
#if defined (EXT_INPUT)
						if (!(temp_interactive_node_editor->input_widget = create_input_widget(
							temp_interactive_node_editor->menu,
							temp_interactive_node_editor->position_form)))
						{
							display_message(ERROR_MESSAGE,
								"create_interactive_node_editor_widget.  Could not create input widget");
							init_widgets = 0;
						}
#endif
						if (!(temp_interactive_node_editor->control_widget = create_control_widget(
							temp_interactive_node_editor->menu,"Control")))
						{
							display_message(ERROR_MESSAGE,
								"create_interactive_node_editor_widget.  Could not create control widget");
							init_widgets = 0;
						}
						if (!(temp_interactive_node_editor->coord_widget = create_coord_widget(
							temp_interactive_node_editor->coord_form)))
						{
							display_message(ERROR_MESSAGE,
								"create_interactive_node_editor_widget.  Could not create coord widget.");
							init_widgets = 0;
						}
						if (init_widgets)
						{
#if defined (EXT_INPUT)
							/* now link all the widgets together */
							input_set_data(temp_interactive_node_editor->input_widget,
								INPUT_POSITION_WIDGET,temp_interactive_node_editor->position_widget);
#endif
							control_set_data(temp_interactive_node_editor->control_widget,
								CONTROL_DOF3_WIDGET,temp_interactive_node_editor->position_widget);
							/* get the global coordinate system */
								/*???GMH.  A bit of a hack at the moment */
							temp_interactive_node_editor->current_coordinate = global_coordinate_ptr;
							coord_set_data(temp_interactive_node_editor->coord_widget,
								COORD_COORD_DATA,
								temp_interactive_node_editor->current_coordinate);
							/* set coord callbacks */
							callback.procedure = interactive_node_editor_update_coord;
							callback.data = temp_interactive_node_editor;
							coord_set_data(temp_interactive_node_editor->coord_widget,COORD_UPDATE_CB,
								&callback);
							/* set dof3 callbacks */
							callback.procedure = interactive_node_editor_update_position;
							callback.data = temp_interactive_node_editor;
							dof3_set_data(temp_interactive_node_editor->position_widget,DOF3_UPDATE_CB,
								&callback);
							/* register for any changes */
							temp_interactive_node_editor->manager_callback_id=MANAGER_REGISTER(FE_node)(
								interactive_node_editor_global_object_change,temp_interactive_node_editor,
								temp_interactive_node_editor->manager);
							if (temp_interactive_node_editor->global_value==
								temp_interactive_node_editor->default_value)
							{
								XtUnmanageChild(temp_interactive_node_editor->widget);
							}
							return_widget = temp_interactive_node_editor->widget;
						}
						else
						{
							DEALLOCATE(temp_interactive_node_editor);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_interactive_node_editor_widget.  Could not fetch interactive_node_editor dialog");
						DEALLOCATE(temp_interactive_node_editor);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_interactive_node_editor_widget.  Could not register identifiers");
					DEALLOCATE(temp_interactive_node_editor);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_interactive_node_editor_widget.  Could not register callbacks");
				DEALLOCATE(temp_interactive_node_editor);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_interactive_node_editor_widget.  Could not allocate interactive_node_editor widget structure");
			if (temp_interactive_node_editor)
			{
				if (temp_interactive_node_editor->default_value)
				{
					DESTROY(FE_node)(&(temp_interactive_node_editor->default_value));
				}
				DEALLOCATE(temp_interactive_node_editor);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_interactive_node_editor_widget.  Could not open hierarchy");
	}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_interactive_node_editor_widget.  Invalid argument(s)");
	}
	if (interactive_node_editor_widget)
	{
		*interactive_node_editor_widget = return_widget;
	}
	LEAVE;

	return (return_widget);
} /* create_interactive_node_editor_widget */

int interactive_node_editor_set_data(Widget interactive_node_editor_widget,
	enum Interactive_node_editor_data_type data_type,void *data)
/*******************************************************************************
LAST MODIFIED : 2 November 1998

DESCRIPTION :
Changes a data item of the interactive_node_editor widget.
==============================================================================*/
{
	FE_value node_x,node_y,node_z;
	int return_code;
	struct Interactive_node_editor_struct *temp_interactive_node_editor;
	struct Dof3_data node_position;

	ENTER(interactive_node_editor_set_data);
	/* Get the pointer to the data for the interactive_node_editor dialog */
	XtVaGetValues(interactive_node_editor_widget,XmNuserData,
		&temp_interactive_node_editor,NULL);
	switch (data_type)
	{
		case INTERACTIVE_NODE_EDITOR_UPDATE_CB:
		{
			temp_interactive_node_editor->callback_array[INTERACTIVE_NODE_EDITOR_UPDATE_CB].procedure =
				((struct Callback_data *)data)->procedure;
			temp_interactive_node_editor->callback_array[INTERACTIVE_NODE_EDITOR_UPDATE_CB].data =
				((struct Callback_data *)data)->data;
			return_code = 1;
		} break;
		case INTERACTIVE_NODE_EDITOR_DATA:
		{
			if (temp_interactive_node_editor->global_value==
				temp_interactive_node_editor->default_value)
			{
				XtManageChild(temp_interactive_node_editor->widget);
			}
			if (data)
			{
				temp_interactive_node_editor->global_value=(struct FE_node *)data;
				if (!IS_MANAGED(FE_node)(temp_interactive_node_editor->global_value,
					temp_interactive_node_editor->manager))
				{
					display_message(ERROR_MESSAGE,
						"interactive_node_editor_set_data.  Invalid FE_node.");
				}
			}
			else
			{
				temp_interactive_node_editor->global_value=FIRST_OBJECT_IN_MANAGER_THAT(FE_node)(
					(MANAGER_CONDITIONAL_FUNCTION(FE_node) *)NULL,(void *)NULL,
					temp_interactive_node_editor->manager);
				if (!temp_interactive_node_editor->global_value)
				{
					temp_interactive_node_editor->global_value=
						temp_interactive_node_editor->default_value;
					XtUnmanageChild(temp_interactive_node_editor->widget);
				}
			}
			/* copy to the working node */
			overwrite_FE_node_with_cm_node_identifier(temp_interactive_node_editor->current_value,
				temp_interactive_node_editor->global_value);
			/* get the position of the node */
			FE_node_get_position_cartesian(
				temp_interactive_node_editor->current_value,(struct FE_field *)NULL,
				&node_x,&node_y,&node_z,(FE_value *)NULL);
			(node_position.data)[0]=node_x;
			(node_position.data)[1]=node_y;
			(node_position.data)[2]=node_z;
			dof3_set_data(temp_interactive_node_editor->position_widget,DOF3_DATA,
				&node_position);
			return_code = 1;
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"interactive_node_editor_set_data.  Invalid data type.");
			return_code = 0;
		} break;
	}
	LEAVE;

	return (return_code);
} /* interactive_node_editor_set_data */

void *interactive_node_editor_get_data(Widget interactive_node_editor_widget,
	enum Interactive_node_editor_data_type data_type)
/*******************************************************************************
LAST MODIFIED : 4 November 1995

DESCRIPTION :
Changes a data item of the interactive_node_editor widget.
==============================================================================*/
{
	void *return_code;
	struct Interactive_node_editor_struct *temp_interactive_node_editor;
	static struct Callback_data dat_callback;
	static Widget dat_widget;

	ENTER(interactive_node_editor_get_data);
	/* Get the pointer to the data for the interactive_node_editor dialog */
	XtVaGetValues(interactive_node_editor_widget,XmNuserData,
		&temp_interactive_node_editor,NULL);
	switch (data_type)
	{
		case INTERACTIVE_NODE_EDITOR_UPDATE_CB:
		{
			dat_callback.procedure =
				temp_interactive_node_editor->callback_array[INTERACTIVE_NODE_EDITOR_UPDATE_CB].procedure;
			dat_callback.data = temp_interactive_node_editor->callback_array[INTERACTIVE_NODE_EDITOR_UPDATE_CB].data;
			return_code = &dat_callback;
		} break;
		case INTERACTIVE_NODE_EDITOR_DATA:
		{
			return_code = temp_interactive_node_editor->current_value;
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"interactive_node_editor_get_data.  Invalid data type.");
			return_code = NULL;
		} break;
	}
	LEAVE;

	return (return_code);
} /* interactive_node_editor_get_data */


void interactive_node_editor_modify_global(Widget interactive_node_editor_widget)
/*******************************************************************************
LAST MODIFIED : 28 September 1995

DESCRIPTION :
Makes the node change global.
==============================================================================*/
{
	struct Interactive_node_editor_struct *temp_interactive_node_editor;

	ENTER(interactive_node_editor_modify_global);
	/* Get the pointer to the data for the node_editor dialog */
	XtVaGetValues(interactive_node_editor_widget,XmNuserData,&temp_interactive_node_editor,NULL);
	if (temp_interactive_node_editor->global_value!=temp_interactive_node_editor->default_value)
	{
		MANAGER_MODIFY_NOT_IDENTIFIER(FE_node,cm_node_identifier)(temp_interactive_node_editor->
			global_value,temp_interactive_node_editor->current_value,
			temp_interactive_node_editor->manager);
	}
	LEAVE;
} /* interactive_node_editor_modify_global */
