/*******************************************************************************
FILE : computed_variable_finite_element.h

LAST MODIFIED : 23 January 2003

DESCRIPTION :
Implements computed variables which interface to finite element fields:
- cmiss_number.  Returns the cmiss number of a node or an element
- embedded.  Used for node fields that give element/xi - data at material points
???DB.  Why is Computed_variable_set_type_embedded static?
???DB.  Extend to element/xi
- finite_element.  A wrapper for a FE_field
- node_value
- xi_coordinate.  Returns the xi coordinates as a vector of FE_values -
	basically the identity
???DB.  Currently only implemented for element/xi ie. doesn't return the
	xi_coordinates of the node - depends on element

NOTES :

???DB.  Is a computed_variable_finite_element_utilities module needed?
???DB.  Consistency for is_type functions.  Currently some are iterator/
	conditional and some are not
==============================================================================*/
#if !defined (COMPUTED_VARIABLE_FINITE_ELEMENT_H)
#define COMPUTED_VARIABLE_FINITE_ELEMENT_H

#include "computed_variable/computed_variable.h"

/*
Global types
------------
*/
struct Computed_variable_finite_element_package;
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
Private package
==============================================================================*/

/*
Global functions
----------------
*/
int Computed_value_set_type_mesh_location(struct Computed_value *value,
	struct FE_region *region,struct FE_node *node,struct FE_element *element,
	FE_value *xi);
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
Makes <value> of type mesh_location and sets its <region>, <node>, <element>
and <xi).  After success, the <value> is responsible for DEALLOCATEing <xi>.
==============================================================================*/

int Computed_value_is_type_mesh_location(struct Computed_value *value);
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
Returns a non-zero if <value> is a mesh_location and zero otherwise.
==============================================================================*/

int Computed_value_get_type_mesh_location(Computed_value *value,
	struct FE_region **region_address,struct FE_node **node_address,
	struct FE_element **element_address,FE_value **xi_address);
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
If <value> is of type mesh_location, gets its <*region_address>,
<*node_address>, <*element_address> and <*xi_address).

The calling program must not DEALLOCATE the returned <*xi_address>.
==============================================================================*/

int Computed_variable_set_type_element_value(struct Computed_variable *variable,
	struct Computed_variable *fe_variable,struct FE_element *element,
	int *grid_point,int version);
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
Converts the <variable> into a element_value Computed_variable for the
specified <fe_variable> (a finite_element Computed_variable, all finite element
computed variables if NULL), <element> (all elements if NULL), <grid_point> (all
grid points if NULL), <version> (all versions if 0).
==============================================================================*/

int Computed_variable_is_type_element_value(struct Computed_variable *variable);
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
Returns non-zero if <variable> is a element_value Computed_variable.
==============================================================================*/

int Computed_variable_get_type_element_value(
	struct Computed_variable *variable,
	struct Computed_variable **fe_variable_address,
	struct FE_element **element_address,int **grid_point_address,
	int *version_address);
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
If <variable> is of type element_value, gets its <*fe_variable_address>,
<*element_address>, <*grid_point_address> and <*version_address>.

The calling program must not DEALLOCATE the returned <*grid_point_address>.
==============================================================================*/

int Computed_variable_set_type_FE_time(struct Computed_variable *variable,
	struct Computed_variable *fe_variable);
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
Converts the <variable> into a FE_time Computed_variable for the specified
<fe_variable> (a finite_element Computed_variable, all finite element
computed variables if NULL).
==============================================================================*/

int Computed_variable_is_type_FE_time(struct Computed_variable *variable);
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
Returns non-zero if <variable> is a FE_time Computed_variable.
==============================================================================*/

int Computed_variable_get_type_FE_time(struct Computed_variable *variable,
	struct Computed_variable **fe_variable_address,FE_value *fe_time_address);
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
If <variable> is of type FE_time, gets its <*fe_variable_address> and
<*fe_time_address>.
==============================================================================*/

int Computed_variable_set_type_finite_element(
	struct Computed_variable *variable,int component_number,
	struct FE_field *fe_field,struct MANAGER(FE_field) *fe_field_manager);
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
Converts the <variable> into a finite_element Computed_variable for the
specified <fe_field> and <component_number> (all components if -1).

Need pointer to <fe_field_manager> so can call MANAGED_OBJECT_NOT_IN_USE in
Computed_variable_finite_element_not_in_use.
==============================================================================*/

int Computed_variable_is_type_finite_element(
	struct Computed_variable *variable);
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
Returns non-zero if <variable> is a finite_element Computed_variable.
==============================================================================*/

int Computed_variable_get_type_finite_element(
	struct Computed_variable *variable,struct FE_field **fe_field_address,
	int *component_number_address);
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
If <variable> is of type finite_element, gets its <*fe_field_address> and
<*component_number_address>.
==============================================================================*/

