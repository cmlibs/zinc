/*******************************************************************************
FILE : application.h

LAST MODIFIED : 19 June 1996

DESCRIPTION :
The data structures and function prototypes linking cmgui with applications.
==============================================================================*/
#if !defined (APPLICATION_H)
#define APPLICATION_H

#include "command/parser.h"

/*
Global functions
----------------
*/
int set_application_parameters(struct Parse_state *state,
	void *dummy_to_be_modified,void *dummy_user_data);
/*******************************************************************************
LAST MODIFIED : 19 June 1996

DESCRIPTION :
Called in response to the command
	gfx set application_parameters ...
==============================================================================*/
#endif /* !defined (APPLICATION_H) */
