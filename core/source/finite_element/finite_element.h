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

#include "opencmiss/zinc/node.h"
#include "opencmiss/zinc/element.h"
#include "opencmiss/zinc/status.h"
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

class DsLabelsChangeLog;

/**
 * FE_node has pointer to owning FE_nodeset in shared field info.
 */
class FE_nodeset;

/**
 * FE_element has pointer to owning FE_mesh in shared field info.
 */
class FE_mesh;

/**
 * Set of scale factors stored by mesh.
 */
struct cmzn_mesh_scale_factor_set;

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

struct FE_field_external_information;
/*******************************************************************************
LAST MODIFIED : 2 September 2001

DESCRIPTION :
A structure that allows external tools have their field information/identifier.

This was developed for cm so that information about where the fields are stored
in cm can be kept in cmgui.  The version for cm can be found in link/cmiss.c
==============================================================================*/

typedef int (Compare_FE_field_external_information)( \
	struct FE_field_external_information *, \
	struct FE_field_external_information *);

typedef int (Destroy_FE_field_external_information)( \
	struct FE_field_external_information **);

typedef struct FE_field_external_information * \
	(Duplicate_FE_field_external_information)( \
	struct FE_field_external_information *);

struct FE_field_external_information
{
	/* function for ordering this structure.  Returns -1 if first<second, 0 if
		first=second and 1 if first>second */
	/* also serves as an id so that different external tools can tell if the field
		is theirs */
		/*???DB.  Does not allow for more than one external tool of each type */
	Compare_FE_field_external_information *compare;
	/* for destroying the structure */
	Destroy_FE_field_external_information *destroy;
	/* for duplicating the structure */
	Duplicate_FE_field_external_information *duplicate;
	/* the external tool's field information */
	void *information;
}; /* struct FE_field_external_information */

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

struct FE_field_component
/*******************************************************************************
LAST MODIFIED : 27 October 1995

DESCRIPTION :
Used to specify a component of a field.  If the component <number> is < 0 or
>= the number of components, it specifies all components.
==============================================================================*/
{
	struct FE_field *field;
	int number;
}; /* struct FE_field_component */

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
	FE_NODAL_VALUE,
	FE_NODAL_D_DS1,
	FE_NODAL_D_DS2,
	FE_NODAL_D2_DS1DS2,
	FE_NODAL_D_DS3,
	FE_NODAL_D2_DS1DS3,
	FE_NODAL_D2_DS2DS3,
	FE_NODAL_D3_DS1DS2DS3,
	FE_NODAL_UNKNOWN
}; /* enum FE_nodal_value_type */

struct FE_node_field_creator;

struct cmzn_node;
#define FE_node cmzn_node

DECLARE_LIST_TYPES(FE_node);

DECLARE_CHANGE_LOG_TYPES(FE_node);

/**
 * Specifies the type of mapping used to get element DOFs for interpolation
 * from global DOFs in a variety of sources (nodes, field etc.)
 */
enum Global_to_element_map_type
{
	STANDARD_NODE_TO_ELEMENT_MAP,
	GENERAL_ELEMENT_MAP,
	ELEMENT_GRID_MAP
};

struct Standard_node_to_element_map;
/*******************************************************************************
LAST MODIFIED : 9 October 2002

DESCRIPTION :
Stores the information for calculating element values by choosing nodal values
and applying a diagonal scale factor matrix.  The <nodal_values> and
<scale_factors> are stored as offsets so that the arrays stored with the nodes
and elements can be reallocated.
==============================================================================*/

struct FE_element_field_component;
/*******************************************************************************
LAST MODIFIED : 9 October 2002

DESCRIPTION :
Stores the information for calculating element values, with respect to the
<basis>, from global values (this calculation includes the application of scale
factors).  There are two types - <NODE_BASED_MAP> and <GENERAL_LINEAR_MAP>.  For
a node based map, the global values are associated with nodes.  For a general
linear map, the global values do not have to be associated with nodes.  The node
based maps could be specified as general linear maps, but the node based
specification (required by CMISS) cannot be recovered from the general linear
map specification (important when the front end is being used to create meshs).
The <modify> function is called after the element values have been calculated
with respect to the <basis> and before the element values are blended to be with
respect to the standard basis.  The <modify> function is to allow for special
cases, such as nodes that have multiple theta values in cylindrical polar,
spherical polar, prolate spheroidal or oblate spheroidal coordinate systems -
either lying on the z-axis or being the first and last node in a circle.
==============================================================================*/

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

typedef int (*FE_element_field_component_modify)(
	struct FE_element_field_component *,struct FE_element *,struct FE_field *,
	FE_value,int,FE_value *);

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

struct FE_node_order_info
/*******************************************************************************
LAST MODIFIED : 11 August 1999

DESCRIPTION :
Use to pass info about a node group's nodes, their number, and their order.
c.f.  FE_field_order_info. Also store a current node, so can iterate, etc
==============================================================================*/
{
	int number_of_nodes,current_node_number;
	struct FE_node **nodes;
	int access_count;
}; /* FE_node_order_info */

struct FE_element_order_info
/*******************************************************************************
LAST MODIFIED : 10 January 2000

DESCRIPTION :
Use to pass info about a element group's elements, their number, and their
order.
c.f.  FE_field_order_info. Also store a current element, so can iterate, etc
==============================================================================*/
{
	int number_of_elements,current_element_number;
	struct FE_element **elements;
	int access_count;
}; /* FE_element_order_info */

struct FE_field_and_string_data
/*******************************************************************************
LAST MODIFIED : 26 September 2000

DESCRIPTION :
Used by FE_node_has_FE_field_and_string_data
==============================================================================*/
{
	struct FE_field *fe_field;
	char *string;
};

struct Node_is_in_list_data
/*******************************************************************************
LAST MODIFIED : 17 October 2000

DESCRIPTION :
used by all_nodes_in_list
==============================================================================*/
{
	struct LIST(FE_node) *node_list;
};

PROTOTYPE_ENUMERATOR_FUNCTIONS(cmzn_field_domain_type);

/*
Global functions
----------------
*/

struct FE_node_field_creator *CREATE(FE_node_field_creator)(
	int number_of_components);
/*******************************************************************************
LAST MODIFIED : 16 November 2001

DESCRIPTION :
An object for defining the components, number_of_versions,
number_of_derivatives and their types at a node.
By default each component has 1 version and no derivatives.
==============================================================================*/

struct FE_node_field_creator *create_FE_node_field_creator_from_node_field(
	struct FE_node *node, struct FE_field *field);
/*******************************************************************************
LAST MODIFIED : 4 February 2001

DESCRIPTION :
Creates an FE_node_field_creator from <node>,<field>
==============================================================================*/

int DESTROY(FE_node_field_creator)(
	struct FE_node_field_creator **node_field_creator_address);
/*******************************************************************************
LAST MODIFIED : 16 November 2001

DESCRIPTION :
Frees the memory for the node field creator and sets
<*node_field_creator_address> to NULL.
==============================================================================*/

/**
 * Adds the derivative of specified <derivative_type> to the <component_number>
 * specified.
 * @return  CMZN_OK on success, CMZN_ERROR_ALREADY_EXISTS if derivative already
 * defined, CMZN_ERROR_ARGUMENT if invalid argument(s) supplied.
 */
int FE_node_field_creator_define_derivative(
	struct FE_node_field_creator *node_field_creator, int component_number,
	enum FE_nodal_value_type derivative_type);

/**
 * Removes the derivative of specified <derivative_type> for the <component_number>
 * specified.
 */
void FE_node_field_creator_undefine_derivative(
	struct FE_node_field_creator *node_field_creator, int component_number,
	enum FE_nodal_value_type derivative_type);

/**
 * Returns the number of derivatives defined for a given component in the
 * node field creator (not including value).
 *
 * @param node_field_creator  Node field creator to query.
 * @param component_number  The component from 0 to the number of field
 * components-1.
 * @return  Number of versions for component of field, or 0 if field not
 * defined or invalid arguments are supplied.
 */
int FE_node_field_creator_get_number_of_derivatives(
	struct FE_node_field_creator *node_field_creator, int component_number);

/**
 * Specifies the <number_of_versions> for <component_number> specified.
 * @return  CMZN_OK  on success, otherwise any other error.
 */
int FE_node_field_creator_define_versions(
	struct FE_node_field_creator *node_field_creator, int component_number,
	int number_of_versions);

/**
 * Returns the number of versions defined for a given component in the
 * node field creator.
 *
 * @param node_field_creator  Node field creator to query.
 * @param component_number  The component from 0 to the number of field
 * components-1, or negative to get maximum number of versions in any component.
 * @return  Number of versions for component of field, or maximum in any
 * component if component_number is -1). Returns 0 if field not defined or
 * invalid arguments are supplied.
 */
int FE_node_field_creator_get_number_of_versions(
	struct FE_node_field_creator *node_field_creator, int component_number);

/***************************************************************************//**
 * Returns whether a nodal derivative is defined for a component in the
 * node field creator.
 *
 * @param node_field_creator  Node field creator to query.
 * @param component_number  The component from 0 to the number of field
 * components-1, or negative to check if *any* component has the nodal derivative.
 * @param derivative_type  The type of nodal derivative to check.
 * @return  1 if derivative_type is defined for component_number of field (or
 * for any component if component_number is -1), 0 if not.
 */
int FE_node_field_creator_has_derivative(
	struct FE_node_field_creator *node_field_creator, int component_number,
	enum FE_nodal_value_type derivative_type);

/**
 * Creates and returns a node with the specified <cm_node_identifier>.
 * If <fe_nodeset> is supplied a blank node with the given identifier but no
 * fields is returned. If <template_node> is supplied, a copy of it,
 * including all fields and values but with the new identifier, is returned.
 * Exactly one of <fe_nodeset> or <template_node> must be supplied and either
 * directly or indirectly sets the owning nodeset for the new node.
 * Note that <cm_node_identifier> must be non-negative.
 */
struct FE_node *CREATE(FE_node)(int cm_node_identifier,
	FE_nodeset *fe_nodeset, struct FE_node *template_node);

/**
 * Frees the memory for the node, sets <*node_address> to NULL.
 */
int DESTROY(FE_node)(struct FE_node **node_address);

/**
 * Clear content of node and disconnect it from owning nodeset.
 * Use when removing node from nodeset or deleting nodeset to safely orphan any
 * externally accessed nodes.
 */
void FE_node_invalidate(struct FE_node *node);

PROTOTYPE_OBJECT_FUNCTIONS(FE_node);
PROTOTYPE_COPY_OBJECT_FUNCTION(FE_node);
PROTOTYPE_GET_OBJECT_NAME_FUNCTION(FE_node);

/**
 * Creates a copy of <node> containing only FE_node_fields from <node> which
 * have their FE_field listed in <fe_field_list>.
 */
struct FE_node *FE_node_copy_with_FE_field_list(struct FE_node *node,
	struct LIST(FE_field) *fe_field_list);

