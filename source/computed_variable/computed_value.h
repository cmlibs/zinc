/*******************************************************************************
FILE : computed_value.h

LAST MODIFIED : 9 April 2003

DESCRIPTION :
A module intended to replace general/value .  Testing and developing in
conjunction with Cmiss_variables.

???DB.  Should _get_s try and get a representation if not specified type?
???DB.  Merge FE_value, FE_value_vector and FE_value_matrix?
==============================================================================*/
#if !defined (__CMISS_VALUE_H__)
#define __CMISS_VALUE_H__

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
typedef struct Cmiss_value *Cmiss_value_id;
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

int Cmiss_value_multiply_and_accumulate(Cmiss_value_id total,
	Cmiss_value_id value_1,Cmiss_value_id value_2);
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
Calculates <total>+<value_1>*<value_2> and puts in <total>.
==============================================================================*/

int Cmiss_value_FE_value_set_type(Cmiss_value_id value,
	FE_value fe_value);
/*******************************************************************************
LAST MODIFIED : 22 January 2003

DESCRIPTION :
Makes <value> of type FE_value and sets its <fe_value>.
==============================================================================*/

PROTOTYPE_CMISS_VALUE_IS_TYPE_FUNCTION(FE_value);

int Cmiss_value_FE_value_get_type(Cmiss_value_id value,
	FE_value *fe_value_address);
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
If <value> is of type FE_value, gets its <*fe_value_address>.
==============================================================================*/

int Cmiss_value_FE_value_vector_set_type(Cmiss_value_id value,
	int number_of_fe_values,FE_value *fe_value_vector);
/*******************************************************************************
LAST MODIFIED : 12 February 2003

DESCRIPTION :
Makes <value> of type FE_value_vector and sets its <number_of_fe_values> and
<fe_value_vector>.  After success, the <value> is responsible for DEALLOCATEing
<fe_value_vector>.
==============================================================================*/

PROTOTYPE_CMISS_VALUE_IS_TYPE_FUNCTION(FE_value_vector);

int Cmiss_value_FE_value_vector_get_type(Cmiss_value_id value,
	int *number_of_fe_values_address,FE_value **fe_value_vector_address);
/*******************************************************************************
LAST MODIFIED : 12 February 2003

DESCRIPTION :
If <value> is of type FE_value_vector, gets its <*number_of_fe_values_address>
and <*fe_value_vector_address>.

The calling program must not DEALLOCATE the returned <*fe_value_vector_address>.
==============================================================================*/

int Cmiss_value_FE_value_matrix_set_type(Cmiss_value_id value,
	int number_of_rows,int number_of_columns,FE_value *fe_value_matrix);
/*******************************************************************************
LAST MODIFIED : 12 February 2003

DESCRIPTION :
Makes <value> of type FE_value_matrix and sets its <number_of_rows>,
<number_of_columns> and <fe_value_matrix> (column number varying fastest).
After success, the <value> is responsible for DEALLOCATEing <fe_value_matrix>.
==============================================================================*/

PROTOTYPE_CMISS_VALUE_IS_TYPE_FUNCTION(FE_value_matrix);

int Cmiss_value_FE_value_matrix_get_type(Cmiss_value_id value,
	int *number_of_rows_address,int *number_of_columns_address,
	FE_value **fe_value_matrix_address);
/*******************************************************************************
LAST MODIFIED : 12 February 2003

DESCRIPTION :
If <value> is of type FE_value_matrix, gets its <*number_of_rows_address>,
<*number_of_columns_address> and <*fe_value_matrix_address>.

The calling program must not DEALLOCATE the returned <*fe_value_matrix_address>.
==============================================================================*/

int Cmiss_value_string_set_type(Cmiss_value_id value,char *string);
/*******************************************************************************
LAST MODIFIED : 22 January 2003

DESCRIPTION :
Makes <value> of type string and sets its <string>.  After success, the <value>
is responsible for DEALLOCATEing <string>.
==============================================================================*/

PROTOTYPE_CMISS_VALUE_IS_TYPE_FUNCTION(string);

int Cmiss_value_string_get_type(Cmiss_value_id value,char **string_address);
/*******************************************************************************
LAST MODIFIED : 13 February 2003

DESCRIPTION :
If <value> is of type string, gets its <*string_address>.

The calling program must not DEALLOCATE the returned <*string_address>.
==============================================================================*/
#endif /* !defined (__CMISS_VALUE_H__) */
