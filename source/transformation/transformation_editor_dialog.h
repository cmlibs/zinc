/*******************************************************************************
FILE : transformation_editor_dialog.h

LAST MODIFIED : 26 November 2001

DESCRIPTION :
This module creates a free transformation_editor_dialog input device.
???DB.  At present just a shell around a transformation_editor
==============================================================================*/
#if !defined (TRANSFORMATION_EDITOR_DIALOG_H)
#define TRANSFORMATION_EDITOR_DIALOG_H

#include "general/callback.h"
#include "view/coord.h"
	/*???DB.  For Cmgui_coordinate */

/*
Global types
------------
*/
enum Transformation_editor_dialog_data_type
/*******************************************************************************
LAST MODIFIED : 7 September 1998

DESCRIPTION :
Contains the different types of data items that are valid for the
transformation_editor_dialog widget.
==============================================================================*/
{
	TRANSFORMATION_EDITOR_DIALOG_UPDATE_CB=0,
	TRANSFORMATION_EDITOR_DIALOG_DESTROY_CB=1,
	TRANSFORMATION_EDITOR_DIALOG_DATA
}; /* enum Transformation_editor_dialog_data_type */

#define TRANSFORMATION_EDITOR_DIALOG_NUM_CALLBACKS 2

struct Transformation_editor_dialog_struct
/*******************************************************************************
LAST MODIFIED : 17 April 1995

DESCRIPTION :
Contains all the information carried by the transformation_editor_dialog widget.
Note that we just hold a pointer to the transformation_editor_dialog, and must
access and deaccess it.
==============================================================================*/
{
	struct Cmgui_coordinate *current_value;
	struct Callback_data
		callback_array[TRANSFORMATION_EDITOR_DIALOG_NUM_CALLBACKS];
	Widget *dialog_address,dialog,widget,dialog_parent;
	Widget editor_widget;
}; /* struct Transformation_editor_dialog_struct */

/*
Global functions
----------------
*/
int transformation_editor_dialog_set_data(
	Widget transformation_editor_dialog_widget,
	enum Transformation_editor_dialog_data_type data_type,void *data);
/*******************************************************************************
LAST MODIFIED : 17 April 1995

DESCRIPTION :
If <transformation_editor_dialog_widget> is not NULL, then change the data item
on <transformation_editor_dialog widget>.  Otherwise, change the data item on
<transformation_editor_dialog>.
==============================================================================*/

void *transformation_editor_dialog_get_data(
	Widget transformation_editor_dialog_widget,
	enum Transformation_editor_dialog_data_type data_type);
/*******************************************************************************
LAST MODIFIED : 17 April 1995

DESCRIPTION :
If <transformation_editor_dialog_widget> is not NULL, then get the data item
from <transformation_editor_dialog widget>.  Otherwise, get the data item from
<transformation_editor_dialog>.
==============================================================================*/

int bring_up_transformation_editor_dialog(
	Widget *transformation_editor_dialog_address,
	struct Cmgui_coordinate *init_data,struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 26 November 2001

DESCRIPTION :
If there is a transformation editor dialog in existence, then bring it to the
front, otherwise it creates a new one.
???DB.  Assumes we will only ever want one transformation editor at a time.
	This may be changed later.
==============================================================================*/
#endif
