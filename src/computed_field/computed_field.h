/*******************************************************************************
FILE : computed_field.h

LAST MODIFIED : 18 April 2005

DESCRIPTION :
A cmzn_field is an abstraction of an FE_field. For each FE_field there is
a wrapper cmzn_field automatically generated that can be called on to
evaluate the field in an element or node. The interface for evaluating
Computed_fields is much simpler than for FE_field, since they hide details of
evaluation caches, for example. Their main benefit is in
allowing new types of fields to be defined as functions of other fields and
source information, such as scale, offset, magnitude, gradient,
coordinate transformations etc., thus providing cmgui with the ability to
provide customised features without cluttering dialogs such as the graphical
element editor. Also, finite_element_to_graphics_object is greatly simplified
because it can assume all coordinate and other fields are in rectangular
cartesian coordinates - if they are not already, the rest of the program can
make a simple wrapper computed_field effecting this change. A positive
consequence of this change is that this file should in time contain the only
code for invoking coordinate transformations in the entire program.

In addition to these functional benefits, computed fields cache the last
position they were evaluated at and the values and derivatives of the field at
that point so that if values at that point are requested again they can be
immediately returned. This allows functions using a number of fields, some
possibly depending on each other to get the maximum benefit out of values that
have already been calculated, without requiring complex logic for determining
if a value is already known.
==============================================================================*/
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (COMPUTED_FIELD_H)
#define COMPUTED_FIELD_H

#include "opencmiss/zinc/types/meshid.h"
#include "opencmiss/zinc/field.h"
#include "finite_element/finite_element.h"
#include "general/geometry.h"
#include "general/list.h"
#include "general/manager.h"
#include "general/object.h"
#include "general/message.h"

struct cmzn_region;

/*
The Public interface to cmzn_field is contained in
zinc/field.h however most of the functions come directly from
this module.  So that these functions match the public declarations the
functions are given their public names.
*/

/* Convert legacy object type name to use external field object type name */
#define Computed_field cmzn_field

// Enumerations of fields implemented in json field IO
// Note enumeration is not exposed in public API so values are not API
enum cmzn_field_type
{
	CMZN_FIELD_TYPE_INVALID = 0,
	// apply
	CMZN_FIELD_TYPE_APPLY,
	CMZN_FIELD_TYPE_ARGUMENT_REAL,
	// arithmetic operators
	CMZN_FIELD_TYPE_ABS,
	CMZN_FIELD_TYPE_ADD,
	CMZN_FIELD_TYPE_DIVIDE,
	CMZN_FIELD_TYPE_EXP,
	CMZN_FIELD_TYPE_LOG,
	CMZN_FIELD_TYPE_MULTIPLY,
	CMZN_FIELD_TYPE_POWER,
	CMZN_FIELD_TYPE_SQRT,
	CMZN_FIELD_TYPE_SUBTRACT,
	// composite
	CMZN_FIELD_TYPE_COMPONENT,
	CMZN_FIELD_TYPE_CONCATENATE,
	CMZN_FIELD_TYPE_IDENTITY,
	// conditional
	CMZN_FIELD_TYPE_IF,
	// constant
	CMZN_FIELD_TYPE_CONSTANT,
	CMZN_FIELD_TYPE_STRING_CONSTANT,
	// coordinate transformation
	CMZN_FIELD_TYPE_COORDINATE_TRANSFORMATION,
	CMZN_FIELD_TYPE_VECTOR_COORDINATE_TRANSFORMATION,
	// derivatives
	CMZN_FIELD_TYPE_CURL,
	CMZN_FIELD_TYPE_DERIVATIVE,
	CMZN_FIELD_TYPE_DIVERGENCE,
	CMZN_FIELD_TYPE_GRADIENT,
	// fibre
	CMZN_FIELD_TYPE_FIBRE_AXES,
	// finite element
	CMZN_FIELD_TYPE_EDGE_DISCONTINUITY,
	CMZN_FIELD_TYPE_EMBEDDED,
	CMZN_FIELD_TYPE_FIND_MESH_LOCATION,
	CMZN_FIELD_TYPE_FINITE_ELEMENT,
	CMZN_FIELD_TYPE_IS_EXTERIOR,
	CMZN_FIELD_TYPE_IS_ON_FACE,
	CMZN_FIELD_TYPE_NODE_VALUE,
	CMZN_FIELD_TYPE_STORED_MESH_LOCATION,
	CMZN_FIELD_TYPE_STORED_STRING,
	// logical operators
	CMZN_FIELD_TYPE_AND,
	CMZN_FIELD_TYPE_EQUAL_TO,
	CMZN_FIELD_TYPE_GREATER_THAN,
	CMZN_FIELD_TYPE_IS_DEFINED,
	CMZN_FIELD_TYPE_LESS_THAN,
	CMZN_FIELD_TYPE_NOT,
	CMZN_FIELD_TYPE_OR,
	CMZN_FIELD_TYPE_XOR,
	// matrix operators
	CMZN_FIELD_TYPE_DETERMINANT,
	CMZN_FIELD_TYPE_EIGENVALUES,
	CMZN_FIELD_TYPE_EIGENVECTORS,
	CMZN_FIELD_TYPE_MATRIX_INVERT,
	CMZN_FIELD_TYPE_MATRIX_MULTIPLY,
	CMZN_FIELD_TYPE_PROJECTION,
	CMZN_FIELD_TYPE_TRANSPOSE,
	// mesh operators
	CMZN_FIELD_TYPE_MESH_INTEGRAL,
	CMZN_FIELD_TYPE_MESH_INTEGRAL_SQUARES,
	// nodeset operators
	CMZN_FIELD_TYPE_NODESET_MAXIMUM,
	CMZN_FIELD_TYPE_NODESET_MEAN,
	CMZN_FIELD_TYPE_NODESET_MEAN_SQUARES,
	CMZN_FIELD_TYPE_NODESET_MINIMUM,
	CMZN_FIELD_TYPE_NODESET_SUM,
	CMZN_FIELD_TYPE_NODESET_SUM_SQUARES,
	// time
	CMZN_FIELD_TYPE_TIME_LOOKUP,
	CMZN_FIELD_TYPE_TIME_VALUE,
	// trigonometry */
	CMZN_FIELD_TYPE_ACOS,
	CMZN_FIELD_TYPE_ASIN,
	CMZN_FIELD_TYPE_ATAN,
	CMZN_FIELD_TYPE_ATAN2,
	CMZN_FIELD_TYPE_COS,
	CMZN_FIELD_TYPE_SIN,
	CMZN_FIELD_TYPE_TAN,
	// vector operators
	CMZN_FIELD_TYPE_CROSS_PRODUCT,
	CMZN_FIELD_TYPE_DOT_PRODUCT,
	CMZN_FIELD_TYPE_MAGNITUDE,
	CMZN_FIELD_TYPE_NORMALISE,
	CMZN_FIELD_TYPE_SUM_COMPONENTS
};


