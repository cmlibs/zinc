/*******************************************************************************
FILE : computed_value_private.h

LAST MODIFIED : 27 January 2003

DESCRIPTION :
==============================================================================*/
#if !defined (COMPUTED_VALUE_PRIVATE_H)
#define COMPUTED_VALUE_PRIVATE_H

#include "computed_variable/computed_value.h"

/*
Friend types
------------
*/
struct Computed_value;
/*******************************************************************************
LAST MODIFIED : 22 January 2003

DESCRIPTION :
A value that knows what type it is.

???DB.  An update of Value_type (general/value).  Want to be able to add new
	value types (in the same way that Computed_field types can be added).  Will
	need a package?  Will replace Value_type?
???DB.  At present just extend?
==============================================================================*/

/*
Friend functions
----------------
*/
struct Computed_value *CREATE(Computed_value)(void);
/*******************************************************************************
LAST MODIFIED : 22 January 2003

DESCRIPTION :
Creates an empty value with no type.  Each type of value has its own "set_type"
function.
==============================================================================*/

int DESTROY(Computed_value)(struct Computed_value **value_address);
/*******************************************************************************
LAST MODIFIED : 22 January 2003

DESCRIPTION :
Frees memory/deaccess objects for Computed_value at <*value_address>.
==============================================================================*/

char *Computed_value_get_type(struct Computed_value *value);
/*******************************************************************************
LAST MODIFIED : 22 January 2003

DESCRIPTION :
Returns the string which identifies the type.  The calling function must not
DEALLOCATE the returned string.
==============================================================================*/

int Computed_value_set_type_FE_value(struct Computed_value *value,
	FE_value fe_value);
/*******************************************************************************
LAST MODIFIED : 22 January 2003

DESCRIPTION :
Makes <value> of type FE_value and sets its <fe_value>.
==============================================================================*/

int Computed_value_is_type_FE_value(struct Computed_value *value);
/*******************************************************************************
LAST MODIFIED : 22 January 2003

DESCRIPTION :
Returns a non-zero if <value> is a FE_value and zero otherwise.
==============================================================================*/

int Computed_value_get_type_FE_value(struct Computed_value *value,
	FE_value *fe_value_address);
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
If <value> is of type FE_value, gets its <*fe_value_address>.
==============================================================================*/

int Computed_value_set_type_FE_value_array(struct Computed_value *value,
	int number_of_fe_values,FE_value *fe_value_array);
/*******************************************************************************
LAST MODIFIED : 22 January 2003

DESCRIPTION :
Makes <value> of type FE_value_array and sets its <number_of_fe_values> and
<fe_value_array>.  After success, the <value> is responsible for DEALLOCATEing
<fe_value_array>.
==============================================================================*/

int Computed_value_is_type_FE_value_array(struct Computed_value *value);
/*******************************************************************************
LAST MODIFIED : 22 January 2003

DESCRIPTION :
Returns a non-zero if <value> is a FE_value_array and zero otherwise.
==============================================================================*/

int Computed_value_get_type_FE_value_array(struct Computed_value *value,
	int *number_of_fe_values_address,FE_value **fe_value_array_address);
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
If <value> is of type FE_value_array, gets its <*number_of_fe_values_address>
and <*fe_value_array_address>.

The calling program must not DEALLOCATE the returned <*fe_value_array_address>.
==============================================================================*/

int Computed_value_set_type_string(struct Computed_value *value,
	char *string);
/*******************************************************************************
LAST MODIFIED : 22 January 2003

DESCRIPTION :
Makes <value> of type string and sets its <string>.  After success, the <value>
is responsible for DEALLOCATEing <string>.
==============================================================================*/

int Computed_value_is_type_string(struct Computed_value *value);
/*******************************************************************************
LAST MODIFIED : 22 January 2003

DESCRIPTION :
Returns a non-zero if <value> is a string and zero otherwise.
==============================================================================*/

int Computed_value_get_type_string(struct Computed_value *value,
	char **string_address);
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
If <value> is of type string, gets its <*string_address>.

The calling program must not DEALLOCATE the returned <*string_address>.
==============================================================================*/
#endif /* !defined (COMPUTED_VALUE_PRIVATE_H) */
