/**
 * FILE : finite_element.h
 *
 * Internal header for finite element data structures and functions.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (FINITE_ELEMENT_H)
#define FINITE_ELEMENT_H

#include "opencmiss/zinc/types/elementid.h"
#include "opencmiss/zinc/types/fieldid.h"
#include "opencmiss/zinc/types/nodeid.h"
#include "finite_element/finite_element_basis.h"
#include "finite_element/finite_element_time.h"
#include "datastore/labels.hpp"
#include "general/change_log.h"
#include "general/enumerator.h"
#include "general/geometry.h"
#include "general/list.h"
#include "general/manager.h"
#include "general/object.h"
#include "general/value.h"

/*
Global types
------------
*/

class DsLabelsGroup;
class DsLabelsChangeLog;

class FE_node_field_template;

/**
 * FE_node has pointer to owning FE_nodeset in shared field info.
 */
class FE_nodeset;

/**
 * FE_element has pointer to owning FE_mesh in shared field info.
 */
class FE_mesh_field_data;
class FE_mesh;

/**
 * FE_field and FE_element haves pointers to owning FE_region in shared field info.
 */
struct FE_region;

enum CM_field_type
/*******************************************************************************
LAST MODIFIED : 30 August 2001

DESCRIPTION :
Information about what the field represents physically.

It is derived from how fields are used in cm, but does not correspond to a
field type in cm or identify fields in cm.

Note: the first value will be 0 by the ANSI standard, with each subsequent entry
incremented by 1. This pattern is expected by the ENUMERATOR macros.  Must
ensure the ENUMERATOR_STRING function returns a string for each value here.
==============================================================================*/
{
	CM_ANATOMICAL_FIELD,
	CM_COORDINATE_FIELD,
	CM_GENERAL_FIELD
}; /* enum CM_field_type */

enum FE_field_type
/*******************************************************************************
LAST MODIFIED : 31 August 1999

DESCRIPTION :
==============================================================================*/
{
	CONSTANT_FE_FIELD, /* fixed values */
	INDEXED_FE_FIELD,  /* indexed set of fixed values */
	GENERAL_FE_FIELD,  /* values held in nodes, elements */
	UNKNOWN_FE_FIELD
}; /* enum FE_field_type */

struct FE_field;
/*******************************************************************************
LAST MODIFIED : 9 October 2002

DESCRIPTION :
Stores the information for calculating the value of a field at a point.  The
position of the point should be specified by giving the element that contains
the point and the Xi coordinates of the point within the element.
==============================================================================*/

DECLARE_LIST_TYPES(FE_field);

DECLARE_CHANGE_LOG_TYPES(FE_field);

/* API uses external type enum cmzn_node_value_label */
enum FE_nodal_value_type
/*******************************************************************************
LAST MODIFIED : 27 January 1998

DESCRIPTION :
The type of a nodal value.
Must add new enumerators and keep values in sync with functions
ENUMERATOR_STRING, ENUMERATOR_GET_VALID_STRINGS and STRING_TO_ENUMERATOR.
Note these functions expect the first enumerator to be number 1, and all
subsequent enumerators to be sequential, unlike the default behaviour which
starts at 0.
==============================================================================*/
{
	FE_NODAL_UNKNOWN = CMZN_NODE_VALUE_LABEL_INVALID,
	FE_NODAL_VALUE = CMZN_NODE_VALUE_LABEL_VALUE,
	FE_NODAL_D_DS1 = CMZN_NODE_VALUE_LABEL_D_DS1,
	FE_NODAL_D_DS2 = CMZN_NODE_VALUE_LABEL_D_DS2,
	FE_NODAL_D2_DS1DS2 = CMZN_NODE_VALUE_LABEL_D2_DS1DS2,
	FE_NODAL_D_DS3 = CMZN_NODE_VALUE_LABEL_D_DS3,
	FE_NODAL_D2_DS1DS3 = CMZN_NODE_VALUE_LABEL_D2_DS1DS3,
	FE_NODAL_D2_DS2DS3 = CMZN_NODE_VALUE_LABEL_D2_DS2DS3,
	FE_NODAL_D3_DS1DS2DS3 = CMZN_NODE_VALUE_LABEL_D3_DS1DS2DS3
}; /* enum FE_nodal_value_type */

/** Convert external node value label to internal finite element value type. */
inline FE_nodal_value_type cmzn_node_value_label_to_FE_nodal_value_type(
	enum cmzn_node_value_label node_value_label)
{
	return static_cast<FE_nodal_value_type>(node_value_label);
}

/** Convert internal finite element value type to external node value label. */
inline enum cmzn_node_value_label FE_nodal_value_type_to_cmzn_node_value_label(
	FE_nodal_value_type nodal_value_type)
{
	return static_cast<cmzn_node_value_label>(nodal_value_type);
}

struct cmzn_node;
#define FE_node cmzn_node

DECLARE_LIST_CONDITIONAL_FUNCTION(FE_node);
DECLARE_LIST_ITERATOR_FUNCTION(FE_node);

struct FE_element_field_values;
/*******************************************************************************
LAST MODIFIED : 9 October 2002

DESCRIPTION :
The values need to calculate a field on an element.  These structures are
calculated from the element field as required and are then destroyed.
==============================================================================*/

DECLARE_LIST_TYPES(FE_element_field_values);

enum FE_element_shape_type
/*******************************************************************************
LAST MODIFIED : 25 February 2003

DESCRIPTION :
The different shape types available.
Note that UNSPECIFIED_SHAPE is only used to establish elements where the
dimension alone is known.
==============================================================================*/
{
	UNSPECIFIED_SHAPE,
	LINE_SHAPE,
	POLYGON_SHAPE,
	SIMPLEX_SHAPE
}; /* enum FE_element_shape_type */

struct FE_element_shape;
/*******************************************************************************
LAST MODIFIED : 9 October 2002

DESCRIPTION :
A description of the shape of an element in Xi space.  It includes how to
calculate face coordinates from element coordinates and how to calculate element
coordinates from face coordinates.
==============================================================================*/

DECLARE_LIST_TYPES(FE_element_shape);

#define FE_element cmzn_element
/*******************************************************************************
LAST MODIFIED : 9 October 2002

DESCRIPTION :
A region in space with functions defined on the region.  The region is
parameterized and the functions are known in terms of the parameterized
variables.
==============================================================================*/

DECLARE_LIST_CONDITIONAL_FUNCTION(FE_element);
DECLARE_LIST_ITERATOR_FUNCTION(FE_element);

PROTOTYPE_ENUMERATOR_FUNCTIONS(cmzn_element_face_type);

PROTOTYPE_ENUMERATOR_FUNCTIONS(cmzn_element_point_sampling_mode);

PROTOTYPE_ENUMERATOR_FUNCTIONS(cmzn_element_quadrature_rule);

typedef int (FE_node_field_iterator_function)(struct FE_node *node, \
	struct FE_field *field,void *user_data);

typedef int (FE_element_field_iterator_function)(struct FE_element *element, \
	struct FE_field *field,void *user_data);

struct Set_FE_field_conditional_data
/*******************************************************************************
LAST MODIFIED : 3 December 2002

DESCRIPTION :
User data structure passed to set_FE_field_conditional, containing the
fe_field_list and the optional conditional_function (and
conditional_function_user_data) for selecting a field out of a subset of the
fields in the list.
==============================================================================*/
{
	LIST_CONDITIONAL_FUNCTION(FE_field) *conditional_function;
	void *conditional_function_user_data;
	struct LIST(FE_field) *fe_field_list;
}; /* struct Set_FE_field_conditional_data */

struct FE_field_order_info;
/*******************************************************************************
LAST MODIFIED : 4 September 2001

DESCRIPTION :
Stores a list of fields in the order they are added.
The contents of this object are private.
==============================================================================*/

PROTOTYPE_ENUMERATOR_FUNCTIONS(cmzn_field_domain_type);

/*
Global functions
----------------
*/

PROTOTYPE_OBJECT_FUNCTIONS(FE_node);
PROTOTYPE_GET_OBJECT_NAME_FUNCTION(FE_node);

/** Define a field at a node. Does not assign values.
  * @param componentTemplates  Pointer to array of node field templates. These
  * are copied for internal use and valuesOffset set appropriately.
  * @param timeSequence  Optional time sequence for time-varying parameters. */
int define_FE_field_at_node(struct FE_node *node,struct FE_field *field,
	const FE_node_field_template *componentTemplatesIn,
	struct FE_time_sequence *timesequence);

/**
 * Removes definition of <field> at <node>. If field is of type GENERAL_FE_FIELD
 * then removes values storage for it and shifts values storage for all subsequent
 * fields down.
 * Note: Must ensure that the node field is not in-use by any elements before it
 * is undefined!
 * @return  Result code OK if undefined CMZN_NOT_FOUND if field undefined or
 * any other error if failed.
 */
int undefine_FE_field_at_node(struct FE_node *node,struct FE_field *field);

int for_FE_field_at_node(struct FE_field *field,
	FE_node_field_iterator_function *iterator,void *user_data,
	struct FE_node *node);
