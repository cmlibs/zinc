/*******************************************************************************
FILE : node_editor.h

LAST MODIFIED : 30 September 1995

DESCRIPTION :
This module creates a node editor widget.  A node editor widget consists of
multiple node field widgets.  Buttons allow fields to be created and destroyed
in the node.
==============================================================================*/
#if !defined (NODE_EDITOR_H)
#define NODE_EDITOR_H

#include "general/callback.h"
#include "finite_element/finite_element.h"

/*
UIL Identifiers
---------------
*/
#define node_editor_name_ID            1
#define node_editor_field_form_ID            3
#define node_editor_toggle_position_ID            4
#define node_editor_toggle_rowcol_ID            5

/*
Global Types
------------
*/
enum Node_editor_data_type
/*******************************************************************************
LAST MODIFIED : 4 January 1995

DESCRIPTION :
Contains the different types of data items that are valid for the
node_editor widget.
==============================================================================*/
{
	NODE_EDITOR_UPDATE_CB,
	NODE_EDITOR_DATA
}; /* Node_editor_data_type */

#define NODE_EDITOR_NUM_CALLBACKS 1

struct Node_editor_struct
/*******************************************************************************
LAST MODIFIED : 28 September 1995

DESCRIPTION :
Contains all the information carried by the node_editor widget.
Note that we just hold a pointer to the node_editor, and must access and
deaccess it.
==============================================================================*/
{
	struct MANAGER(FE_node) *manager;
	void *manager_callback_id;
	struct FE_node *current_value,*default_value,*global_value;
	struct Callback_data callback_array[NODE_EDITOR_NUM_CALLBACKS];
	Widget widget,widget_parent,field_form,field_widget,toggle_rowcol,toggle_position,
		first_toggle;
#if defined (NODE_EDITOR_NAME)
	Widget name;
#endif
	Widget *widget_address;
}; /* Node_editor_struct */

/*
Global Functions
----------------
*/
Widget create_node_editor_widget(Widget *node_editor_widget,
	Widget parent,struct MANAGER(FE_node) *manager,
	struct FE_node *init_data);
/*******************************************************************************
LAST MODIFIED : 28 September 1995

DESCRIPTION :
Creates a node_editor widget that allows the user to edit the properties of
the node.
==============================================================================*/

int node_editor_set_data(Widget node_editor_widget,
	enum Node_editor_data_type data_type,void *data);
/*******************************************************************************
LAST MODIFIED : 4 January 1995

DESCRIPTION :
Changes a data item of the node_editor widget.
==============================================================================*/

void *node_editor_get_data(Widget node_editor_widget,
	enum Node_editor_data_type data_type);
/*******************************************************************************
LAST MODIFIED : 4 January 1995

DESCRIPTION :
Returns a pointer to a data item of the node_editor widget.
==============================================================================*/

void node_editor_modify_global(Widget node_editor_widget);
/*******************************************************************************
LAST MODIFIED : 22 February 1995

DESCRIPTION :
Makes the node change global.
==============================================================================*/
#endif
