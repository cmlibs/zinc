/*******************************************************************************
FILE : data_grabber_dialog.h

LAST MODIFIED : 9 January 2003

DESCRIPTION :
Brings up a window which holds a data_grabber.  Allows the user to change what
data is accepted - pos,tangent,normal.
==============================================================================*/
#if !defined (DATA_GRABBER_DIALOG_H)
#define DATA_GRABBER_DIALOG_H

/*
Global Functions
----------------
*/

int bring_up_data_grabber_dialog(Widget *data_grabber_dialog_address,
	Widget parent, struct Execute_command *execute_command,
	struct User_interface *user_interface,
	struct Cmiss_region *root_region, struct Cmiss_region *current_region);
/*******************************************************************************
LAST MODIFIED : 9 January 2003

DESCRIPTION :
If there is a data_grabber dialog in existence, then bring it to the front,
else create a new one.
==============================================================================*/

#endif /* !defined (DATA_GRABBER_DIALOG_H) */
