/*******************************************************************************
FILE : time_editor_dialog.h

LAST MODIFIED : 8 December 1998

DESCRIPTION :
Header description for time_editor_dialog widget.
==============================================================================*/
#if !defined (TIME_EDITOR_DIALOG_H)
#define TIME_EDITOR_DIALOG_H

#include "general/callback.h"
#include "user_interface/user_interface.h"

/*
Global Functions
----------------
*/
int time_editor_dialog_get_callback(Widget time_editor_dialog_widget,
	struct Callback_data *callback);
/*******************************************************************************
LAST MODIFIED : 8 December 1998

DESCRIPTION :
Returns the update_callback for the time editor_dialog widget.
==============================================================================*/

int time_editor_dialog_set_callback(Widget time_editor_dialog_widget,
	struct Callback_data *callback);
/*******************************************************************************
LAST MODIFIED : 8 December 1998

DESCRIPTION :
Changes the update_callback for the time editor_dialog widget.
==============================================================================*/

struct Time_keeper *time_editor_dialog_get_time_keeper(
	Widget time_editor_dialog_widget);
/*******************************************************************************
LAST MODIFIED : 8 December 1998

DESCRIPTION :
If <time_editor_dialog_widget> is not NULL, then get the data item from
<time_editor_dialog widget>.  Otherwise, get the data item from
<time_editor_dialog>.
==============================================================================*/

int time_editor_dialog_set_time_keeper(Widget time_editor_dialog_widget,
	struct Time_keeper *time_keeper);
/*******************************************************************************
LAST MODIFIED : 8 December 1998

DESCRIPTION :
If <time_editor_dialog_widget> is not NULL, then change the data item on
<time_editor_dialog widget>.  Otherwise, change the data item on
<time_editor_dialog>.
==============================================================================*/

int bring_up_time_editor_dialog(Widget *time_editor_dialog_address,
	Widget parent, struct Time_keeper *time_keeper,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 8 December 1998

DESCRIPTION :
If there is a time_editor dialog in existence, then bring it to the front,
else create a new one.
==============================================================================*/
#endif