int define_FE_field_at_node(struct FE_node *node,struct FE_field *field,
	struct FE_time_sequence *fe_time_seqence,
	struct FE_node_field_creator *fe_node_field_creator);
/*******************************************************************************
LAST MODIFIED : 16 November 2001

DESCRIPTION :
Defines a field at a node (does not assign values).
==============================================================================*/

struct Node_list_field_data
/*******************************************************************************
LAST MODIFIED : 28 April 2003

DESCRIPTION :
Iterator/conditional function data containing a node list and FE_field.
==============================================================================*/
{
	struct LIST(FE_node) *fe_node_list;
	struct FE_field *fe_field;
};

int FE_element_ensure_FE_field_nodes_are_not_in_list(
	struct FE_element *element,void *node_list_field_data_void);
/*******************************************************************************
LAST MODIFIED : 28 April 2003

DESCRIPTION :
Iterator function which, if <element> has nodes and the listed <fe_field>
defined on it, ensures those nodes contributing to <fe_field> are not in the
<fe_node_list>.
==============================================================================*/

/**
 * Removes definition of <field> at <node>. If field is of type GENERAL_FE_FIELD
 * then removes values storage for it and shifts values storage for all subsequent
 * fields down.
 * Note: Must ensure that the node field is not in-use by any elements before it
 * is undefined!
 */
int undefine_FE_field_at_node(struct FE_node *node,struct FE_field *field);

int define_FE_field_at_node_simple(struct FE_node *node, struct FE_field *field,
	int number_of_derivatives, enum FE_nodal_value_type *derivative_value_types);
/*******************************************************************************
LAST MODIFIED : 19 September 2002

DESCRIPTION :
Defines <field> at <node> using the same <number_of_derivatives>
and <nodal_value_types> for each component, and only 1 version.
==============================================================================*/

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

int FE_nodal_value_version_exists(struct FE_node *node,
	struct FE_field *field, int component_number,int version,
	enum FE_nodal_value_type type);
/*******************************************************************************
LAST MODIFIED : 21 April 2005

DESCRIPTION :
Returns 1 if the <field>, <component_number>, <version> and <type> are stored at the
node.
???DB.  May need speeding up
==============================================================================*/

int get_FE_nodal_value_as_string(struct FE_node *node,
	struct FE_field *field,int component_number,int version,
	enum FE_nodal_value_type type, FE_value time, char **string);
/*******************************************************************************
LAST MODIFIED : 22 November 2001

DESCRIPTION :
Returns as a string the value for the (<version>, <type>) for the <field>
<component_number> at the <node>.
It is up to the calling function to DEALLOCATE the returned string.
==============================================================================*/

#define PROTOTYPE_GET_FE_NODAL_VALUE_FUNCTION( value_type, value_enum ) \
int get_FE_nodal_ ## value_type ## _value(struct FE_node *node, \
	struct FE_field *field, int component_number, int version, \
	enum FE_nodal_value_type type, FE_value time, value_type *value); \
/******************************************************************************* \
LAST MODIFIED : 21 April 2005 \
 \
DESCRIPTION : \
Gets a particular value (<version>, <type>) for the field> \
and <component_number> at the <node> and <time>. \
==============================================================================*/

#define PROTOTYPE_SET_FE_NODAL_VALUE_FUNCTION( value_type, value_enum ) \
int set_FE_nodal_ ## value_type ## _value(struct FE_node *node, \
	struct FE_field *field, int component_number, int version, \
	enum FE_nodal_value_type type, FE_value time, value_type value); \
/******************************************************************************* \
LAST MODIFIED : 21 April 2005 \
 \
DESCRIPTION : \
Sets a particular value (<version>, <type>) for the <field> \
and <component_number> at the <node>. \
==============================================================================*/

#define PROTOTYPE_GET_FE_NODAL_VALUE_STORAGE_FUNCTION( value_type, value_enum ) \
int get_FE_nodal_ ## value_type ## _storage(struct FE_node *node, \
	struct FE_field *field, int component_number, int version, \
	enum FE_nodal_value_type type, FE_value time, value_type **value); \
/******************************************************************************* \
LAST MODIFIED : 8 May 2007 \
 \
DESCRIPTION : \
Returns a pointer to the memory which contains the values storage for this  \
degree of freedom.  This pointer will be invalid if the node is modified so \
it should only be used temporarily. \
==============================================================================*/

#define PROTOTYPE_FE_NODAL_VALUE_FUNCTIONS( value_type , value_enum ) \
PROTOTYPE_GET_FE_NODAL_VALUE_FUNCTION(value_type,value_enum) \
PROTOTYPE_SET_FE_NODAL_VALUE_FUNCTION(value_type,value_enum) \
PROTOTYPE_GET_FE_NODAL_VALUE_STORAGE_FUNCTION(value_type,value_enum)

PROTOTYPE_FE_NODAL_VALUE_FUNCTIONS( FE_value , FE_VALUE_VALUE )
PROTOTYPE_FE_NODAL_VALUE_FUNCTIONS( double , DOUBLE_VALUE )
PROTOTYPE_FE_NODAL_VALUE_FUNCTIONS( float , FLOAT_VALUE )
PROTOTYPE_FE_NODAL_VALUE_FUNCTIONS( int , INT_VALUE )
PROTOTYPE_FE_NODAL_VALUE_FUNCTIONS( short , SHORT_VALUE )

int get_FE_nodal_element_xi_value(struct FE_node *node,
	struct FE_field *field, int component_number, int version,
	enum FE_nodal_value_type type, struct FE_element **element, FE_value *xi);
/*******************************************************************************
LAST MODIFIED : 23 April 1999

DESCRIPTION :
Gets a particular element_xi_value (<version>, <type>) for the field <component> at the
<node>.  SAB Note: It doesn't use a FE_field_component as I don't think any of them
should.
==============================================================================*/

/**
 * Sets a particular element_xi_value (<version>, <type>) for the field
 * <component> at the <node>.
 * SAB Note: It doesn't use a FE_field_component as I don't think any of them
 * should.
 */
int set_FE_nodal_element_xi_value(struct FE_node *node,
	struct FE_field *field, int component_number, int version,
	enum FE_nodal_value_type type,struct FE_element *element, const FE_value *xi);

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

int add_FE_node_to_list(struct FE_node *node, void *node_list_void);
/*******************************************************************************
LAST MODIFIED : 3 March 2003

DESCRIPTION :
Iterator function for adding <node> to <node_list>. Does not expect <node> to
already be in list so more efficient than ensure_FE_node_is_in_list
==============================================================================*/

int ensure_FE_node_is_in_list(struct FE_node *node,void *node_list_void);
/*******************************************************************************
LAST MODIFIED : 20 April 1999

DESCRIPTION :
Iterator function for adding <node> to <node_list> if not currently in it.
==============================================================================*/

struct FE_node_list_conditional_data
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Data for passing to ensure_FE_node_is_in_list_conditional.
==============================================================================*/
{
	struct LIST(FE_node) *node_list;
	LIST_CONDITIONAL_FUNCTION(FE_node) *function;
	void *user_data;
}; /* FE_node_list_conditional_data */

int ensure_FE_node_is_in_list_conditional(struct FE_node *node,
	void *list_conditional_data_void);
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Iterator function for adding <node> to a list - if not already in it - if a
conditional function with user_data is true.
The node_list, conditional function and user_data are passed in a
struct FE_node_list_conditional_data * in the second argument.
Warning: Must not be iterating over the list being added to!
==============================================================================*/

int ensure_FE_node_is_not_in_list(struct FE_node *node,void *node_list_void);
/*******************************************************************************
LAST MODIFIED : 20 April 1999

DESCRIPTION :
Iterator function for removing <node> from <node_list> if currently in it.
==============================================================================*/

int toggle_FE_node_in_list(struct FE_node *node,void *node_list_void);
/*******************************************************************************
LAST MODIFIED : 15 February 2000

DESCRIPTION :
If <node> is in <node_list> it is taken out, otherwise it is added.
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

struct FE_node_can_be_merged_data
/*******************************************************************************
LAST MODIFIED : 15 November 2002

DESCRIPTION :
Data to be passed to FE_node_can_be_merged.
<number_of_compatible_node_field_info> and <compatible_node_field_info> must be
cleared before first call to 0 and NULL respectively. After using the function,
the <compatible_node_field_info> array must be deallocated. Note this array is
grows in increments of 2 since first node_field_info of the pair is that of a
node passed to the function, every second info is that of its counterpart
obtained from <node_list>
==============================================================================*/
{
	int number_of_compatible_node_field_info;
	/* store in pairs in the single array to reduce allocations */
	struct FE_node_field_info **compatible_node_field_info;
	struct LIST(FE_node) *node_list;
}; /* struct FE_node_can_be_merged_data */

int FE_node_can_be_merged(struct FE_node *node, void *data_void);
/*******************************************************************************
LAST MODIFIED : 15 November 2002

DESCRIPTION :
Fetches a node with the same identifier as <node> from <data>->node_list.
Returns true if there is either no such node in the list or the two nodes have
the same node field definitions for all fields of the same name.
Note that the actual field may be different, but it is assumed that the same
name fields are already proven to be compatible. <data_void> should point at a
properly initialised struct FE_node_can_be_merged_data.
After using the function, deallocate data->compatible_node_field_info!
==============================================================================*/

int FE_node_has_FE_field_and_string_data(struct FE_node *node,void *data_void);
/*******************************************************************************
LAST MODIFIED : 26 September 2000

DESCRIPTION :
Returns true(1) if the <data_void>->fe_field is define at the <node> AND
the nodal string at <node>,<data_void>->fe_field is equal to <data_void>->string.
Otherwise returns false (0)
==============================================================================*/

int FE_node_is_in_list(struct FE_node *node,void *node_list_void);
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Returns true if <node> is in <node_list>.
==============================================================================*/

int FE_node_is_not_in_list(struct FE_node *node,void *node_list_void);
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Returns true if <node> is not in <node_list>.
==============================================================================*/

PROTOTYPE_ENUMERATOR_FUNCTIONS(FE_nodal_value_type);

int get_FE_field_time_array_index_at_FE_value_time(struct FE_field *field,
	FE_value time,FE_value *the_time_high,FE_value *the_time_low,
	int *the_array_index,int *the_index_high,int *the_index_low);
/*******************************************************************************
LAST MODIFIED : 1 August 2000

DESCRIPTION
Given a <field> and <time>, checks that <field> has times defined and returns:
<the_array_index>, the array index of <field> times closest to <time>.
<the_index_high>, <the_index_low> the upper and lower limits for <the_array_index>
(ideally <the_index_high>==<the_index_low>==<the_array_index>).
<the_time_low> the time corresponding to <the_index_low>.
<the_time_high> the time corresponding to <the_index_high>.

All this information (rather than just <the_array_index> ) is returned so can
perform interpolation, etc.
==============================================================================*/