/*******************************************************************************
LAST MODIFIED : 30 October 1998

DESCRIPTION :
If an <iterator> is supplied and the <field> is defined at the <node> then the
result of the <iterator> is returned.  Otherwise, if an <iterator> is not
supplied and the <field> is defined at the <node> then a non-zero is returned.
Otherwise, zero is returned.
???DB.  Multiple behaviour dangerous ?
==============================================================================*/

int for_each_FE_field_at_node(FE_node_field_iterator_function *iterator,
	void *user_data,struct FE_node *node);
/*******************************************************************************
LAST MODIFIED : 30 October 1998

DESCRIPTION :
Calls the <iterator> for each field defined at the <node> until the <iterator>
returns 0 or it runs out of fields.  Returns the result of the last <iterator>
called.
==============================================================================*/

/***************************************************************************//**
 * Calls the <iterator> for each field defined at the <node> in alphabetical
 * order and with indexer fields prioritised so they are defined before the
 * fields indexed by them. Runs until the <iterator> returns 0 or it runs out
 * of fields.  Returns the result of the last <iterator>.
 * Note: expensive
 */
int for_each_FE_field_at_node_alphabetical_indexer_priority(
	FE_node_field_iterator_function *iterator,void *user_data,
	struct FE_node *node);

int FE_node_has_FE_field_values(struct FE_node *node);
/*******************************************************************************
LAST MODIFIED : 24 September 1999

DESCRIPTION :
Returns true if any single field defined at <node> has values stored with
the field.
==============================================================================*/

/**
 * Get the number of elements referencing the supplied node
 */
int FE_node_get_element_usage_count(struct FE_node *node);

/**
 * Returns the FE_nodeset that <node> belongs to.
 */
FE_nodeset *FE_node_get_FE_nodeset(struct FE_node *node);

int FE_node_to_node_string(struct FE_node *node, char **string_address);
/*****************************************************************************
LAST MODIFIED : 20 March 2003

DESCRIPTION :
Returns an allocated <string> of the identifier of <node>.
============================================================================*/

int equivalent_FE_field_at_nodes(struct FE_field *field,struct FE_node *node_1,
	struct FE_node *node_2);
/*******************************************************************************
LAST MODIFIED : 30 October 1998

DESCRIPTION :
Returns true if the <field> is defined in the same way at the two nodes.
==============================================================================*/

int equivalent_FE_fields_at_nodes(struct FE_node *node_1,
	struct FE_node *node_2);
/*******************************************************************************
LAST MODIFIED : 23 May 2000

DESCRIPTION :
Returns true if all fields are defined in the same way at the two nodes.
==============================================================================*/

/** Get node field parameters for valueLabel, version at time.
  * If valueLabel is VALUE all components must be defined, otherwise at least
  * one component, and undefined components are returned with value 0.
  * @param componentNumber  Component number starting at 0, or negative to get
  * all components. 
  * @return  Result OK on full success, WARNING_PART_DONE if only some components
  * have parameters, ERROR_NOT_FOUND if field not defined or none of the requested
  * components have parameters, otherwise any other error code. */
#define PROTOTYPE_GET_FE_NODAL_VALUE_FUNCTION( value_typename, value_enum ) \
int get_FE_nodal_ ## value_typename ## _value(cmzn_node *node, FE_field *field, \
	int componentNumber, cmzn_node_value_label valueLabel, int version, \
	FE_value time, value_typename *valuesOut);

/** Set node field parameters for valueLabel, version at time.
  * If valueLabel is VALUE all components must be defined, otherwise at least
  * one component, and undefined components are returned with value 0.
  * @param componentNumber  Field component number starting at 0, or negative to
  * set all component values.
  * @return  Result OK on full success, WARNING_PART_DONE if only some components
  * have parameters (and which were set), ERROR_NOT_FOUND if field not defined
  * or none of the requested components have parameters, otherwise any other error code. */
#define PROTOTYPE_SET_FE_NODAL_VALUE_FUNCTION( value_typename, value_enum ) \
int set_FE_nodal_ ## value_typename ## _value(cmzn_node *node,  FE_field *field, \
	int componentNumber, cmzn_node_value_label valueLabel, int version, \
	FE_value time, const value_typename *valuesIn);

/** Get address of node field parameter for valueLabel, version at time.
  * @param componentNumber  Field component number starting at 0.
  * @param valueAddress  On success, address of node parameter. Will be invalid
  * if the node is modified so it should only be used temporarily.
  * @return  1 on success, otherwise 0. */
#define PROTOTYPE_GET_FE_NODAL_VALUE_STORAGE_FUNCTION( value_typename, value_enum ) \
int get_FE_nodal_ ## value_typename ## _storage(cmzn_node *node, FE_field *field, \
	int componentNumber, cmzn_node_value_label valueLabel, int version, \
	FE_value time, value_typename **valueAddress);

#define PROTOTYPE_FE_NODAL_VALUE_FUNCTIONS( value_type , value_enum ) \
PROTOTYPE_GET_FE_NODAL_VALUE_FUNCTION(value_type,value_enum) \
PROTOTYPE_SET_FE_NODAL_VALUE_FUNCTION(value_type,value_enum) \
PROTOTYPE_GET_FE_NODAL_VALUE_STORAGE_FUNCTION(value_type,value_enum)

PROTOTYPE_FE_NODAL_VALUE_FUNCTIONS(FE_value, FE_VALUE_VALUE)
PROTOTYPE_FE_NODAL_VALUE_FUNCTIONS(double, DOUBLE_VALUE)
PROTOTYPE_FE_NODAL_VALUE_FUNCTIONS(float, FLOAT_VALUE)
PROTOTYPE_FE_NODAL_VALUE_FUNCTIONS(int, INT_VALUE)
PROTOTYPE_FE_NODAL_VALUE_FUNCTIONS(short, SHORT_VALUE)

/**
 * Gets an element:xi value stored at a node.
 * Note doesn't support value labels and versions.
 */
int get_FE_nodal_element_xi_value(struct FE_node *node,
	FE_field *field, int component_number,
	cmzn_element **element, FE_value *xi);

/**
 * Sets a particular element_xi_value (<version>, <type>) for the field
 * <component> at the <node>.
 */
int set_FE_nodal_element_xi_value(struct FE_node *node,
	FE_field *field, int component_number,
	cmzn_element *element, const FE_value *xi);

/** Returns a copy of the string for field component at the <node. 
  * Up to the calling function to DEALLOCATE the returned string.
  * @param stringOut  On success, set to allocated string value, or 0 if no
  * string. */
int get_FE_nodal_string_value(struct FE_node *node,
	FE_field *field, int componentNumber, char **stringOut);

/** Copies and sets the string value of field component at node.
  * @param stringIn  May be NULL. */
int set_FE_nodal_string_value(struct FE_node *node,
	FE_field *field, int componentNumber, const char *stringIn);

/** Get as a string the node field component with valueLabel, version at time.
  * @return  Allocated string, or 0 if faile. */
char *get_FE_nodal_value_as_string(struct FE_node *node, FE_field *field,
	int componentNumber, cmzn_node_value_label valueLabel, int version,
	FE_value time);

int FE_node_is_in_Multi_range(struct FE_node *node,void *multi_range_void);
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Conditional function returning true if <node> identifier is in the
<multi_range>.
==============================================================================*/

int FE_node_is_not_in_Multi_range(struct FE_node *node,void *multi_range_void);
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Conditional function returning true if <node> identifier is NOT in the
<multi_range>.
==============================================================================*/

int add_FE_node_number_to_Multi_range(struct FE_node *node,
	void *multi_range_void);
/*******************************************************************************
LAST MODIFIED : 20 February 2000

DESCRIPTION :
Iterator function for adding the number of <node> to <multi_range>.
==============================================================================*/

/**
 * Clears any embedded locations from nodes in nodeset for fields in
 * field_list. This is to avoid circular dependencies which prevent clean-up.
 * Note to handle merging from a separate region where both the source and
 * target region reference the same nodes, only embedded locations in nodes
 * belonging to the supplied nodeset are cleared.
 */
int FE_nodeset_clear_embedded_locations(FE_nodeset *nodeset,
	struct LIST(FE_field) *field_list);

/**
 * @return  true if definition of fields in source node are compatible with
 * target node.
 */
bool FE_node_can_merge(struct FE_node *targetNode, struct FE_node *sourceNode);

PROTOTYPE_ENUMERATOR_FUNCTIONS(FE_nodal_value_type);

/**
 * Get all parameters for field component at node and time.
 * Parameter order cycles slowest for derivative / value label, with versions
 * for a given value label consecutive.
 *
 * @param node  The node which stores the field values.
 * @param field  The fields whose values are to be returned from.
 * @param componentNumber  The component number starting at 0.
 * @param time  The time to get values at. If node field is not time-varying,
 * this argument is ignored. If time varying, parameters are interpolated to
 * in-between times, or if out of range the parameters at minimum or maximum
 * time are returned.
 * @param valuesCount  The size of the values array. Must be at least as big
 * as number of values for the component.
 * @param valuesOut  Array to receive node field values.
 * @return  Result OK on success, any other value on failure.
 */
int cmzn_node_get_field_component_FE_value_values(cmzn_node *node,
	FE_field *field, int componentNumber, FE_value time, int valuesCount,
	FE_value *valuesOut);

