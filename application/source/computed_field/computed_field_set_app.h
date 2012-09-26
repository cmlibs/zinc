
#include "command/parser.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


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

int Option_table_add_Computed_field_conditional_entry(
	struct Option_table *option_table, const char *token,
	struct Computed_field **field_address, 
	struct Set_Computed_field_conditional_data *set_field_data);
/*******************************************************************************
LAST MODIFIED : 4 December 2003

DESCRIPTION :
Adds the given <token> to the <option_table>.
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

#ifdef __cplusplus
}
#endif /* __cplusplus */