int get_FE_nodal_string_value(struct FE_node *node,
	struct FE_field *field,int component_number,int version,
	enum FE_nodal_value_type type,char **string);
/*******************************************************************************
LAST MODIFIED : 3 September 1999

DESCRIPTION :
Returns a copy of the string for <version>, <type> of <field><component_number>
at the <node>. Up to the calling function to DEALLOCATE the returned string.
Returned <*string> may be a valid NULL if that is what is in the node.
==============================================================================*/

int set_FE_nodal_string_value(struct FE_node *node,
	struct FE_field *field, int component_number, int version,
	enum FE_nodal_value_type type, const char *string);
/*******************************************************************************
LAST MODIFIED : 3 September 1999

DESCRIPTION :
Copies and sets the <string> for <version>, <type> of <field><component_number>
at the <node>. <string> may be NULL.
==============================================================================*/

int set_FE_nodal_field_double_values(struct FE_field *field,
	struct FE_node *node,double *values, int *number_of_values);
/*******************************************************************************
LAST MODIFIED : 30 August 1999

DESCRIPTION :
Sets the node field's values storage (at node->values_storage, NOT
field->values_storage) with the doubles in values.
Returns the number of doubles copied in number_of_values.
Assumes that values is set up with the correct number of doubles.
Assumes that the node->values_storage has been allocated with enough
memory to hold all the values.
Assumes that the nodal fields have been set up, with information to
place the values.
==============================================================================*/

int get_FE_nodal_field_number_of_values(struct FE_field *field,
	struct FE_node *node);
/*******************************************************************************
LAST MODIFIED : 20 September 1999

DESCRIPTION :
Returns the total number of values stored for that field at the node, equals
sum of (1+num_derivatives)*num_versions for each component.
==============================================================================*/

/**
 * Get derivative and version labels for all parameters for all components of
 * field at node. These are in the same order as returned by
 * get_FE_nodal_field_FE_value_values, cycling from slowest to fastest:
 * components, versions, derivatives.
 *
 * @param field  The field to query; must be FE_value valued.
 * @param node  The node to get parameters for.
 * @param time  The time to get parameters at. If between time indexes the
 * parameters are interpolated. If outside range of times the nearest (first or
 * last) time is returned.
 * @param lastNode  Optional last node for comparing field layout. If node has
 * same field info, parameter labels are returned as-is for efficiency.
 * @param componentParameterCounts  Array of component count size for receiving
 * numbers of parameters per component. Passed in pre-allocated to sufficient size.
 * @param componentDerivatives  Array of per-component arrays for receiving
 * derivative / value type arrays. Passed in pre-allocated to sufficient size
 * for each component. First derivative type is 1 = value.
 * @param componentVersions  Array of per-component arrays for receiving
 * version arrays. Passed in pre-allocated to sufficient size for each component.
 * version numbers start at 1.
 * @param isHomogeneous  On successful return, set to true if a multicomponent
 * field with all components having the same number of parameters with labels
 * in the same order, so the caller can use values for all components together.
 * @return  CMZN_OK if node parameter labels are correctly returned,
 * CMZN_ERROR_NOT_FOUND if field not defined, otherwise any other error code.
 */
int FE_field_get_node_parameter_labels(FE_field *field, FE_node *node, FE_value time,
	FE_node *lastNode, int *componentParameterCounts, int **componentDerivatives, 
	int **componentVersions, bool &isHomogeneous);

/**
 * Allocates and returns a copy of the <number_of_values>-length <values> array
 * stored at the <node> for all components derivatives and versions of <field>.
 * It will return the values specified at <time> if time sequence is found on
 * node field. If time is out of its range then it will return either the values
 * at minimum or maximum time.
 * It is up to the calling function to DEALLOCATE the returned array.
 *
 * @param  field  fields value to be returned from.
 * @param  node  the node which stores the field values.
 * @param  time  returns the values at <time> if nodal field has the concept of
 *    time, otherwise this argument is ignored.
 * @param  values  <number_of_values>-length <values> array to be returned, it
 *   stores all components derivatives and versions of <field> at <node>. Array
 *   is allocated by this function and must be be deallocated by the caller.
 * @return  1 if successfully returns a value otherwise 0.
 */
int get_FE_nodal_field_FE_value_values(struct FE_field *field,
	struct FE_node *node,int *number_of_values,FE_value time, FE_value **values);

/**
 * Sets all FE_value-type parameters for field at node.
 * Assumes that values is set up with the correct number of FE_values.
 * Assumes that the node->values_storage has been allocated with enough
 * memory to hold all the values.
 * Assumes that the nodal fields have been set up, with information to
 * place the values.
 *
 * @param time  The time to set values at, ignored for non-time-varying field.
 * For a time-varying field, must exactly equal a time at which parameters are
 * stored.
 * @param number_of_values  On return equals the number of values set.
 */
int set_FE_nodal_field_FE_value_values(struct FE_field *field,
	struct FE_node *node, FE_value *values, int *number_of_values, FE_value time);

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
	int arraySize, FE_value *values, int *valueExists, int valuesCount,
	int componentsSize, int componentsOffset,
	int derivativesSize, int derivativesOffset,
	int versionsSize, int versionsOffset);

int set_FE_nodal_field_float_values(struct FE_field *field,
	struct FE_node *node,float *values, int *number_of_values);
/*******************************************************************************
LAST MODIFIED : 30 August 1999

DESCRIPTION :
Sets the node field's values storage (at node->values_storage, NOT
field->values_storage) with the floats in values.
Returns the number of floats copied in number_of_values.
Assumes that values is set up with the correct number of floats.
Assumes that the node->values_storage has been allocated with enough
memory to hold all the values.
Assumes that the nodal fields have been set up, with information to
place the values.
==============================================================================*/

/**
 * Allocates and returns a copy of the <number_of_values>-length <values> array
 * stored at the <node> for all components derivatives and versions of <field>.
 * It will return the values specified at <time> if time sequence is found on
 * node field. If time is out of its range then it will return either the values
 * at minimum or maximum time.
 * It is up to the calling function to DEALLOCATE the returned array.
 *
 * @param  field  fields value to be returned from.
 * @param  node  the node which stores the field values.
 * @param  time  returns the values at <time> if nodal field has the concept of
 *    time, otherwise this argument is ignored.
 * @param  values  <number_of_values>-length <values> array to be returned, it
 *   stores all components derivatives and versions of <field> at <node>. Array
 *   is allocated by this function and must be be deallocated by the caller.
 * @return  1 if successfully returns a value otherwise 0.
 */
int get_FE_nodal_field_int_values(struct FE_field *field,
	struct FE_node *node, int *number_of_values, FE_value time, int **values);

int set_FE_nodal_field_int_values(struct FE_field *field,
	struct FE_node *node,int *values, int *number_of_values);
/*******************************************************************************
LAST MODIFIED : 30 August 1999

DESCRIPTION :
Sets the node field's values storage (at node->values_storage, NOT
field->values_storage) with the integers in values.
Returns the number of integers copied in number_of_values.
Assumes that values is set up with the correct number of ints.
Assumes that the node->values_storage has been allocated with enough
memory to hold all the values.
Assumes that the nodal fields have been set up, with information to
place the values.
==============================================================================*/

int get_FE_node_number_of_values(struct FE_node *node);
/*******************************************************************************
LAST MODIFIED : 29 October 1998

DESCRIPTION :
Returns the number of values stored at the <node>.
==============================================================================*/

int get_FE_node_number_of_fields(struct FE_node *node);
/*******************************************************************************
LAST MODIFIED : 30 October 1998

DESCRIPTION :
Returns the number of fields stored at the <node>.
==============================================================================*/

struct FE_time_sequence *get_FE_node_field_FE_time_sequence(struct FE_node *node,
	struct FE_field *field);
/*******************************************************************************
LAST MODIFIED : 15 November 2004

DESCRIPTION :
Returns the <fe_time_sequence> corresponding to the <node> and <field>.  If the
<node> and <field> have no time dependence then the function will return NULL.
==============================================================================*/

enum FE_nodal_value_type *get_FE_node_field_component_nodal_value_types(
	struct FE_node *node,struct FE_field *field,int component_number);
/*******************************************************************************
LAST MODIFIED : 21 September 1999

DESCRIPTION :
Returns an array of the (1+number_of_derivatives) value types for the
node field component.
It is up to the calling function to DEALLOCATE the returned array.
==============================================================================*/

int get_FE_node_field_component_number_of_derivatives(struct FE_node *node,
	struct FE_field *field,int component_number);
/*******************************************************************************
LAST MODIFIED : 21 September 1999

DESCRIPTION :
Returns the number of derivatives for the node field component.
==============================================================================*/

int get_FE_node_field_component_number_of_versions(struct FE_node *node,
	struct FE_field *field,int component_number);
/*******************************************************************************
LAST MODIFIED : 21 September 1999

DESCRIPTION :
Returns the number of versions for the node field component.
==============================================================================*/

int get_FE_node_identifier(struct FE_node *node);
/*******************************************************************************
LAST MODIFIED : 16 January 2003

DESCRIPTION :
Returns the integer identifier of the <node>.
==============================================================================*/

int set_FE_node_identifier(struct FE_node *node, int identifier);
/*******************************************************************************
LAST MODIFIED : 16 January 2003

DESCRIPTION :
Changes the identifier of <node> to <identifier>.
Caution! Should only call for nodes that are NOT in indexed lists;
Must wrap in LIST_BEGIN_IDENTIFIER_CHANGE/LIST_END_IDENTIFIER_CHANGE to ensure
node is temporarily removed from all the indexed lists it is in and re-added
afterwards. FE_region should be the only object that needs to call this.
==============================================================================*/

/***************************************************************************//**
 * Returns the first coordinate field define at the node, currently in
 * alphabetical order. Not reliable for finding the correct coordinate field
 * if multiple defined such as reference, deformed, texture coordinates.
 */
struct FE_field *get_FE_node_default_coordinate_field(struct FE_node *node);

int FE_node_find_default_coordinate_field_iterator(
	struct FE_node *node, void *fe_field_ptr_void);
/*******************************************************************************
LAST MODIFIED : 30 November 2001

DESCRIPTION :
An FE_node iterator that returns 1 when an appropriate default_coordinate
fe_field is found.  The fe_field found is returned as fe_field_void.
==============================================================================*/

/***************************************************************************//**
 * Writes to the console the node identifier and details of the fields and
 * parameters defined there.
 */
int list_FE_node(struct FE_node *node);

PROTOTYPE_LIST_FUNCTIONS(FE_node);

PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(FE_node,cm_node_identifier,int);

PROTOTYPE_CREATE_LIST_ITERATOR_FUNCTION(FE_node,cmzn_nodeiterator);