/**
 * Set all parameters for field component at node and time.
 * Parameter order cycles slowest for derivative / value label, with versions
 * for a given value label consecutive.
 *
 * @param node  The node which stores the field values.
 * @param field  The fields whose values are to be set.
 * @param componentNumber  The component number starting at 0.
 * @param time  The time to set values at, ignored for non-time-varying field.
 * For a time-varying field, must exactly equal a time at which parameters are
 * stored.
 * @param valuesCount  The size of the values array. Must match number of
 * values for the component.
 * @param valuesIn  The array of values to set.
 * @return  Result OK on success, any other value on failure.
 */
int cmzn_node_set_field_component_FE_value_values(cmzn_node *node,
	FE_field *field, int componentNumber, FE_value time, int valuesCount,
	const FE_value *valuesIn);

/**
 * Get all parameters for field component at node and time. Integer variant.
 * Parameter order cycles slowest for derivative / value label, with versions
 * for a given value label consecutive.
 *
 * @param node  The node which stores the field values.
 * @param field  The fields whose values are to be returned from.
 * @param componentNumber  The component number starting at 0.
 * @param time  The time to get values at. If node field is not time-varying,
 * this argument is ignored. If time varying, parameters are interpolated to
 * in-between times, or if out of range the parameters at minimum or maximum
 * time are returned.
 * @param valuesCount  The size of the values array. Must be at least as big
 * as number of values for the component.
 * @param valuesOut  Array to receive node field values.
 * @return  Result OK on success, any other value on failure.
 */
int cmzn_node_get_field_component_int_values(cmzn_node *node,
	FE_field *field, int componentNumber, FE_value time, int valuesCount,
	int *valuesOut);

/**
 * Set all parameters for field component at node and time. Integer variant.
 * Parameter order cycles slowest for derivative / value label, with versions
 * for a given value label consecutive.
 *
 * @param node  The node which stores the field values.
 * @param field  The fields whose values are to be set.
 * @param componentNumber  The component number starting at 0.
 * @param time  The time to set values at, ignored for non-time-varying field.
 * For a time-varying field, must exactly equal a time at which parameters are
 * stored.
 * @param valuesCount  The size of the values array. Must match number of
 * values for the component.
 * @param valuesIn  The array of values to set.
 * @return  Result OK on success, any other value on failure.
 */
int cmzn_node_set_field_component_int_values(cmzn_node *node,
	FE_field *field, int componentNumber, FE_value time, int valuesCount,
	const int *valuesIn);

/**
 * Assigns all parameters for the field at the node, taken from sparse arrays.
 * Works around current limitations of node fields (that they must have VALUE
 * parameters for all components, and the same number of versions for all
 * values/derivatives in a component). Values not found in the sparse 
 * Note this is not time-aware yet.
 * @param field  The field whose parameters are to be assigned.
 * @param node  The node at which the parameters are stored.
 * @param arraySize  The full size of the values and valueExists arrays, equal
 * to componentsSize*derivativesSize*versionsSize
 * @param values  The parameters to set.
 * @param valueExists  Sparsity array; 1 if value set, 0 if not.
 * @param valuesCount  The number of valid parameters in the sparse array.
 * @param componentsSize  Number of components.
 * @param componentsOffset  Offset in array to next component for same
 * derivative and version.
 * @param derivativesSize  Number of derivatives. Derivatives are indexed in
 * order of FE_nodal_value_type, index 0 = VALUE.
 * @param derivativesOffset  Offset in array to next derivative for same
 * component and version.
 * @param versionsSize  Number of versions.
 * @param versionsOffset  Offset in array to next version for same
 * component and derivative.
 */
int FE_field_assign_node_parameters_sparse_FE_value(FE_field *field, FE_node *node,
	int arraySize, const FE_value *values, const int *valueExists, int valuesCount,
	int componentsSize, int componentsOffset,
	int derivativesSize, int derivativesOffset,
	int versionsSize, int versionsOffset);

int get_FE_node_number_of_values(struct FE_node *node);
/*******************************************************************************
LAST MODIFIED : 29 October 1998

DESCRIPTION :
Returns the number of values stored at the <node>.
==============================================================================*/

/**
 * @return  The non-negative node identifier, otherwise
 * DS_LABEL_IDENTIFIER_INVALID on error or if node is orphaned from nodeset.
 */
int get_FE_node_identifier(struct FE_node *node);

/**
 * @return  The non-negative node index within the owning nodeset otherwise
 * DS_LABEL_INDEX_INVALID on error or if node is orphaned from nodeset.
 */
DsLabelIndex get_FE_node_index(struct FE_node *node);

/**
 * Set the index of the node in owning nodeset. Used only by FE_nodeset when
 * merging nodes from another region's nodeset.
 * @param node  The node to modify.
 * @param index  The new index, non-negative. Value is not checked due
 * to use by privileged caller.
 */
void set_FE_node_index(struct FE_node *node, DsLabelIndex index);

/***************************************************************************//**
 * Writes to the console the node identifier and details of the fields and
 * parameters defined there.
 */
int list_FE_node(struct FE_node *node);

/***************************************************************************//**
 * Internal variant of public cmzn_nodeiterator_next() which does not access
 * the returned node, for more efficient if less safe usage.
 *
 * @param node_iterator  Node iterator to query and advance.
 * @return  Non-accessed pointer to the next node, or NULL if none remaining.
 */
cmzn_node_id cmzn_nodeiterator_next_non_access(
	cmzn_nodeiterator_id node_iterator);

int calculate_grid_field_offsets(int element_dimension,
	int top_level_element_dimension, const int *top_level_number_in_xi,
	FE_value *element_to_top_level,int *number_in_xi,int *base_grid_offset,
	int *grid_offset_in_xi);
/*******************************************************************************
LAST MODIFIED : 13 June 2000

DESCRIPTION :
Calculates the factors for converting a grid position on a element of
<element_dimension> to a top_level_element of <top_level_element_dimension>
with <top_level_number_in_xi>, given affine transformation
<element_to_top_level> which has as many rows as <top_level_element_dimension>
and 1 more column than <element_dimension>, converting xi from element to
top_level as follows:
top_level_xi = b + A xi, with b the first column.
The <number_in_xi> of the element is returned, as is the <base_grid_offset> and
the <grid_offset_in_xi> which make up the grid point number conversion:
eg. top_level_grid_point_number = base_grid_offset +
grid_offset_in_xi[i]*grid_number_in_xi[i] (i summed over element_dimension).
Sets values appropriately if element_dimension = top_level_element_dimension.
==============================================================================*/

struct FE_element_field_values *CREATE(FE_element_field_values)(void);
/*******************************************************************************
LAST MODIFIED : 4 March 2003

DESCRIPTION :
Creates a blank struct FE_element_field_values which can be passed to
calculate_FE_element_field_values. The structure can be restored to its
blank state by calling clear_FE_element_field_values; this should be done
before calling calculate_FE_element_field_values again, and if leaving the
structure unused for some time so it is not accessing objects.
==============================================================================*/

/**
* Fill the element_field_values structure; must have already been created.
* @param topLevelElement  Optional element to inherit field from.
*/
int calculate_FE_element_field_values(cmzn_element *element,
	struct FE_field *field, FE_value time, char calculate_derivatives,
	struct FE_element_field_values *element_field_values,
	cmzn_element *topLevelElement);

int FE_element_field_values_differentiate(
	struct FE_element_field_values *element_field_values, int xi_index);
/*******************************************************************************
LAST MODIFIED : 9 May 2007

DESCRIPTION :
Modifies the calculated values for an FE_field so that it will calculate
derivatives wrt xi_index for the original field.  The <element_field_values>
must have already been calculated.  Currently only implemented for monomials.
==============================================================================*/

int clear_FE_element_field_values(
	struct FE_element_field_values *element_field_values);
/*******************************************************************************
LAST MODIFIED : 4 March 2003

DESCRIPTION :
Frees the memory for the fields of the <element_field_values> structure.
Restores <element_field_values> to the blank state it was created with. This
function must be called before calling calculate_FE_element_field_values again.
==============================================================================*/

int DESTROY(FE_element_field_values)(
	struct FE_element_field_values **element_field_values_address);
/*******************************************************************************
LAST MODIFIED : 4 March 2003

DESCRIPTION :
Destroys the element_field_values at *<element_field_info_address>. Frees the
memory for the information and sets <*element_field_info_address> to NULL.
==============================================================================*/

PROTOTYPE_LIST_FUNCTIONS(FE_element_field_values);

PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(FE_element_field_values,element,struct FE_element *);

int FE_element_field_values_get_component_values(
	struct FE_element_field_values *element_field_values, int component_number,
	int *number_of_component_values_address, FE_value **component_values_address);
/*******************************************************************************
LAST MODIFIED : 4 March 2003

DESCRIPTION :
Allocates and returns to <component_values_address> the component values for
<component_number> in <element_field_values>. The number of values is returned
in <number_of_component_values>.
It is up to the calling function to deallocate any returned component values.
==============================================================================*/

int FE_element_field_values_get_monomial_component_info(
	struct FE_element_field_values *element_field_values, int component_number,
	int *monomial_info);
/*******************************************************************************
LAST MODIFIED : 4 March 2003

DESCRIPTION :
If <component_number> in the <element_field_values> is monomial, integer values
describing the monomial basis are returned. The first number is the dimension,
the following numbers are the order of the monomial in each direction, where
3=cubic, for example.
<monomial_info> should point to a block of memory big enough to take
1 + MAXIMUM_ELEMENT_XI_DIMENSIONS integers.
==============================================================================*/

