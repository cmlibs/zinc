/*******************************************************************************
FILE : node_field_editor.h

LAST MODIFIED : 3 November 1998

DESCRIPTION :
This module creates a node_field editor widget.  A node_field editor widget
consists of multiple node field component widgets.
==============================================================================*/
#if !defined (NODE_FIELD_EDITOR_H)
#define NODE_FIELD_EDITOR_H

#include "general/callback.h"
#include "finite_element/finite_element.h"

#define TEMP_STRING_SIZE 100

/*
UIL Identifiers
---------------
*/
#define node_field_editor_name_ID 1
#define node_field_editor_component_ID 2
#define node_field_editor_comp_form_ID 3
#define NODE_FIELD_MAX_DERIVATIVES 10

/*
Global Types
------------
*/
enum Node_field_editor_data_type
/*******************************************************************************
LAST MODIFIED : 4 January 1995

DESCRIPTION :
Contains the different types of data items that are valid for the
node_field_editor widget.
==============================================================================*/
{
	NODE_FIELD_EDITOR_UPDATE_CB,
	NODE_FIELD_EDITOR_DATA,
	NODE_FIELD_EDITOR_VALUE
}; /* node_field_editor_data_type */

#define NODE_FIELD_EDITOR_NUM_CALLBACKS 1

struct Node_field_editor_struct
/*******************************************************************************
LAST MODIFIED : 3 November 1998

DESCRIPTION :
Contains all the information carried by the node_field_editor widget.
Note that we just hold a pointer to the node_field_editor, and must access and
deaccess it.
==============================================================================*/
{
	struct FE_field *global_value;
	int total_derivatives;
	struct FE_node *node;
	struct Callback_data callback_array[NODE_FIELD_EDITOR_NUM_CALLBACKS];
	Widget widget,widget_parent,component_rowcol,comp_form;
#if defined (NODE_FIELD_EDITOR_NAME)
	Widget name;
#endif
	Widget *widget_address;
}; /* node_field_editor_struct */

/*
Global Functions
----------------
*/
Widget create_node_field_editor_widget(Widget *node_field_editor_widget,
	Widget parent,struct FE_field *init_data,struct FE_node *init_value);
/*******************************************************************************
LAST MODIFIED : 2 November 1998

DESCRIPTION :
Creates a node_field_editor widget that allows the user to edit the properties
of the node.
==============================================================================*/

int node_field_editor_set_data(Widget node_field_editor_widget,
	enum Node_field_editor_data_type data_type,void *data);
/*******************************************************************************
LAST MODIFIED : 4 January 1995

DESCRIPTION :
Changes a data item of the node_field_editor widget.
==============================================================================*/

#if defined (OLD_CODE)
void *node_field_editor_get_data(Widget node_field_editor_widget,
	enum Node_field_editor_data_type data_type);
/*******************************************************************************
LAST MODIFIED : 4 January 1995

DESCRIPTION :
Returns a pointer to a data item of the node_field_editor widget.
==============================================================================*/
#endif /* defined (OLD_CODE) */
#endif /* !defined (NODE_FIELD_EDITOR_H) */