/***************************************************************************//**
 * Internal variant of public cmzn_nodeiterator_next() which does not access
 * the returned node, for more efficient if less safe usage.
 *
 * @param node_iterator  Node iterator to query and advance.
 * @return  Non-accessed pointer to the next node, or NULL if none remaining.
 */
cmzn_node_id cmzn_nodeiterator_next_non_access(
	cmzn_nodeiterator_id node_iterator);

/***************************************************************************//**
 * List statistics about btree efficiency for node list.
 */
void FE_node_list_write_btree_statistics(struct LIST(FE_node) *node_list);

PROTOTYPE_CHANGE_LOG_FUNCTIONS(FE_node);

/**
 * Creates map with value type and version arrays allocated.
 * Allocates memory and assigns fields for a standard node to element map.
 * Allocates storage for the nodal value and scale factor indices and sets to -1.
 */
Standard_node_to_element_map *Standard_node_to_element_map_create(
	int node_index, int number_of_nodal_values);

/**
 * Legacy variant of Standard_node_to_element_map_create which defines
 * node value index arrays to be converted into node value type and version
 * arrays at a later time.
 * Use only when reading from EX format.
 */
Standard_node_to_element_map *Standard_node_to_element_map_create_legacy(
	int node_index, int number_of_nodal_values);

/**
 * Frees the memory for the map and sets <*map_address> to NULL.
 */
int Standard_node_to_element_map_destroy(
	struct Standard_node_to_element_map **map_address);

/**
 * Returns the node index from <standard_node_map>.
 * If fails, sets *<node_index_address> to zero.
 */
int Standard_node_to_element_map_get_node_index(
	struct Standard_node_to_element_map *standard_node_map,
	int *node_index_address);

/**
 * Returns the number of nodal values used by <standard_node_map>.
 * If fails, sets *<number_of_nodal_values_address> to zero.
 */
int Standard_node_to_element_map_get_number_of_nodal_values(
	struct Standard_node_to_element_map *standard_node_map,
	int *number_of_nodal_values_address);

/**
 * Returns the nodal value index at <nodal_value_number> in <standard_node_map>.
 * If fails, sets *<nodal_value_index_address> to zero.
 * Legacy function, currently not called, but kept to help debug old EX files.
 */
int Standard_node_to_element_map_get_nodal_value_index(
	struct Standard_node_to_element_map *standard_node_map,
	int nodal_value_number, int *nodal_value_index_address);

/**
 * Sets nodal_value_index <nodal_value_number> of <standard_node_map> to
 * <nodal_value_index>.
 * Note a negative <nodal_value_index> gives a value of zero without needing to
 * get a value from the node.
 * Legacy function only used when reading old EX files.
 */
int Standard_node_to_element_map_set_nodal_value_index(
	struct Standard_node_to_element_map *standard_node_map,
	int nodal_value_number, int nodal_value_index);

/**
 * Returns the nodal value type at <nodal_value_number> in <standard_node_map>.
 * If none, returns FE_NODAL_UNKNOWN.
 */
FE_nodal_value_type Standard_node_to_element_map_get_nodal_value_type(
	struct Standard_node_to_element_map *standard_node_map,
	int nodal_value_number);

/**
 * Sets nodal_value_type <nodal_value_number> of <standard_node_map> to
 * <nodal_value_type>.
 * Must have allocated value types and versions.
 */
int Standard_node_to_element_map_set_nodal_value_type(
	struct Standard_node_to_element_map *standard_node_map,
	int nodal_value_number, FE_nodal_value_type nodal_value_type);

/**
 * Returns the version number of the nodal value obtained for
 * <nodal_value_number> in <standard_node_map>, starting at 1.
 * If none/error, returns 0.
 */
int Standard_node_to_element_map_get_nodal_version(
	struct Standard_node_to_element_map *standard_node_map,
	int nodal_value_number);

/**
 * Sets nodal version <nodal_value_number> of <standard_node_map> to
 * <version>.
 * Must have allocated value types and versions.
 * @param nodal_version  The nodal version to set, starting at 1.
 */
int Standard_node_to_element_map_set_nodal_version(
	struct Standard_node_to_element_map *standard_node_map,
	int nodal_value_number, int nodal_version);

/**
 * @return  The index into the scale factors stored for the basis in the
 * element of the scale factor used in mapping the nodal value number.
 * A return value of -1 means unscaled i.e. unit scale factor 1.0.
 * For invalid arguments also returns -1.
 */
int Standard_node_to_element_map_get_scale_factor_index(
	struct Standard_node_to_element_map *standard_node_map,
	int nodal_value_number);

/**
 * Sets scale_factor_index <nodal_value_number> of <standard_node_map> to
 * <scale_factor_index>.
 * Note a negative <scale_factor_index> gives a unit scale factor without
 * needing to get a value from the scale factor set.
 */
int Standard_node_to_element_map_set_scale_factor_index(
	struct Standard_node_to_element_map *standard_node_map,
	int nodal_value_number, int scale_factor_index);

struct FE_element_field_component *CREATE(FE_element_field_component)(
	enum Global_to_element_map_type type,int number_of_maps,
	struct FE_basis *basis,FE_element_field_component_modify modify);
/*******************************************************************************
LAST MODIFIED : 23 September 1995

DESCRIPTION :
Allocates memory and enters values for a component of a element field.
Allocates storage for the global to element maps and sets to NULL.
==============================================================================*/

int DESTROY(FE_element_field_component)(
	struct FE_element_field_component **component_address);
/*******************************************************************************
LAST MODIFIED : 23 September 1995

DESCRIPTION :
Frees the memory for the component and sets <*component_address> to NULL.
==============================================================================*/

/**
 * Creates and returns a copy of the supplied element field component. Due to
 * references to scale factor sets, the component is valid for use in the
 * current FE_mesh only.
 * @see FE_element_field_component_switch_FE_mesh
 */
struct FE_element_field_component *copy_create_FE_element_field_component(
	struct FE_element_field_component *source_component);

int FE_element_field_component_get_basis(
	struct FE_element_field_component *element_field_component,
	struct FE_basis **basis_address);
/*******************************************************************************
LAST MODIFIED : 5 November 2002

DESCRIPTION :
Gets the <basis> used by <element_field_component>.
If fails, puts NULL in *<basis_address> if supplied.
==============================================================================*/

int FE_element_field_component_get_grid_map_number_in_xi(
	struct FE_element_field_component *element_field_component,
	int xi_number, int *number_in_xi_address);
/*******************************************************************************
LAST MODIFIED : 5 November 2002

DESCRIPTION :
Gets the <number_in_xi> = number of spaces between grid points = one less than
the number of grid points on <xi_number> for <element_field_component> of type
ELEMENT_GRID_MAP. <xi_number> starts at 0 and must be less than the dimension
of the basis in <element_field_component>.
If fails, puts zero in *<number_in_xi_address> if supplied.
==============================================================================*/

int FE_element_field_component_set_grid_map_number_in_xi(
	struct FE_element_field_component *element_field_component,
	int xi_number, int number_in_xi);
/*******************************************************************************
LAST MODIFIED : 16 October 2002

DESCRIPTION :
Sets the <number_in_xi> = number of spaces between grid points = one less than
the number of grid points on <xi_number> for <element_field_component> of type
ELEMENT_GRID_MAP. <xi_number> starts at 0 and must be less than the dimension
of the basis in <element_field_component>. <number_in_xi> must be positive.
The number_in_xi must currently be unset for this <xi_number>.
==============================================================================*/

int FE_element_field_component_set_grid_map_value_index(
	struct FE_element_field_component *element_field_component, int value_index);
/*******************************************************************************
LAST MODIFIED : 16 October 2002

DESCRIPTION :
Sets the <value_index> = starting point in the element's value_storage for the
grid-based values for <element_field_component> of type ELEMENT_GRID_MAP.
<value_index> must be non-negative.
The value_index must currently be 0.
==============================================================================*/

int FE_element_field_component_get_modify(
	struct FE_element_field_component *element_field_component,
	FE_element_field_component_modify *modify_address);
/*******************************************************************************
LAST MODIFIED : 6 November 2002

DESCRIPTION :
Gets the <modify> function used by <element_field_component> -- can be NULL.
If fails, puts NULL in *<modify_address> if supplied.
==============================================================================*/

int FE_element_field_component_set_modify(
	struct FE_element_field_component *element_field_component,
	FE_element_field_component_modify modify);
/*******************************************************************************
LAST MODIFIED : 12 May 2003

DESCRIPTION :
Sets the <modify> function used by <element_field_component> -- can be NULL.
==============================================================================*/

int FE_element_field_component_get_number_of_nodes(
	struct FE_element_field_component *element_field_component,
	int *number_of_nodes_address);
/*******************************************************************************
LAST MODIFIED : 5 November 2002

DESCRIPTION :
Gets the number of local nodes for <element_field_component> of type
STANDARD_NODE_TO_ELEMENT_MAP.
If fails, puts zero in *<number_of_nodes_address> if supplied.
==============================================================================*/

/**
 * Fills array of flags (1=true, 0=false) indicating whether local node i is in
 * use by the component map.
 */
int FE_element_field_component_get_local_node_in_use(
	FE_element_field_component *component, int numberOfLocalNodes, int *localNodeInUse);

/** @return  Non-accessed pointer to scale factor set, or 0 if none or error */
cmzn_mesh_scale_factor_set *FE_element_field_component_get_scale_factor_set(
	FE_element_field_component *component);

/**
 * Set the scale factor set to use with the element field component.
 * Cannot be of type ELEMENT_GRID_MAP.
 *
 * @param component  The element field component to modify.
 * @param scale_factor_set  The scale factor set identifier, or 0 for none.
 * @return CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
int FE_element_field_component_set_scale_factor_set(
	FE_element_field_component *component, cmzn_mesh_scale_factor_set *scale_factor_set);

int FE_element_field_component_get_standard_node_map(
	struct FE_element_field_component *element_field_component, int node_number,
	struct Standard_node_to_element_map **standard_node_map_address);
/*******************************************************************************
LAST MODIFIED : 5 November 2002

DESCRIPTION :
Gets the <standard_node_map> relating global node values to those at local
<node_number> for <element_field_component> of type
STANDARD_NODE_TO_ELEMENT_MAP. <node_number> starts at 0 and must be less than
the number of nodes in the component.
If fails, puts NULL in *<standard_node_map_address> if supplied.
==============================================================================*/

int FE_element_field_component_set_standard_node_map(
	struct FE_element_field_component *element_field_component,
	int node_number, struct Standard_node_to_element_map *standard_node_map);
/*******************************************************************************
LAST MODIFIED : 16 October 2002

DESCRIPTION :
Sets the <standard_node_map> relating global node values to those at local
<node_number> for <element_field_component> of type
STANDARD_NODE_TO_ELEMENT_MAP. <node_number> starts at 0 and must be less than
the number of nodes in the component.
The standard_node_map must currently be unset for this <xi_number>.
On successful return <standard_node_map> will be owned by the component.
==============================================================================*/