int FE_element_field_values_are_for_element_and_time(
	struct FE_element_field_values *element_field_values,
	struct FE_element *element,FE_value time,struct FE_element *field_element);
/*******************************************************************************
LAST MODIFIED : 3 December 2001

DESCRIPTION :
Returns true if the <element_field_values> are valid for time <time> and
originated from <element>, either directly or inherited from <field_element>.
If <field_element> is NULL no match is required with the field_element in the
<element_field_values>.
==============================================================================*/

int FE_element_field_values_have_derivatives_calculated(
	struct FE_element_field_values *element_field_values);
/*******************************************************************************
LAST MODIFIED : 10 March 2003

DESCRIPTION :
Returns true if the <element_field_values> are valid for calculating
derivatives.
==============================================================================*/

/**
 * The function allocates an array, <*element_field_nodes_array_address> to store the
 * pointers to the ACCESS'd element nodes.  Components that are not node-based are
 * ignored.  The element nodes are ordered by increasing xi (fastest in xi1, next
 * fastest in xi2 and so on).
 * Beware: for general parameter maps, only returns first node affecting a given
 * element parameter. Also, in general this information is insufficient to determine
 * a common face since it may differ by versions or general maps, but it is
 * currently used for this purpose.
 * The optional <top_level_element> forces inheritance from it as needed.
 * NB.  The nodes need to be DEACCESS'd before the nodes array is DEALLOCATE'd.
 * @param face_number  If non-negative, calculate nodes for face number of
 * element, as if the face element were supplied to this function.
 * @param field  The field to calculate nodes for, or pass NULL to calculate
 * nodes for the first coordinate field defined at element, provided it can
 * be inherited from the optional top_level_element.
 */
int calculate_FE_element_field_nodes(struct FE_element *element,
	int face_number, struct FE_field *field,
	int *number_of_element_field_nodes_address,
	struct FE_node ***element_field_nodes_array_address,
	struct FE_element *top_level_element);

int calculate_FE_element_field(int component_number,
	struct FE_element_field_values *element_field_values,
	const FE_value *xi_coordinates, FE_value *values, FE_value *jacobian);
/*******************************************************************************
LAST MODIFIED : 2 October 1998

DESCRIPTION :
Calculates the <values> of the field specified by the <element_field_values> at
the <xi_coordinates>.  The storage for the <values> should have been allocated
outside the function.  The <jacobian> will be calculated if it is not NULL (and
the derivatives values have been calculated).  Only the <component_number>+1
component will be calculated if 0<=component_number<number of components.  For a
single component, the value will be put in the first position of <values> and
the derivatives will start at the first position of <jacobian>.
==============================================================================*/

int calculate_FE_element_field_as_string(int component_number,
	struct FE_element_field_values *element_field_values,
	const FE_value *xi_coordinates, char **string);
/*******************************************************************************
LAST MODIFIED : 17 October 1999

DESCRIPTION :
Calculates the values of element field specified by the <element_field_values>
at the <xi_coordinates> and returns them as the allocated <string>. Only the
<component_number>+1 component will be calculated if
0<=component_number<number of components. If more than 1 component is calculated
then values are comma separated. Derivatives are not included in the string,
even if calculated for the <element_field_values>.
It is up to the calling function to DEALLOCATE the returned string.
==============================================================================*/

int calculate_FE_element_field_int_values(int component_number,
	struct FE_element_field_values *element_field_values,
	const FE_value *xi_coordinates, int *values);
/*******************************************************************************
LAST MODIFIED : 14 October 1999

DESCRIPTION :
Calculates the <values> of the integer field specified by the
<element_field_values> at the <xi_coordinates>. The storage for the <values>
should have been allocated outside the function. Only the <component_number>+1
component will be calculated if 0<=component_number<number of components. For a
single component, the value will be put in the first position of <values>.
==============================================================================*/

int calculate_FE_element_field_string_values(int component_number,
	struct FE_element_field_values *element_field_values,
	const FE_value *xi_coordinates, char **values);
/*******************************************************************************
LAST MODIFIED : 19 October 1999

DESCRIPTION :
Returns allocated copies of the string values of the field specified by the
<element_field_values> at the <xi_coordinates>. <values> must be allocated with
enough space for the number_of_components strings, but the strings themselves
are allocated here. Only the <component_number>+1 component will be calculated
if 0<=component_number<number of components. For a single component, the value
will be put in the first position of <values>.
It is up to the calling function to deallocate the returned string values.
==============================================================================*/

struct FE_element_shape *CREATE(FE_element_shape)(int dimension,
	const int *type, struct FE_region *fe_region);
/*******************************************************************************
LAST MODIFIED : 8 July 2003

DESCRIPTION :
Requests from the <fe_region> for a shape with the specified <dimension> and
<type>.  If one is not found, a shape is created (with <type> duplicated) and
added to the region.  The shape is returned.
<type> is analogous to the basis type array, except that the entries are 0 or 1.
If <type> is omitted an "unspecified" shape of the given <dimension> is
returned. An element with such a shape may not have fields defined on it until
it is given a proper shape.
==============================================================================*/

int DESTROY(FE_element_shape)(struct FE_element_shape **element_shape_address);
/*******************************************************************************
LAST MODIFIED : 23 September 1995

DESCRIPTION :
Remove the shape from the list of all shapes.  Free the memory for the shape and
sets <*element_shape_address> to NULL.
==============================================================================*/

PROTOTYPE_OBJECT_FUNCTIONS(FE_element_shape);

PROTOTYPE_LIST_FUNCTIONS(FE_element_shape);

/***************************************************************************//**
 * Creates an element shape object given just a cmzn_element_shape_type.
 *
 * @return  Accessed shape object or NULL on error.
 */
struct FE_element_shape *FE_element_shape_create_simple_type(
	struct FE_region *fe_region, enum cmzn_element_shape_type shape_type);

/**
 * Returns the number of faces of the element shape.
 */
int FE_element_shape_get_number_of_faces(const FE_element_shape *element_shape);

/***************************************************************************//**
 * Returns a cmzn_element_shape_type describing the shape if possible.
 *
 * @param element_shape   The shape object to query.
 * @return  The shape type, or unknown if not able to be described by enum.
 */
enum cmzn_element_shape_type FE_element_shape_get_simple_type(
	struct FE_element_shape *element_shape);

/***************************************************************************//**
 * Returns an allocated string with the EX file description of the shape, e.g.
 * - line*line*line
 * - simplex(3)*line*simplex
 * - simplex(2;3)*simplex*simplex
 * - polygon(5;2)*polygon*line
 */
char *FE_element_shape_get_EX_description(struct FE_element_shape *element_shape);

/***************************************************************************//**
 * Creates an unspecified element shape of the supplied dimension.
 *
 * @return  Accessed shape object or NULL on error.
 */
struct FE_element_shape *FE_element_shape_create_unspecified(
	struct FE_region *fe_region, int dimension);

int FE_element_shape_is_line(struct FE_element_shape *element_shape);
/*******************************************************************************
LAST MODIFIED : 12 March 2003

DESCRIPTION :
Returns true if the <element_shape> has only LINE_SHAPE in each dimension.
==============================================================================*/

struct FE_element_shape *get_FE_element_shape_of_face(
	const FE_element_shape *shape,int face_number, struct FE_region *fe_region);
/*******************************************************************************
LAST MODIFIED : 7 July 2003

DESCRIPTION :
From the parent <shape> returns the FE_element_shape for its face <face_number>.
The <shape> must be of dimension 2 or 3. Faces of 2-D elements are always lines.
==============================================================================*/

/**
 * @return  The dimension of the element shape, or 0 if error.
 */
int get_FE_element_shape_dimension(struct FE_element_shape *element_shape);

/**
 * Return face_to_element map, a square matrix b + A xi for the face_number of
 * shape.
 *
 * @return  Address of internal face_to_element map, or 0 if invalid arguments.
 * Note: do not deallocate!
 */
const FE_value *get_FE_element_shape_face_to_element(
	struct FE_element_shape *element_shape, int face_number);

int get_FE_element_shape_xi_linkage_number(
	struct FE_element_shape *element_shape, int xi_number1, int xi_number2,
	int *xi_linkage_number_address);
/*******************************************************************************
LAST MODIFIED : 13 March 2003

DESCRIPTION :
Returns a number indicating how the dimension <xi_number1> and <xi_number2> are
linked in <element_shape>.
If they are linked in a simplex, a non-zero return indicates they are linked.
If they are linked in a polygon, the returned number is the number of sides in
the polygon.
A value of zero indicates the dimensions are not linked.
Note the first xi_number is 0.
==============================================================================*/

int get_FE_element_shape_xi_shape_type(struct FE_element_shape *element_shape,
	int xi_number, enum FE_element_shape_type *shape_type_address);
/*******************************************************************************
LAST MODIFIED : 5 November 2002

DESCRIPTION :
Returns the shape type of <element_shape> on <xi_number> -- on main diagonal of
type array. The first xi_number is 0.
==============================================================================*/

int get_FE_element_shape_next_linked_xi_number(
	struct FE_element_shape *element_shape, int xi_number,
	int *next_xi_number_address, int *xi_link_number_address);
