/*******************************************************************************
FILE : comp_new_field_utilities.h

LAST MODIFIED : 19 January 2003

DESCRIPTION :
Utilities for Comp_new_field's.
==============================================================================*/
#if !defined (COMP_NEW_FIELD_UTILITIES_H)
#define COMP_NEW_FIELD_UTILITIES_H

#include "comp_new_field/comp_new_field.h"
	/*???DB.  May have to be _private */

#if defined (OLD_CODE)
#include "finite_element/finite_element.h"
#include "general/geometry.h"
#include "user_interface/message.h"
#endif /* defined (OLD_CODE) */

/*
Global types
------------
*/
#if defined (OLD_CODE)
???DB.  element/xi, time and nodes are losing their special place.  Now set
	like other dofs
struct Comp_new_field_conditional_data
/*******************************************************************************
LAST MODIFIED : 2 January 2003

DESCRIPTION :
==============================================================================*/
{
	struct Comp_new_field *conditional_field;
	FE_value time;
}; /* struct Comp_new_field_conditional_data */
#endif /* defined (OLD_CODE) */

/*
Global functions
----------------
*/
#if defined (OLD_CODE)
???DB.  element/xi, time and nodes are losing their special place.  Now set
	like other dofs
int Comp_new_field_is_defined_in_element_conditional(
	struct Comp_new_field *field,void *element_void);
/*******************************************************************************
LAST MODIFIED : 23 May 2000

DESCRIPTION :
Manager conditional function version of Comp_new_field_is_defined_in_element.
==============================================================================*/

int Comp_new_field_is_true_in_element(struct Comp_new_field *field,
	struct FE_element *element,FE_value time);
/*******************************************************************************
LAST MODIFIED : 3 December 2002

DESCRIPTION :
Returns true if <field> is determined to be "true" at the centre of <element>.
This is currently that the field is defined and any of the components are
nonzero.
==============================================================================*/

int FE_element_Comp_new_field_is_not_true_iterator(struct FE_element *element,
	void *comp_new_field_conditional_data_void);
/*******************************************************************************
LAST MODIFIED : 5 December 2002

DESCRIPTION :
Iterator version of NOT Comp_new_field_is_true_in_element.
==============================================================================*/

int Comp_new_field_is_defined_at_node(struct Comp_new_field *field,
	struct FE_node *node);
/*******************************************************************************
LAST MODIFIED : 13 May 1999

DESCRIPTION :
Returns true if <field> can be calculated at <node>. If the field depends on
any other fields, this function is recursively called for them.
==============================================================================*/

int Comp_new_field_is_defined_at_node_conditional(struct Comp_new_field *field,
	void *node_void);
/*******************************************************************************
LAST MODIFIED : 28 June 2000

DESCRIPTION :
Comp_new_field conditional function version of
Comp_new_field_is_defined_at_node.
==============================================================================*/

int Comp_new_field_is_true_at_node(struct Comp_new_field *field,
	struct FE_node *node, FE_value time);
/*******************************************************************************
LAST MODIFIED : 3 December 2002

DESCRIPTION :
Returns true if <field> is determined to be "true" at <node>.  This is currently
that the field is defined and any of the components are non zero.
==============================================================================*/

int FE_node_Comp_new_field_is_not_true_iterator(struct FE_node *node,
	void *fe_node_comp_new_field_conditional_data_void);
/*******************************************************************************
LAST MODIFIED : 3 December 2002

DESCRIPTION :
Iterator version of NOT Comp_new_field_is_true_at_node.
==============================================================================*/

int FE_node_has_Comp_new_field_defined(struct FE_node *node,void *field_void);
/*******************************************************************************
LAST MODIFIED : 28 June 2000

DESCRIPTION :
FE_node conditional function version of Comp_new_field_is_defined_at_node.
==============================================================================*/
#endif /* defined (OLD_CODE) */