int FE_element_field_component_get_type(
	struct FE_element_field_component *element_field_component,
	enum Global_to_element_map_type *type_address);
/*******************************************************************************
LAST MODIFIED : 5 November 2002

DESCRIPTION :
Returns the type of mapping used by <element_field_component>.
==============================================================================*/

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

int calculate_FE_element_field_values(struct FE_element *element,
	struct FE_field *field, FE_value time, char calculate_derivatives,
	struct FE_element_field_values *element_field_values,
	struct FE_element *top_level_element);
/*******************************************************************************
LAST MODIFIED : 30 November 2001

DESCRIPTION :
If <field> is NULL, element values are calculated for the coordinate field.  The
function fills in the fields of the <element_field_values> structure, but does
not allocate memory for the structure.
The optional <top_level_element> forces inheritance from it as needed.
==============================================================================*/

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

int FE_element_field_values_set_no_modify(
	struct FE_element_field_values *element_field_values);
/*******************************************************************************
LAST MODIFIED : 1 May 2003

DESCRIPTION :
Sets the FE_element_field_values no_modify flag.  When an element field values
structure is created, the no_modify flag is unset.
clear_FE_element_field_values also unsets the no_modify flag.

When calculate_FE_element_field_values is called, if the no_modify flag is set
then the field component modify function, if present, is not called.

???DB.  This was added to fix calculating nodal value derivatives for computed
	variables.  It was added as a set function because it is specialized and
	will hopefully be replaced (either by a specialized function for calculating
	nodal value derivatives instead of calculate_FE_element_field_values or a
	better way of doing the modify).
==============================================================================*/

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
 * If <field> is NULL, element nodes are calculated for the coordinate field.  The
 * function allocates an array, <*element_field_nodes_array_address> to store the
 * pointers to the ACCESS'd element nodes.  Components that are not node-based are
 * ignored.  The element nodes are ordered by increasing xi (fastest in xi1, next
 * fastest in xi2 and so on).
 * The optional <top_level_element> forces inheritance from it as needed.
 * NB.  The nodes need to be DEACCESS'd before the nodes array is DEALLOCATE'd.
 * @param face_number  If non-negative, calculate nodes for face number of
 * element, as if the face element were supplied to this function.
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
int FE_element_shape_get_number_of_faces(FE_element_shape *element_shape);

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

/**
 * Returns true if the only thing know about <element_shape> is its dimension.
 * Elements are only in this state when defined for element:xi locations.
 */
bool FE_element_shape_is_unspecified(struct FE_element_shape *element_shape);

int FE_element_shape_is_line(struct FE_element_shape *element_shape);
/*******************************************************************************
LAST MODIFIED : 12 March 2003

DESCRIPTION :
Returns true if the <element_shape> has only LINE_SHAPE in each dimension.
==============================================================================*/

struct FE_element_shape *get_FE_element_shape_of_face(
	struct FE_element_shape *shape,int face_number, struct FE_region *fe_region);
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
PROTOTYPE_COPY_OBJECT_FUNCTION(FE_element);

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

int equivalent_FE_fields_in_elements(struct FE_element *element_1,
	struct FE_element *element_2);
/*******************************************************************************
LAST MODIFIED : 10 September 2001

DESCRIPTION :
Returns true if all fields are defined in the same way at the two elements.
==============================================================================*/

/**
 * @return  The dimension of the element, or 0 if it cannot be determined.
 */
int get_FE_element_dimension(struct FE_element *element);

/**
 * @return  The non-negative element identifier, otherwise
 * DS_LABEL_IDENTIFIER_INVALID on error or if not a true element in the mesh.
 */
DsLabelIdentifier get_FE_element_identifier(struct FE_element *element);

/**
 * @return  The non-negative element index within the mesh otherwise
 * DS_LABEL_INDEX_INVALID on error or if not a true element in the mesh.
 */
DsLabelIndex get_FE_element_index(struct FE_element *element);

/**
 * Set the index of the element in the mesh. Used only by FE_mesh when
 * merging elements from another region's mesh.
 * @param element  The element to modify.
 * @param index  The new index, non-negative. Value is not checked due
 * to use by privileged caller.
 */
void set_FE_element_index(struct FE_element *element, DsLabelIndex index);

/**
 * Returns true if <element> or any of its parent elements is listed in the
 * element change logs with any of IDENTIFIER, DEFINITION or REMOVED flags.
 * Since element fields depend on node fields, the element is also considered as
 * changed if it or any of its parents uses a node listed in the
 * <fe_node_change_log> with OBJECT_IDENTIFIER_CHANGED and/or
 * OBJECT_NOT_IDENTIFIER_CHANGED.
 */
bool FE_element_or_parent_changed(struct FE_element *element,
	DsLabelsChangeLog *elementChangeLogs[MAXIMUM_ELEMENT_XI_DIMENSIONS],
	struct CHANGE_LOG(FE_node) *fe_node_change_log);

int get_FE_element_number_of_fields(struct FE_element *element);
/*******************************************************************************
LAST MODIFIED : 4 November 2002

DESCRIPTION :
Returns the number of fields defined at <element>.
Does not include fields inherited from parent elements.
==============================================================================*/

/**
 * Returns the <shape> of the <element>, if any. Invalid elements (in process
 * of being constructed or destroyed, or orphaned) have no shape.
 */
FE_element_shape *get_FE_element_shape(struct FE_element *element);

/**
 * Returns the <face_element> for face <face_number> of <element>, where NULL
 * means there is no face. Element must have a shape and face.
 */
struct FE_element *get_FE_element_face(struct FE_element *element, int face_number);

int set_FE_element_number_of_nodes(struct FE_element *element,
	int number_of_nodes);
/*******************************************************************************
LAST MODIFIED : 4 November 2002

DESCRIPTION :
Establishes storage for <number_of_nodes> in <element>. Can only be increased.
Should only be called for unmanaged elements.
==============================================================================*/

int get_FE_element_number_of_nodes(struct FE_element *element,
	int *number_of_nodes_address);
/*******************************************************************************
LAST MODIFIED : 19 December 2002

DESCRIPTION :
Returns the number of nodes directly referenced by <element>; does not include
nodes used by fields inherited from parent elements.
If fails, puts zero at <number_of_nodes_address>.
==============================================================================*/

int FE_element_has_FE_node(struct FE_element *element, void *node_void);
/*******************************************************************************
LAST MODIFIED : 13 May 2003

DESCRIPTION :
Returns true if <element> references the <node>.
==============================================================================*/

int get_FE_element_node(struct FE_element *element,int node_number,
	struct FE_node **node);
/*******************************************************************************
LAST MODIFIED : 10 November 1999

DESCRIPTION :
Gets node <node_number>, from 0 to number_of_nodes-1 of <element> in <node>.
<element> must already have a shape and node_scale_field_information.
==============================================================================*/

int set_FE_element_node(struct FE_element *element,int node_number,
	struct FE_node *node);
/*******************************************************************************
LAST MODIFIED : 11 October 1999

DESCRIPTION :
Sets node <node_number>, from 0 to number_of_nodes-1 of <element> to <node>.
<element> must already have a shape and node_scale_field_information.
Should only be called for unmanaged elements.
==============================================================================*/

/**
 * Get the number and address of the scale factors for the given set in element.
 * @return  The number of scale factors for the set, or 0 if none or invalid
 * arguments.
 */
int get_FE_element_scale_factors_address(struct FE_element *element,
	cmzn_mesh_scale_factor_set *scale_factor_set, FE_value **scale_factors_address);

/**
 * Establishes storage for <number_of_scale_factor_sets> in <element>, each
 * containing <numbers_in_scale_factor_sets> and identifier by
 * <scale_factor_set_identifiers>.
 * May only be set once; should only be called for unmanaged elements.
 * @return CMZN_OK on success, otherwise any other error code.
 */
int set_FE_element_number_of_scale_factor_sets(struct FE_element *element,
	int number_of_scale_factor_sets,
	cmzn_mesh_scale_factor_set **scale_factor_set_identifiers,
	int *numbers_in_scale_factor_sets);

/**
 * Returns the number of scale factor_sets in <element>.
 * If fails, puts zero at <number_of_scale_factor_sets_address>.
 */
int get_FE_element_number_of_scale_factor_sets(struct FE_element *element,
	int *number_of_scale_factor_sets_address);

/**
 * Returns the number of scale factors in <scale_factor_set_number> of <element>,
 * where <scale_factor_set_number> is from 0 to one less than the number of sets.
 */
int get_FE_element_number_in_scale_factor_set_at_index(struct FE_element *element,
	int scale_factor_set_number);

/**
 * Returns the identifier of <scale_factor_set_number> of <element>,
 * where <scale_factor_set_number> is from 0 to one less than the number of sets.
 * @return  Non-accessed pointer to mesh scale factor set, or 0 if none or error.
 */
cmzn_mesh_scale_factor_set *get_FE_element_scale_factor_set_identifier_at_index(
	struct FE_element *element, int scale_factor_set_number);

/**
 * Sets the identifier of <scale_factor_set_number> of <element>,
 * where <scale_factor_set_number> is from 0 to one less than the number of sets.
 * Be careful: only use when merging elements from another region.
 *
 * @param scale_factor_set_identifier  New scale factor set identifier from the
 * owning region of the element.
 * @return  CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
int set_FE_element_scale_factor_set_identifier_at_index(
	struct FE_element *element, int scale_factor_set_number,
	cmzn_mesh_scale_factor_set *scale_factor_set_identifier);

int get_FE_element_number_of_scale_factors(struct FE_element *element,
	int *number_of_scale_factors_address);
/*******************************************************************************
LAST MODIFIED : 5 November 2002

DESCRIPTION :
Returns the number of scale factors stored with <element>.
If fails, puts zero at <number_of_scale_factors_address>.
==============================================================================*/

int get_FE_element_scale_factor(struct FE_element *element,
	int scale_factor_number, FE_value *scale_factor_address);
/*******************************************************************************
LAST MODIFIED : 5 November 2002

DESCRIPTION :
Gets scale_factor <scale_factor_number>, from 0 to number_of_scale_factors-1 of
<element> to <scale_factor>.
<element> must already have a shape and node_scale_field_information.
If fails, sets *<scale_factor_address> to 0.0;
==============================================================================*/

/**
 * Sets scale_factor <scale_factor_number>, from 0 to number_of_scale_factors-1
 * of <element> to <scale_factor>.
 * <element> must already have a shape and node_scale_field_information.
 * Should only be called for unmanaged elements.
 * Dangerous; use with care.
 */
int set_FE_element_scale_factor(struct FE_element *element,
	int scale_factor_number, FE_value scale_factor);

int define_FE_field_at_element(struct FE_element *element,
	struct FE_field *field,struct FE_element_field_component **components);
