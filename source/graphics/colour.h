/*******************************************************************************
FILE : colour.h

LAST MODIFIED : 18 June 1996

DESCRIPTION :
Colour structures and support code.
???DB.  I'm not sure that colour needs to be abstracted/formalized.
==============================================================================*/
#if !defined( COLOUR_H )
#define COLOUR_H
#include "command/parser.h"

#define COLOUR_PRECISION float
#define COLOUR_PRECISION_STRING "f"
#define COLOUR_NUM_FORMAT "%6.4" COLOUR_PRECISION_STRING

#define COLOUR_VECTOR(colour_struct_ptr) \
((COLOUR_PRECISION *)colour_struct_ptr)
/*
Global types
------------
*/
struct Colour
/*******************************************************************************
LAST MODIFIED : 9 November 1994

DESCRIPTION :
==============================================================================*/
{
	COLOUR_PRECISION blue,green,red;
}; /* struct Colour */

/*
Global functions
----------------
*/
struct Colour *create_Colour(COLOUR_PRECISION red,COLOUR_PRECISION green,
	COLOUR_PRECISION blue);
/*******************************************************************************
LAST MODIFIED : 11 November 1994

DESCRIPTION :
Allocates memory and assigns field for a colour.
==============================================================================*/

void destroy_Colour(struct Colour **colour_address);
/*******************************************************************************
LAST MODIFIED : 11 November 1994

DESCRIPTION :
Frees the memory for the colour and sets <*colour_address> to NULL.
==============================================================================*/

int set_Colour(struct Parse_state *state,void *colour_void,
	void *dummy_user_data);
/*******************************************************************************
LAST MODIFIED : 18 June 1996

DESCRIPTION :
A modifier function to set the colour rgb values.
==============================================================================*/
#endif
