
#include "command/parser.h"



#define COLOUR_PRECISION float
#define COLOUR_PRECISION_STRING "f"
#define COLOUR_NUM_FORMAT "%6.4" COLOUR_PRECISION_STRING

int set_Colour(struct Parse_state *state,void *colour_void,
	void *dummy_user_data);
/*******************************************************************************
LAST MODIFIED : 18 June 1996

DESCRIPTION :
A modifier function to set the colour rgb values.
==============================================================================*/