/*
Global types
------------
*/

DECLARE_LIST_TYPES(cmzn_field);

DECLARE_MANAGER_TYPES(cmzn_field);


struct List_Computed_field_commands_data
{
	const char *command_prefix;
	int listed_fields;
	struct LIST(cmzn_field) *computed_field_list;
	struct MANAGER(cmzn_field) *computed_field_manager;
}; /* struct List_Computed_field_commands_data */

struct Computed_field_conditional_data
{
	struct cmzn_field *conditional_field;
	FE_value time;
}; /* struct Computed_field_conditional_data */

/***************************************************************************//**
 * Stores data and settings needed to create field, including the region to add
 * it to.
 */
struct cmzn_fieldmodule;

/*
Global functions
----------------
*/
PROTOTYPE_OBJECT_FUNCTIONS(cmzn_field);
PROTOTYPE_GET_OBJECT_NAME_FUNCTION(cmzn_field);

PROTOTYPE_LIST_FUNCTIONS(cmzn_field);
PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(cmzn_field,name,const char *);

/***************************************************************************//**
 * Internal variant of public cmzn_fielditerator_next() which does not access
 * the returned field, for more efficient if less safe usage.
 *
 * @param iterator  Field iterator to query and advance.
 * @return  Non-accessed pointer to the next field, or NULL if none remaining.
 */
cmzn_field_id cmzn_fielditerator_next_non_access(
	cmzn_fielditerator_id iterator);

PROTOTYPE_MANAGER_COPY_FUNCTIONS(cmzn_field,name,const char *);
PROTOTYPE_MANAGER_FUNCTIONS(cmzn_field);
PROTOTYPE_MANAGER_IDENTIFIER_WITHOUT_MODIFY_FUNCTIONS(cmzn_field,name,const char *);

/***************************************************************************//**
 * Returns true if <field> can be calculated in <element>. If the field depends
 * on any other fields, this function is recursively called for them.
 * Warning: inefficient as field cache is created and destroyed with each call.
 * For new code, prefer managing your own field cache and directly using
 * cmzn_field_id_defined_at_location.
 */