int Comp_new_field_is_defined_conditional(struct Comp_new_field *field,
	void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 3 January 2003

DESCRIPTION :
Comp_new_field manager conditional function version of
Comp_new_field_is_defined.
==============================================================================*/

int Comp_new_field_is_true(struct Comp_new_field *field);
/*******************************************************************************
LAST MODIFIED : 3 January 2003

DESCRIPTION :
Returns true if <field> can be evaluated and any of its components are nonzero.
==============================================================================*/

int FE_element_Comp_new_field_is_not_true_iterator(struct FE_element *element,
	void *comp_new_field_void);
/*******************************************************************************
LAST MODIFIED : 3 January 2003

DESCRIPTION :
FE_element manager conditional function version of NOT Comp_new_field_is_true.
==============================================================================*/

int FE_node_Comp_new_field_is_not_true_iterator(struct FE_node *node,
	void *comp_new_field_void);
/*******************************************************************************
LAST MODIFIED : 3 January 2003

DESCRIPTION :
FE_node manager conditional function version of NOT Comp_new_field_is_true.
==============================================================================*/

int FE_node_has_Comp_new_field_defined(struct FE_node *node,void *field_void);
/*******************************************************************************
LAST MODIFIED : 3 January 2003

DESCRIPTION :
FE_node manager conditional function version of Comp_new_field_is_defined.
==============================================================================*/

#if defined (OLD_CODE)
???DB.  Should be able to be replace by Degree_of_freedom_set_value(s)?  What
	about transformations?

int Comp_new_field_set_values_at_node(struct Comp_new_field *field,
	struct FE_node *node,FE_value *values);
/*******************************************************************************
LAST MODIFIED : 20 April 2000

DESCRIPTION :
Sets the <values> of the computed <field> at <node>. Only certain computed field
types allow their values to be set. Fields that deal directly with FE_fields eg.
FINITE_ELEMENT and NODE_VALUE fall into this category, as do the various
transformations, RC_COORDINATE, RC_VECTOR, OFFSET, SCALE, etc. which convert
the values into what they expect from their source field, and then call the same
function for it. If a field has more than one source field, eg. RC_VECTOR, it
can in many cases still choose which one is actually being changed, for example,
the 'vector' field in this case - coordinates should not change. This process
continues until the actual FE_field values at the node are changed or a field
is reached for which its calculation is not reversible, or is not supported yet.
Note that you must only call this function for nodes that are not managed as it
will change values inside them. Also, this function does not clear the cache at
any time, so up to the calling function to do so.
Note that the values array will not be modified by this function. Also, <node>
should not be managed at the time it is modified by this function.
???RC Note that some functions are not reversible in this way.
==============================================================================*/

int Comp_new_field_set_values_at_managed_node(struct Comp_new_field *field,
	struct FE_node *node,FE_value *values,struct MANAGER(FE_node) *node_manager);
/*******************************************************************************
LAST MODIFIED : 20 April 2000

DESCRIPTION :
Sets the <values> of the computed <field> at <node>. Only certain computed field
types allow their values to be set. Fields that deal directly with FE_fields eg.
FINITE_ELEMENT and NODE_VALUE fall into this category, as do the various
transformations, RC_COORDINATE, RC_VECTOR, OFFSET, SCALE, etc. which convert
the values into what they expect from their source field, and then call the same
function for it. If a field has more than one source field, eg. RC_VECTOR, it
can in many cases still choose which one is actually being changed, for example,
the 'vector' field in this case - coordinates should not change. This process
continues until the actual FE_field values at the node are changed or a field
is reached for which its calculation is not reversible, or is not supported yet.

This function works by making a copy of the node, then performing all
modifications to it. If these are successful then MANAGER_MODIFY_NOT_IDENTIFIER
is called to effect the change on the real <node>, thus sending manager messages
to the rest of the program. Because all changes are made on a temporary copy of
the node, all current cache values will be ignored. For safety, however, the
cache is always cleared after calling.
It is up to calling function to begin/end node manager cache if more than one
node is being modified.
Note that the values array will not be modified by this function.
==============================================================================*/
#endif /* defined (OLD_CODE) */

#if defined (OLD_CODE)
???DB.  Should be able to be replace by Degree_of_freedom_get_value(s)?  What
	about transformations?

int Comp_new_field_get_values_in_element(struct Comp_new_field *field,
	struct FE_element *element,int *number_in_xi,FE_value **values,FE_value time);
/*******************************************************************************
LAST MODIFIED : 3 December 2001

DESCRIPTION :
Companion function to Comp_new_field_set_values_in_element.
Returns the <field> calculated at the corners of the <number_in_xi> cells,
evenly spaced in xi, over the element. <values> should be allocated with enough
space for number_of_components * product of number_in_xi+1 in each element
direction, the returned values cycling fastest through number of grid points in
xi1, number of grid points in xi2 etc. and lastly components.
It is up to the calling function to deallocate the returned values.
==============================================================================*/
#endif /* defined (OLD_CODE) */

#if defined (OLD_CODE)
???DB.  Should be able to be replace by Degree_of_freedom_set_value(s)?  What
	about transformations?

int Comp_new_field_set_values_in_element(struct Comp_new_field *field,
	struct FE_element *element,int *number_in_xi,FE_value *values);
/*******************************************************************************
LAST MODIFIED : 20 April 2000

DESCRIPTION :
Sets the <values> of the computed <field> over the <element>. Only certain
computed field types allow their values to be set. Fields that deal directly
with FE_fields eg. FINITE_ELEMENT fall into this category, as do the various
transformations, RC_COORDINATE, RC_VECTOR, OFFSET, SCALE, etc. which convert
the values into what they expect from their source field, and then call the
same function for it. If a field has more than one source field, eg. RC_VECTOR,
it can in many cases still choose which one is actually being changed, for
example, the 'vector' field in this case - coordinates should not change. This
process continues until the actual FE_field values in the element are changed or
a field is reached for which its calculation is not reversible, or is not
supported yet.

<number_in_xi> has the number of grid cells in each xi direction of <element>,
such that there is one more grid point in each direction than this number. Grid
points are evenly spaced in xi. There are as many <values> as there are grid
points X number_of_components, cycling fastest through number of grid points in
xi1, number of grid points in xi2 etc. and lastly components.

Note that the values array will not be modified by this function. Also,
<element> should not be managed at the time it is modified by this function.
???RC Note that some functions are not reversible in this way.
==============================================================================*/

int Comp_new_field_set_values_in_managed_element(struct Comp_new_field *field,
	struct FE_element *element,int *number_in_xi,FE_value *values,
	struct MANAGER(FE_element) *element_manager);
/*******************************************************************************
LAST MODIFIED : 20 April 2000

DESCRIPTION :
Sets the <values> of the computed <field> over the <element>. Only certain
computed field types allow their values to be set. Fields that deal directly
with FE_fields eg. FINITE_ELEMENT fall into this category, as do the various
transformations, RC_COORDINATE, RC_VECTOR, OFFSET, SCALE, etc. which convert
the values into what they expect from their source field, and then call the
same function for it. If a field has more than one source field, eg. RC_VECTOR,
it can in many cases still choose which one is actually being changed, for
example, the 'vector' field in this case - coordinates should not change. This
process continues until the actual FE_field values in the element are changed or
a field is reached for which its calculation is not reversible, or is not
supported yet.

<number_in_xi> has the number of grid cells in each xi direction of <element>,
such that there is one more grid point in each direction than this number. Grid
points are evenly spaced in xi. There are as many <values> as there are grid
points X number_of_components, cycling fastest through number of grid points in
xi1, number of grid points in xi2 etc. and lastly components.

This function works by making a copy of the element, then performing all
modifications to it. If these are successful then MANAGER_MODIFY_NOT_IDENTIFIER
is called to effect the change on the real <element>, thus sending manager
messages to the rest of the program. Because all changes are made on a temporary
copy of the element, all current cache values will be ignored. For safety,
however, the cache is always cleared after calling.
It is up to calling function to begin/end node manager cache if more than one
node is being modified.
Note that the values array will not be modified by this function.
==============================================================================*/
#endif /* defined (OLD_CODE) */

int Comp_new_field_get_native_discretization_in_element(
	struct Comp_new_field *field,struct FE_element *element,int *number_in_xi);
/*******************************************************************************
LAST MODIFIED : 28 October 1999

DESCRIPTION :
If the <field> or its source field is grid-based in <element>, returns in
<number_in_xi> the numbers of finite difference cells in each xi-direction of
<element>. Note that this number is one less than the number of grid points in
each direction. <number_in_xi> should be allocated with at least as much space
as the number of dimensions in <element>, but is assumed to have no more than
MAXIMUM_ELEMENT_XI_DIMENSIONS so that
int number_in_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS] can be passed to this function.
Returns 0 with no errors if the field is not grid-based.
==============================================================================*/

int Comp_new_field_has_3_components(struct Comp_new_field *field,
	void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 10 March 1999

DESCRIPTION :
Conditional function returning true if <field> has exactly three components.
==============================================================================*/

int Comp_new_field_has_4_components(struct Comp_new_field *field,
	void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 10 March 1999

DESCRIPTION :
Conditional function returning true if <field> has exactly four components.
==============================================================================*/

int Comp_new_field_has_up_to_4_numerical_components(
	struct Comp_new_field *field,void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 23 May 2000

DESCRIPTION :
Conditional function returning true if <field> has up to 4 components and they
are numerical - useful for selecting vector/coordinate fields.
==============================================================================*/

int Comp_new_field_has_n_components(struct Comp_new_field *field,
	void *components_ptr_void);
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Iterator/conditional function returning true if <field> has the same number of
components as that specified by <components_ptr_void>.
==============================================================================*/

int Comp_new_field_has_at_least_2_components(struct Comp_new_field *field,
	void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 16 March 1999

DESCRIPTION :
Iterator/conditional function returning true if <field> has at least 2
components.
==============================================================================*/

int Comp_new_field_has_numerical_components(struct Comp_new_field *field,
	void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 23 May 2000

DESCRIPTION :
Conditional function returning true if <field> returns numerical components.
Note that whether the numbers were integer, FE_value or double, they may be
returned as FE_value when evaluated.
==============================================================================*/

int Comp_new_field_is_scalar(struct Comp_new_field *field,void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 23 May 2000

DESCRIPTION :
Conditional function returning true if <field> has 1 component and it is
numerical.
==============================================================================*/

int Comp_new_field_has_up_to_3_numerical_components(
	struct Comp_new_field *field,void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 23 May 2000

DESCRIPTION :
Conditional function returning true if <field> has up to 3 components and they
are numerical - useful for selecting vector/coordinate fields.
==============================================================================*/

int Comp_new_field_has_up_to_4_numerical_components(
	struct Comp_new_field *field,void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 23 May 2000

DESCRIPTION :
Conditional function returning true if <field> has up to 4 components and they
are numerical - useful for selecting vector/coordinate fields.
==============================================================================*/

int Comp_new_field_has_multiple_times(struct Comp_new_field *field);
/*******************************************************************************
LAST MODIFIED : 22 November 2001

DESCRIPTION :
Conditional function returning true if <field> depends on time.
==============================================================================*/

int Comp_new_field_is_orientation_scale_capable(struct Comp_new_field *field,
	void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 12 February 1999

DESCRIPTION :
Conditional function returning true if the field can be used to orient or scale
glyphs. Generally, this means it has 1,2,3,4,6 or 9 components, where:
1 = scalar (no vector, isotropic scaling).
2 = 1 2-D vector (2nd axis is normal in plane, 3rd is out of 2-D plane);
3 = 1 3-D vector (orthogonal 2nd and 3rd axes are arbitrarily chosen);
4 = 2 2-D vectors (3rd axis taken as out of 2-D plane);
6 = 2 3-D vectors (3rd axis found from cross product);
9 = 3 3-D vectors = complete definition of 3 axes.
???RC.  Include coordinate system in test?
==============================================================================*/

int Comp_new_field_is_stream_vector_capable(struct Comp_new_field *field,
	void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 15 March 1999

DESCRIPTION :
Conditional function returning true if the field is suitable for 3-D streamline
tracking. This means it has either 3, 6 or 9 components (with 3 components per
vector), or has a FIBRE coordinate_system, meaning it can be wrapped to produce
9-component fibre_axes.
The number of components controls how the field is interpreted:
3 = 1 3-D vector (lateral direction and normal worked out from curl of field);
6 = 2 3-D vectors (2nd vector is lateral direction. Stream ribbon normal found
		from cross product);
9 = 3 3-D vectors (2nd vector is lateral direction; 3rd vector is stream ribbon
		normal).
==============================================================================*/

int Comp_new_field_is_read_only(struct Comp_new_field *field);
/*******************************************************************************
LAST MODIFIED : 26 January 1999

DESCRIPTION :
Returns true if the field is read_only - use this to check if you are allowed
to modify or remove it from the manager.
==============================================================================*/

int Comp_new_field_find_element_xi(struct Comp_new_field *field,
	FE_value *values, int number_of_values, struct FE_element **element,
	FE_value *xi, struct GROUP(FE_element) *search_element_group,
	int propagate_field);
/*******************************************************************************
LAST MODIFIED : 22 April 2002

DESCRIPTION :
This function implements the reverse of some certain comp_new_fields
(Comp_new_field_is_find_element_xi_capable) so that it tries to find an element
and xi which would evaluate to the given values.
This has been implemented so that the texture_coordinates can be used to extract
information from textures (sample_texture comp_new_field) and then modified and
then put back into another texture.
The <search_element_group> is the set of elements from which the chosen element
will belong.
If <propagate_field> is set and the field has a find_element_xi_function, it
is called to undo its field calculation and resume the search on its source
field. This can be result in less computation, but can fail if the source field
is multivalued, a common case being when it is in a polar coordinate system
since valid values may be a multiple of  2*PI out.
If <propagate_field> is not set or there is no <find_element_xi_function> this
function searches all elements in <search_element_group> trying to find a point
at which the field evaluates to the <values>.
==============================================================================*/

int Comp_new_field_is_find_element_xi_capable(struct Comp_new_field *field,
	void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 16 June 2000

DESCRIPTION :
This function returns true if the <field> can find element and xi given
a set of values.
==============================================================================*/

int equivalent_comp_new_fields_at_elements(struct FE_element *element_1,
	struct FE_element *element_2);
/*******************************************************************************
LAST MODIFIED : 23 May 2000

DESCRIPTION :
Returns true if all fields are defined in the same way at the two elements.
==============================================================================*/

int equivalent_comp_new_fields_at_nodes(struct FE_node *node_1,
	struct FE_node *node_2);
/*******************************************************************************
LAST MODIFIED : 23 May 2000

DESCRIPTION :
Returns true if all fields are defined in the same way at the two nodes.
==============================================================================*/
#endif /* !defined (COMP_NEW_FIELD_UTILITIES_H) */
