/*******************************************************************************
FILE : computed_field_component_operations.h

LAST MODIFIED : 13 July 2000

DESCRIPTION :
Implements a number of basic component wise operations on computed fields.
==============================================================================*/
#if !defined (COMPUTED_FIELD_COMPONENT_OPERATIONS_H)
#define COMPUTED_FIELD_COMPONENT_OPERATIONS_H

char *Computed_field_multiply_components_type_string(void);
/*******************************************************************************
LAST MODIFIED : 13 July 2000

DESCRIPTION :
Return the static type string which identifies this type.
==============================================================================*/

char *Computed_field_divide_components_type_string(void);
/*******************************************************************************
LAST MODIFIED : 13 July 2000

DESCRIPTION :
Return the static type string which identifies this type.
==============================================================================*/

int Computed_field_register_types_component_operations(
	struct Computed_field_package *computed_field_package);
/*******************************************************************************
LAST MODIFIED : 13 July 2000

DESCRIPTION :
==============================================================================*/

#endif /* !defined (COMPUTED_FIELD_COMPONENT_OPERATIONS_H) */
