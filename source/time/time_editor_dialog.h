/*******************************************************************************
FILE : time_editor_dialog.h

LAST MODIFIED : 17 May 2002

DESCRIPTION :
Header description for time_editor_dialog widget.
==============================================================================*/
#if !defined (TIME_EDITOR_DIALOG_H)
#define TIME_EDITOR_DIALOG_H

#include "general/callback.h"
#include "user_interface/user_interface.h"

/*
Global Types
------------
*/
struct Time_editor_dialog;

/*
Global Functions
----------------
*/
int time_editor_dialog_get_callback(
	struct Time_editor_dialog *time_editor_dialog,struct Callback_data *callback);
/*******************************************************************************
LAST MODIFIED : 17 May 2002

DESCRIPTION :
Get the update <callback> information for the <time_editor_dialog>.
==============================================================================*/

int time_editor_dialog_set_callback(
	struct Time_editor_dialog *time_editor_dialog,struct Callback_data *callback);
/*******************************************************************************
LAST MODIFIED : 17 May 2002

DESCRIPTION :
Set the update <callback> information for the <time_editor_dialog>.
==============================================================================*/

int time_editor_dialog_get_time_keeper(
	struct Time_editor_dialog *time_editor_dialog,
	struct Time_keeper **time_keeper_address);
/*******************************************************************************
LAST MODIFIED : 17 May 2002

DESCRIPTION :
Get the <*time_keeper_address> for the <time_editor_dialog>.
==============================================================================*/

int time_editor_dialog_set_time_keeper(
	struct Time_editor_dialog *time_editor_dialog,
	struct Time_keeper *time_keeper);
/*******************************************************************************
LAST MODIFIED : 17 May 2002

DESCRIPTION :
Set the <time_keeper> for the <time_editor_dialog>.
==============================================================================*/

int time_editor_dialog_get_step(struct Time_editor_dialog *time_editor_dialog,
	float *step);
/*******************************************************************************
LAST MODIFIED : 17 May 2002

DESCRIPTION :
Get the <*step> for the <time_editor_dialog>.
==============================================================================*/

int time_editor_dialog_set_step(struct Time_editor_dialog *time_editor_dialog,
	float step);
/*******************************************************************************
LAST MODIFIED : 17 May 2002

DESCRIPTION :
Set the <step> for the <time_editor_dialog>.
==============================================================================*/

int bring_up_time_editor_dialog(
	struct Time_editor_dialog **time_editor_dialog_address,
	Widget parent,struct Time_keeper *time_keeper,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 17 May 2002

DESCRIPTION :
If there is a <*time_editor_dialog_address> in existence, then bring it to the
front, otherwise create a new one.
==============================================================================*/

int DESTROY(Time_editor_dialog)(
	struct Time_editor_dialog **time_editor_dialog_address);
/*******************************************************************************
LAST MODIFIED : 17 May 2002

DESCRIPTION :
Destroy the <*time_editor_dialog_address> and sets <*time_editor_dialog_address>
to NULL.
==============================================================================*/
#endif /* !defined (TIME_EDITOR_DIALOG_H) */
