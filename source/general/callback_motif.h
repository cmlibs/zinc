/*******************************************************************************
FILE : callback_motif.h

LAST MODIFIED : 17 June 2004

DESCRIPTION :
Old legacy motif callbacks.
==============================================================================*/
#if ! defined (CALLBACK_MOTIF_H)
#define CALLBACK_MOTIF_H
#if defined (MOTIF)
#include <Xm/Xm.h>
#endif /* defined (MOTIF) */

/*
Global Types
------------
*/

#if defined (MOTIF)
/*???DB.  Should have a return_code ? */
typedef void Callback_procedure(Widget,void *,void *);

struct Callback_data
/*******************************************************************************
LAST MODIFIED : 4 January 1995

DESCRIPTION :
Contains all information necessary for a callback.
==============================================================================*/
{
	Callback_procedure *procedure;
	void *data;
}; /* struct Callback_data */
#endif /* defined (MOTIF) */

#endif /* ! defined (CALLBACK_MOTIF_H) */