/*******************************************************************************
LAST MODIFIED : 6 November 2002

DESCRIPTION :
Returns in <next_xi_number_address> the next xi number higher than <xi_number>
which is linked in shape with it, plus in <xi_link_number_address> the number
denoting how it is linked; currently used only for polygon shapes to denote the
number of polygon sides.
If there is no remaining linked dimension, 0 is returned in both addresses.
<xi_number> is from 0 to one less than the shape dimension.
Also checks that the linked xi numbers have the same shape type.
==============================================================================*/

int FE_element_shape_limit_xi_to_element(struct FE_element_shape *shape,
	FE_value *xi, FE_value tolerance);
/*******************************************************************************
LAST MODIFIED : 12 March 2003

DESCRIPTION :
Checks that the <xi> location is valid for elements with <shape>.
The <tolerance> allows the location to go slightly outside.  If the values for
<xi> location are further than <tolerance> outside the element then the values
are modified to put it on the nearest face.
==============================================================================*/

/**
 * Adds the <increment> to <xi>.  If this moves <xi> outside of the shape, then
 * the step is limited to take <xi> to the boundary, <face_number> is set to be
 * the limiting face, <fraction> is updated with the fraction of the <increment>
 * actually used, the <increment> is updated to contain the part not used,
 * the <xi_face> are calculated for that face and the <xi> are changed to be
 * on the boundary of the shape.
 */
int FE_element_shape_xi_increment(struct FE_element_shape *shape,
	FE_value *xi,FE_value *increment, FE_value *step_size,
	int *face_number_address, FE_value *xi_face);

PROTOTYPE_OBJECT_FUNCTIONS(FE_element);

/**
 * Returns the list of <adjacent_elements> not including <element> which share
 * the face indicated by <face_number>.
 * Limitation: does not handle element being connected to itself.
 * On success, caller must DEALLOCATE element array.
 * @return  CMZN_OK on success, any error on failure.
 */
int adjacent_FE_element(struct FE_element *element,
	int face_number, int *number_of_adjacent_elements,
	struct FE_element ***adjacent_elements);

/**
 * Returns true if <field> is equivalently listed in the field information for
 * <element_1> and <element_2>. If neither element has field information or if they
 * do but the field is not defined in either, this is also equivalent.
 */
bool equivalent_FE_field_in_elements(struct FE_field *field,
	struct FE_element *element_1, struct FE_element *element_2);

/**
 * @return  True if all fields are defined in the same way at the two elements.
 * A prerequisite is that they must be valid elements on the same mesh.
 */
bool equivalent_FE_fields_in_elements(struct FE_element *element_1,
	struct FE_element *element_2);

/**
 * @return  The dimension of the element, or 0 if it cannot be determined.
 * @deprecated  Use element->getDimension() instead.
 */
int get_FE_element_dimension(struct FE_element *element);

/**
 * @return  The non-negative element identifier, otherwise
 * DS_LABEL_IDENTIFIER_INVALID on error or if not a true element in the mesh.
 * @deprecated  Use element->getIdentifier() instead.
 */
DsLabelIdentifier get_FE_element_identifier(struct FE_element *element);

/**
 * @return  The non-negative element index within the mesh otherwise
 * DS_LABEL_INDEX_INVALID on error or if not a true element in the mesh.
 * @deprecated  Use element->getIndex() instead.
 */
DsLabelIndex get_FE_element_index(struct FE_element *element);

/** @return  First coordinate field defined on element or inherited from
  * parent. Note inheritence is in same order as FE_field::getOrInheritOnElement */
struct FE_field *FE_element_get_default_coordinate_field(struct FE_element *element);

/** @return  The number of fields defined at <element>.
  * Does not include fields inherited from parent elements. */
int get_FE_element_number_of_fields(struct FE_element *element);

/**
 * Returns the <shape> of the <element>, if any. Invalid elements (in process
 * of being constructed or destroyed, or orphaned) have no shape.
 * @deprecated Use element->getElementShape()
 */
FE_element_shape *get_FE_element_shape(struct FE_element *element);

/**
 * Returns the <face_element> for face <face_number> of <element>, where NULL
 * means there is no face. Element must have a shape and face.
 * Use is discouraged; add new element/mesh API if needed.
 */
struct FE_element *get_FE_element_face(struct FE_element *element, int face_number);

/***************************************************************************//**
 * Calls the <iterator> for each field defined at the <element> in alphabetical
 * order and with indexer fields prioritised so they are defined before the
 * fields indexed by them. Runs until the <iterator> returns 0 or it runs out
 * of fields.  Returns the result of the last <iterator>.
 * Note: expensive
 */
int for_each_FE_field_at_element_alphabetical_indexer_priority(
	FE_element_field_iterator_function *iterator,void *user_data,
	struct FE_element *element);

/***************************************************************************//**
 * Conditional function returning true if <element> number is in the
 * multi range.
 * @param multi_range_void  A struct Multi_range *.
 */
int FE_element_number_is_in_Multi_range(struct FE_element *element,
	void *multi_range_void);

/***************************************************************************//**
 * Iterator function which adds the element number into the multi range.
 * @param multi_range_void  A struct Multi_range *.
 */
int FE_element_add_number_to_Multi_range(
	struct FE_element *element, void *multi_range_void);

/***************************************************************************//**
 * Internal variant of public cmzn_elementiterator_next() which does not
 * access the returned element, for more efficient if less safe usage.
 *
 * @param element_iterator  Element iterator to query and advance.
 * @return  Non-accessed pointer to the next element, or NULL if none remaining.
 */
cmzn_element_id cmzn_elementiterator_next_non_access(
	cmzn_elementiterator_id element_iterator);

PROTOTYPE_ENUMERATOR_FUNCTIONS(CM_field_type);

struct FE_field *CREATE(FE_field)(const char *name, struct FE_region *fe_region);
/*******************************************************************************
LAST MODIFIED : 27 February 2003

DESCRIPTION :
Creates and returns a struct FE_field of <name> belonging to the ultimate
master FE_region of <fe_region>. The new field has no name/identifier, zero
components, field_type FIELD, NOT_APPLICABLE coordinate system, no field values.
???RC Used to pass <fe_time> in here and store in FE_field; can now get it from
FE_region.
==============================================================================*/

int DESTROY(FE_field)(struct FE_field **field_address);
/*******************************************************************************
LAST MODIFIED : 23 September 1995

DESCRIPTION :
Frees the memory for <**field_address> and sets <*field_address> to NULL.
==============================================================================*/

PROTOTYPE_OBJECT_FUNCTIONS(FE_field);

PROTOTYPE_GET_OBJECT_NAME_FUNCTION(FE_field);

PROTOTYPE_LIST_FUNCTIONS(FE_field);

PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(FE_field,name,const char *);

PROTOTYPE_CHANGE_LOG_FUNCTIONS(FE_field);

/**
 * Copies the field definition from source to destination, except for name,
 * mesh field data and node definition. Assumes source field has been
 * proven compatible with destination including that source indexer field has
 * already been merged.
 * If source and destination fields belong to different FE_regions, substitute
 * local indexer_field and element_xi_host_mesh if appropriate.
 */
int FE_field_copy_without_identifier(struct FE_field *destination,
	struct FE_field *source);

/**
 * Returns true if <field1> and <field2> have the same fundamental definition,
 * namely they:
 * 1. Have the same value type
 * 2. Have the same fe field type (general, indexed etc.)
 * 3. Have the same number of components
 * 4. Have the same coordinate system
 * If so, they can be merged without affecting the rest of the model.
 * Other attributes such as cm field type (field, coordinate, anatomical) are
 * not considered fundamental. The name is also not compared.
 * Must ensure this function fits with FE_fields_match_exact.
 */
bool FE_fields_match_fundamental(struct FE_field *field1,
	struct FE_field *field2);

/**
 * Returns true if <field1> and <field2> have exactly the same definition,
 * comparing all attributes.
 * @see FE_fields_match_fundamental
 */
bool FE_fields_match_exact(struct FE_field *field1, struct FE_field *field2);

/**
 * List iterator function which fetches a field with the same name as <field>
 * from <field_list>. Returns 1 (true) if there is either no such field in the
 * list or the two fields return true for FE_fields_match_fundamental(),
 * otherwise returns 0 (false).
 */
int FE_field_can_be_merged_into_list(struct FE_field *field, void *field_list_void);

int FE_field_has_multiple_times(struct FE_field *fe_field);
/*******************************************************************************
LAST MODIFIED : 26 February 2003

DESCRIPTION :
Returns true if any node_fields corresponding to <field> have time_seqences.
This will be improved when regionalised, so that hopefully the node field
list we will be looking at will not be global but will belong to the region.
==============================================================================*/

/**
 * Return true if any basis functions used by the field is non-linear i.e.
 * quadratic, cubic, Fourier etc.
 */
bool FE_field_uses_non_linear_basis(struct FE_field *fe_field);

struct FE_field *find_first_time_field_at_FE_node(struct FE_node *node);
/*******************************************************************************
LAST MODIFIED : 9 June 1999

Find the first time based field at a node
==============================================================================*/

int ensure_FE_field_is_in_list(struct FE_field *field, void *field_list_void);
/*******************************************************************************
LAST MODIFIED : 29 March 2006

DESCRIPTION :
Iterator function for adding <field> to <field_list> if not currently in it.
==============================================================================*/

