/*******************************************************************************
FILE : data_2d.c

LAST MODIFIED : 31 August 2001

DESCRIPTION :
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
#include "data/data_2d.h"
#include "data/data_2d.uidh"
#include "finite_element/finite_element.h"
#include "general/debug.h"
#include "general/compare.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module constants
----------------
*/
#define COORDINATES_2D_FIELD_NAME "coordinates_2d_rc"

/*
Module variables
----------------
*/
#if defined (MOTIF)
static int data_2d_hierarchy_open=0;
static MrmHierarchy data_2d_hierarchy;
#endif /* defined (MOTIF) */
/*???GMH.  dont delete this hack (yet) */
#define HACK
#define HACK_MAX_SELECTED 10000
#if defined (HACK)
int global_selected[HACK_MAX_SELECTED];
int global_num_selected;
#endif /* HACK */

/*
Module functions
----------------
*/
static void d2_call_callback(struct D2_struct *temp_data_2d,
	enum D2_data_type callback_number,void *callback_data)
/*******************************************************************************
LAST MODIFIED : 25 September 1994

DESCRIPTION :
Calls a callback
==============================================================================*/
{
	ENTER(d2_call_callback);
	if ((callback_number>=0)&&(callback_number<DATA_2D_NUM_CALLBACKS))
	{
		if (temp_data_2d->callback_array[callback_number].procedure)
		{
			(temp_data_2d->callback_array[callback_number].procedure)
				(temp_data_2d->widget,
					temp_data_2d->callback_array[callback_number].data,
					callback_data);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"d2_call_callback.  Invalid callback number");
	}
	LEAVE;
} /* d2_call_callback */

#if defined (OLD_CODE)
static void d2_update_select(struct D2_struct *temp_data_2d)
/*******************************************************************************
LAST MODIFIED : 25 September 1995

DESCRIPTION :
A button has been pressed, so tell any clients.
==============================================================================*/
{
	ENTER(d2_update_select);
	if (temp_data_2d->callback_array[DATA_2D_SELECT_CB].procedure)
	{
		(temp_data_2d->callback_array[DATA_2D_SELECT_CB].procedure)
			(temp_data_2d->widget,
			temp_data_2d->callback_array[DATA_2D_SELECT_CB].data,
			&temp_data_2d->current_value);
	}
	LEAVE;
} /* d2_update_select */
#endif /* defined (OLD_CODE) */

static void d2_identify_button(Widget w,int button_num,unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 25 September 1995

DESCRIPTION :
Finds the id of the buttons on the data_2d widget.
==============================================================================*/
{
	struct D2_struct *temp_data_2d;

	ENTER(d2_identify_button);
	USE_PARAMETER(reason);
	XtVaGetValues(w,XmNuserData,&temp_data_2d,NULL);
	switch (button_num)
	{
		case data_2d_mode_ID:
		{
			temp_data_2d->mode_label=w;
		} break;
		case data_2d_datalist_ID:
		{
			temp_data_2d->data_list=w;
		} break;
		case data_2d_control_ID:
		{
			temp_data_2d->control=w;
		} break;
		case data_2d_control_digitise_ID:
		{
			temp_data_2d->control_digitise=w;
		} break;
		case data_2d_control_delete_ID:
		{
			temp_data_2d->control_delete=w;
		} break;
		case data_2d_control_move_ID:
		{
			temp_data_2d->control_move=w;
		} break;
		default:
		{
			display_message(WARNING_MESSAGE,
				"d2_identify_button.  Invalid button number");
		} break;
	}
	LEAVE;
} /* d2_identify_button */

static void d2_destroy_CB(Widget w,int *tag,unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 23 March 1999

DESCRIPTION :
Callback for the data_2d dialog - tidies up all memory allocation
==============================================================================*/
{
	struct D2_struct *temp_data_2d;

	ENTER(d2_destroy_CB);
	USE_PARAMETER(tag);
	USE_PARAMETER(reason);
	/* Get the pointer to the data for the data_2d dialog */
	XtVaGetValues(w,XmNuserData,&temp_data_2d,NULL);
	if (temp_data_2d)
	{
		if (temp_data_2d->data_2d_field)
		{
			DEACCESS(FE_field)(&(temp_data_2d->data_2d_field));
		}
		*(temp_data_2d->widget_address)=(Widget)NULL;
		MANAGER_DEREGISTER(FE_node)(temp_data_2d->manager_callback_id,
			temp_data_2d->node_manager);
		MANAGER_DEREGISTER(GROUP(FE_node))(temp_data_2d->group_manager_callback_id,
			temp_data_2d->group_manager);
	}
	else
	{
		display_message(WARNING_MESSAGE,"d2_destroy_CB.  Invalid arguments");
	}
	/* deallocate the memory for the user data */
	DEALLOCATE(temp_data_2d);
	LEAVE;
} /* d2_destroy_CB */

static int d2_show_mode(struct D2_struct *temp_data_2d)
/*******************************************************************************
LAST MODIFIED : 25 September 1995

DESCRIPTION :
Displays the current mode.
==============================================================================*/
{
	int return_code;
	XmString new_string;

	ENTER(d2_show_mode);
	return_code=1;
	switch (temp_data_2d->mode)
	{
		case DATA_2D_SELECT:
		{
			new_string=XmStringCreateSimple("Mode: Select");
		} break;
		case DATA_2D_DIGITISE:
		{
			new_string=XmStringCreateSimple("Mode: Digitise");
		} break;
		case DATA_2D_MOVE:
		{
			new_string=XmStringCreateSimple("Mode: Move");
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,"d2_show_mode.  Invalid mode");
			return_code=0;
		} break;
	}
	if (return_code)
	{
		XtVaSetValues(temp_data_2d->mode_label,XmNlabelString,new_string,NULL);
		XmStringFree(new_string);
	}
	LEAVE;

	return return_code;
} /* d2_show_mode */

#if defined (OLD_CODE)
static int d2_get_node_from_string(XmString string)
/*******************************************************************************
LAST MODIFIED : 25 September 1995

DESCRIPTION :
Checks if the current node is within the selection box, if so then it selects
it.
==============================================================================*/
{
	int return_node;
	char *text;
	XmStringContext context;
	XmStringCharSet charset;
	XmStringDirection direction;
	Boolean separator;

	ENTER(d2_get_node_from_string);
	return_node=0;
	if (XmStringInitContext(&context,string))
	{
		if (XmStringGetNextSegment(context,&text,&charset,&direction,&separator))
		{
			if (1!=sscanf(text," %i",&return_node))
			{
				return_node=0;
			}
			XtFree(text);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"d2_get_node_from_string.  Could not get segment");
		}
		XmStringFreeContext(context);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"d2_get_node_from_string.  Could not get context");
	}
	LEAVE;

	return return_node;
} /* d2_get_node_from_string */
#endif /* defined (OLD_CODE) */

