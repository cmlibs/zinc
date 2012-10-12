
#if !defined (GEOMETRY_APP_H_)
#define GEOMETRY_APP_H_

#include "command/parser.h"


int set_Coordinate_system(struct Parse_state *state,
	void *coordinate_system_void,void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 22 January 1999

DESCRIPTION :
Currently only allows rectangular cartesian to be set.
???RC JW to change to struct Coordinate_system, handle parsing of focus for
prolate, etc.
==============================================================================*/

#endif
