/*******************************************************************************
FILE : time_editor.h

LAST MODIFIED : 17 May 2002

DESCRIPTION :
==============================================================================*/
#if !defined (TIME_EDITOR_H)
#define TIME_EDITOR_H

#include "general/callback.h"

/*
Global Types
------------
*/
struct Time_editor;

/*
Global Functions
----------------
*/
struct Time_editor *CREATE(Time_editor)(
	struct Time_editor **time_editor_address,
	Widget parent,struct Time_keeper *time_keeper,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 17 May 2002

DESCRIPTION :
Creates a <*time_editor_address>.
==============================================================================*/

int DESTROY(Time_editor)(struct Time_editor **time_editor_address);
/*******************************************************************************
LAST MODIFIED : 17 May 2002

DESCRIPTION :
Destroy the <*time_editor_address> and sets <*time_editor_address> to NULL.
==============================================================================*/

int time_editor_get_close_callback(struct Time_editor *time_editor,
	struct Callback_data *callback);
/*******************************************************************************
LAST MODIFIED : 8 June 2004

DESCRIPTION :
Get the close <callback> information for the <time_editor>.
==============================================================================*/

int time_editor_set_close_callback(struct Time_editor *time_editor,
	struct Callback_data *callback);
/*******************************************************************************
LAST MODIFIED : 8 June 2004

DESCRIPTION :
Set the close <callback> information for the <time_editor>.
==============================================================================*/

int time_editor_get_time_keeper(struct Time_editor *time_editor,
	struct Time_keeper **time_keeper);
/*******************************************************************************
LAST MODIFIED : 17 May 2002

DESCRIPTION :
Get the <*time_keeper_address> for the <time_editor>.
Do not modify or DEALLOCATE the returned time_keeper.
==============================================================================*/

int time_editor_set_time_keeper(struct Time_editor *time_editor,
	struct Time_keeper *time_keeper);
/*******************************************************************************
LAST MODIFIED : 17 May 2002

DESCRIPTION :
Set the <time_keeper> for the <time_editor>.
==============================================================================*/

int time_editor_get_step(struct Time_editor *time_editor,float *step);
/*******************************************************************************
LAST MODIFIED : 17 May 2002

DESCRIPTION :
Get the <*step> for the <time_editor>.
==============================================================================*/

int time_editor_set_step(struct Time_editor *time_editor,float step);
/*******************************************************************************
LAST MODIFIED : 17 May 2002

DESCRIPTION :
Set the <step> for the <time_editor>.
==============================================================================*/
#endif /* !defined (TIME_EDITOR_H) */
