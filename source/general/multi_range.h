/*******************************************************************************
FILE : multi_range.h

LAST MODIFIED : 21 April 1999

DESCRIPTION :
Structure for storing and manipulating multiple, non-overlapping ranges of
values, eg. 1-5,7-7,29-100.
At present, limited to int type, but could be converted to other number types.
==============================================================================*/
#if !defined (MULTI_RANGE_H)
#define MULTI_RANGE_H

#include "command/parser.h"

/*
Global types
------------
*/
struct Multi_range;

/*
Global functions
----------------
*/
struct Multi_range *CREATE(Multi_range)(void);
/*******************************************************************************
LAST MODIFIED : 11 March 1998

DESCRIPTION :
Creates and returns an empty Multi_range structure.
==============================================================================*/

int DESTROY(Multi_range)(struct Multi_range **multi_range_address);
/*******************************************************************************
LAST MODIFIED : 11 March 1998

DESCRIPTION :
Frees the space used by the Multi_range structure.
==============================================================================*/

int Multi_range_clear(struct Multi_range *multi_range);
/*******************************************************************************
LAST MODIFIED : 25 March 1998

DESCRIPTION :
Clears all the ranges in <multi_range>.
==============================================================================*/

int Multi_range_copy(struct Multi_range *destination,
	struct Multi_range *source);
/*******************************************************************************
LAST MODIFIED : 26 April 1998

DESCRIPTION :
Makes the <destination> Multi_range an exact copy of <source>.
==============================================================================*/

int Multi_range_add_range(struct Multi_range *multi_range,int start,int stop);
/*******************************************************************************
LAST MODIFIED : 15 April 1999

DESCRIPTION :
Makes sure <multi_range> contains a contiguous range from start to stop. Any
ranges that intersect with the new range are combined with it.
If <start> is greater than <stop>, the two are swapped and the range added.
==============================================================================*/

int Multi_range_remove_range(struct Multi_range *multi_range,
	int start,int stop);
/*******************************************************************************
LAST MODIFIED : 15 April 1999

DESCRIPTION :
Makes sure <multi_range> does not have any entries from start to stop.
If <start> is greater than <stop>, the two are swapped and the range added.
==============================================================================*/

int Multi_range_is_value_in_range(struct Multi_range *multi_range,int value);
/*******************************************************************************
LAST MODIFIED : 12 March 1998

DESCRIPTION :
Returns true if <value> is in any range in <multi_range>.
==============================================================================*/

int Multi_ranges_overlap(struct Multi_range *multi_range1,
	struct Multi_range *multi_range2);
/*******************************************************************************
LAST MODIFIED : 21 April 1999

DESCRIPTION :
Returns true if <multi_range1> and <multi_range2> have any overlapping ranges.
==============================================================================*/

int Multi_range_get_last_start_value(struct Multi_range *multi_range,int value,
	int *last_start_value);
/*******************************************************************************
LAST MODIFIED : 1 April 1998

DESCRIPTION :
Returns the next lower start value before value in the <multi_range>.
If there is none or an error occurs, the return_code will be 0.
==============================================================================*/

int Multi_range_get_last_stop_value(struct Multi_range *multi_range,int value,
	int *last_stop_value);
/*******************************************************************************
LAST MODIFIED : 12 March 1998

DESCRIPTION :
Returns the next lower stop value to value in the <multi_range>. If there is
none or an error occurs, the return_code will be 0.
==============================================================================*/

int Multi_range_get_next_start_value(struct Multi_range *multi_range,int value,
	int *next_start_value);
/*******************************************************************************
LAST MODIFIED : 12 March 1998

DESCRIPTION :
Returns the next higher start value to value in the <multi_range>. If there is
none or an error occurs, the return_code will be 0.
==============================================================================*/

int Multi_range_get_next_stop_value(struct Multi_range *multi_range,int value,
	int *next_stop_value);
/*******************************************************************************
LAST MODIFIED : 1 April 1998

DESCRIPTION :
Returns the next higher stop value to value in the <multi_range>. If there is
none or an error occurs, the return_code will be 0.
==============================================================================*/

int Multi_range_get_number_of_ranges(struct Multi_range *multi_range);
/*******************************************************************************
LAST MODIFIED : 12 March 1998

DESCRIPTION :
==============================================================================*/

int Multi_range_get_range(struct Multi_range *multi_range,int range_no,
	int *start,int *stop);
/*******************************************************************************
LAST MODIFIED : 12 March 1998

DESCRIPTION :
Returns the start and stop values for range[range_no] in <multi_range>.
Valid range numbers are from 0 to number_of_ranges-1.
==============================================================================*/

int Multi_range_print(struct Multi_range *multi_range);
/*******************************************************************************
LAST MODIFIED : 12 March 1998

DESCRIPTION :
Writes out the contents of the <multi_range>.
==============================================================================*/

int Multi_range_test(void);
/*******************************************************************************
LAST MODIFIED : 12 March 1998

DESCRIPTION :
Temporary.
==============================================================================*/

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
==============================================================================*/

#endif /* !defined (MULTI_RANGE_H) */