struct FE_region *FE_field_get_FE_region(struct FE_field *fe_field);
/*******************************************************************************
LAST MODIFIED : 26 February 2003

DESCRIPTION :
Returns the FE_region that <fe_field> belongs to.
==============================================================================*/

/*******************************************************************************
 * Records that a Computed_field_finite_element is wrapping this FE_field.
 * @return  The number of wrappers existing.
 */
int FE_field_add_wrapper(struct FE_field *field);

/*******************************************************************************
 * Records that a Computed_field_finite_element is no longer wrapping this
 * FE_field.
 * @return  The number of wrappers remaining.
 */
int FE_field_remove_wrapper(struct FE_field *field);

/***************************************************************************//**
 * @return  number of objects using fe_field.
 */
int FE_field_get_access_count(struct FE_field *fe_field);

/***************************************************************************//**
 * @return  number of objects using fe_node.
 */
int FE_node_get_access_count(struct FE_node *fe_node);

char *get_FE_field_component_name(struct FE_field *field,int component_no);
/*******************************************************************************
LAST MODIFIED : 10 May 2000

DESCRIPTION :
Returns the name of component <component_no> of <field>. If no name is stored
for the component, a string comprising the value component_no+1 is returned.
Up to calling function to DEALLOCATE the returned string.
==============================================================================*/

int set_FE_field_component_name(struct FE_field *field,int component_no,
	const char *component_name);
/*******************************************************************************
LAST MODIFIED : 10 May 2000

DESCRIPTION :
Sets the name of component <component_no> of <field>. Only sets name if it is
different from that already returned for field to preserve default names if can.
==============================================================================*/

struct Coordinate_system *get_FE_field_coordinate_system(
	struct FE_field *field);
/*******************************************************************************
LAST MODIFIED : 22 January 1999

DESCRIPTION :
Returns the coordinate system for the <field>.
==============================================================================*/

int set_FE_field_coordinate_system(struct FE_field *field,
	struct Coordinate_system *coordinate_system);
/*******************************************************************************
LAST MODIFIED : 28 January 1999

DESCRIPTION :
Sets the coordinate system of the <field>.
==============================================================================*/

/** Ensure field is not defined on mesh
  * Assumes called with FE_region change caching on; records change but doesn't notify */
void FE_field_clearMeshFieldData(struct FE_field *field, FE_mesh *mesh);

/**
 * Note: must get no field data from FE_field_getMeshFieldData first!
 * @return  New mesh field data defining field over the given mesh, or 0 if failed.
 * @param field  The field to create data for.
 * @param mesh  The mesh to create data for. Must be from same region as field.
 */
FE_mesh_field_data *FE_field_createMeshFieldData(struct FE_field *field,
	FE_mesh *mesh);

/**
 * @return  Mesh field data defining field over the given mesh, or 0 if none.
 * @param field  The field to create data for.
 * @param mesh  The mesh to create data for. Must be from same region as field.
 */
FE_mesh_field_data *FE_field_getMeshFieldData(struct FE_field *field,
	const FE_mesh *mesh);

int get_FE_field_number_of_components(struct FE_field *field);
/*******************************************************************************
LAST MODIFIED : 16 November 1998

DESCRIPTION :
Returns the number of components for the <field>.
==============================================================================*/

int set_FE_field_number_of_components(struct FE_field *field,
	int number_of_components);
/*******************************************************************************
LAST MODIFIED : 1 September 1999

DESCRIPTION :
Sets the number of components in the <field>. Automatically assumes names for
any new components. Clears/reallocates the values_storage for FE_field_types
that use them, eg. CONSTANT_FE_FIELD and INDEXED_FE_FIELD - but only if number
of components changes. If function fails the field is left exactly as it was.
Should only call this function for unmanaged fields.
ELEMENT_XI_VALUE, STRING_VALUE and URL_VALUE fields may only have 1 component.
==============================================================================*/

int get_FE_field_number_of_values(struct FE_field *field);
/*******************************************************************************
LAST MODIFIED : 16 November 1998

DESCRIPTION :
Returns the number of global values for the <field>.
==============================================================================*/

int get_FE_field_number_of_times(struct FE_field *field);
/*******************************************************************************
LAST MODIFIED : 9 June 1999

DESCRIPTION :
Returns the number of global times for the <field>.
==============================================================================*/

int set_FE_field_number_of_times(struct FE_field *field,
	int number_of_times);
/*******************************************************************************
LAST MODIFIED : 9 June 1999

DESCRIPTION :
Sets the number of times stored with the <field>
REALLOCATES the requires memory in field->value_storage, based upon the
field->time_value_type.

For non-array types, the contents of field->times_storage is:
   | data type (eg FE_value) | x number_of_times

For array types, the contents of field->times is:
   ( | int (number of array values) | pointer to array (eg double *) |
	 x number_of_times )

Sets data in this memory to 0, pointers to NULL.

MUST have called set_FE_field_time_value_type() before calling this function.
Should only call this function for unmanaged fields.
==============================================================================*/

enum CM_field_type get_FE_field_CM_field_type(struct FE_field *field);
/*******************************************************************************
LAST MODIFIED : 31 August 1999

DESCRIPTION :
Returns the CM_field_type of the <field>.
==============================================================================*/

int set_FE_field_CM_field_type(struct FE_field *field,
	enum CM_field_type cm_field_type);
/*******************************************************************************
LAST MODIFIED : 31 August 1999

DESCRIPTION :
Sets the CM_field_type of the <field>.
Should only call this function for unmanaged fields.
==============================================================================*/

enum FE_field_type get_FE_field_FE_field_type(struct FE_field *field);
/*******************************************************************************
LAST MODIFIED : 31 August 1999

DESCRIPTION :
Returns the FE_field_type for the <field>.
==============================================================================*/

int set_FE_field_type_constant(struct FE_field *field);
/*******************************************************************************
LAST MODIFIED : 1 September 1999

DESCRIPTION :
Converts the <field> to type CONSTANT_FE_FIELD.
Allocates and clears the values_storage of the field to fit
field->number_of_components of the current value_type.
If function fails the field is left exactly as it was.
Should only call this function for unmanaged fields.
==============================================================================*/

int set_FE_field_type_general(struct FE_field *field);
/*******************************************************************************
LAST MODIFIED : 1 September 1999

DESCRIPTION :
Converts the <field> to type GENERAL_FE_FIELD.
Frees any values_storage currently in use by the field.
If function fails the field is left exactly as it was.
Should only call this function for unmanaged fields.
==============================================================================*/

int get_FE_field_type_indexed(struct FE_field *field,
	struct FE_field **indexer_field,int *number_of_indexed_values);
/*******************************************************************************
LAST MODIFIED : 1 September 1999

DESCRIPTION :
If the field is of type INDEXED_FE_FIELD, the indexer_field and
number_of_indexed_values it uses are returned - otherwise an error is reported.
Use function get_FE_field_FE_field_type to determine the field type.
==============================================================================*/

int set_FE_field_type_indexed(struct FE_field *field,
	struct FE_field *indexer_field,int number_of_indexed_values);
/*******************************************************************************
LAST MODIFIED : 1 September 1999

DESCRIPTION :
Converts the <field> to type INDEXED_FE_FIELD, indexed by the given
<indexer_field> and with the given <number_of_indexed_values>. The indexer_field
must return a single integer value to be valid.
Allocates and clears the values_storage of the field to fit
field->number_of_components x number_of_indexed_values of the current
value_type. If function fails the field is left exactly as it was.
Should only call this function for unmanaged fields.
==============================================================================*/

int get_FE_field_time_FE_value(struct FE_field *field,int number,FE_value *value);
/*******************************************************************************
LAST MODIFIED : 10 June 1999

DESCRIPTION :
Gets the specified global time value for the <field>.
==============================================================================*/

enum Value_type get_FE_field_time_value_type(struct FE_field *field);
/*******************************************************************************
LAST MODIFIED : 9 June 1999

DESCRIPTION :
Returns the time_value_type of the <field>.
==============================================================================*/

int set_FE_field_time_value_type(struct FE_field *field,
	enum Value_type time_value_type);
/*******************************************************************************
LAST MODIFIED : 9 June 1999

DESCRIPTION :
Sets the time_value_type of the <field>.
Should only call this function for unmanaged fields.
=========================================================================*/

enum Value_type get_FE_field_value_type(struct FE_field *field);
/*******************************************************************************
LAST MODIFIED : 19 February 1999

DESCRIPTION :
Returns the value_type of the <field>.
==============================================================================*/

int set_FE_field_value_type(struct FE_field *field,enum Value_type value_type);
/*******************************************************************************
LAST MODIFIED : 1 September 1999

DESCRIPTION :
Sets the value_type of the <field>. Clears/reallocates the values_storage for
FE_field_types that use them, eg. CONSTANT_FE_FIELD and INDEXED_FE_FIELD - but
only if the value_type changes. If function fails the field is left exactly as
it was. Should only call this function for unmanaged fields.
ELEMENT_XI_VALUE, STRING_VALUE and URL_VALUE fields may only have 1 component.
=========================================================================*/

