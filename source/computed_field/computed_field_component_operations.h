/*******************************************************************************
FILE : computed_field_component_operations.h

LAST MODIFIED : 13 July 2000

DESCRIPTION :
Implements a number of basic component wise operations on computed fields.
==============================================================================*/
#if !defined (COMPUTED_FIELD_COMPONENT_OPERATIONS_H)
#define COMPUTED_FIELD_COMPONENT_OPERATIONS_H

int Computed_field_register_types_component_operations(
	struct Computed_field_package *computed_field_package);
/*******************************************************************************
LAST MODIFIED : 13 July 2000

DESCRIPTION :
==============================================================================*/

int Computed_field_set_type_add(struct Computed_field *field,
	struct Computed_field *source_field_one, FE_value scale_factor1,
	struct Computed_field *source_field_two, FE_value scale_factor2);
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_ADD with the supplied
fields, <source_field_one> and <source_field_two>.  Sets the number of 
components equal to the source_fields.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
#endif /* !defined (COMPUTED_FIELD_COMPONENT_OPERATIONS_H) */