int Computed_field_is_defined_in_element(struct cmzn_field *field,
	struct FE_element *element);

/***************************************************************************//**
 * List iterator version of Computed_field_is_defined_in_element.
 * Returns true if <field> can be calculated in <element>.
 * Warning: inefficient as field cache is created and destroyed with each call.
 * For new code, prefer managing your own field cache and directly using
 * cmzn_field_id_defined_at_location.
 */
int Computed_field_is_defined_in_element_conditional(
	struct cmzn_field *field,void *element_void);

/***************************************************************************//**
 * Returns true if <field> can be calculated in <node>. If the field depends
 * on any other fields, this function is recursively called for them.
 * Warning: inefficient as field cache is created and destroyed with each call.
 * For new code, prefer managing your own field cache and directly using
 * cmzn_field_id_defined_at_location.
 */
int Computed_field_is_defined_at_node(struct cmzn_field *field,
	struct FE_node *node);

/***************************************************************************//**
 * List iterator version of Computed_field_is_defined_at_node.
 * Returns true if <field> can be calculated in <element>.
 * Warning: inefficient as field cache is created and destroyed with each call.
 * For new code, prefer managing your own field cache and directly using
 * cmzn_field_id_defined_at_location.
 */
int Computed_field_is_defined_at_node_conditional(struct cmzn_field *field,
	void *node_void);

/***************************************************************************//**
 * List conditional function returning true if field value type is
 * CMZN_FIELD_VALUE_TYPE_STRING.
 */
int Computed_field_has_string_value_type(struct cmzn_field *field,
	void *dummy_void);

/**
 * For <field> and all of its source fields (and their source fields) from the
 * same region/manager as field, calls <iterator_function> with <user_data>.
 * Iteration stops if a single iterator_function call returns 0.
 */
int Computed_field_for_each_ancestor_same_region(struct cmzn_field *field,
	LIST_ITERATOR_FUNCTION(cmzn_field) *iterator_function, void *user_data);

/***************************************************************************//**
 * Prescribes a location in an element for field evaluation or assignment with
 * the cache. Variant with optional prescribed parent element.
 * Note: replaces any other spatial location in cache (e.g. node.) but time
 * is unchanged.
 *
 * @param cache  The field cache to set the location in.
 * @param element  The element the location is in. Must belong to same region
 * as cache.
 * @param number_of_chart_coordinates  The size of the chart_coordinates array,
 * checked to be not less than the element dimension.
 * @param chart_coordinates  Location in element chart. Value is not checked;
 * caller is responsible for supplying locations within the bounds of the
 * element shape.
 * @param top_level_element  Optional highest dimensional parent element to
 * inherit fields from.
 * @return  1 on success, 0 on failure.
 */
int cmzn_fieldcache_set_mesh_location_with_parent(
	cmzn_fieldcache_id cache, cmzn_element_id element,
	int number_of_chart_coordinates, const double *chart_coordinates,
	cmzn_element_id top_level_element);

/***************************************************************************//**
 * Internal function which if set means subsequent values are assigned into the
 * cache only, not into field values. Subsequent evaluations return this and
 * other fields as if the values had been set in the field.
 * Remember to clear flag after experimental evaluation with assigned values.
 * Warning: only a single assign works with the current code.
 * Changing the cache location clears these temporary cached values.
 *
 * @param cache  Store of location to assign at and intermediate field values.
 * @param assign_to_cache  The new state of the assign-to-cache-only flag.
 * @return  1 on success, 0 on failure.
 */
int cmzn_fieldcache_set_assign_in_cache(cmzn_fieldcache_id cache, int assign_in_cache);

/**
 * Internal function.
 * Note: no warnings if not evaluated so can be used for is_defined
 *
 * @return  true if any field component value is non zero with a small tolerance,
 * otherwise false.
 */
bool cmzn_field_evaluate_boolean(cmzn_field_id field,
	cmzn_fieldcache_id cache);

/***************************************************************************//**
 * Temporary function to allow first derivatives to be evaluated with value.
 * @param number_of_derivatives  Expected number of derivatives for domain.
 * @param derivatives  Array of size number_of_values*number_of_derivatives.
 * IMPORTANT: Not approved for external API!
 * @deprecated
 * Try to remove its use as soon as possible.
 */
int cmzn_field_evaluate_real_with_derivatives(cmzn_field_id field,
	cmzn_fieldcache_id cache, int number_of_values, double *values,
	int number_of_derivatives, double *derivatives);

