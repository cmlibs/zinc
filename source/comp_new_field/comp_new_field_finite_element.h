/*******************************************************************************
FILE : comp_new_field_finite_element.h

LAST MODIFIED : 19 January 2003

DESCRIPTION :
Implements computed fields which interface to finite element fields:
- cmiss_number.  Returns the cmiss number of a node or an element
- embedded.  Used for node fields that give element/xi - data at material points
???DB.  Why is Comp_new_field_set_type_embedded static?
???DB.  Extend to element/xi
- finite_element.  A wrapper for a FE_field
- node_value
- xi_coordinate.  Returns the xi coordinates as a vector of FE_values -
	basically the identity
???DB.  Currently only implemented for element/xi ie. doesn't return the
	xi_coordinates of the node - depends on element

NOTES :
1 To be able to use this with the current version have swapped
	- computed_field -> comp_new_field
	- Computed_field -> Comp_new_field
	- COMPUTED_FIELD -> COMP_NEW_FIELD
	When finished can change back

???DB.  Is a comp_new_field_finite_element_utilities module needed?
???DB.  Consistency for is_type functions.  Currently some are iterator/
	conditional and some are not
==============================================================================*/
#if !defined (COMP_NEW_FIELD_FINITE_ELEMENT_H)
#define COMP_NEW_FIELD_FINITE_ELEMENT_H

/*
Global types
------------
*/
struct Mesh_location;
/*******************************************************************************
LAST MODIFIED : 10 January 2003

DESCRIPTION :
Specifies a location in a mesh eg node or element/xi.

???DB.  Should be in finite_element (mesh)?
==============================================================================*/

struct Comp_new_field_finite_element_package;
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
Private package
==============================================================================*/

/*
Global functions
----------------
*/
int Comp_new_field_variable_set_element_value(
	struct Comp_new_field_variable *variable,
	struct FE_field *fe_field,int component_number,struct FE_element *element,
	int *grid_point,int version,FE_value *values);
/*******************************************************************************
LAST MODIFIED : 16 January 2003

DESCRIPTION :
Converts the <variable> into a element_value Comp_new_field_variable for the
specified <fe_field> (all finite element fields if NULL), <component_number>
(all components if -1), <element> (all elements if NULL), <grid_point> (all grid
points if NULL), <version> (all versions if 0).  If <values> is not NULL then
they are copied into <variable>.
==============================================================================*/

int Comp_new_field_variable_set_mesh_location(
	struct Comp_new_field_variable *variable,struct Mesh_location *mesh_location);
/*******************************************************************************
LAST MODIFIED : 16 January 2003

DESCRIPTION :
Converts the <variable> into a mesh_location Comp_new_field_variable.  If
<mesh_location> is not NULL then it is copied into <variable>.
==============================================================================*/

int Comp_new_field_variable_set_nodal_value(
	struct Comp_new_field_variable *variable,struct FE_field *fe_field,
	int component_number,struct FE_element *node,
	enum FE_nodal_value_type value_type,int version,FE_value *values);
/*******************************************************************************
LAST MODIFIED : 16 January 2003

DESCRIPTION :
Converts the <variable> into a nodal_value Comp_new_field_variable for the
specified <fe_field> (all finite element fields if NULL), <component_number>
(all components if -1), <node> (all nodes if NULL), <value_type> (all types if
FE_NODAL_UNKNOWN), <version> (all versions if 0).  If <values> is not NULL then
they are copied into <variable>.
==============================================================================*/

int Comp_new_field_variable_set_scale_factor(
	struct Comp_new_field_variable *variable,
	struct FE_field *fe_field,int component_number,struct FE_element *element,
	int local_node_number,enum FE_nodal_value_type value_type,int version,
	FE_value *values);
/*******************************************************************************
LAST MODIFIED : 16 January 2003

DESCRIPTION :
Converts the <variable> into a scale_factor Comp_new_field_variable for the
specified <fe_field> (all finite element fields if NULL), <component_number>
(all components if -1), <element> (all elements if NULL), <local_node_number>
(all local nodes if -1), <value_type> (all types if FE_NODAL_UNKNOWN), <version>
(all versions if 0).  If <values> is not NULL then they are copied into
<variable>.
==============================================================================*/

int Comp_new_field_variable_set_FE_time(
	struct Comp_new_field_variable *variable,FE_value *time);
/*******************************************************************************
LAST MODIFIED : 16 January 2003

DESCRIPTION :
Converts the <variable> into a FE_time Comp_new_field_variable.  If <time> is
not NULL then it is copied into <variable>.
==============================================================================*/

