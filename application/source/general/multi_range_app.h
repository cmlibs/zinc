
#include "command/parser.h"


/*==============================================================================*/

int set_Multi_range(struct Parse_state *state,
	void *multi_range_void,void *dummy_user_data);
/*******************************************************************************
LAST MODIFIED : 15 April 1999

DESCRIPTION :
Clears the existing Multi_range (which must have been created outside here) and
fills it with a new set of ranges. Ranges are entered as # or #..# in separate
tokens - as long as there is number in the token it is assumed to be a new
range. Numbers may start with + or - and have 0-9 as the first real entry.
Ranges may overlap and be increasing or decreasing. Typical inputs are:
4
5..7
3,5,7 (both commas and spaces are valid token separators).
1..2 3 6..9
3..5,9..1 (has the same result as 1..9)
*/