static void d2_update_selected(struct D2_struct *temp_data_2d)
/*******************************************************************************
LAST MODIFIED : 19 January 1997

DESCRIPTION :
Goes through the list finding all selected items, and calls the select callback
with the list of selected points.
==============================================================================*/
{
#if !defined (HACK)
	int *selected_nodes,num_selected,i;
	XmString *selected_items;
#endif /* !defined (HACK) */
	struct Data_2d_select_data select_data;

	ENTER(d2_update_selected);
	if (temp_data_2d)
	{
#if defined (HACK)
		select_data.num_selected=global_num_selected;
		select_data.selected_nodes=global_selected;
#else /* HACK */
		XtVaGetValues(temp_data_2d->data_list,XmNselectedItemCount,&num_selected,
			XmNselectedItems,&selected_items,NULL);
		selected_nodes=(int *)NULL;
		if (num_selected>0)
		{
			/* use this as the number of selected items */
			if (ALLOCATE(selected_nodes,int,num_selected))
			{
				for (i=0;i<num_selected;i++)
				{
					/* get the node number at a given position */
					selected_nodes[i]=d2_get_node_from_string(selected_items[i]);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"d2_update_selected.  Could not create selected list");
			}
		}
		select_data.num_selected=num_selected;
		select_data.selected_nodes=selected_nodes;
#endif /* HACK */
		d2_call_callback(temp_data_2d,DATA_2D_SELECT_CB,&select_data);
#if !defined (HACK)
		if (selected_nodes)
		{
			DEALLOCATE(selected_nodes);
		}
#endif /* HACK */
	}
	else
	{
		display_message(ERROR_MESSAGE,"d2_update_selected.  Invalid data struct");
	}
	LEAVE;
} /* d2_update_selected */

static void d2_control_CB(Widget w,int button_num,XmAnyCallbackStruct *reason)
/*******************************************************************************
LAST MODIFIED : 25 September 1995

DESCRIPTION :
Performs actions depending upon the button pressed.
==============================================================================*/
{
	int j,node_number,num_selected;
	struct D2_struct *temp_data_2d;
	struct FE_node *node;
#if !defined (HACK)
	XmStringTable selected_items;
#endif /* HACK */

	ENTER(d2_control_CB);
	USE_PARAMETER(reason);
	XtVaGetValues(w,XmNuserData,&temp_data_2d,NULL);
	if (temp_data_2d)
	{
		switch (button_num)
		{
			case data_2d_control_digitise_ID:
			{
				if (DATA_2D_DIGITISE==temp_data_2d->mode)
				{
					temp_data_2d->mode=DATA_2D_SELECT;
				}
				else
				{
					temp_data_2d->mode=DATA_2D_DIGITISE;
				}
			} break;
			case data_2d_control_delete_ID:
			{
				MANAGER_BEGIN_CACHE(FE_node)(temp_data_2d->node_manager);
#if defined (HACK)
				num_selected = global_num_selected;
#else /* HACK */
				XtVaGetValues(temp_data_2d->data_list,
					XmNselectedItemCount,&num_selected,
					XmNselectedItems,&selected_items,NULL);
#endif /* HACK */
				for (j=0;j<num_selected;j++)
				{
#if defined (HACK)
					node_number = global_selected[j];
#else /* HACK */
					node_number=d2_get_node_from_string(selected_items[j]);
#endif /* HACK */
					if (node=FIND_BY_IDENTIFIER_IN_GROUP(FE_node,cm_node_identifier)
						(node_number,temp_data_2d->current_value))
					{
						REMOVE_OBJECT_FROM_GROUP(FE_node)(node,temp_data_2d->current_value);
						REMOVE_OBJECT_FROM_MANAGER(FE_node)(node,
							temp_data_2d->node_manager);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"data_2d_selection.  Could not find node");
					}
				}
				MANAGER_END_CACHE(FE_node)(temp_data_2d->node_manager);
				/* global object change via manager will notify us of change */
/*         d2_call_callback(temp_data_2d,DATA_2D_UPDATE_CB, */
/*           temp_data_2d->current_value); */
				/* there will be no nodes selected */
/*         d2_update_selected(temp_data_2d); */
				temp_data_2d->mode=DATA_2D_SELECT;
			} break;
			case data_2d_control_move_ID:
			{
				if (DATA_2D_MOVE==temp_data_2d->mode)
				{
					temp_data_2d->mode=DATA_2D_SELECT;
				}
				else
				{
					temp_data_2d->mode=DATA_2D_MOVE;
				}
			} break;
			default:
			{
				display_message(WARNING_MESSAGE,
					"d2_control_CB.  Invalid button number");
			} break;
		}
		d2_show_mode(temp_data_2d);
	}
	else
	{
		display_message(WARNING_MESSAGE,"d2_control_CB.  Invalid arguments");
	}
	LEAVE;
} /* d2_control_CB */