/**
 * If the FE_field has value_type ELEMENT_XI_VALUE, this returns the
 * host mesh the embedded locations are within, or 0 if not yet set (legacy
 * input only).
 *
 * @param field  The field to query.
 * @return  Host mesh (not accessed) or 0 if none.
 */
const FE_mesh *FE_field_get_element_xi_host_mesh(struct FE_field *field);

/**
 * If the FE_field has value_type ELEMENT_XI_VALUE, sets the host mesh the
 * embedded locations are within.
 *
 * @param field  The field to modify.
 * @param hostMesh  The host mesh to set.
 * @return  Standard result code.
 */
int FE_field_set_element_xi_host_mesh(struct FE_field *field,
	const FE_mesh *hostMesh);

/**
 * Return the highest derivative and version numbers used to label any node
 * parameter of the finite element field.
 * @see cmzn_node_value_label
 *
 * @param field  The finite element field to query.
 * @param highest_derivative  On success, set to the highest derivative number
 * used to label any node parameter, using definition of cmzn_node_value_label
 * i.e. 1 = value.
 * @return highest_version  On success, set to the highest version number used
 * to label any node parameter, starting at 1.
 * @return  CMZN_OK on success, otherwise any other value.
 */
int FE_field_get_highest_node_derivative_and_version(FE_field *field,
	int& highest_derivative, int& highest_version);

int get_FE_field_max_array_size(struct FE_field *field,
	int *max_number_of_array_values,enum Value_type *value_type);
/*******************************************************************************
LAST MODIFIED : 4 March 1999

DESCRIPTION :
Given the field, search vaules_storage  for the largest array, and return it in
max_number_of_array_values. Return the field value_type.
==============================================================================*/

int get_FE_field_array_attributes(struct FE_field *field, int value_number,
 int *number_of_array_values, enum Value_type *value_type);
/*******************************************************************************
LAST MODIFIED : 4 March 1999

DESCRIPTION :
Get the value_type and the number of array values for the array in
field->values_storage specified by value_number.
Give an error if field->values_storage isn't storing array types.
==============================================================================*/

int get_FE_field_string_value(struct FE_field *field,int value_number,
	char **string);
/*******************************************************************************
LAST MODIFIED : 22 September 1999

DESCRIPTION :
Returns a copy of the string stored at <value_number> in the <field>.
Up to the calling function to DEALLOCATE the returned string.
Returned <*string> may be a valid NULL if that is what is in the field.
==============================================================================*/

int set_FE_field_string_value(struct FE_field *field,int value_number,
	char *string);
/*******************************************************************************
LAST MODIFIED : 22 September 1999

DESCRIPTION :
Copies and sets the <string> stored at <value_number> in the <field>.
<string> may be NULL.
==============================================================================*/

int get_FE_field_element_xi_value(struct FE_field *field,int number,
	struct FE_element **element, FE_value *xi);
/*******************************************************************************
LAST MODIFIED : 20 April 1999

DESCRIPTION :
Gets the specified global value for the <field>.
==============================================================================*/

int set_FE_field_element_xi_value(struct FE_field *field,int number,
	struct FE_element *element, FE_value *xi);
/*******************************************************************************
LAST MODIFIED : 20 April 1999

DESCRIPTION :
Sets the specified global value for the <field>, to the passed Element and xi
The field value MUST have been previously allocated with
set_FE_field_number_of_values
==============================================================================*/

int get_FE_field_FE_value_value(struct FE_field *field,int number,
	FE_value *value);
/*******************************************************************************
LAST MODIFIED : 2 September 1999

DESCRIPTION :
Gets the specified global FE_value <value> from <field>.
==============================================================================*/

int set_FE_field_FE_value_value(struct FE_field *field,int number,
	FE_value value);
/*******************************************************************************
LAST MODIFIED : 2 September 1999

DESCRIPTION :
Sets the specified global FE_value <value> in <field>.
The <field> must be of type FE_VALUE_VALUE to have such values and
<number> must be within the range from get_FE_field_number_of_values.
==============================================================================*/

int get_FE_field_int_value(struct FE_field *field,int number,int *value);
/*******************************************************************************
LAST MODIFIED : 2 September 1999

DESCRIPTION :
Gets the specified global int <value> from <field>.
==============================================================================*/

int set_FE_field_int_value(struct FE_field *field,int number,int value);
/*******************************************************************************
LAST MODIFIED : 2 September 1999

DESCRIPTION :
Sets the specified global int <value> in <field>.
The <field> must be of type INT_VALUE to have such values and
<number> must be within the range from get_FE_field_number_of_values.
==============================================================================*/

int set_FE_field_time_FE_value(struct FE_field *field,int number,
	FE_value value);
/*******************************************************************************
LAST MODIFIED : l0 June 1999

DESCRIPTION :
Sets the specified global time value for the <field>, to the passed FE_value
The field value MUST have been previously allocated with
set_FE_field_number_of_times
==============================================================================*/

const char *get_FE_field_name(struct FE_field *field);
/*******************************************************************************
LAST MODIFIED : 19 February 1999

DESCRIPTION :
Returns a pointer to the name for the <field>.
Should only call this function for unmanaged fields.
==============================================================================*/

/***************************************************************************//**
 * Sets the name of the <field>.
 * Should only call this function for unmanaged fields.
 * All others should use FE_region_set_FE_field_name.
 */
int set_FE_field_name(struct FE_field *field, const char *name);

/**
 * Returns true if element is a top_level parent of other_element.
 */
bool FE_element_is_top_level_parent_of_element(
	struct FE_element *element, struct FE_element *other_element);

/**
 * Returns the/a top level [ultimate parent] element for <element>. If supplied,
 * the function attempts to verify that the <check_top_level_element> is in
 * fact a valid top_level_element for <element>, otherwise it tries to find one
 * with the specified face, if supplied.
 *
 * If the returned element is different to <element> (ie. is of higher dimension),
 * then this function also fills the matrix <element_to_top_level> with values for
 * converting the xi coordinates in <element> to those in the returned element.
 * <element_to_top_level> should be preallocated to store at least nine FE_values.
 *
 * The use of the <element_to_top_level> matrix is similar to <face_to_element> in
 * FE_element_shape - in fact it is either a copy of it, or calculated from it.
 * It gives the transformation xi(top_level) = b + A xi(element), where b is in the
 * first column of the matrix, and the rest of the matrix is A. Its size depends
 * on the dimension of element:top_level, ie.,
 * 1:2 First 4 values, in form of 2 row X 2 column matrix, used only.
 * 1:3 First 6 values, in form of 3 row X 2 column matrix, used only.
 * 2:3 First 9 values, in form of 3 row X 3 column matrix, used only.
 * NOTE: recursive to handle 1-D to 3-D case.
 */
struct FE_element *FE_element_get_top_level_element_conversion(
	struct FE_element *element,struct FE_element *check_top_level_element,
	cmzn_element_face_type specified_face, FE_value *element_to_top_level);

int FE_element_get_top_level_element_and_xi(struct FE_element *element,
	const FE_value *xi, int element_dimension,
	struct FE_element **top_level_element, FE_value *top_level_xi,
	int *top_level_element_dimension);
/*******************************************************************************
LAST MODIFIED : 13 March 2003

DESCRIPTION :
Finds the <top_level_element>, <top_level_xi> and <top_level_element_dimension>
for the given <element> and <xi>.  If <top_level_element> is already set it
is checked and the <top_level_xi> calculated.
==============================================================================*/

/**
 * Returns in <number_in_xi> the equivalent discretization of <element> for its
 * position - element, face or line - in <top_level_element>. Uses
 * <element_to_top_level> array for line/face conversion as returned by
 * FE_element_get_top_level_element_conversion.
 * <number_in_xi> must have space at least MAXIMUM_ELEMENT_XI_DIMENSIONS
 * integers, as remaining values up to this size are cleared to zero.
 */
int get_FE_element_discretization_from_top_level(struct FE_element *element,
	int *number_in_xi, struct FE_element *top_level_element,
	int *top_level_number_in_xi, FE_value *element_to_top_level);

/**
 * Returns the discretization in <number_in_xi> for displaying graphics over
 * <element>, using the suggested <*top_level_element> if any.
 * If <native_discretization_field> is defined over the
 * element and is grid-based, it's native discretization is used in preference
 * to the <top_level_number_in_xi>.
 * <*top_level_element> can be NULL; final element used will be returned.
 * <top_level_number_in_xi> should be set by the caller as it will be used if
 * there is no native_discretization field or it is not defined over the element;
 * in either case the top_level_number_in_xi used is returned.
 */
int get_FE_element_discretization(struct FE_element *element,
	cmzn_element_face_type face, struct FE_field *native_discretization_field,
	int *top_level_number_in_xi,struct FE_element **top_level_element,
	int *number_in_xi);

/**
 * Checks if the element is 2-D and exterior i.e. a face of exactly one parent
 * element, and if so whether the standard element/face mapping gives it an
 * inward normal.
 *
 * @param element  The element to test.
 * @return  True if element is an exterior face with inward normal, otherwise false.
 */
bool FE_element_is_exterior_face_with_inward_normal(struct FE_element *element);

/**
 * Add nodes used by this element, including inherited from parent elements, to
 * the supplied labels group.
 * Caller must ensure the group was created for the appropriate FE_nodeset.
 * Note node inheritance is currently only for default coordinate field.
 * @return  CMZN_OK on success, any other value on failure.
 */