int Comp_new_field_variable_get_mesh_location(
	struct Comp_new_field_variable *result,struct FE_node **node_address,
	struct FE_element **element_address,FE_value **xi_address);
/*******************************************************************************
LAST MODIFIED : 12 January 2003

DESCRIPTION :
If <result> is representable as a Mesh_location, then returns nonzero and
- if it is a node and <node_address> is not NULL, <*node_address> is set to the
	node
- if it is element_xi and <element_address> is not NULL and <xi_address> is not
	NULL then <*element_address> is set to the element and <*xi_address> is set to
	the xi coordinates.

The calling routine is not responsible for DEALLOCATEing <*xi_address> or
DEACCESSing <*node_address> or <*element_address>.
==============================================================================*/

int Comp_new_field_is_type_finite_element(struct Comp_new_field *field);
/*******************************************************************************
LAST MODIFIED : 12 January 2003

DESCRIPTION :
Returns true if <field> is a finite_element Comp_new_field.
==============================================================================*/

int Comp_new_field_set_type_finite_element(struct Comp_new_field *field,
	struct FE_field *fe_field,struct MANAGER(FE_field) *fe_field_manager);
/*******************************************************************************
LAST MODIFIED : 12 January 2003

DESCRIPTION :
Converts <field> into a finite_element Comp_new_field that wraps the given
<fe_field>.  Makes the number of components the same as for the <fe_field>.

If function fails, <field> is guaranteed to be unchanged from its original
state, although its cache may be lost.

Need pointer to <fe_field_manager> so can call MANAGED_OBJECT_NOT_IN_USE in
Comp_new_field_finite_element_not_in_use.
==============================================================================*/

int Comp_new_field_get_type_finite_element(struct Comp_new_field *field,
	struct FE_field **fe_field);
/*******************************************************************************
LAST MODIFIED : 12 January 2003

DESCRIPTION :
If <field> is a finite_element Comp_new_field, then the <fe_field> it wraps is
set and true is returned.
==============================================================================*/

int Comp_new_field_is_read_only_with_fe_field(
	struct Comp_new_field *field,void *fe_field_void);
/*******************************************************************************
LAST MODIFIED : 12 January 2003

DESCRIPTION :
Iterator/conditional function returning true if <field> is read only and a
wrapper for the finite element field <(struct FE_field *)fe_field_void>.

???DB.  Move to comp_new_field_finite_element_utilities?
==============================================================================*/

int Comp_new_field_has_coordinate_fe_field(struct Comp_new_field *field,
	void *dummy);
/*******************************************************************************
LAST MODIFIED : 12 January 2003

DESCRIPTION :
Iterator/conditional function returning true if <field> is a finite_element
Comp_new_field and the FE_field it wraps is of coordinate type.

???DB.  Move to comp_new_field_finite_element_utilities?
==============================================================================*/