#if defined (OLD_CODE)
static XmString d2_create_string(struct FE_node *node,
	struct MANAGER(FE_field) *fe_field_manager,int dimension)
/*******************************************************************************
LAST MODIFIED : 29 January 1999

DESCRIPTION :
Creates a string from a node.
==============================================================================*/
{
	char temp_string[100],temp_value[100];
	FE_value nodal_value;
	int i;
	struct FE_field_component data_2d_field_component;
	XmString return_string;

	ENTER(d2_create_string);
	return_string=(XmString)NULL;
	if (node&&fe_field_manager)
	{
		if (data_2d_field_component.field=FIND_BY_IDENTIFIER_IN_MANAGER(FE_field,
			name)(COORDINATES_2D_FIELD_NAME,fe_field_manager))
		{
			sprintf(temp_string,"%4i:",get_FE_node_cm_node_identifier(node));
			for (i=0;i<dimension;i++)
			{
				data_2d_field_component.number=i;
				get_FE_nodal_FE_value_value(node,&data_2d_field_component,0,FE_NODAL_VALUE,
					&nodal_value);
				sprintf(temp_value,"%10.4f"FE_VALUE_STRING,nodal_value);
				strcat(temp_string,temp_value);
			}
			return_string=XmStringCreateSimple(temp_string);
		}
		else
		{
			display_message(ERROR_MESSAGE,"d2_create_string.  %s not defined",
				COORDINATES_2D_FIELD_NAME);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"d2_create_string.  Invalid argument(s)");
	}
	LEAVE;

	return (return_string);
} /* d2_create_string */
#endif /* defined (OLD_CODE) */

#if defined (OLD_CODE)
static int d2_add_data_item(struct FE_node *node,void *user_data)
/*******************************************************************************
LAST MODIFIED : 19 January 1997

DESCRIPTION :
Adds an item to the data list widget.
==============================================================================*/
{
	int return_code;
	struct D2_struct *temp_data_2d=(struct D2_struct *)user_data;
#if !defined (HACK)
	XmString new_string;
#endif /* !defined (HACK) */

	ENTER(d2_add_data_item);
	return_code=0;
	if (temp_data_2d)
	{
		if (node)
		{
#if !defined (HACK)
			if (new_string=d2_create_string(node,temp_data_2d->fe_field_manager,
				temp_data_2d->dimension))
			{
				XmListAddItemUnselected(temp_data_2d->data_list,new_string,0);
				XmStringFree(new_string);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"d2_add_data_item.  Could not create string");
			}
#endif /* HACK */
		}
		else
		{
			display_message(ERROR_MESSAGE,"d2_add_data_item.  Invalid FE_node");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"d2_add_data_item.  Invalid data struct");
	}
	LEAVE;

	return return_code;
} /* d2_add_data_item */
#endif /* defined (OLD_CODE) */

static int d2_in_box(struct FE_node *node,void *user_data)
/*******************************************************************************
LAST MODIFIED : 29 January 1999

DESCRIPTION :
Checks if the current node is within the selection box, if so then it selects
it.
==============================================================================*/
{
	FE_value nodal_value;
	int i,return_code,in_box;
	struct D2_struct *temp_data_2d=(struct D2_struct *)user_data;
	struct FE_field_component data_2d_field_component;
#if !defined (HACK)
	XmString item_string;
#endif /* HACK */

	ENTER(d2_in_box);
	return_code=0;
	if (temp_data_2d)
	{
		if (node)
		{
			if (data_2d_field_component.field=FIND_BY_IDENTIFIER_IN_MANAGER(FE_field,
				name)(COORDINATES_2D_FIELD_NAME,temp_data_2d->fe_field_manager))
			{
				in_box=1;
				for (i=0;i<temp_data_2d->dimension;i++)
				{
					data_2d_field_component.number=i;
					get_FE_nodal_FE_value_value(node,&data_2d_field_component,0,FE_NODAL_VALUE,
						&nodal_value);
					if ((nodal_value<temp_data_2d->button_down[i])&&
						(nodal_value<temp_data_2d->button_up[i]))
					{
						in_box=0;
					}
					if ((nodal_value>temp_data_2d->button_down[i])&&
						(nodal_value>temp_data_2d->button_up[i]))
					{
						in_box=0;
					}
				}
				if (in_box)
				{
#if defined (HACK)
					global_selected[global_num_selected]=get_FE_node_cm_node_identifier(node);
					global_num_selected++;
					if (global_num_selected>=HACK_MAX_SELECTED)
					{
						display_message(ERROR_MESSAGE,
							"d2_in_box.  Too many selected nodes");
					}
#else /* HACK */
					if (item_string=d2_create_string(node,temp_data_2d->fe_field_manager,
						temp_data_2d->dimension))
					{
						XmListSelectItem(temp_data_2d->data_list,item_string,FALSE);
						XmStringFree(item_string);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"d2_in_box.  Could not create string");
					}
#endif /* HACK */
				}
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,"d2_in_box.  %s not defined",
					COORDINATES_2D_FIELD_NAME);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"d2_in_box.  Invalid FE_node");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"d2_in_box.  Invalid data struct");
	}
	LEAVE;

	return return_code;
} /* d2_in_box */