/*******************************************************************************
LAST MODIFIED : 11 October 1999

DESCRIPTION :
Defines <field> at <element> using the given <components>. <element> must
already have a shape and node_scale_field_information.
Checks the range of nodes, scale factors etc. referred to by the components are
within the range of the node_scale_field_information, and that the basis
functions are compatible with the element shape.
If the components indicate the field is grid-based, checks that all the
components are grid-based with the same number_in_xi.
The <components> are duplicated by this functions, so the calling function must
destroy them.
Should only be called for unmanaged elements.
==============================================================================*/

int FE_element_has_grid_based_fields(struct FE_element *element);
/*******************************************************************************
LAST MODIFIED : 24 October 2002

DESCRIPTION :
Returns true if any of the fields defined for element is grid-based.
==============================================================================*/

int FE_element_field_is_standard_node_based(struct FE_element *element,
	struct FE_field *fe_field);
/*******************************************************************************
LAST MODIFIED : 12 March 2003

DESCRIPTION :
Returns true if <fe_field> is defined on <element> using a standard node to
element map for any element. Does not consider inherited fields.
==============================================================================*/

int FE_element_has_FE_field_values(struct FE_element *element);
/*******************************************************************************
LAST MODIFIED : 19 October 1999

DESCRIPTION :
Returns true if any single field defined at <element> has values stored with
the field. Returns 0 without error if no field information at element.
==============================================================================*/

int FE_element_has_values_storage(struct FE_element *element);
/*******************************************************************************
LAST MODIFIED : 24 October 2002

DESCRIPTION :
Returns true if <element> has values_storage, eg. for grid-based fields.
==============================================================================*/

int for_FE_field_at_element(struct FE_field *field,
	FE_element_field_iterator_function *iterator,void *user_data,
	struct FE_element *element);
/*******************************************************************************
LAST MODIFIED : 5 October 1999

DESCRIPTION :
If an <iterator> is supplied and the <field> is defined at the <element> then
the result of the <iterator> is returned.  Otherwise, if an <iterator> is not
supplied and the <field> is defined at the <element> then a non-zero is
returned. Otherwise, zero is returned.
???DB.  Multiple behaviour dangerous ?
==============================================================================*/

int for_each_FE_field_at_element(FE_element_field_iterator_function *iterator,
	void *user_data,struct FE_element *element);
/*******************************************************************************
LAST MODIFIED : 5 October 1999

DESCRIPTION :
Calls the <iterator> for each field defined at the <element> until the
<iterator> returns 0 or it runs out of fields.  Returns the result of the last
<iterator> called.
==============================================================================*/

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

/**
 * Merges/adds fields from <source> into <destination>. Where existing fields
 * in <destination> are passed in <source>, the <source> definition and values
 * replace them, otherwise the existing definition and values are maintained.
 * Function is atomic; <destination> is unchanged if <source> cannot be merged.
 * The <changed_fe_field_list> must be supplied. On return it contains the list
 * of FE_fields that have been changed or added to <destination>. Note it is
 * not sufficient to assume just the fields in <source> are changed since
 * changes to common scale factors affect different fields in <destination>;
 * the <change_fe_field_list> includes these fields.
 */
int merge_FE_element(struct FE_element *destination, struct FE_element *source,
	struct LIST(FE_field) *changed_fe_field_list);

/**
 * Writes to the console the element identifier and details of the fields
 * defined over it.
 */
int list_FE_element(struct FE_element *element);

/***************************************************************************//**
 * Internal variant of public cmzn_elementiterator_next() which does not
 * access the returned element, for more efficient if less safe usage.
 *
 * @param element_iterator  Element iterator to query and advance.
 * @return  Non-accessed pointer to the next element, or NULL if none remaining.
 */
cmzn_element_id cmzn_elementiterator_next_non_access(
	cmzn_elementiterator_id element_iterator);

int theta_closest_in_xi1(struct FE_element_field_component *component,
	struct FE_element *element,struct FE_field *field,FE_value time,
	int number_of_values,FE_value *values);
/*******************************************************************************
LAST MODIFIED : 1 February 2002

DESCRIPTION :
Calls modify_theta_in_xi1 with mode MODIFY_THETA_CLOSEST_IN_XI1.
???RC.  Needs to be global to allow writing function in export_finite_element.
==============================================================================*/

int theta_decreasing_in_xi1(struct FE_element_field_component *component,
	struct FE_element *element,struct FE_field *field,FE_value time,
	int number_of_values,FE_value *values);
/*******************************************************************************
LAST MODIFIED : 3 December 2001

DESCRIPTION :
Modifies the already calculated <values>.
???DB.  Only for certain bases
???RC.  Needs to be global to allow writing function in export_finite_element.
==============================================================================*/

int theta_increasing_in_xi1(struct FE_element_field_component *component,
	struct FE_element *element,struct FE_field *field,FE_value time,
	int number_of_values,FE_value *values);
/*******************************************************************************
LAST MODIFIED : 3 December 2001

DESCRIPTION :
Modifies the already calculated <values>.
???DB.  Only for certain bases
???RC.  Needs to be global to allow writing function in export_finite_element.
==============================================================================*/

int theta_non_decreasing_in_xi1(
	struct FE_element_field_component *component,struct FE_element *element,
	struct FE_field *field,FE_value time,int number_of_values,FE_value *values);
/*******************************************************************************
LAST MODIFIED : 3 December 2001

DESCRIPTION :
Modifies the already calculated <values>.
???DB.  Only for certain bases
???RC.  Needs to be global to allow writing function in export_finite_element.
==============================================================================*/

int theta_non_increasing_in_xi1(
	struct FE_element_field_component *component,struct FE_element *element,
	struct FE_field *field,FE_value time,int number_of_values,FE_value *values);
/*******************************************************************************
LAST MODIFIED : 3 December 2001

DESCRIPTION :
Modifies the already calculated <values>.
???DB.  Only for certain bases
???RC.  Needs to be global to allow writing function in export_finite_element.
==============================================================================*/

PROTOTYPE_ENUMERATOR_FUNCTIONS(CM_field_type);

int get_FE_field_external_information(struct FE_field *field,
	struct FE_field_external_information **external_information);
/*******************************************************************************
LAST MODIFIED : 2 September 2001

DESCRIPTION :
Creates a copy of the <external_information> of the <field>.
==============================================================================*/

int set_FE_field_external_information(struct FE_field *field,
	struct FE_field_external_information *external_information);
/*******************************************************************************
LAST MODIFIED : 2 September 2001

DESCRIPTION :
Copies the <external_information> into the <field>.

Should only call this function for unmanaged fields.
==============================================================================*/

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

int FE_field_copy_without_identifier(struct FE_field *destination,
	struct FE_field *source);
/*******************************************************************************
LAST MODIFIED : 16 December 2002

DESCRIPTION :
Copies the contents but not the name identifier of <source> to <destination>.
???RC Change to macro so identifier member can vary?
?COPY_WITHOUT_IDENTIFIER object_type,identifier
==============================================================================*/

int FE_field_matches_description(struct FE_field *field, const char *name,
	enum FE_field_type fe_field_type,struct FE_field *indexer_field,
	int number_of_indexed_values,enum CM_field_type cm_field_type,
	struct Coordinate_system *coordinate_system,enum Value_type value_type,
	int number_of_components,char **component_names,
	int number_of_times,enum Value_type time_value_type,
	struct FE_field_external_information *external);
/*******************************************************************************
LAST MODIFIED : 31 August 2001

DESCRIPTION :
Returns true if <field> has exactly the same <name>, <field_info>... etc. as
those given in the parameters.
==============================================================================*/

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

/***************************************************************************//**
 * Return true if any basis functions used by the field is non-linear i.e.
 * quadratic, cubic, Fourier etc.
 */
int FE_field_uses_non_linear_basis(struct FE_field *fe_field);

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

/***************************************************************************//**
 * @return  number of objects using fe_element.
 */
int FE_element_get_access_count(struct FE_element *fe_element);

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

/***************************************************************************//**
 * If the FE_field has value_type ELEMENT_XI_VALUE, this returns the
 * element dimension the field values are restricted to, or 0 if unrestricted.
 *
 * @param field  The field to query.
 * @return  Mesh dimension from 1 to MAXIMUM_ELEMENT_XI_DIMENSIONS or 0 for any
 * dimension or invalid field type.
 */
int FE_field_get_element_xi_mesh_dimension(struct FE_field *field);

/***************************************************************************//**
 * If the FE_field has value_type ELEMENT_XI_VALUE, this restricts the
 * element locations that can be stored to mesh of the supplied dimension.
 *
 * @param field  The field to modify.
 * @param mesh_dimension  A fixed mesh dimension from 1 to
 * MAXIMUM_ELEMENT_XI_DIMENSIONS or 0 for any dimension.
 * @return  1 on success, 0 on failure.
 */
int FE_field_set_element_xi_mesh_dimension(struct FE_field *field,
	int mesh_dimension);

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

int get_FE_field_double_array_value(struct FE_field *field, int value_number,
	double *array, int number_of_array_values);
/*******************************************************************************
LAST MODIFIED : 4 March 1999

DESCRIPTION :
Get the double array in field->values_storage specified by value_number, of
of length number_of_array_values. If number_of_array_values > the stored arrays
max length, gets the max length.
MUST allocate space for the array before calling this function.

Use get_FE_field_array_attributes() or get_FE_field_max_array_size()
to get the size of an array.
==============================================================================*/

int set_FE_field_double_array_value(struct FE_field *field, int value_number,
	double *array, int number_of_array_values);
/*******************************************************************************

DESCRIPTION :
Finds any existing double array at the place specified by  value_number in
field->values_storage.
Frees it.
Allocates a new array, according to number_of_array_values.
Copies the contents of the passed array to this allocated one.
Copies number of array values, and the pointer to the allocated array to the
specified place in the field->values_storage.

Therefore, should free the passed array, after passing it to this function

The field value MUST have been previously allocated with
set_FE_field_number_of_values
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

int get_FE_field_FE_value(struct FE_field *field,int number,FE_value *value);
/*******************************************************************************
LAST MODIFIED : 3 March 1999

DESCRIPTION :
Gets the specified global value for the <field>.
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

PROTOTYPE_GET_OBJECT_NAME_FUNCTION(FE_field_component);

int calculate_FE_field(struct FE_field *field,int component_number,
	struct FE_node *node,struct FE_element *element,FE_value *xi_coordinates,
	FE_value time, FE_value *value);
/*******************************************************************************
LAST MODIFIED : 3 December 2001

DESCRIPTION :
Calculates the <value> of the <field> for the specified <node> or <element> and
<xi_coordinates>.  If 0<=component_number<=number_of_components, then only the
specified component is calculated, otherwise all components are calculated.  The
storage for the <value> should have been allocated outside the function.
==============================================================================*/

/**
 * Returns the FE_mesh that <element> belongs to.
 */
FE_mesh *FE_element_get_FE_mesh(struct FE_element *element);

/**
 * Returns the FE_region that <element> belongs to.
 */
