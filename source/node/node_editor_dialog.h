/*******************************************************************************
FILE : node_editor_dialog.h

LAST MODIFIED : 6 November 1998

DESCRIPTION :
This module creates a free node_editor_dialog input device, using two dof3,
two control and one input widget.  The position is given relative to some
coordinate system, and the returned value is a global one.
==============================================================================*/
#if !defined (NODE_EDITOR_DIALOG_H)
#define NODE_EDITOR_DIALOG_H

#include "general/callback.h"
#include "finite_element/finite_element.h"

/*
UIL Identifiers
---------------
*/
/*???DB.  Why do these have to be global ? */
#define node_editor_dialog_select_form_ID                          1
#define node_editor_dialog_editor_form_ID        2
#define node_editor_dialog_ok_ID                 3
#define node_editor_dialog_apply_ID                 4
#define node_editor_dialog_revert_ID               5
#define node_editor_dialog_cancel_ID               6

/*
Global Types
------------
*/
enum Node_editor_dialog_data_type
/*******************************************************************************
LAST MODIFIED : 22 February 1995

DESCRIPTION :
Contains the different types of data items that are valid for the
node_editor widget.
==============================================================================*/
{
	NODE_EDITOR_DIALOG_UPDATE_CB,
	NODE_EDITOR_DIALOG_DATA
}; /* node_editor_dialog_data_type */

#define NODE_EDITOR_DIALOG_NUM_CALLBACKS 1

struct Node_editor_dialog_struct
/*******************************************************************************
LAST MODIFIED : 20 February 1996

DESCRIPTION :
Contains all the information carried by the node_editor_dialog widget.
Note that we just hold a pointer to the node_editor_dialog, and must access
and deaccess it.
==============================================================================*/
{
	struct FE_node *current_value;
	struct MANAGER(FE_node) *manager;
	struct Callback_data callback_array[NODE_EDITOR_DIALOG_NUM_CALLBACKS];
	Widget editor_form,editor_widget,select_form,select_widget;
	Widget apply_button,cancel_button,ok_button,revert_button;
	Widget *dialog_address,dialog,widget,dialog_parent;
}; /* node_editor_dialog_struct */

/*
Global Functions
----------------
*/
int node_editor_dialog_set_data(Widget node_editor_dialog_widget,
	enum Node_editor_dialog_data_type data_type,void *data);
/*******************************************************************************
LAST MODIFIED : 2 April 1995

DESCRIPTION :
If <node_editor_dialog_widget> is not NULL, then change the data item on
<node_editor_dialog widget>.  Otherwise, change the data item on
<node_editor_dialog>.
==============================================================================*/

void *node_editor_dialog_get_data(Widget node_editor_dialog_widget,
	enum Node_editor_dialog_data_type data_type);
/*******************************************************************************
LAST MODIFIED : 2 April 1995

DESCRIPTION :
If <node_editor_dialog_widget> is not NULL, then get the data item from
<node_editor_dialog widget>.  Otherwise, get the data item from
<node_editor_dialog>.
==============================================================================*/

int bring_up_node_editor_dialog(Widget *node_editor_dialog_address,
	char *create_name,Widget parent,struct MANAGER(FE_node) *node_manager,
	struct FE_node *init_data);
/*******************************************************************************
LAST MODIFIED : 6 November 1998

DESCRIPTION :
If there is a material editor dialog in existence, then bring it to the front,
otherwise it creates a new one.  Assumes we will only ever want one material
editor at a time.  This implementation may be changed later.
???DB.  Should this be somewhere else ?
==============================================================================*/
#endif /* !defined (NODE_EDITOR_DIALOG_H) */