static void d2_update_list(struct D2_struct *temp_data_2d)
/*******************************************************************************
LAST MODIFIED : 25 September 1995

DESCRIPTION :
Deletes all items in the list, then regenerates the list.  Used when changing
node groups.
==============================================================================*/
{
#if defined (HACK)
	XmString new_string;
#endif /* HACK */

	ENTER(d2_update_list);
	if (temp_data_2d)
	{
		/* delete all items */
		XmListDeleteAllItems(temp_data_2d->data_list);
#if defined (HACK)
		new_string=XmStringCreateSimple("Optimised version - select in client!");
		XmListAddItemUnselected(temp_data_2d->data_list,new_string,0);
		XmStringFree(new_string);
#else /* HACK */
		/* add the new items */
		if (temp_data_2d->current_value)
		{
			FOR_EACH_OBJECT_IN_GROUP(FE_node)(d2_add_data_item,temp_data_2d,
				temp_data_2d->current_value);
		}
#endif /* HACK */
	}
	else
	{
		display_message(ERROR_MESSAGE,"d2_update_list.  Invalid data struct");
	}
	LEAVE;
} /* d2_update_list */

static void d2_select_CB(Widget w,int button_num,XmAnyCallbackStruct *reason)
/*******************************************************************************
LAST MODIFIED : 25 September 1995

DESCRIPTION :
The user has selected data items in the data list
==============================================================================*/
{
	struct D2_struct *temp_data_2d;

	ENTER(d2_select_CB);
	USE_PARAMETER(button_num);
	USE_PARAMETER(reason);
	XtVaGetValues(w,XmNuserData,&temp_data_2d,NULL);
#if defined (HACK)
	global_num_selected = 0;
	display_message(ERROR_MESSAGE,
		"d2_select_CB.  Optimised version - select in client!");
#endif /* HACK */
	d2_update_selected(temp_data_2d);
	LEAVE;
} /* d2_select_CB */

static void d2_global_node_change(
	struct MANAGER_MESSAGE(FE_node) *message,void *data)
/*******************************************************************************
LAST MODIFIED : 23 May 2001

DESCRIPTION :
Nodes have either been added or deleted or modified etc, so refresh the widget.
==============================================================================*/
{
	struct D2_struct *temp_data_2d=(struct D2_struct *)data;

	ENTER(d2_global_node_change);
	switch (message->change)
	{
		case MANAGER_CHANGE_IDENTIFIER(FE_node):
		case MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(FE_node):
		case MANAGER_CHANGE_OBJECT(FE_node):
		{
			if (temp_data_2d->current_value)
			{
				if (FIRST_OBJECT_IN_LIST_THAT(FE_node)(FE_node_is_in_group,
					(void *)temp_data_2d->current_value, message->changed_object_list))
				{
					d2_update_list(temp_data_2d);
					/* update any clients */
					d2_call_callback(temp_data_2d,DATA_2D_UPDATE_CB,
						temp_data_2d->current_value);
					d2_update_selected(temp_data_2d);
				}
			}
		} break;
#if defined (OLD_CODE)
		case MANAGER_CHANGE_ADD(FE_node):
		case MANAGER_CHANGE_REMOVE(FE_node):
		{
			/* do nothing; these are picked up by group messages */
		} break;
#endif /* defined (OLD_CODE) */
	}
	LEAVE;
} /* d2_global_node_change */

static void d2_global_node_group_change(
	struct MANAGER_MESSAGE(GROUP(FE_node)) *message,void *data)
/*******************************************************************************
LAST MODIFIED : 23 May 2001

DESCRIPTION :
See if our node_group has been modified - if so, then refresh.  Note that if
our actual node_group has been changed, then we will be notified by our
selection widget.
==============================================================================*/
{
	struct D2_struct *temp_data_2d=(struct D2_struct *)data;

	ENTER(d2_global_node_group_change);
	switch (message->change)
	{
		case MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(GROUP(FE_node)):
		case MANAGER_CHANGE_OBJECT(GROUP(FE_node)):
		{
			if (IS_OBJECT_IN_LIST(GROUP(FE_node))(temp_data_2d->current_value,
				message->changed_object_list))
			{
				d2_update_list(temp_data_2d);
				/* update any clients */
				d2_call_callback(temp_data_2d,DATA_2D_UPDATE_CB,
					temp_data_2d->current_value);
				d2_update_selected(temp_data_2d);
			}
		} break;
	}
	LEAVE;
} /* d2_global_node_group_change */

/*
Global Functions
---------------
*/
Widget create_data_2d_widget(Widget *data_2d_widget,Widget parent,int dimension,
	struct MANAGER(FE_field) *fe_field_manager,
	struct MANAGER(FE_node) *node_manager,
	struct MANAGER(GROUP(FE_node)) *group_manager,
	struct GROUP(FE_node) *init_data,DATA_2D_PRECISION select_radius)