struct FE_region *FE_element_get_FE_region(struct FE_element *element);

/**
 * Returns true if <top_level_element> is a top_level parent of <element>.
 */
int FE_element_is_top_level_parent_of_element(
	struct FE_element *element, void *other_element_void);

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
 * Add nodes directly referenced in element information to the node list.
 * @param element  The element to add nodes from.
 * @param nodeList  The node list to fill.
 * @param onlyFromNodeList  List of nodes which can be added.
 * @return  CMZN_OK on success, otherwise any other value.
 */
int cmzn_element_add_stored_nodes_to_list(cmzn_element *element,
	LIST(cmzn_node) *nodeList, LIST(cmzn_node) *onlyFromNodeList);

/**
 * Add nodes used by this element, including inherited from parent elements, to
 * the supplied list.
 * @return  CMZN_OK on success, any other value on failure.
 */
int cmzn_element_add_nodes_to_list(cmzn_element *element, LIST(cmzn_node) *nodeList);

/**
 * Remove nodes used by this element, including inherited from parent elements,
 * from the supplied list.
 * @return CMZN_OK on success, any other value on failure.
 */
int cmzn_element_remove_nodes_from_list(cmzn_element *element, LIST(cmzn_node) *nodeList);

/**
 * Conditional function.
 * @return  1 if element is top-level i.e. has no parents, otherwise 0.
 */
int FE_element_is_top_level(struct FE_element *element,void *dummy_void);

/***************************************************************************//**
 * Evaluates the supplied coordinate_field. Sets non-present components to zero
 * (eg if only had x and y, z would be set to zero).  Converts to rectangular
 * Cartesian coordinates: x,y,z.
 * Note: Does not handle multiple versions.
 *
 * @param coordinate_field  The coordinate field to evaluate.
 * @param coordinate_jacobian  If supplied then fills with the Jacobian for the
 * transformation from native coordinates to rectangular Cartesian.
 * @return  1 on success, 0 on failure.
 */
int FE_node_get_position_cartesian(struct FE_node *node,
	struct FE_field *coordinate_field, FE_value *node_x, FE_value *node_y,
	FE_value *node_z, FE_value *coordinate_jacobian);

/***************************************************************************//**
 * Sets the position of <node> in Cartesian coordinates: x[,y[,z]] using the
 * supplied coordinate_field. The givenCartesian coordinates are converted into
 * the coordinate system of the coordinate_field.
 * Sets all versions.
 *
 * @param coordinate_field  The coordinate field to modify.
 * @return  1 on success, 0 on failure.
 */
int FE_node_set_position_cartesian(struct FE_node *node,
	struct FE_field *coordinate_field,
	FE_value node_x, FE_value node_y, FE_value node_z);

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

int FE_field_is_defined_at_node(struct FE_field *field, struct FE_node *node);
/*******************************************************************************
LAST MODIFIED : 4 May 1999

DESCRIPTION :
Returns true if the <field> is defined for the <node>.
==============================================================================*/

int FE_node_field_is_not_defined(struct FE_node *node,void *field_void);
/*******************************************************************************
LAST MODIFIED : 15 September 2000

DESCRIPTION :
FE_node iterator version of FE_field_is_defined_at_node.
==============================================================================*/

/**
 * Returns true if the <field> is defined for the <element>.
 */
bool FE_field_is_defined_in_element(struct FE_field *field,
	struct FE_element *element);

/**
 * Returns true if <field> is defined in <element> directly, not inherited from
 * a parent element it is a face of.
 */
bool FE_field_is_defined_in_element_not_inherited(struct FE_field *field,
	struct FE_element *element);

int FE_element_field_is_grid_based(struct FE_element *element,
	struct FE_field *field);
/*******************************************************************************
LAST MODIFIED : 5 October 1999

DESCRIPTION :
Returns true if <field> is grid-based in <element>. Only checks the first
component since we assume all subsequent components have the same basis and
numbers of grid cells in xi.
Returns 0 with no error if <field> is not defined over element or not element-
based in it.
==============================================================================*/

/***************************************************************************//**
 * If <field> <component_number> is grid-based in <element>, returns in
 * <number_in_xi> the numbers of finite difference cells in each xi-direction
 * of element (equals one less than number of grid points in each direction).
 * <number_in_xi> should be allocated with at least as much space as the number
 * of dimensions in <element>, but is assumed to have no more than
 * MAXIMUM_ELEMENT_XI_DIMENSIONS so that int
 * number_in_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS] can be passed to this function.
 */
int get_FE_element_field_component_grid_map_number_in_xi(struct FE_element *element,
	struct FE_field *field, int component_number, int *number_in_xi);

/***************************************************************************//**
 * Returns the number of element values used by a component of field in element.
 * If a component is grid based the number of element values is equal
 * to the product of (1+number_in_xi) for each direction. Per-element constant
 * components require 1 value. Nodally interpolated components require 0 values.
 */
int get_FE_element_field_component_number_of_grid_values(struct FE_element *element,
	struct FE_field *field, int component_number);

int get_FE_element_field_component(struct FE_element *element,
	struct FE_field *field, int component_number,
	struct FE_element_field_component **component_address);
/*******************************************************************************
LAST MODIFIED : 27 February 2003

DESCRIPTION :
Returns the element field component structure for <component_number> of <field>
at <element> if defined there; otherwise reports an error.
If fails, puts NULL in *<component_address> if supplied.
Note: returned component must not be modified or destroyed!
==============================================================================*/

#define PROTOTYPE_GET_FE_ELEMENT_FIELD_COMPONENT_FUNCTION( macro_value_type, value_enum ) \
int get_FE_element_field_component_grid_ ## macro_value_type ## _values( \
	struct FE_element *element,struct FE_field *field,int component_number, \
	macro_value_type **values);														\
/******************************************************************************* \
LAST MODIFIED : 22 April 2005 \
 \
DESCRIPTION : \
If <field> is grid-based in <element>, returns an allocated array of the grid \
values stored for <component_number>. To get number of values returned, call \
get_FE_element_field_component_number_of_grid_values; Grids change in xi0 fastest. \
It is up to the calling function to DEALLOCATE the returned values. \
==============================================================================*/

#define PROTOTYPE_SET_FE_ELEMENT_FIELD_COMPONENT_FUNCTION( macro_value_type, value_enum ) \
int set_FE_element_field_component_grid_ ## macro_value_type ## _values( \
	struct FE_element *element,struct FE_field *field,int component_number, \
	macro_value_type *values);															\
/******************************************************************************* \
LAST MODIFIED : 21 April 2005 \
\
DESCRIPTION : \
If <field> is grid-based in <element>, copies <values> into the values storage \
for <component_number>. To get number of values to pass, call \
get_FE_element_field_component_number_of_grid_values; Grids change in xi0 fastest. \
==============================================================================*/

#define PROTOTYPE_FE_ELEMENT_FIELD_COMPONENT_FUNCTIONS( macro_value_type , value_enum ) \
PROTOTYPE_GET_FE_ELEMENT_FIELD_COMPONENT_FUNCTION(macro_value_type,value_enum) \
PROTOTYPE_SET_FE_ELEMENT_FIELD_COMPONENT_FUNCTION(macro_value_type,value_enum)

PROTOTYPE_FE_ELEMENT_FIELD_COMPONENT_FUNCTIONS( FE_value , FE_VALUE_VALUE )
PROTOTYPE_FE_ELEMENT_FIELD_COMPONENT_FUNCTIONS( int , INT_VALUE )

int FE_element_field_get_component_FE_basis(struct FE_element *element,
	struct FE_field *field, int component_number, struct FE_basis **fe_basis);
/*******************************************************************************
LAST MODIFIED : 19 March 2003

DESCRIPTION :
If <field> is standard node based in <element>, returns the <fe_basis> used for
<component_number>.
==============================================================================*/

/**
 * Partner function to FE_element_smooth_FE_field.
 * Divides the nodal first derivatives of <fe_field> at <time> in <node> by the
 * corresponding integer from <element_count_fe_field>, then undefines
 * <element_count_fe_field>.
 * <node> is not a global object, as from FE_element_smooth_FE_field.
 * Handles multiple versions.
 */
int FE_node_smooth_FE_field(struct FE_node *node, struct FE_field *fe_field,
	FE_value time, struct FE_field *element_count_fe_field);

/**
 * For each node contributing to <fe_field> in <element> either its copy is found
 * in <copy_node_list> or one is made and added to this list. The copy node will
 * have <fe_field> defined on it in the same way as the original, plus the integer
 * <element_count_fe_field> with the same number of components as <fe_field> for
 * accumulating the number of elements each first derivative of <fe_field> is
 * referenced by. The copy node starts with all derivatives set to zero.
 *
 * After making calls to this function for all the intended elements, call
 * FE_node_smooth_FE_field for each node to divide the accumulated derivatives
 * by the number of elements they are over and to undefine the
 * <element_count_fe_field>.
 * It is up to the calling funcion to merge the copied information into global
 * elements and nodes.
 *
 * Sets all scale factors used for <fe_field> to 1.
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
int FE_element_smooth_FE_field(struct FE_element *element,
	struct FE_field *fe_field, FE_value time,
	struct FE_field *element_count_fe_field,
	struct LIST(FE_node) *copy_node_list);

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

int define_node_field_and_field_order_info(struct FE_node *node,
	struct FE_field *field, struct FE_node_field_creator *node_field_creator,
	struct FE_field_order_info *field_order_info);
/*******************************************************************************
LAST MODIFIED : 16 November 2001

DESCRIPTION :
Helper function for create_config_template_node() and
create_mapping_template_node() that, given the node, field and
field_order_info, defines the field at the node, and places it at the end of
the field_order_info list.
==============================================================================*/

struct FE_node_order_info *CREATE(FE_node_order_info)(
	int number_of_nodes);
/*******************************************************************************
LAST MODIFIED : 13 July 1999

DESCRIPTION :
Allocate space for an array of pointers to nodes of length number_of_nodes,
set these to NULL, copy the number_of_nodes.
==============================================================================*/

int DESTROY(FE_node_order_info)(
	struct FE_node_order_info **node_order_info_address);
/*******************************************************************************
LAST MODIFIED : 13 July 1999

DESCRIPTION :
Frees them memory used by node_order_info.
==============================================================================*/

PROTOTYPE_OBJECT_FUNCTIONS(FE_node_order_info);
PROTOTYPE_COPY_OBJECT_FUNCTION(FE_node_order_info);

int get_FE_node_order_info_number_of_nodes(
	struct FE_node_order_info *node_order_info);
/*******************************************************************************
LAST MODIFIED : 13 July 1999

DESCRIPTION :
Gets the <node_order_info> number_of_nodes
==============================================================================*/

struct FE_node *get_FE_node_order_info_node(
	struct FE_node_order_info *node_order_info,int node_number);
/*******************************************************************************
LAST MODIFIED : 13 July 1999

DESCRIPTION :
Gets the <node_order_info> node at the specified node_number
==============================================================================*/

