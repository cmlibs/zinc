/*******************************************************************************
FILE : setup_dialog.h

LAST MODIFIED : 31 December 1996

DESCRIPTION :
???DB.  Regions with different types ?
==============================================================================*/
#if !defined (SETUP_DIALOG_H)
#define SETUP_DIALOG_H

#include <stddef.h>
#if defined (MOTIF)
#include <X11/Intrinsic.h>
#include <X11/Xlib.h>
#include <Xm/Xm.h>
#endif /* defined (MOTIF) */
#include "user_interface/user_interface.h"

/*
Global types
------------
*/
struct Electrodes_in_row
/*******************************************************************************
LAST MODIFIED : 5 March 1993

DESCRIPTION :
The "widget" that controls the number of electrodes in a particular row.
==============================================================================*/
{
	Widget form;
	Widget label;
	Widget down_arrow;
	Widget number_label;
	Widget up_arrow;
	Widget separator;
	int number,row;
	struct Electrodes_in_row *next;
}; /* struct Electrodes_in_row */

struct Setup_dialog
/*******************************************************************************
LAST MODIFIED : 31 December 1996

DESCRIPTION :
The dialog associated with the set up button.
==============================================================================*/
{
	Widget activation,dialog,shell;
	struct Shell_list_item *shell_list_item;
	Widget rig_choice;
	struct
	{
		Widget sock;
		Widget patch;
		Widget torso;
	} region_type;
	struct
	{
		Widget down_arrow;
		Widget number;
		Widget up_arrow;
	} auxiliary;
	int number_of_auxiliary_devices;
	struct
	{
		Widget down_arrow;
		Widget number;
		Widget up_arrow;
	} regions;
	int number_of_regions;
	struct
	{
		Widget form;
		Widget down_arrow;
		Widget number;
		Widget up_arrow;
	} rows;
	int number_of_rows;
	struct Electrodes_in_row *electrodes_in_row;
	Widget separator;
	Widget setup_button;
	Widget cancel_button;
	struct Setup_dialog **address;
	struct User_interface *user_interface;
}; /* struct Setup_dialog */

/*
Global functions
----------------
*/
struct Setup_dialog *create_Setup_dialog(
	struct Setup_dialog **setup_dialog_address,Widget activation,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 31 December 1996

DESCRIPTION :
Allocates the memory for a setup dialog.  Retrieves the necessary widgets and
initializes the appropriate fields.
==============================================================================*/

void close_setup_dialog(Widget widget,XtPointer setup_dialog_structure,
	XtPointer call_data);
/*******************************************************************************
LAST MODIFIED : 22 November 1992

DESCRIPTION :
Closes the windows associated withe the setup dialog box.
==============================================================================*/
#endif
