/*******************************************************************************
FILE : computed_field_finite_element.h

LAST MODIFIED : 17 July 2000

DESCRIPTION :
Implements computed fields which interface to finite element fields.
==============================================================================*/
#if !defined (COMPUTED_FIELD_FINITE_ELEMENT_H)
#define COMPUTED_FIELD_FINITE_ELEMENT_H

struct Computed_field_finite_element_package;
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
Private package
==============================================================================*/

struct Computed_field_finite_element_package *
Computed_field_register_types_finite_element(
	struct Computed_field_package *computed_field_package,
	struct MANAGER(FE_field) *fe_field_manager);
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
This function registers the finite_element related types of Computed_fields and
also registers with the <fe_field_manager> so that any fe_fields are
automatically wrapped in corresponding computed_fields.
==============================================================================*/

int Computed_field_deregister_types_finite_element(
	struct Computed_field_finite_element_package
	*computed_field_finite_element_package);
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
==============================================================================*/

int Computed_field_is_type_finite_element(struct Computed_field *field);
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
==============================================================================*/

int Computed_field_set_type_finite_element(struct Computed_field *field,
  struct FE_field *fe_field);
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_FINITE_ELEMENT, wrapping the given
<fe_field>. Makes the number of components the same as in the <fe_field>.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/

int Computed_field_get_type_finite_element(struct Computed_field *field,
	struct FE_field **fe_field);
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
If the field is of type COMPUTED_FIELD_FINITE_ELEMENT, the FE_field being
"wrapped" by it is returned - otherwise an error is reported.
==============================================================================*/

int Computed_field_is_type_default_coordinate(struct Computed_field *field);
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
==============================================================================*/

int Computed_field_is_read_only_with_fe_field(
	struct Computed_field *field,void *fe_field_void);
/*******************************************************************************
LAST MODIFIED : 3 February 1999

DESCRIPTION :
Iterator/conditional function returning true if <field> is read only and a
wrapper for <fe_field>.
==============================================================================*/

int Computed_field_is_scalar_integer(struct Computed_field *field,
	void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 20 June 2000

DESCRIPTION :
Returns true if <field> is a 1 integer component FINITE_ELEMENT wrapper.
==============================================================================*/

int Computed_field_is_scalar_integer_grid_in_element(
	struct Computed_field *field,void *element_void);
/*******************************************************************************
LAST MODIFIED : 26 May 2000

DESCRIPTION :
Returns true if <field> is a 1 integer component FINITE_ELEMENT wrapper which
is defined in <element> AND is grid-based.
Used for choosing field suitable for identifying grid points.
==============================================================================*/

int remove_computed_field_from_manager_given_FE_field(
	struct MANAGER(Computed_field) *computed_field_manager,struct FE_field *field);
/*******************************************************************************
LAST MODIFIED : August 27 1999

DESCRIPTION :
Frees the computed fields from the computed field manager, given the FE_field
==============================================================================*/

int destroy_computed_field_given_fe_field(
	struct MANAGER(Computed_field) *computed_field_manager,
	struct MANAGER(FE_field) *fe_field_manager,
	struct FE_field *fe_field);
/*******************************************************************************
LAST MODIFIED : 17 May 2000

DESCRIPTION :
Given <fe_field>, destroys the associated computed field, and fe_field
==============================================================================*/
#endif /* !defined (COMPUTED_FIELD_FINITE_ELEMENT_H) */
