/*******************************************************************************
FILE : computed_field_finite_element.h

LAST MODIFIED : 25 July 2000

DESCRIPTION :
Implements computed fields which interface to finite element fields.
==============================================================================*/
#if !defined (COMPUTED_FIELD_FINITE_ELEMENT_H)
#define COMPUTED_FIELD_FINITE_ELEMENT_H

/*
Global types
------------
*/
struct Computed_field_finite_element_package;
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
Private package
==============================================================================*/

/*
Global functions
----------------
*/
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

int Computed_field_get_type_default_coordinate(struct Computed_field *field,
	struct MANAGER(Computed_field) **computed_field_manager);
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
If the field is of type COMPUTED_FIELD_DEFAULT_COORDINATE, the 
<source_field> and <xi_index> used by it are returned.
==============================================================================*/

int Computed_field_set_type_default_coordinate(struct Computed_field *field,
	struct MANAGER(Computed_field) *computed_field_manager);
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_DEFAULT_COORDINATE, which returns the
values/derivatives of the first [coordinate] field defined for the element/node
in rectangular cartesian coordinates. This type is intended to replace the
NULL coordinate_field option in the calculate_FE_element_field_values function.
When a field of this type is calculated at and element/node, the evaluate
function finds the first FE_field (coordinate type) defined over it, then gets
its Computed_field wrapper from the manager and proceeds from there.
Consequences of this behaviour are:
- the field allocates its source_fields to point to the computed_field for the
actual coordinate field in the evaluate phase.
- when the source field changes the current one's cache is cleared and it is
deaccessed.
- when the cache is cleared, so is any reference to the source_field.
- always performs the conversion to RC since cannot predict the coordinate
system used by the eventual source_field. Coordinate_system of this type of
field need not be RC, although it usually will be.
Sets number of components to 3.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/

int Computed_field_is_type_cmiss_number(struct Computed_field *field);
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
==============================================================================*/

int Computed_field_set_type_cmiss_number(struct Computed_field *field);
/*******************************************************************************
LAST MODIFIED : 20 July 2000

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_CMISS_NUMBER with the supplied
fields, <source_field_one> and <source_field_two>.  Sets the number of 
components equal to the source_fields.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
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

int Computed_field_depends_on_embedded_field(struct Computed_field *field);
/*******************************************************************************
LAST MODIFIED : 28 April 1999

DESCRIPTION :
Returns true if the field is of an embedded type or depends on any computed
fields which are or an embedded type.
==============================================================================*/

int remove_computed_field_from_manager_given_FE_field(
	struct MANAGER(Computed_field) *computed_field_manager,struct FE_field *field);
/*******************************************************************************
LAST MODIFIED : August 27 1999

DESCRIPTION :
Frees the computed fields from the computed field manager, given the FE_field
==============================================================================*/

int Computed_field_is_type_node_array_value_at_time(struct Computed_field *field);
/*******************************************************************************
LAST MODIFIED : 20 July 2000

DESCRIPTION :
==============================================================================*/

int Computed_field_set_type_node_array_value_at_time(struct Computed_field *field,
	struct FE_field *fe_field,enum FE_nodal_value_type nodal_value_type,
	int version_number,struct Computed_field *time_field);
/*******************************************************************************
LAST MODIFIED : 20 July 2000

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_NODE_ARRAY_VALUE_AT_TIME, returning the values for the
given <nodal_value_type> and <version_number> of <fe_field> at a node.
Makes the number of components the same as in the <fe_field>.
Field automatically takes the coordinate system of the source fe_field. See note
at start of this file about changing use of coordinate systems.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
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

int Computed_field_set_type_node_array_value_at_time(
	struct Computed_field *field,struct FE_field *fe_field,
	enum FE_nodal_value_type nodal_value_type,int version_number,
	struct Computed_field *time_field);
/*******************************************************************************
LAST MODIFIED : 20 July 2000

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_NODE_ARRAY_VALUE_AT_TIME, returning the
values for the given <nodal_value_type> and <version_number> of <fe_field> at a
node.  Makes the number of components the same as in the <fe_field>.  Field
automatically takes the coordinate system of the source fe_field. See note at
start of this file about changing use of coordinate systems.  If function fails,
field is guaranteed to be unchanged from its original state, although its cache
may be lost.
==============================================================================*/

int Computed_field_is_type_xi_coordinates(struct Computed_field *field);
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
==============================================================================*/

int Computed_field_set_type_xi_coordinates(struct Computed_field *field);
/*******************************************************************************
LAST MODIFIED : 20 July 2000

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_XI_COORDINATES with the supplied
fields, <source_field_one> and <source_field_two>.  Sets the number of 
components equal to the source_fields.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/

int Computed_field_is_type_node_value(struct Computed_field *field);
/*******************************************************************************
LAST MODIFIED : 19 July 2000

DESCRIPTION :
==============================================================================*/
#endif /* !defined (COMPUTED_FIELD_FINITE_ELEMENT_H) */
