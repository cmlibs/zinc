
#if !defined (FINITE_ELEMENT_APP_H_)
#define FINITE_ELEMENT_APP_H_

#include "command/parser.h"

int set_FE_field(struct Parse_state *state,void *field_address_void,
	void *fe_field_list_void);
/*******************************************************************************
LAST MODIFIED : 3 December 2002

DESCRIPTION :
Modifier function to set the field from the command line.
==============================================================================*/

int set_FE_field_conditional(struct Parse_state *state,
	void *field_address_void,void *set_field_data_void);
/*******************************************************************************
LAST MODIFIED : 3 December 2002

DESCRIPTION :
Modifier function to set the field from a command. <set_field_data_void> should
point to a struct Set_FE_field_conditional_data containing the
fe_field_list and an optional conditional function for narrowing the
range of fields available for selection. If the conditional_function is NULL,
this function works just like set_FE_field.
==============================================================================*/

int set_FE_fields(struct Parse_state *state,
	void *field_order_info_address_void, void *fe_field_list_void);
/*******************************************************************************
LAST MODIFIED : 3 December 2002

DESCRIPTION :
Modifier function to set an ordered list of fields, each separated by white
space until an unrecognised field name is encountered. Two special tokens are
understood in place of any fields: 'all' and 'none'.
For the case of 'all', a NULL FE_field_order_info structure is returned.
For the case of 'none', an empty FE_field_order_info structure is returned.
It is up to the calling function to destroy any FE_field_order_info structure
returned by this function, however, any such structure passed to this function
may be destroyed here - ie. in the 'all' case.
==============================================================================*/



int set_FE_field_component(struct Parse_state *state,void *component_void,
	void *fe_field_list_void);
/*******************************************************************************
LAST MODIFIED : 3 December 2002

DESCRIPTION :
Used in command parsing to translate a field component name into an field
component.
???DB.  Should it be here ?
???RC.  Does not ACCESS the field (unlike set_FE_field, above).
==============================================================================*/
#endif