/*******************************************************************************
LAST MODIFIED : 23 March 1999

DESCRIPTION :
Creates a data_2d widget that will allow the user to choose an object based
upon its name.
==============================================================================*/
{
	int i,init_widgets;
	MrmType data_2d_dialog_class;
	struct D2_struct *temp_data_2d=NULL;
	static MrmRegisterArg callback_list[]=
	{
		{"d2_identify_button",(XtPointer)d2_identify_button},
		{"d2_destroy_CB",(XtPointer)d2_destroy_CB},
		{"d2_control_CB",(XtPointer)d2_control_CB},
		{"d2_select_CB",(XtPointer)d2_select_CB},
	};
	static MrmRegisterArg identifier_list[]=
	{
		{"D2_structure",(XtPointer)NULL},
		{"d2_datalist_ID",(XtPointer)data_2d_datalist_ID},
		{"d2_mode_ID",(XtPointer)data_2d_mode_ID},
		{"d2_control_ID",(XtPointer)data_2d_control_ID},
		{"d2_control_digitise_ID",(XtPointer)data_2d_control_digitise_ID},
		{"d2_control_delete_ID",(XtPointer)data_2d_control_delete_ID},
		{"d2_control_move_ID",(XtPointer)data_2d_control_move_ID},
	};
	Widget return_widget;

	ENTER(create_data_2d_widget);
	return_widget=(Widget)NULL;
	if (MrmOpenHierarchy_base64_string(data_2d_uidh,
		&data_2d_hierarchy,&data_2d_hierarchy_open))
	{
		if (fe_field_manager&&node_manager&&group_manager)
		{
			/* allocate memory */
			if (ALLOCATE(temp_data_2d,struct D2_struct,1))
			{
				/* initialise the structure */
				temp_data_2d->group_manager=group_manager;
				temp_data_2d->data_2d_field=(struct FE_field *)NULL;
				temp_data_2d->fe_field_manager=fe_field_manager;
				temp_data_2d->node_manager=node_manager;
				temp_data_2d->widget_parent=parent;
				temp_data_2d->widget=(Widget)NULL;
				temp_data_2d->widget_address=data_2d_widget;
				temp_data_2d->data_list=(Widget)NULL;
				temp_data_2d->control=(Widget)NULL;
				temp_data_2d->mode_label=(Widget)NULL;
				temp_data_2d->control_digitise=(Widget)NULL;
				temp_data_2d->control_delete=(Widget)NULL;
				temp_data_2d->control_move=(Widget)NULL;
				if (select_radius>=0)
				{
					temp_data_2d->select_radius=select_radius;
				}
				else
				{
					temp_data_2d->select_radius=0;
					display_message(WARNING_MESSAGE,
						"create_data_2d_widget.  Invalid selection radius.");
				}
				temp_data_2d->mode=DATA_2D_SELECT;
				if ((dimension>0)&&(dimension<=DATA_2D_MAX_DIMENSION))
				{
					temp_data_2d->dimension=dimension;
				}
				else
				{
					temp_data_2d->dimension=2;
					display_message(WARNING_MESSAGE,
						"create_data_2d_widget.  Invalid dimension.");
				}
				for (i=0;i<temp_data_2d->dimension;i++)
				{
					temp_data_2d->button_down[i]=0.0;
					temp_data_2d->button_up[i]=0.0;
				}
				if (init_data)
				{
					temp_data_2d->current_value=init_data;
					if (!IS_MANAGED(GROUP(FE_node))(temp_data_2d->current_value,
						temp_data_2d->group_manager))
					{
						display_message(ERROR_MESSAGE,
							"create_data_2d_widget.  Invalid GROUP(FE_node).");
					}
				}
				else
				{
					temp_data_2d->current_value=
						FIRST_OBJECT_IN_MANAGER_THAT(GROUP(FE_node))(
						(MANAGER_CONDITIONAL_FUNCTION(GROUP(FE_node)) *)NULL,(void *)NULL,
						temp_data_2d->group_manager);
				}
				for (i=0;i<DATA_2D_NUM_CALLBACKS;i++)
				{
					temp_data_2d->callback_array[i].procedure=
						(Callback_procedure *)NULL;
					temp_data_2d->callback_array[i].data=NULL;
				}
				/* register the callbacks */
				if (MrmSUCCESS==MrmRegisterNamesInHierarchy(data_2d_hierarchy,
					callback_list,XtNumber(callback_list)))
				{
					/* assign and register the identifiers */
					identifier_list[0].value=(XtPointer)temp_data_2d;
					if (MrmSUCCESS==MrmRegisterNamesInHierarchy(data_2d_hierarchy,
						identifier_list,XtNumber(identifier_list)))
					{
						/* fetch data_2d control widget */
						if (MrmSUCCESS==MrmFetchWidget(data_2d_hierarchy,
							"d2_widget",temp_data_2d->widget_parent,
							&(temp_data_2d->widget),&data_2d_dialog_class))
						{
							init_widgets=1;
							if (init_widgets)
							{
								XtManageChild(temp_data_2d->widget);
								d2_update_list(temp_data_2d);
								d2_show_mode(temp_data_2d);
								temp_data_2d->manager_callback_id=MANAGER_REGISTER(FE_node)(
									d2_global_node_change,temp_data_2d,
									temp_data_2d->node_manager);
								temp_data_2d->group_manager_callback_id=
									MANAGER_REGISTER(GROUP(FE_node))(
									d2_global_node_group_change,temp_data_2d,
									temp_data_2d->group_manager);
								return_widget=temp_data_2d->widget;
							}
							else
							{
								DEALLOCATE(temp_data_2d);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"create_data_2d_widget.  Could not fetch data_2d widget");
							DEALLOCATE(temp_data_2d);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_data_2d_widget.  Could not register identifiers");
						DEALLOCATE(temp_data_2d);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_data_2d_widget.  Could not register callbacks");
					DEALLOCATE(temp_data_2d);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
				"create_data_2d_widget.  Could not allocate control window structure");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_data_2d_dialog.  Invalid argument(s)");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_data_2d_widget.  Could not open hierarchy");
	}
	if (data_2d_widget)
	{
		*data_2d_widget=return_widget;
	}
	LEAVE;

	return (return_widget);
} /* create_data_2d_widget */

int data_2d_set_data(Widget data_2d_widget,
	enum D2_data_type data_type,void *data)
/*******************************************************************************
LAST MODIFIED : 25 September 1995

DESCRIPTION :
Changes a data item of the input widget.
==============================================================================*/
{
	int return_code;
	struct D2_struct *temp_data_2d;

	ENTER(data_2d_set_data);
	/* Get the pointer to the data for the data_2d dialog */
	XtVaGetValues(data_2d_widget,XmNuserData,&temp_data_2d,NULL);
	switch (data_type)
	{
		case DATA_2D_UPDATE_CB:
		{
			temp_data_2d->callback_array[DATA_2D_UPDATE_CB].procedure=
				((struct Callback_data *)data)->procedure;
			temp_data_2d->callback_array[DATA_2D_UPDATE_CB].data=
				((struct Callback_data *)data)->data;
			return_code=1;
		} break;
		case DATA_2D_SELECT_CB:
		{
			temp_data_2d->callback_array[DATA_2D_SELECT_CB].procedure=
				((struct Callback_data *)data)->procedure;
			temp_data_2d->callback_array[DATA_2D_SELECT_CB].data=
				((struct Callback_data *)data)->data;
			return_code=1;
		} break;
		case DATA_2D_DATA:
		{
			if (data)
			{
				if (IS_MANAGED(GROUP(FE_node))((struct GROUP(FE_node) *)data,
					temp_data_2d->group_manager))
				{
					temp_data_2d->current_value=(struct GROUP(FE_node) *)data;
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"data_2d_set_data.  Invalid GROUP(FE_node).");
					return_code=0;
				}
			}
			else
			{
				temp_data_2d->current_value=
					FIRST_OBJECT_IN_MANAGER_THAT(GROUP(FE_node))(
					(MANAGER_CONDITIONAL_FUNCTION(GROUP(FE_node)) *)NULL,(void *)NULL,
					temp_data_2d->group_manager);
				return_code=1;
			}
			if (return_code)
			{
#if defined (DATA_2D_NAME)
				/* set the name of the data_2d */
				data_2d_set_name(temp_data_2d);
#endif
				d2_update_list(temp_data_2d);
				d2_call_callback(temp_data_2d,DATA_2D_UPDATE_CB,
					temp_data_2d->current_value);
				d2_update_selected(temp_data_2d);
			}
		} break;
		case DATA_2D_SELECT_RADIUS:
		{
			if (data)
			{
				temp_data_2d->select_radius=*((DATA_2D_PRECISION *)data);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"data_2d_set_data.  Invalid selection radius");
				return_code=0;
			}
		} break;
		default:
		{
			return_code=0;
		} break;
	}
	LEAVE;

	return (return_code);
} /* data_2d_set_data */

void *data_2d_get_data(Widget data_2d_widget,
	enum D2_data_type data_type)
/*******************************************************************************
LAST MODIFIED : 25 September 1995

DESCRIPTION :
Returns a pointer to a data item of the input widget.
???DB.  Why is this returning void * ?
==============================================================================*/
{
	void *return_code;
	static struct Callback_data dat_callback;
	static DATA_2D_PRECISION dat_select_radius;
	struct D2_struct *temp_data_2d;

	ENTER(data_2d_get_data);
	/* Get the pointer to the data for the data_2d dialog */
	XtVaGetValues(data_2d_widget,XmNuserData,&temp_data_2d,NULL);
	switch (data_type)
	{
		case DATA_2D_UPDATE_CB:
		{
			dat_callback.procedure=
				(temp_data_2d->callback_array)[DATA_2D_UPDATE_CB].procedure;
			dat_callback.data=(temp_data_2d->callback_array)[DATA_2D_UPDATE_CB].data;
			return_code= &dat_callback;
		} break;
		case DATA_2D_SELECT_CB:
		{
			dat_callback.procedure=
				temp_data_2d->callback_array[DATA_2D_SELECT_CB].procedure;
			dat_callback.data=(temp_data_2d->callback_array)[DATA_2D_SELECT_CB].data;
			return_code= &dat_callback;
		} break;
		case DATA_2D_DATA:
		{
			return_code=temp_data_2d->current_value;
		} break;
		case DATA_2D_SELECT_RADIUS:
		{
			dat_select_radius=temp_data_2d->select_radius;
			return_code= &dat_select_radius;
		} break;
		default:
		{
			return_code=NULL;
		} break;
	}
	LEAVE;

	return (return_code);
} /* data_2d_get_data */

int data_2d_selection(Widget data_2d_widget,DATA_2D_PRECISION *data,
	int button_down,int single)
/*******************************************************************************
LAST MODIFIED : 31 August 2001

DESCRIPTION :
Notifies the grabber about the selection process.
==============================================================================*/
{
	int i,j,return_code,node_number,num_selected;
#if defined (HACK)
	struct Data_2d_select_data select_data;
#else
	int num_items;
	XmStringTable selected_items;
#endif /* HACK */
	DATA_2D_PRECISION difference;
	struct D2_struct *temp_data_2d;
	FE_value temp_data[DATA_2D_MAX_DIMENSION];
	struct FE_node *new_node,*node,*copy_node,*template_node;
	struct FE_field *data_2d_field;
	struct FE_field_component data_2d_field_component;
	enum FE_nodal_value_type *components_nodal_value_types[3]=
	{
		{
			FE_NODAL_VALUE
		},
		{
			FE_NODAL_VALUE
		},
		{
			FE_NODAL_VALUE
		}
	};
	char *component_names[3]=
	{
		"X","Y","Z"
	};
	int components_number_of_derivatives[3]={0,0,0},
		components_number_of_versions[3]={1,1,1};
	static int last_node_number = 1;
	struct Coordinate_system rect_cart_coords;

	rect_cart_coords.type =  RECTANGULAR_CARTESIAN;

	ENTER(data_2d_selection);
	return_code=0;
	if (data_2d_widget)
	{
		/* Get the pointer to the data for the data_2d dialog */
		XtVaGetValues(data_2d_widget,XmNuserData,&temp_data_2d,NULL);
		if (temp_data_2d)
		{
			if (temp_data_2d->current_value)
			{
				return_code=1;
				/* update the button values */
				if (button_down)
				{
					for (i=0;i<temp_data_2d->dimension;i++)
					{
						temp_data_2d->button_down[i]=data[i];
					}
				}
				else
				{
					for (i=0;i<temp_data_2d->dimension;i++)
					{
						temp_data_2d->button_up[i]=data[i];
					}
				}
				switch (temp_data_2d->mode)
				{
					case DATA_2D_DIGITISE:
					{
						if (!button_down)
						{
							/* tell the widget no nodes selected */
							/* unselect all currently selected */
#if defined (HACK)
							global_num_selected = 0;
#else /* HACK */
							XtVaGetValues(temp_data_2d->data_list,XmNitemCount,&num_items,
								NULL);
							for (i=0;i<num_items;i++)
							{
								XmListDeselectPos(temp_data_2d->data_list,i);
							}
#endif /* HACK */
							/* ignore single, just add a new node */
							/* get a node number */
							node_number=get_next_FE_node_number(temp_data_2d->node_manager,
								last_node_number);
							last_node_number = node_number;
							/* change data to FE_value */
							for (i=0;i<temp_data_2d->dimension;i++)
							{
								temp_data[i]=data[i];
							}
							/* component_names array dimensioned to 3 only */
							if (temp_data_2d->dimension>3)
							{
								display_message(WARNING_MESSAGE,
									"data_2d_selection.  Increase number of component names");
							}
#if defined (OLD_CODE)
							if (data_2d_field=CREATE(FE_field)("digitised(2d)",COORDINATE,
								0,(FE_value *)NULL,RECTANGULAR_CARTESIAN,
								temp_data_2d->dimension,component_names))
#endif /* defined (OLD_CODE) */
							if (data_2d_field=get_FE_field_manager_matched_field(
								temp_data_2d->fe_field_manager,COORDINATES_2D_FIELD_NAME,
								GENERAL_FE_FIELD,/*indexer_field*/(struct FE_field *)NULL,
								/*number_of_indexed_values*/0,CM_COORDINATE_FIELD,
								&rect_cart_coords,FE_VALUE_VALUE,
								/*number_of_components*/temp_data_2d->dimension,component_names,
								/*number_of_times*/0,/*time_value_type*/UNKNOWN_VALUE,
								(struct FE_field_external_information *)NULL))
							{
								if (!temp_data_2d->data_2d_field)
								{
									temp_data_2d->data_2d_field=ACCESS(FE_field)(data_2d_field);
								}
								if ((template_node=CREATE(FE_node)(0,(struct FE_node *)NULL))
									&&define_FE_field_at_node(template_node,
									data_2d_field,components_number_of_derivatives,
									components_number_of_versions,components_nodal_value_types))
								{								
									if (new_node=CREATE(FE_node)(node_number,template_node))
									{
										int length;
										set_FE_nodal_field_FE_value_values(data_2d_field,new_node,
											temp_data,&length);
										if (ADD_OBJECT_TO_GROUP(FE_node)(new_node,
											temp_data_2d->current_value))
										{
											if (ADD_OBJECT_TO_MANAGER(FE_node)(new_node,
												temp_data_2d->node_manager))
											{
												d2_call_callback(temp_data_2d,DATA_2D_UPDATE_CB,
													temp_data_2d->current_value);
												d2_update_selected(temp_data_2d);
											}
											else
											{
												display_message(ERROR_MESSAGE,
										"data_2d_selection.  Could not add FE_node to manager");
											}
										}
										else
										{
											display_message(ERROR_MESSAGE,
											"data_2d_selection.  Could not add FE_node to group");
										}
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"data_2d_selection.  Could not create FE_node");
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"data_2d_selection.  Could not create template_node");
								}
								DESTROY(FE_node)(&template_node);
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"data_2d_selection.  Could not create FE_field");
							}
						}
					} break;
					case DATA_2D_SELECT:
					{
						if (button_down)
						{
							if (single)
							{
#if defined (HACK)
								global_num_selected = 0;
#else /* HACK */
								/* unselect all currently selected */
								XtVaGetValues(temp_data_2d->data_list,XmNitemCount,&num_items,
									NULL);
								for (i=0;i<num_items;i++)
								{
									XmListDeselectPos(temp_data_2d->data_list,i);
								}
#endif /* HACK */
							}
						}
						else
						{
							/* modify the selection box so that it is at least the
							select_radius wide */
							for (i=0;i<temp_data_2d->dimension;i++)
							{
								/* find which way the box was dragged */
								difference=(temp_data_2d->button_up[i])-
									(temp_data_2d->button_down[i]);
								if (difference>0)
								{
									/* increase button_up */
									/* only change if it is a small select box */
									if (difference<2*temp_data_2d->select_radius)
									{
										temp_data_2d->button_down[i] -=
											(2*temp_data_2d->select_radius-difference)/2.0;
										temp_data_2d->button_up[i] +=
											(2*temp_data_2d->select_radius-difference)/2.0;
									}
								}
								else
								{
									/* decrease button_up */
									/* only change if it is a small select box */
									difference= -difference;
									if (difference<2*temp_data_2d->select_radius)
									{
										temp_data_2d->button_down[i] +=
											(2*temp_data_2d->select_radius-difference)/2.0;
										temp_data_2d->button_up[i] -=
											(2*temp_data_2d->select_radius-difference)/2.0;
									}
								}
							}
#if defined (HACK)
							global_num_selected = 0;
#endif /* HACK */
							/* we have a box from button_down to current, select any nodes
								inside */
							FOR_EACH_OBJECT_IN_GROUP(FE_node)(d2_in_box,temp_data_2d,
								temp_data_2d->current_value);
						}
#if defined (HACK)
						select_data.num_selected = global_num_selected;
						select_data.selected_nodes = global_selected;
						d2_call_callback(temp_data_2d,DATA_2D_SELECT_CB,&select_data);
#else /* HACK */
						d2_update_selected(temp_data_2d);
#endif /* HACK */
					} break;
					case DATA_2D_MOVE:
					{
						/* GMH This code assumes the data field is the only field
							defined for the node - must generalise */
						if (!button_down)
						{
							MANAGER_BEGIN_CACHE(FE_node)(temp_data_2d->node_manager);
#if defined (HACK)
							num_selected = global_num_selected;
#else /* HACK */
							XtVaGetValues(temp_data_2d->data_list,
								XmNselectedItemCount,&num_selected,
								XmNselectedItems,&selected_items,NULL);
#endif /* HACK */
							if (1==num_selected)
							{
#if defined (HACK)
								node_number = global_selected[0];
#else /* HACK */
								node_number=d2_get_node_from_string(selected_items[0]);
#endif /* HACK */
								if (node=FIND_BY_IDENTIFIER_IN_GROUP(FE_node,cm_node_identifier)
									(node_number,temp_data_2d->current_value))
								{
									for (i=0;i<temp_data_2d->dimension;i++)
									{
										temp_data[i]=temp_data_2d->button_up[i];
									}
									if (copy_node=CREATE(FE_node)(0,node))
									{
										int length;
										set_FE_nodal_field_FE_value_values(
											temp_data_2d->data_2d_field,copy_node,temp_data,&length);

										MANAGER_MODIFY_NOT_IDENTIFIER(FE_node,cm_node_identifier)(node,
											copy_node,temp_data_2d->node_manager);
										DESTROY(FE_node)(&copy_node);
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"data_2d_selection.  Could not create new node");
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"data_2d_selection.  Could not find node");
								}
							}
							else
							{
								for (j=0;j<num_selected;j++)
								{
#if defined (HACK)
									node_number = global_selected[j];
#else /* HACK */
									node_number=d2_get_node_from_string(selected_items[j]);
#endif /* HACK */
									if (node=FIND_BY_IDENTIFIER_IN_GROUP(FE_node,cm_node_identifier)
										(node_number,temp_data_2d->current_value))
									{
										if (data_2d_field_component.field=
											FIND_BY_IDENTIFIER_IN_MANAGER(FE_field,name)(
											COORDINATES_2D_FIELD_NAME,temp_data_2d->fe_field_manager))
										{
											for (i=0;i<temp_data_2d->dimension;i++)
											{
												data_2d_field_component.number=i;
												get_FE_nodal_FE_value_value(node,&data_2d_field_component,0,
													FE_NODAL_VALUE,temp_data+i);
												temp_data[i] += temp_data_2d->button_up[i]-
													temp_data_2d->button_down[i];
											}
											if (copy_node=CREATE(FE_node)(0,node))
											{											
												int length;
												set_FE_nodal_field_FE_value_values(
													data_2d_field_component.field,
													copy_node,temp_data,&length);

												MANAGER_MODIFY_NOT_IDENTIFIER(FE_node,cm_node_identifier)
													(node,copy_node,temp_data_2d->node_manager);
												DESTROY(FE_node)(&copy_node);
											}
											else
											{
												display_message(ERROR_MESSAGE,
													"data_2d_selection.  Could not create new node");
											}
										}
										else
										{
											display_message(ERROR_MESSAGE,
												"data_2d_selection.  %s not defined",
												COORDINATES_2D_FIELD_NAME);
										}
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"data_2d_selection.  Could not find node");
									}
								}
							}
							MANAGER_END_CACHE(FE_node)(temp_data_2d->node_manager);
							d2_call_callback(temp_data_2d,DATA_2D_UPDATE_CB,
								temp_data_2d->current_value);
#if defined (HACK)
							select_data.num_selected = global_num_selected;
							select_data.selected_nodes = global_selected;
							d2_call_callback(temp_data_2d,DATA_2D_SELECT_CB,&select_data);
#else /* HACK */
							d2_update_selected(temp_data_2d);
#endif /* HACK */
							temp_data_2d->mode=DATA_2D_SELECT;
							d2_show_mode(temp_data_2d);
						}
					} break;
					default:
					{
						display_message(ERROR_MESSAGE,
							"data_2d_selection.  Invalid mode");
					} break;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"data_2d_selection.  Invalid node group");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"data_2d_selection.  Invalid data");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"data_2d_selection.  Invalid widget");
	}
	LEAVE;

	return (return_code);
} /* data_2d_selection */
