/*******************************************************************************
FILE : computed_field_vector_operations.h

LAST MODIFIED : 4 July 2000

DESCRIPTION :
==============================================================================*/
#if !defined (COMPUTED_FIELD_VECTOR_OPERATIONS_H)
#define COMPUTED_FIELD_VECTOR_OPERATIONS_H

int Computed_field_register_types_vector_operations(
	struct Computed_field_package *computed_field_package);
/*******************************************************************************
LAST MODIFIED : 6 July 2000

DESCRIPTION :
==============================================================================*/

int Computed_field_set_type_dot_product(struct Computed_field *field,
	struct Computed_field *source_field_one,
	struct Computed_field *source_field_two);
/*******************************************************************************
LAST MODIFIED : 13 July 2000

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_DOT_PRODUCT with the supplied
fields, <source_field_one> and <source_field_two>.  Sets the number of 
components to one.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
#endif /* !defined (COMPUTED_FIELD_VECTOR_OPERATIONS_H) */
