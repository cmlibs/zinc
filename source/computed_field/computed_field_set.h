/*******************************************************************************
FILE : computed_field_set.h

LAST MODIFIED : 17 December 2001

DESCRIPTION :
Functions for selecting computed fields from the commands line.
==============================================================================*/
#if !defined (COMPUTED_FIELD_SET_H)
#define COMPUTED_FIELD_SET_H

#include "command/parser.h"
#include "computed_field/computed_field.h"

/*
Global types
------------
*/

struct Set_Computed_field_conditional_data
/*******************************************************************************
LAST MODIFIED : 3 February 1999

DESCRIPTION :
User data structure passed to set_Computed_field_conditional, containing the
computed_field_package and the optional conditional_function (and
conditional_function_user_data) for selecting a field out of a subset of the
fields in the manager.
==============================================================================*/
{
	MANAGER_CONDITIONAL_FUNCTION(Computed_field) *conditional_function;
	void *conditional_function_user_data;
	struct MANAGER(Computed_field) *computed_field_manager;
}; /* struct Set_Computed_field_conditional_data */

struct Set_Computed_field_array_data
/*******************************************************************************
LAST MODIFIED : 9 March 1999

DESCRIPTION :
User data structure passed to set_Computed_field_array, containing the number of
fields in the array, and a pointer to a Set_Computed_field_conditional_data
structure for passing to set_Computed_field_conditional - this is done for each
element in the array.
==============================================================================*/
{
	int number_of_fields;
	struct Set_Computed_field_conditional_data *conditional_data;
}; /* struct Set_Computed_field_array_data */

/*
Global functions
----------------
*/

int set_Computed_field_conditional(struct Parse_state *state,
	void *field_address_void, void *set_field_data_void);
/*******************************************************************************
LAST MODIFIED : 17 December 2001

DESCRIPTION :
Modifier function to set the field from a command. <set_field_data_void> should
point to a struct Set_Computed_field_conditional_data containing the
computed_field_manager and an optional conditional function for narrowing the
range of fields available for selection. If the conditional_function is NULL,
no criteria are placed on the chosen field.
Allows the construction field.component name to automatically make a component
wrapper for field and add it to the manager.
==============================================================================*/

int set_Computed_field_array(struct Parse_state *state,
	void *field_array_void, void *set_field_array_data_void);
/*******************************************************************************
LAST MODIFIED : 17 December 2001

DESCRIPTION :
Modifier function to set an array of field from a command.
<set_field_array_data_void> should point to a struct
Set_Computed_field_array_conditional_data containing the number_of_fields in the
array, the computed_field_package and an optional conditional function for
narrowing the range of fields available for selection.
Works by repeatedly calling set_Computed_field_conditional.
???RC Make this globally available for calling any modifier function?
==============================================================================*/

#endif /* !defined (COMPUTED_FIELD_SET_H) */
