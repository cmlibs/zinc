/*******************************************************************************
FILE : node_group_slider_dialog.h

LAST MODIFIED : 9 November 1999

DESCRIPTION :
This module creates a node_group_slider input device.  A node group slider is
used to scale the distance between a fixed node and a group of nodes.
???DB.  Extend to other options
==============================================================================*/
#if !defined (NODE_GROUP_SLIDER_DIALOG_H)
#define NODE_GROUP_SLIDER_DIALOG_H

#include "finite_element/finite_element.h"
#include "user_interface/user_interface.h"

/*
Global types
------------
*/
struct Create_node_group_slider_data
/*******************************************************************************
LAST MODIFIED : 9 November 1999

DESCRIPTION :
==============================================================================*/
{
	struct Execute_command *execute_command;
	struct MANAGER(FE_field) *fe_field_manager;
	struct MANAGER(FE_node) *node_manager;
	struct MANAGER(GROUP(FE_node)) *node_group_manager;
	struct MANAGER(Control_curve) *control_curve_manager;
	Widget *node_group_slider_dialog_address,parent;
	struct User_interface *user_interface;
}; /* struct Create_node_group_slider_data */

/*
Global functions
----------------
*/
int create_muscle_slider(struct Parse_state *state,void *dummy_to_be_modified,
	void *create_node_group_slider_data_void);
/*******************************************************************************
LAST MODIFIED : 1 June 1997

DESCRIPTION :
Executes a GFX CREATE MUSCLE_SLIDER command.  If there is a node group slider
dialog in existence, then bring it to the front, otherwise create new one.  If
the fixed node and node group don't have a slider in the slider dialog then add
a new slider.
???DB.  Temporary command ?
==============================================================================*/

int create_pivot_slider(struct Parse_state *state,void *dummy_to_be_modified,
	void *create_node_group_slider_data_void);
/*******************************************************************************
LAST MODIFIED : 2 June 1997

DESCRIPTION :
Executes a GFX CREATE PIVOT_SLIDER command.  If there is a node group slider
dialog in existence, then bring it to the front, otherwise create new one.
???DB.  Temporary command ?
==============================================================================*/

int create_em_sliders(struct Parse_state *state,void *dummy_to_be_modified,
	void *create_node_group_slider_data_void);
/*******************************************************************************
LAST MODIFIED : 24 February 1998

DESCRIPTION :
Executes a GFX CREATE EM_SLIDERS command.  If there is a node group slider
dialog in existence, then bring it to the front, otherwise create new one.
???DB.  Temporary command ?
==============================================================================*/

int set_node_group_slider_value(struct Parse_state *state,
	void *dummy_to_be_modified,void *node_group_slider_dialog_widget_void);
/*******************************************************************************
LAST MODIFIED : 1 June 1997

DESCRIPTION :
==============================================================================*/

int set_node_group_slider_function(Widget slider, float value);
/*******************************************************************************
LAST MODIFIED : 2 April 1998

DESCRIPTION :
Sets the value for the given slider where the slider is not in x units but
in the range of the minimum and maximum specified by the slider.
==============================================================================*/

int list_node_group_slider(struct Parse_state *state,
	void *dummy_to_be_modified,void *node_group_slider_dialog_widget_void);
/*******************************************************************************
LAST MODIFIED : 2 March 1998

DESCRIPTION :
==============================================================================*/
#endif /* !defined (NODE_GROUP_SLIDER_DIALOG_H) */
