/*******************************************************************************
FILE : computed_field_composite.h

LAST MODIFIED : 15 January 2002

DESCRIPTION :
Implements a "composite" computed_field which converts fields, field components
and real values in any order into a single vector field.
==============================================================================*/
#if !defined (COMPUTED_FIELD_COMPOSITE_H)
#define COMPUTED_FIELD_COMPOSITE_H

int Computed_field_set_type_constant(struct Computed_field *field,
	int number_of_values, FE_value *values);
/*******************************************************************************
LAST MODIFIED : 14 December 2001

DESCRIPTION :
Changes <field> into type composite with <number_of_values> values listed in
the <values> array.
Since a constant field performs a subset of the capabilities of the composite
field type but the latter is somewhat complicated to set up, this is a
convenience function for building a composite field which has <number_of_values>
<values>. This function handles sorting so that no values are repeated.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/

int Computed_field_is_constant(struct Computed_field *field);
/*******************************************************************************
LAST MODIFIED : 14 December 2001

DESCRIPTION :
Returns true if the field is of type composite but with no source_fields, only
source_values.
==============================================================================*/

int Computed_field_is_constant_scalar(struct Computed_field *field,
	FE_value scalar);
/*******************************************************************************
LAST MODIFIED : 15 January 2002

DESCRIPTION :
Returns true if the field is of type composite but with no source_fields, only
a single source value, equal to <scalar>.
==============================================================================*/

struct Computed_field *Computed_field_manager_get_component_wrapper(
	struct MANAGER(Computed_field) *computed_field_manager,
	struct Computed_field *field, int component_number);
/*******************************************************************************
LAST MODIFIED : 14 December 2001

DESCRIPTION :
Returns a composite field that computes <component_number> of <field>. First
tries to find one in the manager that does this, otherwise makes one of name
'field.component_name', adds it to the manager and returns it.
==============================================================================*/

int Computed_field_register_types_composite(
	struct Computed_field_package *computed_field_package);
/*******************************************************************************
LAST MODIFIED : 25 October 2000

DESCRIPTION :
==============================================================================*/

int Computed_field_set_type_composite(struct Computed_field *field,
	int number_of_components,
	int number_of_source_fields,struct Computed_field **source_fields,
	int number_of_source_values,FE_value *source_values,
	int *source_field_numbers,int *source_value_numbers);
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
Converts <field> into a composite field which returns a combination of field
components and constants in a given order.
The <number_of_source_fields> <source_fields> may be zero if the field is
purely constant, but fields may not be repeated.
The <number_of_source_values> <source_values> may similarly be omitted.
The <source_field_numbers> and <source_value_numbers> must each contain
<number_of_components> integers. For each component the <source_field_number>
says which <source_field> to use, and the <source_value_number> specifies the
component number. If the <source_field_number> is -1 for any component, the
<source_value_number> is the offset into the <source_values>.

Note: rigorous checking is performed to ensure that no values are invalid and
that the <source_fields> are presented in the order they are first used in the
<source_field_numbers>, and also that <source_values> are used in the order
they are given and that none are used more than once.

Some of these restrictions are enforced to ensure type-specific contents can be
compared easily -- otherwise there would be more than one way to describe the
same source data.

If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/

#endif /* !defined (COMPUTED_FIELD_COMPOSITE_H) */
