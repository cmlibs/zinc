/*******************************************************************************
FILE : interactive_node_editor.h

LAST MODIFIED : 21 July 1996

DESCRIPTION :
This module creates a free interactive_node_editor input device, using two dof3,
two control and one input widget.  The position is given relative to some
coordinate system, and the returned value is a global one.
==============================================================================*/
#if !defined (INTERACTIVE_NODE_EDITOR_H)
#define INTERACTIVE_NODE_EDITOR_H

#include "dof3/dof3.h"
#include "finite_element/finite_element.h"
#include "view/coord.h"
#include "general/callback.h"

/*
UIL Identifiers
---------------
*/
#define interactive_node_editor_menu_ID 1
#define interactive_node_editor_position_form_ID 2
#define interactive_node_editor_coord_form_ID 3

/*
Global Types
------------
*/
enum Interactive_node_editor_data_type
/*******************************************************************************
LAST MODIFIED : 4 January 1995

DESCRIPTION :
Contains the different types of data items that are valid for the interactive_node_editor
widget.
==============================================================================*/
{
	INTERACTIVE_NODE_EDITOR_UPDATE_CB,
	INTERACTIVE_NODE_EDITOR_DATA
}; /* Interactive_Node_Editor_data_type */
#define INTERACTIVE_NODE_EDITOR_NUM_CALLBACKS 1

struct Interactive_node_editor_struct
/*******************************************************************************
LAST MODIFIED : 11 January 1995

DESCRIPTION :
Contains all the information carried by the interactive_node_editor widget
==============================================================================*/
{
	struct MANAGER(FE_node) *manager;
	void *manager_callback_id;
	struct FE_node *current_value,*default_value,*global_value;
	struct Callback_data callback_array[INTERACTIVE_NODE_EDITOR_NUM_CALLBACKS];
	struct Cmgui_coordinate *current_coordinate;
	Widget coord_widget,position_form,
		control_widget,position_widget,input_widget,
		menu,widget_parent,widget,coord_form,*widget_address;
}; /* Interactive_Node_Editor_struct */

/*
Global Functions
----------------
*/
Widget create_interactive_node_editor_widget(Widget *interactive_node_editor_widget,
	Widget parent,struct MANAGER(FE_node) *manager,
	struct FE_node *init_data);
/*******************************************************************************
LAST MODIFIED : 5 January 1995

DESCRIPTION :
Creates a interactive_node_editor widget that allows the user to vary the nodal
positions of a given node.
==============================================================================*/

int interactive_node_editor_set_data(Widget interactive_node_editor_widget,
	enum Interactive_node_editor_data_type data_type,void *data);
/*******************************************************************************
LAST MODIFIED : 4 January 1995

DESCRIPTION :
Changes a data item of the interactive_node_editor widget.
==============================================================================*/

void *interactive_node_editor_get_data(Widget interactive_node_editor_widget,
	enum Interactive_node_editor_data_type data_type);
/*******************************************************************************
LAST MODIFIED : 4 January 1995

DESCRIPTION :
Returns a interactive_node_editornter to a data item of the interactive_node_editor widget.
==============================================================================*/

void interactive_node_editor_modify_global(Widget interactive_node_editor_widget);
/*******************************************************************************
LAST MODIFIED : 22 February 1995

DESCRIPTION :
Makes the node change global.
==============================================================================*/


#endif

