/*******************************************************************************
FILE : data_grabber_dialog.h

LAST MODIFIED : 29 January 1999

DESCRIPTION :
Brings up a window which holds a data_grabber.  Allows the user to change what
data is accepted - pos,tangent,normal.
==============================================================================*/
#if !defined (DATA_GRABBER_DIALOG_H)
#define DATA_GRABBER_DIALOG_H

#include "command/command.h"
#include "data/data_grabber.h"
#include "user_interface/user_interface.h"

/*
UIL Identifiers
---------------
*/
#define data_grabber_dialog_grabber_form_ID (1)
#define data_grabber_dialog_ok_ID (2)
#define data_grabber_dialog_record_ID (3)
#define data_grabber_dialog_toggle_form_ID (4)
#define data_grabber_dialog_mode_form_ID (5)
#define data_grabber_dialog_tangent_form_ID (6)
#define data_grabber_dialog_type_form_ID (7)
#define data_grabber_dialog_node_group_form_ID (8)

/*
Global Types
------------
*/
enum data_grabber_dialog_data_type
/*******************************************************************************
LAST MODIFIED : 03 April 1995

DESCRIPTION :
Contains the different types of data items that are valid for the data_grabber
widget.
==============================================================================*/
{
	DATA_GRABBER_DIALOG_MODE
}; /* data_grabber_dialog_data_type */

struct DG_dialog_struct
/*******************************************************************************
LAST MODIFIED : 29 January 1999

DESCRIPTION :
Contains all the information carried by the data_grabber_dialog widget.
Note that we just hold a pointer to the data_grabber_dialog, and must access and
deaccess it.
==============================================================================*/
{
	int current_mode,tangent_mode,type_mode;
	Widget grabber_form,grabber_widget,ok_button,record_button,toggle_form,
		mode_form,data_group_chooser, node_group_chooser, 
		node_group_form, tangent_form,type_form;
	Widget *dialog_address,dialog,widget,dialog_parent;
	struct Execute_command *execute_command;
	struct GROUP(FE_node) *node_group;
	struct MANAGER(FE_field) *fe_field_manager;
	struct MANAGER(FE_node) *data_manager, *node_manager;
	struct MANAGER(GROUP(FE_node)) *data_group_manager, *node_group_manager;
}; /* data_grabber_dialog_struct */

/*
Global Functions
----------------
*/
int bring_up_data_grabber_dialog(Widget *data_grabber_dialog_address,
	Widget parent,struct Execute_command *execute_command,
	struct User_interface *user_interface,
	struct MANAGER(FE_field) *fe_field_manager,
	struct MANAGER(FE_node) *node_manager,
	struct MANAGER(FE_node) *data_manager, 
	struct MANAGER(GROUP(FE_node)) *node_group_manager,
	struct MANAGER(GROUP(FE_node)) *data_group_manager);
/*******************************************************************************
LAST MODIFIED : 29 January 1999

DESCRIPTION :
If there is a data_grabber dialog in existence, then bring it to the front,
else create a new one.
==============================================================================*/
#endif