int set_FE_node_order_info_node(
	struct FE_node_order_info *node_order_info,int node_number,
	struct FE_node *node);
/*******************************************************************************
LAST MODIFIED : 13 July 1999

DESCRIPTION :
Sets the <node_order_info> node at the specified node_number
==============================================================================*/

int get_FE_node_order_info_current_node_number(
	struct FE_node_order_info *node_order_info);
/*******************************************************************************
LAST MODIFIED : 17 August 2000

DESCRIPTION :
gets the <node_order_info> <current_node_number>
==============================================================================*/

int set_FE_node_order_info_current_node_number(
	struct FE_node_order_info *node_order_info,int current_node_number);
/*******************************************************************************
LAST MODIFIED : 17 August 2000

DESCRIPTION :
Sets the <node_order_info> <current_node_number>
==============================================================================*/

struct FE_node *get_FE_node_order_info_current_node(
	struct FE_node_order_info *node_order_info);
/*******************************************************************************
LAST MODIFIED : 17 August 2000

DESCRIPTION :
Gets the <node_order_info> node at the current_node_number
==============================================================================*/

struct FE_node *get_FE_node_order_info_next_node(
	struct FE_node_order_info *node_order_info);
/*******************************************************************************
LAST MODIFIED : 17 August 2000

DESCRIPTION :
Gets the <node_order_info> next node by incrementing the current_node_number,
and returning the new current node. If at the end of the array, return null.
==============================================================================*/

struct FE_node *get_FE_node_order_info_prev_node(
	struct FE_node_order_info *node_order_info);
/*******************************************************************************
LAST MODIFIED : 17 August 2000

DESCRIPTION :
Gets the <node_order_info> next node by incrementing the current_node_number,
and returning the new current node. If at the start of the array, return null.
==============================================================================*/

int add_nodes_FE_node_order_info(int number_of_nodes_to_add,
	struct FE_node_order_info *node_order_info);
/*******************************************************************************
LAST MODIFIED : 6 July 1999

DESCRIPTION :
As FE_node to previously created FE_node_order_info
==============================================================================*/

int fill_FE_node_order_info(struct FE_node *node,void *dummy);
/*******************************************************************************
LAST MODIFIED : 6 July 1999

DESCRIPTION :
As FE_node to previously created FE_node_order_info (passed in dummy)
==============================================================================*/

int get_FE_node_order_info_number_of_nodes(
	struct FE_node_order_info *node_order_info);
/*******************************************************************************
LAST MODIFIED : 13 July 1999

DESCRIPTION :
Gets the <node_order_info> number_of_nodes
==============================================================================*/

struct FE_node *get_FE_node_order_info_node(
	struct FE_node_order_info *node_order_info,int node_number);
/*******************************************************************************
LAST MODIFIED : 13 July 1999

DESCRIPTION :
Gets the <node_order_info> node at the specified node_number
==============================================================================*/

int set_FE_node_order_info_node(
	struct FE_node_order_info *node_order_info,int node_number,
	struct FE_node *node);
/*******************************************************************************
LAST MODIFIED : 13 July 1999

DESCRIPTION :
Sets the <node_order_info> node at the specified node_number
==============================================================================*/

int get_FE_node_order_info_current_node_number(
	struct FE_node_order_info *node_order_info);
/*******************************************************************************
LAST MODIFIED : 17 August 2000

DESCRIPTION :
gets the <node_order_info> <current_node_number>
==============================================================================*/

int set_FE_node_order_info_current_node_number(
	struct FE_node_order_info *node_order_info,int current_node_number);
/*******************************************************************************
LAST MODIFIED : 17 August 2000

DESCRIPTION :
Sets the <node_order_info> <current_node_number>
==============================================================================*/

struct FE_node *get_FE_node_order_info_current_node(
	struct FE_node_order_info *node_order_info);
/*******************************************************************************
LAST MODIFIED : 17 August 2000

DESCRIPTION :
Gets the <node_order_info> node at the current_node_number
==============================================================================*/

struct FE_node *get_FE_node_order_info_next_node(
	struct FE_node_order_info *node_order_info);
/*******************************************************************************
LAST MODIFIED : 17 August 2000

DESCRIPTION :
Gets the <node_order_info> next node by incrementing the current_node_number,
and returning the new current node. If at the end of the array, return null.
==============================================================================*/

struct FE_node *get_FE_node_order_info_prev_node(
	struct FE_node_order_info *node_order_info);
/*******************************************************************************
LAST MODIFIED : 17 August 2000

DESCRIPTION :
Gets the <node_order_info> next node by incrementing the current_node_number,
and returning the new current node. If at the start of the array, return null.
==============================================================================*/

int add_nodes_FE_node_order_info(int number_of_nodes_to_add,
	struct FE_node_order_info *node_order_info);
/*******************************************************************************
LAST MODIFIED : 6 July 1999

DESCRIPTION :
As FE_node to previously created FE_node_order_info
==============================================================================*/

int fill_FE_node_order_info(struct FE_node *node,void *dummy);
/*******************************************************************************
LAST MODIFIED : 6 July 1999

DESCRIPTION :
As FE_node to previously created FE_node_order_info (passed in dummy)
==============================================================================*/


struct FE_element_order_info *CREATE(FE_element_order_info)(
	int number_of_elements);
/*******************************************************************************
LAST MODIFIED : 10 January 2001

DESCRIPTION :
Allocate space for an array of pointers to elements of length number_of_elements,
set these to NULL, copy the number_of_elements.
==============================================================================*/

int DESTROY(FE_element_order_info)(
	struct FE_element_order_info **element_order_info_address);
/*******************************************************************************
LAST MODIFIED : 10 January 2001

DESCRIPTION :
Frees them memory used by element_order_info.
==============================================================================*/

PROTOTYPE_OBJECT_FUNCTIONS(FE_element_order_info);
PROTOTYPE_COPY_OBJECT_FUNCTION(FE_element_order_info);

int get_FE_element_order_info_number_of_elements(
	struct FE_element_order_info *element_order_info);
/*******************************************************************************
LAST MODIFIED : 10 January 2001

DESCRIPTION :
Gets the <element_order_info> number_of_elements
==============================================================================*/

struct FE_element *get_FE_element_order_info_element(
	struct FE_element_order_info *element_order_info,int element_number);
/*******************************************************************************
LAST MODIFIED : 10 January 2001

DESCRIPTION :
Gets the <element_order_info> element at the specified element_number
==============================================================================*/

int set_FE_element_order_info_element(
	struct FE_element_order_info *element_order_info,int element_number,
	struct FE_element *element);
/*******************************************************************************
LAST MODIFIED : 10 January 2001

DESCRIPTION :
Sets the <element_order_info> element at the specified element_number
==============================================================================*/

int get_FE_element_order_info_current_element_number(
	struct FE_element_order_info *element_order_info);
/*******************************************************************************
LAST MODIFIED : 10 January 2001

DESCRIPTION :
gets the <element_order_info> <current_element_number>
==============================================================================*/

int set_FE_element_order_info_current_element_number(
	struct FE_element_order_info *element_order_info,int current_element_number);
/*******************************************************************************
LAST MODIFIED : 10 January 2001

DESCRIPTION :
Sets the <element_order_info> <current_element_number>
==============================================================================*/

struct FE_element *get_FE_element_order_info_current_element(
	struct FE_element_order_info *element_order_info);
/*******************************************************************************
LAST MODIFIED : 10 January 2001

DESCRIPTION :
Gets the <element_order_info> element at the current_element_number
==============================================================================*/

struct FE_element *get_FE_element_order_info_next_element(
	struct FE_element_order_info *element_order_info);
/*******************************************************************************
LAST MODIFIED : 10 January 2001

DESCRIPTION :
Gets the <element_order_info> next element by incrementing the current_element_number,
and returning the new current element. If at the end of the array, return null.
==============================================================================*/

struct FE_element *get_FE_element_order_info_prev_element(
	struct FE_element_order_info *element_order_info);
/*******************************************************************************
LAST MODIFIED : 10 January 2001

DESCRIPTION :
Gets the <element_order_info> next element by incrementing the current_element_number,
and returning the new current element. If at the start of the array, return null.
==============================================================================*/

int add_elements_FE_element_order_info(int number_of_elements_to_add,
	struct FE_element_order_info *element_order_info);
/*******************************************************************************
LAST MODIFIED : 10 January 2001

DESCRIPTION :
As FE_element to previously created FE_element_order_info
==============================================================================*/

int fill_FE_element_order_info(struct FE_element *element,void *dummy);
/*******************************************************************************
LAST MODIFIED : 10 January 2001

DESCRIPTION :
As FE_element to previously created FE_element_order_info (passed in dummy)
==============================================================================*/

int FE_element_get_number_of_change_to_adjacent_element_permutations(
	struct FE_element *element, FE_value *xi, int face_number);
/*******************************************************************************
LAST MODIFIED : 8 June 2006

DESCRIPTION :
Returns the number of permutations known for the changing to the adjacent
element at face <face_number>.  It would be better to extend FE_element_shape
to take account of rotations and flipping.
==============================================================================*/

int FE_element_change_to_adjacent_element(struct FE_element **element_address,
	FE_value *xi, FE_value *increment, int *face_number, FE_value *xi_face,
	int permutation);
/*******************************************************************************
LAST MODIFIED : 31 May 2006

DESCRIPTION :
Steps into the adjacent element through face <face_number>, updating the
<element_address> location.
If <xi> is not NULL then the <xi_face> coordinates are converted to an xi
location in the new element.
If <increment> is not NULL then it is converted into an equvalent increment
in the new element.
<permutation> is used to resolve the possible rotation and flipping of the
local face xi coordinates between the two parents.
The shape mapping from parents are reused for all elements of the same shape
and do not take into account the relative orientation of the parents.  It would
be better to do so.  The range of possible permutations is from 0 to the value
returned from FE_element_get_number_of_change_to_adjacent_element_permutations.
==============================================================================*/

int FE_element_xi_increment(struct FE_element **element_address,FE_value *xi,
	FE_value *increment);
/*******************************************************************************
LAST MODIFIED : 21 January 2004

DESCRIPTION :
Adds the <increment> to <xi>.  If this moves <xi> outside of the element, then
if an adjacent element is found then the element and xi location are changed
to this element and the stepping continues using the remaining increment.  If
no adjacent element is found then the <xi> will be on the element boundary and
the <increment> will contain the fraction of the increment not used.
==============================================================================*/

int FE_element_define_tensor_product_basis(struct FE_element *element,
	int dimension, enum FE_basis_type basis_type, struct FE_field *field);
/*******************************************************************************
LAST MODIFIED : 1 December 2004

DESCRIPTION :
Defines a tensor product basis on <element> with the specified <dimension>
and <basis_type>.  This does not support mixed basis types in the tensor product.
==============================================================================*/

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
