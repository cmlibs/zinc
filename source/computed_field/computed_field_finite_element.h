/*******************************************************************************
FILE : computed_field_finite_element.h

LAST MODIFIED : 11 September 2000

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
	struct MANAGER(FE_field) *fe_field_manager, struct FE_time *fe_time);
/*******************************************************************************
LAST MODIFIED : 9 November 2001

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
	struct FE_field *fe_field, struct MANAGER(FE_field) *fe_field_manager);
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_FINITE_ELEMENT, wrapping the given
<fe_field>. Makes the number of components the same as in the <fe_field>.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
Need pointer to fe_field_manager so can call MANAGED_OBJECT_NOT_IN_USE in
Computed_field_finite_element_not_in_use.
==============================================================================*/

int Computed_field_get_type_finite_element(struct Computed_field *field,
	struct FE_field **fe_field);
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
If the field is of type COMPUTED_FIELD_FINITE_ELEMENT, the FE_field being
"wrapped" by it is returned - otherwise an error is reported.
==============================================================================*/

struct LIST(FE_field) *Computed_field_get_defining_FE_field_list(
	struct Computed_field *field,
	struct MANAGER(Computed_field) *computed_field_manager);
/*******************************************************************************
LAST MODIFIED : 11 September 2000

DESCRIPTION :
Returns the list of FE_fields that <field> depends on, by sorting through the
<computed_field_manager>.
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

int Computed_field_has_coordinate_fe_field(struct Computed_field *field,
	void *dummy);
/*******************************************************************************
LAST MODIFIED : 4 December 2001

DESCRIPTION :
Iterator/conditional function returning true if <field> is a wrapper for a
coordinate type fe_field.
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

int Computed_field_is_type_embedded(struct Computed_field *field, void *dummy);
/*******************************************************************************
LAST MODIFIED : 20 July 2000

DESCRIPTION :
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

int Computed_field_is_type_xi_coordinates(struct Computed_field *field,
	void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 15 January 2002

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
