/*******************************************************************************
FILE : value.h

LAST MODIFIED : 12 June 2003

DESCRIPTION :
A value type that knows what its type is.  So that can have a single function
for setting values and a single function for getting values and still have type
checking.
==============================================================================*/
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (VALUE_H)
#define VALUE_H

#include "cmlibs/zinc/zincconfigure.h"

#if defined (UNIX)
#  if defined (CYGWIN)
   /*#include <w32api/winnt.h>*/
#    define MINSHORT  0x8000
#    define MAXSHORT  0x7fff
#  else /* defined (CYGWIN) */
#    if defined (DARWIN)
#    else /* defined (DARWIN) */
#      include <limits.h>
#    endif /* defined (DARWIN) */
#  endif /* defined (CYGWIN) */
#endif /* defined (UNIX) */

/*
Global types
------------
*/

#define CAST_TO_FE_VALUE(FE, OTHER, LENGTH) \
	{ \
		int cast_i; \
		for (cast_i=0;cast_i< LENGTH;cast_i++) FE[cast_i] = static_cast<FE_value>(OTHER[cast_i]); \
	}

#define CAST_TO_FE_VALUE_C(FE, OTHER, LENGTH) \
	{ \
		int cast_i; \
		for (cast_i=0;cast_i< LENGTH;cast_i++) FE[cast_i] = (FE_value)(OTHER[cast_i]); \
	}

#define CAST_TO_OTHER(OTHER, FE_VALUE, TYPE, LENGTH) \
	{ \
		int cast_i; \
		for (cast_i=0;cast_i< LENGTH;cast_i++) OTHER[cast_i] = static_cast< TYPE >(FE_VALUE[cast_i]); \
	}

  // not used
#define CAST_TO_OTHER_C(OTHER, FE_VALUE, TYPE, LENGTH) \
	{ \
		int cast_i; \
		for (cast_i=0;cast_i< LENGTH;cast_i++) OTHER[cast_i] = (TYPE)(FE_VALUE[cast_i]); \
	}

/* the value that FE_value's are initialized to.  Some machines will have
	"not a number" which should be used as the initializer */
#define FE_VALUE_INITIALIZER 0
/* Used with FE_VALUE_STRING in export_finite_element to keep numerical output
	 of arrays to a reasonable page width. A value <= 0 means no columns. */
#define FE_VALUE_MAX_OUTPUT_COLUMNS 5
/* Compare with FE_value to determine a "zero" value */
#define FE_VALUE_ZERO_TOLERANCE (1e-8)

/* May get probs if != unsigned char, see set_FE_field_XXX_value */
typedef unsigned char Value_storage;

enum Value_type
/*******************************************************************************
LAST MODIFIED : 25 August 1999

DESCRIPTION :
The different "basic" types that can be kept in a Value.
Make sure there is an entry in Value_type_string for each added type!
Have members BEFORE_FIRST and AFTER_LAST to enable iterating through the list
without knowing which order the types are in.
==============================================================================*/
{
	UNKNOWN_VALUE,
	VALUE_TYPE_BEFORE_FIRST,
	DOUBLE_ARRAY_VALUE,
	DOUBLE_VALUE,
	ELEMENT_XI_VALUE,
	FE_VALUE_ARRAY_VALUE,
	FE_VALUE_VALUE,
	FLT_ARRAY_VALUE,
	FLT_VALUE, /* FLOAT_VALUE used in /unemap/ and /cell/ */
	INT_ARRAY_VALUE,
	INT_VALUE,
	SHORT_ARRAY_VALUE, 
	SHORT_VALUE, /* beware memory alignment probs. Pointers must be DWORD aligned*/
	STRING_VALUE,
	UNSIGNED_ARRAY_VALUE,
	UNSIGNED_VALUE,
	URL_VALUE, /* actually just a string */
	VALUE_TYPE_AFTER_LAST
}; /* enum Value_type */

/* A replacement for the use of Triple for values that really should be FE_value's */
typedef FE_value FE_value_triple[3];

/*
Global functions
----------------
*/

const char *Value_type_string(enum Value_type value_type);
/*******************************************************************************
LAST MODIFIED : 25 August 1999

DESCRIPTION :
Returns a pointer to a static string describing the <value_type>, eg.
DOUBLE_VALUE == "double". This string should match the command used
to create the edit object. The returned string must not be DEALLOCATEd!
==============================================================================*/

enum Value_type Value_type_from_string(const char *value_type_string);
/*******************************************************************************
LAST MODIFIED : 17 September 1999

DESCRIPTION :
Returns the value_type from the string, eg "integer" = INT_VALUE.
==============================================================================*/

// ==## unused 
const char **Value_type_get_valid_strings_simple(int *number_of_valid_strings);
/*******************************************************************************
LAST MODIFIED : 10 May 2000

DESCRIPTION :
Returns and allocated array of pointers to all static strings for valid
Value_types - obtained from function Value_type_string.
Does not return any array types.
Up to calling function to deallocate returned array - but not the strings in it!
==============================================================================*/

// ==## unused 
int Value_type_is_array(enum Value_type value_type);
/*******************************************************************************
LAST MODIFIED : 16 September 1999

DESCRIPTION :
Returns true if the value_type is an array.
==============================================================================*/

inline bool Value_type_is_non_numeric(enum Value_type value_type)
{
	return (value_type == ELEMENT_XI_VALUE)
		|| (value_type == STRING_VALUE)
		|| (value_type == URL_VALUE)
		|| (value_type == UNKNOWN_VALUE);
}

int Value_type_is_numeric_simple(enum Value_type value_type);
/*******************************************************************************
LAST MODIFIED : 22 May 2000

DESCRIPTION :
Returns true if the value_type is a simple number: real, integer etc.
==============================================================================*/

// ==## unused 
enum Value_type Value_type_simple_to_array(enum Value_type value_type);
/*******************************************************************************
LAST MODIFIED : 16 September 1999

DESCRIPTION :
If the <value_type> is a non-array type with an array equivalent then the latter
is returned, otherwise an error is reported and the original type is returned.
==============================================================================*/

// ==## unused 
enum Value_type Value_type_array_to_simple(enum Value_type value_type);
/*******************************************************************************
LAST MODIFIED : 16 September 1999

DESCRIPTION :
If the <value_type> is an array type with a non-array equivalent then the latter
is returned, otherwise an error is reported and the original type is returned.
==============================================================================*/

#endif /* !defined (VALUE_H) */
