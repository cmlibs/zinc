/*******************************************************************************
FILE : computed_value.h

LAST MODIFIED : 17 February 2003

DESCRIPTION :
A module intended to replace general/value .  Testing and developing in
conjunction with Computed_variables.

???DB.  Should _get_s try and get a representation if not specified type?
???DB.  Merge FE_value, FE_value_vector and FE_value_matrix?
==============================================================================*/
#if !defined (COMPUTED_VALUE_H)
#define COMPUTED_VALUE_H

#include "general/list.h"
#include "general/object.h"
#include "general/value.h"

/*
Global macros
-------------
*/
/*???DB.  Should be in general/debug.h, but while developing have here */
/*???DB.  For optimised ASSERT_IF would be empty.  This means that there should
	be no assignments in the ASSERT_IF expression */
/*???DB.  Could implement __FUNCTION_NAME__ macro using ENTER and LEAVE.  Make
	so that has call tree? */
/*???DB.  Should there be levels of ASSERT_IF */
/*???DB.  Still using if to check ALLOCATE */

#define ASSERT_IF( expression , return_code , error_value ) \
if (!(expression)) \
{ \
	display_message(ERROR_MESSAGE,"file: " __FILE__ ", line: %d.  ASSERT_IF(" \
		#expression ") failed",__LINE__); \
	return_code=error_value; \
} \
else

#define PROTOTYPE_COMPUTED_VALUE_IS_TYPE_FUNCTION( value_type ) \
int Computed_value_is_type_ ## value_type (struct Computed_value *value) \
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
Global functions
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

struct Computed_value *Computed_value_duplicate(struct Computed_value *value);
/*******************************************************************************
LAST MODIFIED : 19 February 2003

DESCRIPTION :
Returns a copy of the <value>.
==============================================================================*/

int Computed_value_same_sub_type(struct Computed_value *value_1,
	struct Computed_value *value_2);
/*******************************************************************************
LAST MODIFIED : 13 February 2003

DESCRIPTION :
Returns nonzero if <value_1> and <value_2> have the same sub-type and zero
otherwise.
==============================================================================*/

char *Computed_value_get_type_id_string(struct Computed_value *value);
/*******************************************************************************
LAST MODIFIED : 12 February 2003

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

PROTOTYPE_COMPUTED_VALUE_IS_TYPE_FUNCTION(FE_value);

int Computed_value_get_type_FE_value(struct Computed_value *value,
	FE_value *fe_value_address);
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
If <value> is of type FE_value, gets its <*fe_value_address>.
==============================================================================*/

int Computed_value_set_type_FE_value_vector(struct Computed_value *value,
	int number_of_fe_values,FE_value *fe_value_vector);
/*******************************************************************************
LAST MODIFIED : 12 February 2003

DESCRIPTION :
Makes <value> of type FE_value_vector and sets its <number_of_fe_values> and
<fe_value_vector>.  After success, the <value> is responsible for DEALLOCATEing
<fe_value_vector>.
==============================================================================*/

PROTOTYPE_COMPUTED_VALUE_IS_TYPE_FUNCTION(FE_value_vector);

int Computed_value_get_type_FE_value_vector(struct Computed_value *value,
	int *number_of_fe_values_address,FE_value **fe_value_vector_address);
/*******************************************************************************
LAST MODIFIED : 12 February 2003

DESCRIPTION :
If <value> is of type FE_value_vector, gets its <*number_of_fe_values_address>
and <*fe_value_vector_address>.

The calling program must not DEALLOCATE the returned <*fe_value_vector_address>.
==============================================================================*/

int Computed_value_set_type_FE_value_matrix(struct Computed_value *value,
	int number_of_rows,int number_of_columns,FE_value *fe_value_matrix);
/*******************************************************************************
LAST MODIFIED : 12 February 2003

DESCRIPTION :
Makes <value> of type FE_value_matrix and sets its <number_of_rows>,
<number_of_columns> and <fe_value_matrix> (column number varying fastest).
After success, the <value> is responsible for DEALLOCATEing <fe_value_matrix>.
==============================================================================*/

PROTOTYPE_COMPUTED_VALUE_IS_TYPE_FUNCTION(FE_value_matrix);

int Computed_value_get_type_FE_value_matrix(struct Computed_value *value,
	int *number_of_rows_address,int *number_of_columns_address,
	FE_value **fe_value_matrix_address);
/*******************************************************************************
LAST MODIFIED : 12 February 2003

DESCRIPTION :
If <value> is of type FE_value_matrix, gets its <*number_of_rows_address>,
<*number_of_columns_address> and <*fe_value_matrix_address>.

The calling program must not DEALLOCATE the returned <*fe_value_matrix_address>.
==============================================================================*/

int Computed_value_set_type_string(struct Computed_value *value,char *string);
/*******************************************************************************
LAST MODIFIED : 22 January 2003

DESCRIPTION :
Makes <value> of type string and sets its <string>.  After success, the <value>
is responsible for DEALLOCATEing <string>.
==============================================================================*/

PROTOTYPE_COMPUTED_VALUE_IS_TYPE_FUNCTION(string);

int Computed_value_get_type_string(struct Computed_value *value,
	char **string_address);
/*******************************************************************************
LAST MODIFIED : 13 February 2003

DESCRIPTION :
If <value> is of type string, gets its <*string_address>.

The calling program must not DEALLOCATE the returned <*string_address>.
==============================================================================*/

int Computed_value_multiply_and_accumulate(struct Computed_value *value_1,
	struct Computed_value *value_2,struct Computed_value *total);
/*******************************************************************************
LAST MODIFIED : 12 February 2003

DESCRIPTION :
Calculates <total>+<value_1>*<value_2> and puts in <total>.
==============================================================================*/
#endif /* !defined (COMPUTED_VALUE_H) */
