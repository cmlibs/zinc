/*******************************************************************************
FILE : computed_field_matrix_operations.h

LAST MODIFIED : 27 September 2000

DESCRIPTION :
==============================================================================*/
#if !defined (COMPUTED_FIELD_MATRIX_OPERATIONS_H)
#define COMPUTED_FIELD_MATRIX_OPERATIONS_H

int Computed_field_set_type_projection(struct Computed_field *field,
	struct Computed_field *source_field, int number_of_components, 
	double *projection_matrix);
/*******************************************************************************
LAST MODIFIED : 27 September 2000

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_PROJECTION, returning the <source_field>
with each component multiplied by the perspective <projection_matrix>.
The <projection_matrix> array must be of size
(source_field->number_of_components + 1) * (field->number_of_components + 1).
The source vector is appended with a 1 to make
source_field->number_of_components + 1 components. The extra calculated value
is a perspective value which divides through each of the other components.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/

int Computed_field_register_types_matrix_operations(
	struct Computed_field_package *computed_field_package);
/*******************************************************************************
LAST MODIFIED : 26 September 2000

DESCRIPTION :
==============================================================================*/
#endif /* !defined (COMPUTED_FIELD_MATRIX_OPERATIONS_H) */