int Computed_variable_set_type_mesh_location(struct Computed_variable *variable,
	struct Computed_variable *fe_variable);
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
Converts the <variable> into a mesh_location Computed_variable for the
specified <fe_variable> (a finite_element Computed_variable, all finite element
computed variables if NULL).
==============================================================================*/

int Computed_variable_is_type_mesh_location(struct Computed_variable *variable);
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
Returns non-zero if <variable> is a mesh_location Computed_variable.
==============================================================================*/

int Computed_variable_get_type_mesh_location(
	struct Computed_variable *variable,
	struct Computed_variable **fe_variable_address);
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
If <variable> is of type element_value, gets its <*fe_variable_address>.
==============================================================================*/

int Computed_variable_set_type_nodal_value(struct Computed_variable *variable,
	struct Computed_variable *fe_variable,struct FE_node *node,
	enum FE_nodal_value_type value_type,int version);
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
Converts the <variable> into a nodal_value Computed_variable for the
specified <fe_variable> (a finite_element Computed_variable, all finite element
computed variables if NULL), <node> (all nodes if NULL), <value_type> (all types
if FE_NODAL_UNKNOWN), <version> (all versions if 0).
==============================================================================*/

int Computed_variable_is_type_nodal_value(struct Computed_variable *variable);
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
Returns non-zero if <variable> is a nodal_value Computed_variable.
==============================================================================*/

int Computed_variable_get_type_nodal_value(struct Computed_variable *variable,
	struct Computed_variable **fe_variable_address,struct FE_node **node_address,
	enum FE_nodal_value_type *value_type_address,int *version_address);
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
If <variable> is of type nodal_value, gets its <*fe_variable_address>,
<*node_address>, <*value_type_address> and <*version_address>.
==============================================================================*/

int Computed_variable_set_type_scale_factor(struct Computed_variable *variable,
	struct Computed_variable *fe_variable,struct FE_element *element,
	int local_node_number,enum FE_nodal_value_type value_type,int version);
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
Converts the <variable> into a scale_factor Computed_variable for the
specified <fe_variable> (a finite_element Computed_variable, all finite element
computed variables if NULL), <element> (all elements if NULL),
<local_node_number> (all local nodes if -1), <value_type> (all types if
FE_NODAL_UNKNOWN), <version> (all versions if 0).
==============================================================================*/

int Computed_variable_is_type_scale_factor(struct Computed_variable *variable);
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
Returns non-zero if <variable> is a scale_factor Computed_variable.
==============================================================================*/

int Computed_variable_get_type_scale_factor(struct Computed_variable *variable,
	struct Computed_variable **fe_variable_address,
	struct FE_element **element_address,int *local_node_number_address,
	enum FE_nodal_value_type *value_type_address,int *version_address);
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
If <variable> is of type scale_factor, gets its <*fe_variable_address>,
<*element_address>, <*local_node_number_address>, <*value_type_address> and
<*version_address>.
==============================================================================*/

???DB.  Where I'm up to

int Computed_variable_is_read_only_with_fe_field(
	struct Computed_variable *field,void *fe_field_void);
/*******************************************************************************
LAST MODIFIED : 12 January 2003

DESCRIPTION :
Iterator/conditional function returning true if <field> is read only and a
wrapper for the finite element field <(struct FE_field *)fe_field_void>.

???DB.  Move to computed_variable_finite_element_utilities?
==============================================================================*/

int Computed_variable_has_coordinate_fe_field(struct Computed_variable *field,
	void *dummy);
/*******************************************************************************
LAST MODIFIED : 12 January 2003

DESCRIPTION :
Iterator/conditional function returning true if <field> is a finite_element
Computed_variable and the FE_field it wraps is of coordinate type.

???DB.  Move to computed_variable_finite_element_utilities?
==============================================================================*/