int Comp_new_field_is_finite_element_and_scalar_integer(
	struct Comp_new_field *field,void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 12 January 2003

DESCRIPTION :
Iterator/conditional function returning true if <field> is a finite_element
Comp_new_field that returns a single integer.

???DB.  Move to comp_new_field_finite_element_utilities?
==============================================================================*/

int Comp_new_field_is_grid_based_finite_element_and_scalar_integer(
	struct Comp_new_field *field,void *element_void);
/*******************************************************************************
LAST MODIFIED : 12 January 2003

DESCRIPTION :
Iterator/conditional function returning true if <field> is a finite_element
Comp_new_field that returns a single integer and for which the wrapped FE_field
is grid-based.

Used for choosing field suitable for identifying grid points.

???DB.  Move to comp_new_field_finite_element_utilities?
==============================================================================*/

int remove_comp_new_field_from_manager_given_FE_field(
	struct MANAGER(Comp_new_field) *comp_new_field_manager,
	struct FE_field *fe_field);
/*******************************************************************************
LAST MODIFIED : 12 January 2003

DESCRIPTION :
Removes the finite_element Comp_new_field that wraps <fe_field> from the
<comp_new_field_manager> - this will mean that the wrapping Comp_new_field is
destroyed.
==============================================================================*/

int destroy_comp_new_field_given_fe_field(
	struct MANAGER(Comp_new_field) *comp_new_field_manager,
	struct MANAGER(FE_field) *fe_field_manager,
	struct FE_field *fe_field);
/*******************************************************************************
LAST MODIFIED : 12 January 2003

DESCRIPTION :
Removes the finite_element Comp_new_field that wraps <fe_field> from the
<comp_new_field_manager> - this will mean that the wrapping Comp_new_field is
destroyed.  Also removes the <fe_field> from the <fe_field_manager> - this will
mean that <fe_field> is destroyed.
==============================================================================*/

int Comp_new_field_is_type_cmiss_number(struct Comp_new_field *field);
/*******************************************************************************
LAST MODIFIED : 12 January 2003

DESCRIPTION :
Returns true if <field> is a cmiss_number Comp_new_field.
==============================================================================*/

int Comp_new_field_set_type_cmiss_number(struct Comp_new_field *field);
/*******************************************************************************
LAST MODIFIED : 12 January 2003

DESCRIPTION :
Converts <field> into a cmiss_number Comp_new_field - returns the cmiss number
of a node or an element.  Sets the number of components to 1.

If function fails, <field> is guaranteed to be unchanged from its original
state, although its cache may be lost.
==============================================================================*/

int Comp_new_field_is_type_embedded(struct Comp_new_field *field,void *dummy);
/*******************************************************************************
LAST MODIFIED : 12 January 2003

DESCRIPTION :
Iterator/conditional function returning true if <field> is an embedded
Comp_new_field.
==============================================================================*/

int Comp_new_field_depends_on_embedded_field(struct Comp_new_field *field);
/*******************************************************************************
LAST MODIFIED : 12 January 2003

DESCRIPTION :
Returns true if the field is of embedded type or depends on any computed fields
which are of embedded type.
==============================================================================*/

int Comp_new_field_is_type_xi_coordinates(struct Comp_new_field *field,
	void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 12 January 2003

DESCRIPTION :
Iterator/conditional function returning true if <field> is an xi_coordinates
Comp_new_field.
==============================================================================*/

int Comp_new_field_set_type_xi_coordinates(struct Comp_new_field *field);
/*******************************************************************************
LAST MODIFIED : 12 January 2003

DESCRIPTION :
Converts <field> into a xi_coordinates Comp_new_field.  Sets the number of
components to 3 - if the element has a dimension < 3 than the later xis are set
to zero.

If function fails, <field> is guaranteed to be unchanged from its original
state, although its cache may be lost.
==============================================================================*/

int Comp_new_field_is_type_node_value(struct Comp_new_field *field);
/*******************************************************************************
LAST MODIFIED : 12 January 2003

DESCRIPTION :
Returns true if <field> is a xi_coordinates Comp_new_field.
==============================================================================*/

int Comp_new_field_set_type_node_value(struct Comp_new_field *field,
	struct FE_field *fe_field,enum FE_nodal_value_type nodal_value_type,
	int version_number);
/*******************************************************************************
LAST MODIFIED : 12 January 2003

DESCRIPTION :
Converts <field> into a node_value Comp_new_field - returns the values for
given <nodal_value_type> and <version_number> of <fe_field> at a node.  Sets the
number of components and coordinate system to be the same as for <fe_field>.

If function fails, <field> is guaranteed to be unchanged from its original
state, although its cache may be lost.

???DB.  Will be replaced by the Comp_new_field_variable Comp_new_fields?
==============================================================================*/

struct LIST(FE_field) *Comp_new_field_get_defining_FE_field_list(
	struct Comp_new_field *field,
	struct MANAGER(Comp_new_field) *comp_new_field_manager);
/*******************************************************************************
LAST MODIFIED : 12 January 2003

DESCRIPTION :
Returns the list of FE_fields that <field> depends on, by looking through the
<comp_new_field_manager>.
==============================================================================*/

struct Comp_new_field_finite_element_package *
	Comp_new_field_register_types_finite_element(
	struct Comp_new_field_package *comp_new_field_package,
	struct MANAGER(FE_field) *fe_field_manager,struct FE_time *fe_time);
/*******************************************************************************
LAST MODIFIED : 9 November 2001

DESCRIPTION :
This function registers the finite_element related types of Comp_new_fields and
also registers with the <fe_field_manager> so that any fe_fields are
automatically wrapped in corresponding comp_new_fields.
==============================================================================*/

int Comp_new_field_deregister_types_finite_element(
	struct Comp_new_field_finite_element_package
	*comp_new_field_finite_element_package);
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
==============================================================================*/
#endif /* !defined (COMP_NEW_FIELD_FINITE_ELEMENT_H) */