int Computed_field_get_native_discretization_in_element(
	struct cmzn_field *field,struct FE_element *element,int *number_in_xi);
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

int Computed_field_get_native_resolution(struct cmzn_field *field,
		int *dimension, int **sizes,
	struct cmzn_field **texture_coordinate_field);
/*******************************************************************************
LAST MODIFIED : 03 February 2005

DESCRIPTION :
Gets the <dimension>, <sizes>, <minimums>, <maximums> and <texture_coordinate_field> from
the <field>. These parameters will be used in image processing.

==============================================================================*/

const char *Computed_field_get_type_string(struct cmzn_field *field);
/*******************************************************************************
LAST MODIFIED : 24 January 2002

DESCRIPTION :
Returns the string which identifies the type.
The calling function must not deallocate the returned string.
==============================================================================*/

int Computed_field_has_3_components(struct cmzn_field *field,
	void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 10 March 1999

DESCRIPTION :
Conditional function returning true if <field> has exactly three components.
==============================================================================*/

int Computed_field_has_4_components(struct cmzn_field *field,
	void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 10 March 1999

DESCRIPTION :
Conditional function returning true if <field> has exactly four components.
==============================================================================*/

int Computed_field_has_16_components(struct cmzn_field *field,
	void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 21 February 2008

DESCRIPTION :
Iterator/conditional function returning true if <field> has exactly sixteen
components - useful for selecting transformation matrix.
==============================================================================*/

int Computed_field_has_up_to_4_numerical_components(
	struct cmzn_field *field,void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 23 May 2000

DESCRIPTION :
Conditional function returning true if <field> has up to 4 components and they
are numerical - useful for selecting vector/coordinate fields.
==============================================================================*/

int Computed_field_has_n_components(struct cmzn_field *field,
	void *components_ptr_void);
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Iterator/conditional function returning true if <field> has the same number of
components as that specified by <components_ptr_void>.
==============================================================================*/

int Computed_field_has_at_least_2_components(struct cmzn_field *field,
	void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 16 March 1999

DESCRIPTION :
Iterator/conditional function returning true if <field> has at least 2 components.
==============================================================================*/

/*******************************************************************************
 * @return  1 if field returns mesh_location value type.
 */
int Computed_field_has_value_type_mesh_location(struct cmzn_field *field,
	void *dummy_void);

int Computed_field_has_numerical_components(struct cmzn_field *field,
	void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 23 May 2000

DESCRIPTION :
Conditional function returning true if <field> returns numerical components.
Note that whether the numbers were integer, FE_value or double, they may be
returned as FE_value when evaluated.
==============================================================================*/

int Computed_field_is_scalar(struct cmzn_field *field,void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 23 May 2000

DESCRIPTION :
Conditional function returning true if <field> has 1 component and it is
numerical.
==============================================================================*/

int Computed_field_has_up_to_3_numerical_components(
	struct cmzn_field *field,void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 23 May 2000

DESCRIPTION :
Conditional function returning true if <field> has up to 3 components and they
are numerical - useful for selecting vector/coordinate fields.
==============================================================================*/

int Computed_field_has_up_to_4_numerical_components(
	struct cmzn_field *field,void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 23 May 2000

DESCRIPTION :
Conditional function returning true if <field> has up to 4 components and they
are numerical - useful for selecting vector/coordinate fields.
==============================================================================*/

int Computed_field_has_multiple_times(struct cmzn_field *field);
/*******************************************************************************
LAST MODIFIED : 22 November 2001

DESCRIPTION :
Conditional function returning true if <field> depends on time.
==============================================================================*/

int Computed_field_is_orientation_scale_capable(struct cmzn_field *field,
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

int Computed_field_is_stream_vector_capable(struct cmzn_field *field,
	void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 15 March 1999

DESCRIPTION :
Conditional function returning true if the field is suitable for 3-D streamline
tracking. This means it has either 3, 6 or 9 components (with 3 components per
vector), or has a FIBRE coordinate_system, meaning it can be wrapped to produce
9-component fibre_axes.  Also now supports 2 components for use with a 2
component coordinate field.
The number of components controls how the field is interpreted:
3 = 1 3-D vector (lateral direction and normal worked out from curl of field);
6 = 2 3-D vectors (2nd vector is lateral direction. Stream ribbon normal found
	from cross product);
9 = 3 3-D vectors (2nd vector is lateral direction; 3rd vector is stream ribbon
	normal).
==============================================================================*/

/* Returns true if all fields are defined in the same way at the two elements. */
bool equivalent_computed_fields_at_elements(struct FE_element *element_1,
	struct FE_element *element_2);

int equivalent_computed_fields_at_nodes(struct FE_node *node_1,
	struct FE_node *node_2);
/*******************************************************************************
LAST MODIFIED : 23 May 2000

DESCRIPTION :
Returns true if all fields are defined in the same way at the two nodes.
==============================================================================*/

int list_Computed_field(struct cmzn_field *field,void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 25 January 1999

DESCRIPTION :
Writes the properties of the <field> to the command window.
==============================================================================*/

int write_Computed_field_commands_to_comfile(struct cmzn_field *field,
	 void *command_prefix_void);
/*******************************************************************************
LAST MODIFIED : 10 August 2007

DESCRIPTION :
Writes the commands needed to reproduce <field> to the com file.
==============================================================================*/

int write_Computed_field_commands_if_managed_source_fields_in_list_to_comfile(
	 struct cmzn_field *field, void *list_commands_data_void);
/*******************************************************************************
LAST MODIFIED : 10 August 2007

DESCRIPTION :
Calls list_Computed_field_commands if the field is not already in the list,
has no source fields, or all its source fields are either not managed or
already in the list. If the field is listed, it is added to the list.
Ensures field command list comes out in the order they need to be created.
Note, must be cycled through as many times as it takes till listed_fields -> 0.
Second argument is a struct List_Computed_field_commands_data.
==============================================================================*/

int list_Computed_field_name(struct cmzn_field *field,void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 4 February 1999

DESCRIPTION :
Lists a single line about a computed field including just name, number of
components, coordinate system and type.
==============================================================================*/

int Computed_field_can_be_destroyed(struct cmzn_field *field);
/*******************************************************************************
LAST MODIFIED : 10 May 2000

DESCRIPTION :
Returns true if the <field> is only accessed once - assumed by its manager. If
it is of type COMPUTED_FIELD_FINITE_ELEMENT further tests that its fe_field can
be destroyed, assuming it is only accessed by this field and its manager.
==============================================================================*/

/***************************************************************************//**
 * Returns the region which this field belongs to, if any.
 *
 * @param field  The field.
 * @return  A handle to the owning region if in one, or NULL if none.
 */
struct cmzn_region *Computed_field_get_region(struct cmzn_field *field);

/**
 * Returns true if the field is a finite element coordinate type field and
 * composed of three or less components, false otherwise.
 *
 * @param field the field to determine whether or not it is a coordinate field
 * @return 1 if the field is a coordinate field, 0 otherwise
 */
int Computed_field_is_coordinate_field(struct cmzn_field *field, void *not_in_use);

/*******************************************************************************
 * Returns the domains of the given field by recursively searching through the
 * field's source fields
 *
 * @param field  The field to find the domain of
 * @param domain_field_list  A handle to the list of domains for the field
 * @return  1 on success, 0 otherwise
 */
int Computed_field_get_domain( struct cmzn_field *field,
	struct LIST(cmzn_field) *domain_field_list );

/*******************************************************************************
 * Returns true if the field value varies non-linearly over its domain.
 * If applicable, e.g. for coordinate fields, callers should first determine
 * whether the coordinate system varies non-linearly with screen space since
 * that is a less expensive query.
 * Note that an image-based field is linear if it is interpolated linearly.
 *
 * @see Coordinate_system_type_is_non_linear
 * @param field  The field to query.
 * @return  1 if non-linear, 0 if linear.
 */
int Computed_field_is_non_linear(struct cmzn_field *field);

/***************************************************************************//**
 * Returns true if field is not a source field of other.
 *
 * @param field  The field.
 * @return  1 if field is not a source field of others otherwise 0.
 */
int Computed_field_is_not_source_field_of_others(struct cmzn_field *field);

#if defined (DEBUG_CODE)
/**
 * Returns the access count for the supplied field.  This function is only
 * available when DEBUG_CODE is defined.
 *
 * @param field the field to query the access count from
 * @return the access count
 */
int cmzn_field_get_access_count(cmzn_field_id field);
#endif /* defined (DEBUG_CODE) */

enum cmzn_field_type cmzn_field_type_enum_from_string(const char *name);

char *cmzn_field_type_enum_to_string(enum cmzn_field_type type);

enum cmzn_field_type cmzn_field_type_enum_from_class_name(const char *name);

char *cmzn_field_type_enum_to_class_name(enum cmzn_field_type type);

enum cmzn_field_type cmzn_field_get_type(cmzn_field_id field);

#endif /* !defined (COMPUTED_FIELD_H) */