int Computed_variable_is_finite_element_and_scalar_integer(
	struct Computed_variable *field,void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 12 January 2003

DESCRIPTION :
Iterator/conditional function returning true if <field> is a finite_element
Computed_variable that returns a single integer.

???DB.  Move to computed_variable_finite_element_utilities?
==============================================================================*/

int Computed_variable_is_grid_based_finite_element_and_scalar_integer(
	struct Computed_variable *field,void *element_void);
/*******************************************************************************
LAST MODIFIED : 12 January 2003

DESCRIPTION :
Iterator/conditional function returning true if <field> is a finite_element
Computed_variable that returns a single integer and for which the wrapped FE_field
is grid-based.

Used for choosing field suitable for identifying grid points.

???DB.  Move to computed_variable_finite_element_utilities?
==============================================================================*/

int remove_computed_variable_from_manager_given_FE_field(
	struct MANAGER(Computed_variable) *computed_variable_manager,
	struct FE_field *fe_field);
/*******************************************************************************
LAST MODIFIED : 12 January 2003

DESCRIPTION :
Removes the finite_element Computed_variable that wraps <fe_field> from the
<computed_variable_manager> - this will mean that the wrapping Computed_variable is
destroyed.
==============================================================================*/

int destroy_computed_variable_given_fe_field(
	struct MANAGER(Computed_variable) *computed_variable_manager,
	struct MANAGER(FE_field) *fe_field_manager,
	struct FE_field *fe_field);
/*******************************************************************************
LAST MODIFIED : 12 January 2003

DESCRIPTION :
Removes the finite_element Computed_variable that wraps <fe_field> from the
<computed_variable_manager> - this will mean that the wrapping Computed_variable is
destroyed.  Also removes the <fe_field> from the <fe_field_manager> - this will
mean that <fe_field> is destroyed.
==============================================================================*/

int Computed_variable_is_type_cmiss_number(struct Computed_variable *field);
/*******************************************************************************
LAST MODIFIED : 12 January 2003

DESCRIPTION :
Returns true if <field> is a cmiss_number Computed_variable.
==============================================================================*/

int Computed_variable_set_type_cmiss_number(struct Computed_variable *field);
/*******************************************************************************
LAST MODIFIED : 12 January 2003

DESCRIPTION :
Converts <field> into a cmiss_number Computed_variable - returns the cmiss number
of a node or an element.  Sets the number of components to 1.

If function fails, <field> is guaranteed to be unchanged from its original
state, although its cache may be lost.
==============================================================================*/

int Computed_variable_is_type_embedded(struct Computed_variable *field,void *dummy);
/*******************************************************************************
LAST MODIFIED : 12 January 2003

DESCRIPTION :
Iterator/conditional function returning true if <field> is an embedded
Computed_variable.
==============================================================================*/

int Computed_variable_depends_on_embedded_field(struct Computed_variable *field);
/*******************************************************************************
LAST MODIFIED : 12 January 2003

DESCRIPTION :
Returns true if the field is of embedded type or depends on any computed fields
which are of embedded type.
==============================================================================*/

int Computed_variable_is_type_xi_coordinates(struct Computed_variable *field,
	void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 12 January 2003

DESCRIPTION :
Iterator/conditional function returning true if <field> is an xi_coordinates
Computed_variable.
==============================================================================*/

int Computed_variable_set_type_xi_coordinates(struct Computed_variable *field);
/*******************************************************************************
LAST MODIFIED : 12 January 2003

DESCRIPTION :
Converts <field> into a xi_coordinates Computed_variable.  Sets the number of
components to 3 - if the element has a dimension < 3 than the later xis are set
to zero.

If function fails, <field> is guaranteed to be unchanged from its original
state, although its cache may be lost.
==============================================================================*/

int Computed_variable_is_type_node_value(struct Computed_variable *field);
/*******************************************************************************
LAST MODIFIED : 12 January 2003

DESCRIPTION :
Returns true if <field> is a xi_coordinates Computed_variable.
==============================================================================*/

int Computed_variable_set_type_node_value(struct Computed_variable *field,
	struct FE_field *fe_field,enum FE_nodal_value_type nodal_value_type,
	int version_number);
/*******************************************************************************
LAST MODIFIED : 12 January 2003

DESCRIPTION :
Converts <field> into a node_value Computed_variable - returns the values for
given <nodal_value_type> and <version_number> of <fe_field> at a node.  Sets the
number of components and coordinate system to be the same as for <fe_field>.

If function fails, <field> is guaranteed to be unchanged from its original
state, although its cache may be lost.

???DB.  Will be replaced by the Computed_variable Computed_variables?
==============================================================================*/

struct LIST(FE_field) *Computed_variable_get_defining_FE_field_list(
	struct Computed_variable *field,
	struct MANAGER(Computed_variable) *computed_variable_manager);
/*******************************************************************************
LAST MODIFIED : 12 January 2003

DESCRIPTION :
Returns the list of FE_fields that <field> depends on, by looking through the
<computed_variable_manager>.
==============================================================================*/

struct Computed_variable_finite_element_package *
	Computed_variable_register_types_finite_element(
	struct Computed_variable_package *computed_variable_package,
	struct MANAGER(FE_field) *fe_field_manager,struct FE_time *fe_time);
/*******************************************************************************
LAST MODIFIED : 9 November 2001

DESCRIPTION :
This function registers the finite_element related types of Computed_variables and
also registers with the <fe_field_manager> so that any fe_fields are
automatically wrapped in corresponding computed_variables.
==============================================================================*/

int Computed_variable_deregister_types_finite_element(
	struct Computed_variable_finite_element_package
	*computed_variable_finite_element_package);
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
==============================================================================*/
#endif /* !defined (COMPUTED_VARIABLE_FINITE_ELEMENT_H) */
