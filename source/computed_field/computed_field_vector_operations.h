/*******************************************************************************
FILE : computed_field_vector_operations.h

LAST MODIFIED : 4 July 2000

DESCRIPTION :
==============================================================================*/
#if !defined (COMPUTED_FIELD_VECTOR_OPERATIONS_H)
#define COMPUTED_FIELD_VECTOR_OPERATIONS_H

char *Computed_field_sample_texture_type_string(void);
/*******************************************************************************
LAST MODIFIED : 6 July 2000

DESCRIPTION :
Return the static type string which identifies this type.
==============================================================================*/

int Computed_field_register_types_vector_operations(
	struct Computed_field_package *computed_field_package);
/*******************************************************************************
LAST MODIFIED : 6 July 2000

DESCRIPTION :
==============================================================================*/

#endif /* !defined (COMPUTED_FIELD_VECTOR_OPERATIONS_H) */
