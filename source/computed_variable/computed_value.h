/*******************************************************************************
FILE : computed_value.h

LAST MODIFIED : 25 July 2003

DESCRIPTION :
A module intended to replace general/value .  Testing and developing in
conjunction with Cmiss_variables.

???DB.  Should _get_s try and get a representation if not specified type?
???DB.  Merge FE_value, FE_value_vector and FE_value_matrix?
==============================================================================*/
#if !defined (__CMISS_VALUE_H__)
#define __CMISS_VALUE_H__

#include "api/cmiss_value.h"
#include "general/list.h"
#include "general/object.h"
#include "general/value.h"

/*
Global macros
-------------
*/
#define CMISS_VALUE_IS_TYPE( value_type ) \
	Cmiss_value_ ## value_type ## _is_type

#define PROTOTYPE_CMISS_VALUE_IS_TYPE_FUNCTION( value_type ) \
int CMISS_VALUE_IS_TYPE(value_type)(Cmiss_value_id value) \
/***************************************************************************** \
LAST MODIFIED : 13 February 2003 \
\
DESCRIPTION : \
Returns a non-zero if <value> is a #value_type and zero otherwise. \
==============================================================================*/

/*
Global types
------------
*/
/* Now defined in the API as this is a public object pointer */
/* typedef struct Cmiss_value *Cmiss_value_id; */
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
A value that knows what type it is.

???DB.  An update of Value_type (general/value).  Want to be able to add new
	value types (in the same way that Computed_field types can be added).  Will
	need a package?  Will replace Value_type?
???DB.  At present just extend?
==============================================================================*/

/*
Global functions
----------------
*/
Cmiss_value_id CREATE(Cmiss_value)(void);
/*******************************************************************************
LAST MODIFIED : 22 January 2003

DESCRIPTION :
Creates an empty value with no type.  Each type of value has its own "set_type"
function.
==============================================================================*/

int DESTROY(Cmiss_value)(Cmiss_value_id *value_address);
/*******************************************************************************
LAST MODIFIED : 22 January 2003

DESCRIPTION :
Frees memory/deaccess objects for Cmiss_value at <*value_address>.
==============================================================================*/

int Cmiss_value_copy(Cmiss_value_id destination,
	Cmiss_value_id source);
/*******************************************************************************
LAST MODIFIED : 23 March 2003

DESCRIPTION :
Copies the type and contents from <source> to <destination>.
==============================================================================*/

PROTOTYPE_OBJECT_FUNCTIONS(Cmiss_value);

int Cmiss_value_same_sub_type(Cmiss_value_id value_1,
	Cmiss_value_id value_2);
/*******************************************************************************
LAST MODIFIED : 13 February 2003

DESCRIPTION :
Returns nonzero if <value_1> and <value_2> have the same sub-type and zero
otherwise.
==============================================================================*/

char *Cmiss_value_get_type_id_string(Cmiss_value_id value);
/*******************************************************************************
LAST MODIFIED : 12 February 2003

DESCRIPTION :
Returns the string which identifies the type.  The calling function must not
DEALLOCATE the returned string.
==============================================================================*/

int Cmiss_value_get_reals(Cmiss_value_id value,int *number_of_reals_address,
	FE_value **reals_address);
/*******************************************************************************
LAST MODIFIED : 16 July 2003

DESCRIPTION :
Gets the <*number_of_reals_address> for the <value>.  This is needed when
calculating derivatives.  If <real_address> is not NULL, then an array is
allocated and the reals put in it.  A zero return code means that <value> is not
represented by reals or the array could not be allocated or the arguments are
invalid.
==============================================================================*/

int Cmiss_value_multiply_and_accumulate(Cmiss_value_id total,
	Cmiss_value_id value_1,Cmiss_value_id value_2);
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
Calculates <total>+<value_1>*<value_2> and puts in <total>.
==============================================================================*/

int Cmiss_value_get_string(Cmiss_value_id value,
	char **result);
/*******************************************************************************
LAST MODIFIED : 25 July 2003

DESCRIPTION :
Creates a string representation of the Cmiss_value useful for output.
If successful <result> contains an ALLOCATED string, it is up to the calling
function to DEALLOCATE the string when it is no longer required.
==============================================================================*/

int Cmiss_value_default_get_string(Cmiss_value_id value,
	char **result);
/*******************************************************************************
LAST MODIFIED : 25 July 2003

DESCRIPTION :
A default implementation of Cmiss_value_get string which calls the
Cmiss_value_get_reals method and concatenates these into a list of numbers.
==============================================================================*/

int Cmiss_value_increment(Cmiss_value_id value,Cmiss_value_id increment);
/*******************************************************************************
LAST MODIFIED : 29 July 2003

DESCRIPTION :
Calculates <value>+<increment> and puts in <value>.
==============================================================================*/

int Cmiss_value_scalar_multiply(Cmiss_value_id value,FE_value scalar);
/*******************************************************************************
LAST MODIFIED : 29 July 2003

DESCRIPTION :
Calculates <scalar>*<value> and puts in <value>.
==============================================================================*/
#endif /* !defined (__CMISS_VALUE_H__) */
