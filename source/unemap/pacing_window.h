/*******************************************************************************
FILE : pacing_window.h

LAST MODIFIED : 15 October 2001

DESCRIPTION :
==============================================================================*/
#if !defined (PACING_WINDOW_H)
#define PACING_WINDOW_H

#include "unemap/rig.h"
#include "user_interface/user_interface.h"

/*
Global types
------------
*/
struct Pacing_window;
/*******************************************************************************
LAST MODIFIED : 10 October 2001

DESCRIPTION :
The pacing window object.
==============================================================================*/

/*
Global functions
----------------
*/
struct Pacing_window *open_Pacing_window(
	struct Pacing_window **pacing_window_address,struct Rig **rig_address,
#if defined (MOTIF)
	Widget activation,
#endif /* defined (MOTIF) */
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 15 October 2001

DESCRIPTION :
If the pacing window does not exist, it is created.  The pacing window is
opened.
==============================================================================*/

int destroy_Pacing_window(struct Pacing_window **pacing_window_address);
/*******************************************************************************
LAST MODIFIED : 10 October 2001

DESCRIPTION :
If the <address> field of the pacing window is not NULL, <*address> is set to
NULL.  If the <activation> field is not NULL, the <activation> widget is
unghosted.  The function frees the memory associated with the fields of the
pacing window and frees the memory associated with the pacing window.
==============================================================================*/
#endif /* !defined (PACING_WINDOW_H) */
