/*******************************************************************************
FILE : time_editor.h

LAST MODIFIED : 8 December 1998

DESCRIPTION :
==============================================================================*/
#if !defined (TIME_EDITOR_H)
#define TIME_EDITOR_H

#include "general/callback.h"

/*
Global Functions
----------------
*/

Widget create_time_editor_widget(Widget *time_editor_widget,
	Widget parent, struct Time_keeper *time_keeper, 
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 8 December 1998

DESCRIPTION :
==============================================================================*/

int time_editor_get_callback(Widget time_editor_widget,
	struct Callback_data *callback);
/*******************************************************************************
LAST MODIFIED : 8 December 1998

DESCRIPTION :
Returns the update_callback for the time editor widget.
==============================================================================*/

int time_editor_set_callback(Widget time_editor_widget,
	struct Callback_data *callback);
/*******************************************************************************
LAST MODIFIED : 8 December 1998

DESCRIPTION :
Changes the update_callback for the time editor widget.
==============================================================================*/

struct Time_keeper *time_editor_get_time_keeper(
	Widget time_editor_widget);
/*******************************************************************************
LAST MODIFIED : 8 December 1998

DESCRIPTION :
Returns the address of the time being edited in the time_editor widget.
Do not modify or DEALLOCATE the returned time_keeper.
==============================================================================*/

int time_editor_set_time_keeper(Widget time_editor_widget,
	struct Time_keeper *time_keeper);
/*******************************************************************************
LAST MODIFIED : 8 December 1998

DESCRIPTION :
Changes the time_keeper in the time_editor widget.
==============================================================================*/
#endif