int cmzn_element_add_nodes_to_labels_group(cmzn_element *element, DsLabelsGroup &nodeLabelsGroup);

/**
 * Remove nodes used by this element, including inherited from parent elements,
 * from the supplied labels group.
 * Caller must ensure the group was created for the appropriate FE_nodeset.
 * Note node inheritance is currently only for default coordinate field.
 * @return CMZN_OK on success, any other value on failure.
 */
int cmzn_element_remove_nodes_from_labels_group(cmzn_element *element, DsLabelsGroup &nodeLabelsGroup);

/**
 * Conditional function.
 * @return  1 if element is top-level i.e. has no parents, otherwise 0.
 */
int FE_element_is_top_level(struct FE_element *element,void *dummy_void);

int FE_field_is_1_component_integer(struct FE_field *field,void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 18 May 2000

DESCRIPTION :
Conditional function returning true if <field> has exactly 1 component and a
value type of integer.
This type of field is used for storing eg. grid_point_number.
==============================================================================*/

int FE_field_is_coordinate_field(struct FE_field *field,void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 18 May 2000

DESCRIPTION :
Conditional function returning true if the <field> is a coodinate field
(defined by having a CM_field_type of coordinate) has a Value_type of
FE_VALUE_VALUE and has from 1 to 3 components.
==============================================================================*/

int FE_field_is_anatomical_fibre_field(struct FE_field *field,void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 16 July 1998

DESCRIPTION :
Conditional function returning true if the <field> is a anatomical field
(defined by having a CM_field_type of anatomical), has a Value_type of
FE_VALUE_VALUE, has from 1 to 3 components, and has a FIBRE coordinate system.
==============================================================================*/

int FE_field_is_embedded(struct FE_field *field, void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 5 June 2003

DESCRIPTION :
Returns true if the values returned by <field> are a location in an FE_region,
either an element_xi value, or eventually a node.
==============================================================================*/

/** @return  True if the field has parameters at the node. */
bool FE_field_has_parameters_at_node(FE_field *field, cmzn_node *node);

/** @return  True if the <field> is defined on the <element>, directly or
  * inherited from any parent element it is a face of.
 */
bool FE_field_is_defined_in_element(struct FE_field *field,
	struct FE_element *element);

/**
 * Returns true if <field> is defined in <element> directly, not inherited from
 * a parent element it is a face of.
 */
bool FE_field_is_defined_in_element_not_inherited(struct FE_field *field,
	struct FE_element *element);

/** @return  True if any component of field is grid-based in element. */
bool FE_element_field_is_grid_based(struct FE_element *element,
	struct FE_field *field);

/** @return  True if any field component defined on element is grid-based. */
bool FE_element_has_grid_based_fields(struct FE_element *element);

/**
 * Partner function to FE_element_smooth_FE_field. Averages node derivatives.
 * Assigns the nodal first derivatives of fe_field at time in node with the
 * values from node_accumulate_fe_field divided by the value of the
 * element_count_fe_field, handling multiple versions.
 * Finally undefines node_accumulate_fe_field and element_count_fe_field.
 * @return  1 on success, 0 on failure.
 */
int FE_node_smooth_FE_field(struct FE_node *node, struct FE_field *fe_field,
	FE_value time, struct FE_field *node_accumulate_fe_field,
	struct FE_field *element_count_fe_field);

/**
 * For each node contributing to <fe_field> in <element>, accumulates delta
 * coordinates along each element edge are accumulated.
 *
 * @param node_accumulate_fe_field  Temporary field for accumulating node
 * values. Must have same number of components as fe_field.
 * @param element_count_fe_field  Field to store number of elements accumulated
 * at node, for final averaging.
 *
 * After making calls to this function for all the intended elements, call
 * FE_node_smooth_FE_field for each node to divide the accumulated derivatives
 * by the number of elements they are over and to undefine the
 * node_accumulate_fe_field and element_count_fe_field.
 *
 * Sets all scale factors used for <fe_field> to 1.0.
 *
 * Notes:
 * Only works for "line" shapes with Hermite basis functions.
 * - <element> shold not be global = not merged into an FE_region.
 * - <fe_field> should be of type FE_VALUE_VALUE.
 * - returns 1 without errors if fe_field is not defined on this element or the
 *   element has no field information, or the field cannot be smoothed.
 * - handles multiple versions at nodes.
 * - may not handle obscure tensor product combinations such as linear in xi1,
 *   cubic Hermite in xi2, since indexing of element parameters assumes order:
 *   value d/dxi1 d/dxi2 d2/dxi1dxi2 d/dxi3 d2/dxi1dxi3 d2/dxi2dxi3 d3/dxi1dxi2dxi3
 */
bool FE_element_smooth_FE_field(struct FE_element *element,
	struct FE_field *fe_field, FE_value time, 
	struct FE_field *node_accumulate_fe_field,
	struct FE_field *element_count_fe_field);

int FE_element_shape_find_face_number_for_xi(struct FE_element_shape *shape,
	FE_value *xi, int *face_number);
/*******************************************************************************
LAST MODIFIED : 11 June 1999

DESCRIPTION :
==============================================================================*/

struct FE_field_order_info *CREATE(FE_field_order_info)(void);
/*******************************************************************************
LAST MODIFIED : 4 September 2001

DESCRIPTION
Frees them memory used by field_order_info.
==============================================================================*/

int DESTROY(FE_field_order_info)(
	struct FE_field_order_info **field_order_info_address);
/*******************************************************************************
LAST MODIFIED : 4 September 2001

Frees memory for FE_field_order_info
==============================================================================*/

int add_FE_field_order_info_field(
	struct FE_field_order_info *field_order_info, struct FE_field *field);
/*******************************************************************************
LAST MODIFIED : 4 September 2001

DESCRIPTION :
Adds <field> to the end of the list of fields in <field_order_info>.
==============================================================================*/

/***************************************************************************//**
 * FE_field iterator for calling add_FE_field_order_info_field.
 */
int FE_field_add_to_FE_field_order_info(struct FE_field *field,
	void *field_order_info_void);

/***************************************************************************//**
 * Ensures indexer fields precede fields they index in FE_field_order_info.
 */
int FE_field_order_info_prioritise_indexer_fields(
	struct FE_field_order_info *field_order_info);

int clear_FE_field_order_info(struct FE_field_order_info *field_order_info);
/*******************************************************************************
LAST MODIFIED : 4 September 2001

DESCRIPTION :
Clears the fields from <field_order_info>.
==============================================================================*/

int get_FE_field_order_info_number_of_fields(
	struct FE_field_order_info *field_order_info);
/*******************************************************************************
LAST MODIFIED : 13 July 1999

DESCRIPTION :
Gets the <field_order_info> number_of_fields
==============================================================================*/

struct FE_field *get_FE_field_order_info_field(
	struct FE_field_order_info *field_order_info,int field_number);
/*******************************************************************************
LAST MODIFIED : 13 July 1999

DESCRIPTION :
Gets the <field_order_info> field at the specified field_number
==============================================================================*/

/**
 * Returns the number of permutations known for the changing to the adjacent
 * element at face <face_number>.  It would be better to extend FE_element_shape
 * to take account of rotations and flipping.
 */
int FE_element_get_number_of_change_to_adjacent_element_permutations(
	struct FE_element *element, FE_value *xi, int face_number);

/**
 * Steps into the adjacent element through face <face_number>, updating the
 * <element_address> location.
 * If <xi> is not NULL then the <xi_face> coordinates are converted to an xi
 * location in the new element.
 * If <increment> is not NULL then it is converted into an equvalent increment
 * in the new element.
 * <permutation> is used to resolve the possible rotation and flipping of the
 * local face xi coordinates between the two parents.
 * The shape mapping from parents are reused for all elements of the same shape
 * and do not take into account the relative orientation of the parents.  It would
 * be better to do so.  The range of possible permutations is from 0 to the value
 * returned from FE_element_get_number_of_change_to_adjacent_element_permutations.
 */
int FE_element_change_to_adjacent_element(struct FE_element **element_address,
	FE_value *xi, FE_value *increment, int *face_number, FE_value *xi_face,
	int permutation);

/**
 * Adds the <increment> to <xi>.  If this moves <xi> outside of the element, then
 * if an adjacent element is found then the element and xi location are changed
 * to this element and the stepping continues using the remaining increment.  If
 * no adjacent element is found then the <xi> will be on the element boundary and
 * the <increment> will contain the fraction of the increment not used.
 */
int FE_element_xi_increment(struct FE_element **element_address,FE_value *xi,
	FE_value *increment);

enum cmzn_field_domain_type cmzn_field_domain_type_enum_from_string(
	const char *string);

char *cmzn_field_domain_type_enum_to_string(enum cmzn_field_domain_type type);

enum cmzn_element_face_type cmzn_element_face_type_enum_from_string(
	const char *string);

char *cmzn_element_face_type_enum_to_string(enum cmzn_element_face_type type);

enum cmzn_element_point_sampling_mode cmzn_element_point_sampling_mode_enum_from_string(
	const char *string);

char *cmzn_element_point_sampling_mode_enum_to_string(
	enum cmzn_element_point_sampling_mode mode);


#endif /* !defined (FINITE_ELEMENT_H) */
