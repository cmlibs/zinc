/*******************************************************************************
FILE : value.h

LAST MODIFIED : 10 May 2000

DESCRIPTION :
A value type that knows what its type is.  So that can have a single function
for setting values and a single function for getting values and still have type
checking.
==============================================================================*/
#if !defined (VALUE_H)
#define VALUE_H

/*
Global types
------------
*/
/*???DB.  Can only use float because of conflict with Triple in
	standard_basis_functions */
typedef float FE_value;
/* the value that FE_value's are initialized to.  Some machines will have
	"not a number" which should be used as the initializer */
#define FE_VALUE_INITIALIZER 0
/* used when reading FE_values */
#define FE_VALUE_INPUT_STRING "%f"
/* necessary if we want to specify %10.2f etc */
#define FE_VALUE_STRING "13.6e"
/* Used with FE_VALUE_STRING in export_finite_element to keep numerical output
	 of arrays to a reasonable page width. A value <= 0 means no columns. */
#define FE_VALUE_MAX_OUTPUT_COLUMNS 5
/* used when reading FE_values */
#define DOUBLE_VALUE_INPUT_STRING "%f"
/* necessary if we want to specify %10.2f etc */
#define DOUBLE_VALUE_STRING "13.6e"
/* Used with DOUBLE_VALUE_STRING */
#define DOUBLE_VALUE_MAX_OUTPUT_COLUMNS 5
/* used when reading Double values */
#define SHORT_VALUE_MAX_OUTPUT_COLUMNS 10

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

#define MAXIMUM_ELEMENT_XI_DIMENSIONS (3)

/*
Global functions
----------------
*/

char *Value_type_string(enum Value_type value_type);
/*******************************************************************************
LAST MODIFIED : 25 August 1999

DESCRIPTION :
Returns a pointer to a static string describing the <value_type>, eg.
DOUBLE_VALUE == "double". This string should match the command used
to create the edit object. The returned string must not be DEALLOCATEd!
==============================================================================*/

enum Value_type Value_type_from_string(char *value_type_string);
/*******************************************************************************
LAST MODIFIED : 17 September 1999

DESCRIPTION :
Returns the value_type from the string, eg "integer" = INT_VALUE.
==============================================================================*/

char **Value_type_get_valid_strings_simple(int *number_of_valid_strings);
/*******************************************************************************
LAST MODIFIED : 10 May 2000

DESCRIPTION :
Returns and allocated array of pointers to all static strings for valid
Value_types - obtained from function Value_type_string.
Does not return any array types.
Up to calling function to deallocate returned array - but not the strings in it!
==============================================================================*/

int Value_type_is_array(enum Value_type value_type);
/*******************************************************************************
LAST MODIFIED : 16 September 1999

DESCRIPTION :
Returns true if the value_type is an array.
==============================================================================*/

int Value_type_is_numeric_simple(enum Value_type value_type);
/*******************************************************************************
LAST MODIFIED : 22 May 2000

DESCRIPTION :
Returns true if the value_type is a simple number: real, integer etc.
==============================================================================*/

enum Value_type Value_type_simple_to_array(enum Value_type value_type);
/*******************************************************************************
LAST MODIFIED : 16 September 1999

DESCRIPTION :
If the <value_type> is a non-array type with an array equivalent then the latter
is returned, otherwise an error is reported and the original type is returned.
==============================================================================*/

enum Value_type Value_type_array_to_simple(enum Value_type value_type);
/*******************************************************************************
LAST MODIFIED : 16 September 1999

DESCRIPTION :
If the <value_type> is an array type with a non-array equivalent then the latter
is returned, otherwise an error is reported and the original type is returned.
==============================================================================*/

#endif /* !defined (VALUE_H) */
