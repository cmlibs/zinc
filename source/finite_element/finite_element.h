/*******************************************************************************
FILE : finite_element.h

LAST MODIFIED : 30 July 2003

DESCRIPTION :
The data structures used for representing finite elements in the graphical
interface to CMISS.
==============================================================================*/
#if !defined (FINITE_ELEMENT_H)
#define FINITE_ELEMENT_H

#include "api/cmiss_finite_element.h"
#include "command/parser.h"
#include "finite_element/finite_element_time.h"
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

struct FE_region;
/*******************************************************************************
LAST MODIFIED : 19 February 2003

DESCRIPTION :
FE_field, FE_node and FE_element maintain pointers to the FE_region that "owns"
them. Only the FE_field has a direct pointer; the other two include it via their
field_info structure.
==============================================================================*/

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

#if defined (OLD_CODE)
Shifted to API

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
	FE_NODAL_D_DS3,
	FE_NODAL_D2_DS1DS2,
	FE_NODAL_D2_DS1DS3,
	FE_NODAL_D2_DS2DS3,
	FE_NODAL_D3_DS1DS2DS3,
	FE_NODAL_UNKNOWN
}; /* enum FE_nodal_value_type */
#endif /* defined (OLD_CODE) */

struct FE_node_field_creator;

struct FE_node;

DECLARE_LIST_TYPES(FE_node);

DECLARE_CHANGE_LOG_TYPES(FE_node);

enum FE_basis_type
/*******************************************************************************
LAST MODIFIED : 20 October 1997

DESCRIPTION :
The different basis types available.
NOTE: Must keep this up to date with functions
FE_basis_type_string
==============================================================================*/
{
	FE_BASIS_TYPE_INVALID=-1,
	NO_RELATION=0,
		/*???DB.  Used on the off-diagonals of the type matrix */
	BSPLINE,
	CUBIC_HERMITE,
	CUBIC_LAGRANGE,
	FOURIER,
	HERMITE_LAGRANGE,
	LAGRANGE_HERMITE,
	LINEAR_LAGRANGE,
	LINEAR_SIMPLEX,
	POLYGON,
	QUADRATIC_LAGRANGE,
	QUADRATIC_SIMPLEX,
	SERENDIPITY,
	SINGULAR,
	TRANSITION
}; /* enum FE_basis_type */

typedef int (Standard_basis_function)(void *,FE_value *,FE_value *);

struct FE_basis;
/*******************************************************************************
LAST MODIFIED : 9 October 2002

DESCRIPTION :
Stores the information for calculating basis function values from xi
coordinates.  For each of basis there will be only one copy stored in a global
list.
==============================================================================*/

DECLARE_LIST_TYPES(FE_basis);

DECLARE_MANAGER_TYPES(FE_basis);

enum Global_to_element_map_type
/*******************************************************************************
LAST MODIFIED : 22 September 1998

DESCRIPTION :
Used for specifying the type of a global to element map.
==============================================================================*/
{
	STANDARD_NODE_TO_ELEMENT_MAP,
	GENERAL_NODE_TO_ELEMENT_MAP,
	FIELD_TO_ELEMENT_MAP,
	ELEMENT_GRID_MAP
}; /* enum Global_to_element_map_type */

struct Linear_combination_of_global_values;
/*******************************************************************************
LAST MODIFIED : 9 October 2002

DESCRIPTION :
Stores the information for calculating an element value as a linear combination
of global values.  The application of scale factors is one of the uses for this
linear combination.
==============================================================================*/

struct Standard_node_to_element_map;
/*******************************************************************************
LAST MODIFIED : 9 October 2002

DESCRIPTION :
Stores the information for calculating element values by choosing nodal values
and applying a diagonal scale factor matrix.  The <nodal_values> and
<scale_factors> are stored as offsets so that the arrays stored with the nodes
and elements can be reallocated.
==============================================================================*/

struct General_node_to_element_map;
/*******************************************************************************
LAST MODIFIED : 9 October 2002

DESCRIPTION :
Stores the information for calculating element values by choosing nodal values
and applying a general scale factor matrix.  The <nodal_values> and
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
cases, such as CMISS nodes that have multiple theta values in cylindrical polar,
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

enum CM_element_type
/*******************************************************************************
LAST MODIFIED : 25 January 1999

DESCRIPTION :
CM element types.
==============================================================================*/
{
  CM_ELEMENT_TYPE_INVALID,
  CM_ELEMENT,
  CM_FACE,
  CM_LINE
}; /* enum CM_element_type */

struct CM_element_information
/*******************************************************************************
LAST MODIFIED : 25 January 1999

DESCRIPTION :
Element information needed by CM.
==============================================================================*/
{
  enum CM_element_type type;
  int number;
}; /* struct CM_element_information */

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

struct FE_element;
/*******************************************************************************
LAST MODIFIED : 9 October 2002

DESCRIPTION :
A region in space with functions defined on the region.  The region is
parameterized and the functions are known in terms of the parameterized
variables.
==============================================================================*/

DECLARE_LIST_TYPES(FE_element);

DECLARE_CHANGE_LOG_TYPES(FE_element);

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

struct FE_element_list_FE_node_list_data
/*******************************************************************************
LAST MODIFIED : 1 June 2001

DESCRIPTION :
Structure for passing an FE_element list and an FE_node list to a list iterator
or conditional function, eg. add_FE_element_using_node_list_to_list.
==============================================================================*/
{
	struct LIST(FE_element) *element_list;
	struct LIST(FE_node) *node_list;
};

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

int FE_node_field_creator_define_derivative(
	struct FE_node_field_creator *node_field_creator, int component_number,
	enum FE_nodal_value_type derivative_type);
/*******************************************************************************
LAST MODIFIED: 16 November 2001

DESCRIPTION:
Adds the derivative of specified <derivative_type> to the <component_number>
specified.
==============================================================================*/

int FE_node_field_creator_define_versions(
	struct FE_node_field_creator *node_field_creator, int component_number,
	int number_of_versions);
/*******************************************************************************
LAST MODIFIED: 16 November 2001

DESCRIPTION:
Specifies the <number_of_versions> for <component_number> specified.
==============================================================================*/

int FE_node_field_creator_get_nodal_derivative_versions(
	struct FE_node_field_creator *node_field_creator, 
	int *number_of_derivatives, 
	enum FE_nodal_value_type **nodal_derivative_types,int **max_versions);
/*******************************************************************************
LAST MODIFIED: 11 February 2002

DESCRIPTION:
Given <node_field_creator>, returns <number_of_derivatives>
which is the size of the arrays <nodal_derivative_types> (containing the 
FE_nodal_value_type of the derivates present at the node_field_creator) 
and <max_versions> (which contains the maximum number of versions of each 
derivative type).
Note: This function allocates the arrays nodal_derivative_types and max_versions, 
the user is responsible for freeing them.
==============================================================================*/

struct FE_node *CREATE(FE_node)(int cm_node_identifier,
	struct FE_region *fe_region, struct FE_node *template_node);
/*******************************************************************************
LAST MODIFIED : 19 February 2003

DESCRIPTION :
Creates and returns a node with the specified <cm_node_identifier>.
If <fe_region> is supplied a blank node with the given identifier but no fields
is returned. If <template_node> is supplied, a copy of it, including all fields
and values but with the new identifier, is returned.
Exactly one of <fe_region> or <template_node> must be supplied.
The new node is set to belong to the ultimate master FE_region of <fe_region>
if supplied, or to the same master FE_region as <template_node> if supplied.
Note that <cm_node_identifier> must be non-negative.
==============================================================================*/

int DESTROY(FE_node)(struct FE_node **node_address);
/*******************************************************************************
LAST MODIFIED : 23 September 1995

DESCRIPTION :
Frees the memory for the node, sets <*node_address> to NULL.
==============================================================================*/

PROTOTYPE_OBJECT_FUNCTIONS(FE_node);
PROTOTYPE_COPY_OBJECT_FUNCTION(FE_node);
PROTOTYPE_GET_OBJECT_NAME_FUNCTION(FE_node);

struct FE_node *FE_node_copy_with_FE_field_list(struct FE_node *node,
	struct LIST(FE_field) *fe_field_list);
/*******************************************************************************
LAST MODIFIED : 6 March 2003

DESCRIPTION :
Creates a copy of <node> containing only FE_node_fields from <node> which have
their FE_field listed in <fe_field_list>.
==============================================================================*/

int define_FE_field_at_node(struct FE_node *node,struct FE_field *field,
	struct FE_time_version *fe_time_version, 
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

int undefine_FE_field_at_node(struct FE_node *node,struct FE_field *field);
/*******************************************************************************
LAST MODIFIED : 13 September 2000

DESCRIPTION :
Removes definition of <field> at <node>. If field is of type GENERAL_FE_FIELD
then removes values storage for it and shifts values storage for all subsequent
fields down.
Note: Must ensure that the node field is not in-use by any elements before it
is undefined!
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

int for_each_FE_field_at_node_indexer_first(
	FE_node_field_iterator_function *iterator,void *user_data,
	struct FE_node *node);
/*******************************************************************************
LAST MODIFIED : 21 September 1999

DESCRIPTION :
Calls the <iterator> for each field defined at the <node> until the <iterator>
returns 0 or it runs out of fields.  Returns the result of the last <iterator>
called. This version insists that any field used as an indexer_field for another
field in the list is output first.
==============================================================================*/

int FE_node_has_FE_field_values(struct FE_node *node);
/*******************************************************************************
LAST MODIFIED : 24 September 1999

DESCRIPTION :
Returns true if any single field defined at <node> has values stored with
the field.
==============================================================================*/

struct FE_region *FE_node_get_FE_region(struct FE_node *node);
/*******************************************************************************
LAST MODIFIED : 13 February 2003

DESCRIPTION :
Returns the FE_region that <node> belongs to.
==============================================================================*/

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
	struct FE_field_component *component,int version,
	enum FE_nodal_value_type type);
/*******************************************************************************
LAST MODIFIED : 23 June 1999

DESCRIPTION :
Returns 1 if the field, component number, version and type are stored at the
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
	struct FE_field_component *component,int version, \
	enum FE_nodal_value_type type, FE_value time, value_type *value); \
/******************************************************************************* \
LAST MODIFIED : 30 January 2002 \
 \
DESCRIPTION : \
Gets a particular value (<version>, <type>) for the field <component> \
at the <node> and <time>. \
???DB.  May need speeding up \
==============================================================================*/

#define PROTOTYPE_SET_FE_NODAL_VALUE_FUNCTION( value_type, value_enum ) \
int set_FE_nodal_ ## value_type ## _value(struct FE_node *node, \
	struct FE_field_component *component,int version, \
	enum FE_nodal_value_type type, FE_value time, value_type value); \
/******************************************************************************* \
LAST MODIFIED : 30 January 2002 \
 \
DESCRIPTION : \
Sets a particular value (<version>, <type>) for the field <component> \
at the <node>. \
==============================================================================*/

#define PROTOTYPE_FE_NODAL_VALUE_FUNCTIONS( value_type , value_enum ) \
PROTOTYPE_GET_FE_NODAL_VALUE_FUNCTION(value_type,value_enum) \
PROTOTYPE_SET_FE_NODAL_VALUE_FUNCTION(value_type,value_enum)

PROTOTYPE_FE_NODAL_VALUE_FUNCTIONS( FE_value , FE_VALUE_VALUE )
PROTOTYPE_FE_NODAL_VALUE_FUNCTIONS( double , DOUBLE_VALUE )
PROTOTYPE_FE_NODAL_VALUE_FUNCTIONS( float , FLOAT_VALUE )
PROTOTYPE_FE_NODAL_VALUE_FUNCTIONS( int , INT_VALUE )
PROTOTYPE_FE_NODAL_VALUE_FUNCTIONS( short , SHORT_VALUE )

int get_FE_nodal_FE_value_array_value_at_FE_value_time(struct FE_node *node,
	struct FE_field_component *component,int version,
	enum FE_nodal_value_type type,FE_value time,FE_value *value);
/*******************************************************************************
LAST MODIFIED : 4 October 1999

DESCRIPTION :
Gets the FE_value <value> from the node's FE_value array at the given
<component> <version> <type>, using the <time> to index the array.
The field must have time defined for it, and the number of times must match
the number of array elements. If <time> is within the node field's time array's 
range, but doesn't correspond exactly to an array element, interpolates to determine 
<value>.
==============================================================================*/

int get_FE_nodal_short_array_value_at_FE_value_time(struct FE_node *node,
	struct FE_field_component *component,int version,
	enum FE_nodal_value_type type,FE_value time,short *value);
/*******************************************************************************
LAST MODIFIED : 4 October 1999

DESCRIPTION :
Gets the short <value> from the node's short array at the given
<component> <version> <type>, using the <time> to index the array.
The field must have time defined for it, and the number of times must match
the number of array elements. If <time> is within the node field's time array's 
range, but doesn't correspond exactly to an array element, interpolates to determine 
<value>.
==============================================================================*/

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

int set_FE_nodal_element_xi_value(struct FE_node *node,
	struct FE_field *field, int component_number, int version,
	enum FE_nodal_value_type type,struct FE_element *element, FE_value *xi);
/*******************************************************************************
LAST MODIFIED : 23 April 1999

DESCRIPTION :
Sets a particular element_xi_value (<version>, <type>) for the field <component> at the
<node>.  SAB Note: It doesn't use a FE_field_component as I don't think any of them
should.
==============================================================================*/

struct FE_node_conditional_iterator_data
/*******************************************************************************
LAST MODIFIED : 15 January 2003

DESCRIPTION :
Data for passing to FE_node_conditional_iterator function.
==============================================================================*/
{
	LIST_CONDITIONAL_FUNCTION(FE_node) *conditional_function;
	void *conditional_user_data;
	LIST_ITERATOR_FUNCTION(FE_node) *iterator_function;
	void *iterator_user_data;
}; /* struct FE_node_conditional_iterator_data */

int FE_node_conditional_iterator(struct FE_node *node,
	void *data_void);
/*******************************************************************************
LAST MODIFIED : 15 January 2003

DESCRIPTION :
If <node> satisfies the <conditional_function> with <conditional_user_data>,
calls <iterator_function> with it and the <iterator_user_data>.
<data_void> points at a struct FE_node_conditional_iterator_data.
==============================================================================*/

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

struct FE_node_is_embedded_in_changed_element_data
/*******************************************************************************
LAST MODIFIED : 25 May 2001

DESCRIPTION :
Data for passing to FE_node_is_embedded_in_changed_element function.
Keeps the current <node> and the <changed_element_list> or <changed_node_list>.
==============================================================================*/
{
	/* node is used internally; do not have to set it */
	struct FE_node *node;
	struct LIST(FE_element) *changed_element_list;
	struct LIST(FE_node) *changed_node_list;
}; /* struct FE_node_is_embedded_in_changed_element_data */

int FE_node_is_embedded_in_changed_element(struct FE_node *node,
	void *data_void);
/*******************************************************************************
LAST MODIFIED : 25 May 2001

DESCRIPTION :
Returns true if <node> contains a field which is embedded in one of the elements
in the <changed_element_list>, or in any elements using nodes from the
<changed_node_list>, both passed in the <data_void>.
==============================================================================*/

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

enum Value_type get_FE_nodal_value_type(struct FE_node *node,
	struct FE_field_component *component,int version);
/*******************************************************************************
LAST MODIFIED : 4 October 1999

DESCRIPTION :
Get's a node's field's value type
==============================================================================*/

int get_FE_nodal_array_number_of_elements(struct FE_node *node,
	struct FE_field_component *component,int version,
	enum FE_nodal_value_type type);
/*******************************************************************************
LAST MODIFIED : 4 October 1999

DESCRIPTION :
Returns the number of elements  for the array in the nodal 
values_storage for the given node, component, version,type.

Returns -1 upon error.

Give an error if field->values_storage isn't storing array types.
==============================================================================*/

int get_FE_nodal_double_array_value(struct FE_node *node,
	struct FE_field_component *component,int version,
	enum FE_nodal_value_type type,double *array, int number_of_array_values);
/*******************************************************************************
LAST MODIFIED : 22 March 1999

DESCRIPTION :
Gets a particular double array value (<version>, <type>) for the field <component> 
at the <node> of length number_of_array_values. 
If number_of_array_values > the stored arrays max length, gets the max length.
MUST allocate space for the array before calling this function.

Use get_FE_nodal_array_attributes() to get the size of an array.
??JW write get_FE_nodal_max_array_size() to avoid many ALLOCATES 
if need to locally and repetatively get many arrays.  
==============================================================================*/

int set_FE_nodal_double_array_value(struct FE_node *node,
	struct FE_field_component *component,int version,
	enum FE_nodal_value_type type,double *array,int number_of_array_values);
/*******************************************************************************
LAST MODIFIED : 22 March 1999

DESCRIPTION :
Finds any existing double array at the place specified by (<version>, <type>) 
for the field <component>  at the <node>.

Frees it.
Allocates a new array, according to number_of_array_values. 
Copies the contents of the passed array to this allocated one.
Copies number of array values, and the pointer to the allocated array to the specified 
place in the node->values_storage. 

Therefore, should free the passed array, after passing it to this function.

The nodal values_storage MUST have been previously allocated within 
define_FE_field_at_node()
==============================================================================*/

int get_FE_nodal_short_array(struct FE_node *node,
	struct FE_field_component *component,int version,
	enum FE_nodal_value_type type,short *array, int number_of_array_values);
/*******************************************************************************
LAST MODIFIED : 8 June 1999

DESCRIPTION :
Gets a particular short array value (<version>, <type>) for the field <component> 
at the <node> of length number_of_array_values. 
If number_of_array_values > the stored arrays max length, gets the max length.
MUST allocate space for the array before calling this function.

Use get_FE_nodal_array_attributes() to get the size of an array.
??JW write get_FE_nodal_max_array_size() to avoid many ALLOCATES 
if need to locally and repetatively get many arrays.  

==============================================================================*/

int set_FE_nodal_short_array(struct FE_node *node,
	struct FE_field_component *component,int version,
	enum FE_nodal_value_type type,short *array,int number_of_array_values);
/*******************************************************************************
LAST MODIFIED : 8 June 1999

DESCRIPTION :
Finds any existing short array at the place specified by (<version>, <type>) 
for the field <component>  at the <node>.

Frees it.
Allocates a new array, according to number_of_array_values. 
Copies the contents of the passed array to this allocated one.
Copies number of array values, and the pointer to the allocated array to the specified 
place in the node->values_storage. 

Therefore, should free the passed array, after passing it to this function.

The nodal values_storage MUST have been previously allocated within 
define_FE_field_at_node()
==============================================================================*/

int get_FE_nodal_FE_value_array(struct FE_node *node,
	struct FE_field_component *component,int version,
	enum FE_nodal_value_type type,FE_value *array, int number_of_array_values);
/*******************************************************************************
LAST MODIFIED : 8 June 1999

DESCRIPTION :
Gets a particular FE_value array value (<version>, <type>) for the field <component> 
at the <node> of length number_of_array_values. 
If number_of_array_values > the stored arrays max length, gets the max length.
MUST allocate space for the array before calling this function.

Use get_FE_nodal_array_attributes() to get the size of an array.
??JW write get_FE_nodal_max_array_size() to avoid many ALLOCATES 
if need to locally and repetatively get many arrays.  
==============================================================================*/

int set_FE_nodal_FE_value_array(struct FE_node *node,
	struct FE_field_component *component,int version,
	enum FE_nodal_value_type type,FE_value *array,int number_of_array_values);
/*******************************************************************************
LAST MODIFIED : 8 June 1999

DESCRIPTION :
Finds any existing FE_value array at the place specified by (<version>, <type>) 
for the field <component>  at the <node>.

Frees it.
Allocates a new array, according to number_of_array_values. 
Copies the contents of the passed array to this allocated one.
Copies number of array values, and the pointer to the allocated array to the specified 
place in the node->values_storage. 

Therefore, should free the passed array, after passing it to this function.

The nodal values_storage MUST have been previously allocated within 
define_FE_field_at_node()
==============================================================================*/

FE_value get_FE_nodal_FE_value_array_element(struct FE_node *node,
	struct FE_field_component *component,int version,
	enum FE_nodal_value_type type,int element_number);
/*******************************************************************************
LAST MODIFIED : 4 October 1999

DESCRIPTION :
Gets a particular FE_value_array_value element for the array 
(<version>, <type>) for the field <component> 
at the <node>. 

==============================================================================*/

FE_value get_FE_nodal_FE_value_array_interpolated_value(struct FE_node *node,
	struct FE_field_component *component,int version,
	enum FE_nodal_value_type type,int element_number,FE_value proportion);
/*******************************************************************************
LAST MODIFIED : 8 May 2000

DESCRIPTION :
returns the interpolated value between the array (*element_number= value_low) 
and *(element_number+1)=value_high, according to 
value=<proportion>*value_low+(1-<proportion>)*value_high;
==============================================================================*/

int set_FE_nodal_FE_value_array_element(struct FE_node *node,
	struct FE_field_component *component,int version,
	enum FE_nodal_value_type type,int element_number,FE_value value);
/*******************************************************************************
LAST MODIFIED : 4 October 1999

DESCRIPTION :
sets a particular FE_value_array_value element for the array 
(<version>, <type>) for the field <component> 
at the <node>. 

==============================================================================*/

short get_FE_nodal_short_array_element(struct FE_node *node,
	struct FE_field_component *component,int version,
	enum FE_nodal_value_type type,int element_number);
/*******************************************************************************
LAST MODIFIED : 5 October 1999

DESCRIPTION :
Gets a particular short_array_value element for the array 
(<version>, <type>) for the field <component> 
at the <node>. 
==============================================================================*/

short get_FE_nodal_short_array_interpolated_value(struct FE_node *node,
	struct FE_field_component *component,int version,
	enum FE_nodal_value_type type,int element_number,FE_value proportion);
/*******************************************************************************
LAST MODIFIED : 8 May 2000

DESCRIPTION :
returns the interpolated value between the array (*element_number= value_low) 
and *(element_number+1)=value_high, according to 
value=<proportion>*value_low+(1-<proportion>)*value_high;
==============================================================================*/

int set_FE_nodal_short_array_element(struct FE_node *node,
	struct FE_field_component *component,int version,
	enum FE_nodal_value_type type,int element_number,short value);
/*******************************************************************************
LAST MODIFIED : 4 October 1999

DESCRIPTION :
sets a particular short_array_value element for the array 
(<version>, <type>) for the field <component> 
at the <node>. 

==============================================================================*/

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

int get_FE_nodal_FE_value_array_min_max(struct FE_node *node,
	struct FE_field_component *component,int version,
	enum FE_nodal_value_type type,FE_value *minimum,FE_value *maximum);
/*******************************************************************************
LAST MODIFIED : 29 September 1999

DESCRIPTION :
Gets a the minimum and maximum values for the FE_value_array 
(<version>, <type>) for the field <component> at the <node>
==============================================================================*/

int get_FE_nodal_short_array_min_max(struct FE_node *node,
	struct FE_field_component *component,int version,
	enum FE_nodal_value_type type,short *minimum,short *maximum);
/*******************************************************************************
LAST MODIFIED : 29 September 1999

DESCRIPTION :
Gets a the minimum and maximum values for the _value_array 
(<version>, <type>) for the field <component> at the <node>
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
	struct FE_field *field,int component_number,int version,
	enum FE_nodal_value_type type,char *string);
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

int get_FE_nodal_field_FE_value_values(struct FE_field *field,
	struct FE_node *node,int *number_of_values,FE_value **values);
/*******************************************************************************
LAST MODIFIED : 20 September 1999

DESCRIPTION :
Allocates and returns a copy of the <number_of_values>-length <values> array
stored at the <node> for all components derivatives and versions of <field>.
It is up to the calling function to DEALLOCATE the returned array.
==============================================================================*/

int set_FE_nodal_field_FE_value_values(struct FE_field *field,
	struct FE_node *node,FE_value *values, int *number_of_values);
/*******************************************************************************
LAST MODIFIED : 30 August 1999

DESCRIPTION :
Sets the node field's values storage (at node->values_storage, NOT 
field->values_storage) with the FE_values in values. 
Returns the number of FE_values copied in number_of_values.
Assumes that values is set up with the correct number of FE_values.
Assumes that the node->values_storage has been allocated with enough 
memory to hold all the values.
Assumes that the nodal fields have been set up, with information to 
place the values.
==============================================================================*/

int set_FE_nodal_field_FE_values_at_time(struct FE_field *field,
  struct FE_node *node,FE_value *values,int *number_of_values,
  FE_value time);
/*******************************************************************************
LAST MODIFIED : 15 November 2001

DESCRIPTION :
Sets The Node Field'S Values Storage (At Node->Values_Storage, Not 
Field->Values_Storage) With The Fe_Values In Values. 
Returns The Number Of Fe_Values Copied In Number_Of_Values.
Assumes That Values Is Set Up With The Correct Number Of Fe_Values.
Assumes That The Node->Values_Storage Has Been Allocated With Enough 
Memory To Hold All The Values.
Assumes That The Nodal Fields Have Been Set Up, With Information To 
Place The Values.
==============================================================================*/

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

int get_FE_nodal_field_int_values(struct FE_field *field,
	struct FE_node *node,int *number_of_values,int **values);
/*******************************************************************************
LAST MODIFIED : 21 September 1999

DESCRIPTION :
Allocates and returns a copy of the <number_of_values>-length <values> array
stored at the <node> for all components derivatives and versions of <field>.
It is up to the calling function to DEALLOCATE the returned array.
==============================================================================*/

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

int get_FE_node_access_count(struct FE_node *node);
/*******************************************************************************
LAST MODIFIED : 26 September 2000

DESCRIPTION :
Returns the acccess count of the <node>.
Useful for drawing the access count as a field when trying to debug why things
cannot be destroyed.
==============================================================================*/

struct FE_field *get_FE_node_default_coordinate_field(struct FE_node *node);
/*******************************************************************************
LAST MODIFIED : 3 November 1998

DESCRIPTION :
Returns the default coordinate field of the <node>.
==============================================================================*/

int FE_node_find_default_coordinate_field_iterator(
	struct FE_node *node, void *fe_field_ptr_void);
/*******************************************************************************
LAST MODIFIED : 30 November 2001

DESCRIPTION :
An FE_node iterator that returns 1 when an appropriate default_coordinate
fe_field is found.  The fe_field found is returned as fe_field_void.
==============================================================================*/

int merge_FE_node(struct FE_node *destination, struct FE_node *source);
/*******************************************************************************
LAST MODIFIED : 23 October 2002

DESCRIPTION :
Merges the fields from <source> into <destination>. Existing fields in the
<destination> keep the same node field description as before with new field
storage following them. Where existing fields in <destination> are passed in
<source>, values from <source> take precedence, but the node field structure
remains unchanged.
Function is atomic; <destination> is unchanged if <source> cannot be merged.
???RC Move to finite_element_private.h?
==============================================================================*/

int list_FE_node(struct FE_node *node,void *list_node_information);
/*******************************************************************************
LAST MODIFIED : 9 February 1998

DESCRIPTION :
Outputs the information contained at the node.
==============================================================================*/

PROTOTYPE_LIST_FUNCTIONS(FE_node);

PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(FE_node,cm_node_identifier,int);

PROTOTYPE_CHANGE_LOG_FUNCTIONS(FE_node);

struct FE_basis *CREATE(FE_basis)(int *type);
/*******************************************************************************
LAST MODIFIED : 1 October 1995

DESCRIPTION :
A basis is created with the specified <type> (duplicated).  The basis is
returned.
==============================================================================*/

int DESTROY(FE_basis)(struct FE_basis **basis_address);
/*******************************************************************************
LAST MODIFIED : 23 September 1995

DESCRIPTION :
Frees the memory for <**basis_address> and sets <*basis_address> to NULL.
==============================================================================*/

struct FE_basis *make_FE_basis(int *basis_type,
	struct MANAGER(FE_basis) *basis_manager);
/*******************************************************************************
LAST MODIFIED : 2 August 1999

DESCRIPTION :
Finds the specfied FE_basis in the basis managed. If it isn't there, creates it,
and adds it to the manager.
==============================================================================*/

PROTOTYPE_OBJECT_FUNCTIONS(FE_basis);

PROTOTYPE_LIST_FUNCTIONS(FE_basis);

PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(FE_basis,type,int *);

PROTOTYPE_MANAGER_COPY_FUNCTIONS(FE_basis,type,int *);

PROTOTYPE_MANAGER_FUNCTIONS(FE_basis);

PROTOTYPE_MANAGER_IDENTIFIER_FUNCTIONS(FE_basis,type,int *);

char *FE_basis_type_string(enum FE_basis_type basis_type);
/*******************************************************************************
LAST MODIFIED : 1 April 1999

DESCRIPTION :
Returns a pointer to a static string token for the given <basis_type>.
The calling function must not deallocate the returned string.
#### Must ensure implemented correctly for new FE_basis_type. ####
==============================================================================*/

int FE_basis_get_dimension(struct FE_basis *basis, int *dimension_address);
/*******************************************************************************
LAST MODIFIED : 6 November 2002

DESCRIPTION :
Returns the dimension of <basis>.
If fails, puts zero at <dimension_address>.
==============================================================================*/

int FE_basis_get_number_of_basis_functions(struct FE_basis *basis,
	int *number_of_basis_functions_address);
/*******************************************************************************
LAST MODIFIED : 15 May 2003

DESCRIPTION :
Returns the number_of_basis_functions of <basis>.
If fails, puts zero at <number_of_basis_functions_address>.
==============================================================================*/

int FE_basis_get_xi_basis_type(struct FE_basis *basis,
	int xi_number, enum FE_basis_type *basis_type_address);
/*******************************************************************************
LAST MODIFIED : 6 November 2002

DESCRIPTION :
Returns the basis type of <basis> on <xi_number> -- on main diagonal of
type array. The first xi_number is 0.
==============================================================================*/

int FE_basis_get_next_linked_xi_number(
	struct FE_basis *basis, int xi_number,
	int *next_xi_number_address, int *xi_link_number_address);
/*******************************************************************************
LAST MODIFIED : 6 November 2002

DESCRIPTION :
Returns in <next_xi_number_address> the next xi number higher than <xi_number>
which is linked in basis with it, plus in <xi_link_number_address> the number
denoting how it is linked; currently used only for polygon basiss to denote the
number of polygon sides.
If there is no remaining linked dimension, 0 is returned in both addresses.
<xi_number> is from 0 to one less than the basis dimension.
Also checks that the linked xi numbers have the same basis type.
==============================================================================*/

struct Linear_combination_of_global_values
	*CREATE(Linear_combination_of_global_values)(int number_of_global_values);
/*******************************************************************************
LAST MODIFIED : 23 September 1995

DESCRIPTION :
Allocates memory and assigns fields for a linear combination of global values.
Allocates storage for the global and coefficient indices and sets to -1.
==============================================================================*/

int DESTROY(Linear_combination_of_global_values)(
	struct Linear_combination_of_global_values **linear_combination_address);
/*******************************************************************************
LAST MODIFIED : 23 September 1995

DESCRIPTION :
Frees the memory for the linear combination and sets
<*linear_combination_address> to NULL.
==============================================================================*/

struct Standard_node_to_element_map *CREATE(Standard_node_to_element_map)(
	int node_index,int number_of_nodal_values);
/*******************************************************************************
LAST MODIFIED : 23 September 1995

DESCRIPTION :
Allocates memory and assigns fields for a standard node to element map.
Allocates storage for the nodal value and scale factor indices and sets to -1.
==============================================================================*/

int DESTROY(Standard_node_to_element_map)(
	struct Standard_node_to_element_map **map_address);
/*******************************************************************************
LAST MODIFIED : 23 September 1995

DESCRIPTION :
Frees the memory for the map and sets <*map_address> to NULL.
==============================================================================*/

int Standard_node_to_element_map_get_node_index(
	struct Standard_node_to_element_map *standard_node_map,
	int *node_index_address);
/*******************************************************************************
LAST MODIFIED : 5 November 2002

DESCRIPTION :
Returns the node index from <standard_node_map>.
If fails, sets *<node_index_address> to zero.
==============================================================================*/

int Standard_node_to_element_map_get_number_of_nodal_values(
	struct Standard_node_to_element_map *standard_node_map,
	int *number_of_nodal_values_address);
/*******************************************************************************
LAST MODIFIED : 5 November 2002

DESCRIPTION :
Returns the number of nodal values used by <standard_node_map>.
If fails, sets *<number_of_nodal_values_address> to zero.
==============================================================================*/

int Standard_node_to_element_map_get_nodal_value_index(
	struct Standard_node_to_element_map *standard_node_map,
	int nodal_value_number, int *nodal_value_index_address);
/*******************************************************************************
LAST MODIFIED : 5 November 2002

DESCRIPTION :
Returns the nodal value index at <nodal_value_number> in <standard_node_map>.
If fails, sets *<nodal_value_index_address> to zero.
==============================================================================*/

int Standard_node_to_element_map_set_nodal_value_index(
	struct Standard_node_to_element_map *standard_node_map,
	int nodal_value_number, int nodal_value_index);
/*******************************************************************************
LAST MODIFIED : 27 March 2003

DESCRIPTION :
Sets nodal_value_index <nodal_value_number> of <standard_node_map> to
<nodal_value_index>.
Note a negative <nodal_value_index> gives a value of zero without needing to
get a value from the node.
==============================================================================*/

int Standard_node_to_element_map_get_scale_factor_index(
	struct Standard_node_to_element_map *standard_node_map,
	int nodal_value_number, int *scale_factor_index_address);
/*******************************************************************************
LAST MODIFIED : 5 November 2002

DESCRIPTION :
Returns the nodal value index at <nodal_value_number> in <standard_node_map>.
If fails, sets *<scale_factor_index_address> to zero.
==============================================================================*/

int Standard_node_to_element_map_set_scale_factor_index(
	struct Standard_node_to_element_map *standard_node_map,
	int nodal_value_number, int scale_factor_index);
/*******************************************************************************
LAST MODIFIED : 27 March 2003

DESCRIPTION :
Sets scale_factor_index <nodal_value_number> of <standard_node_map> to
<scale_factor_index>.
Note a negative <scale_factor_index> gives a unit scale factor without
needing to get a value from the scale factor set.
==============================================================================*/

struct General_node_to_element_map *CREATE(General_node_to_element_map)(
	int node_index,int number_of_nodal_values);
/*******************************************************************************
LAST MODIFIED : 23 September 1995

DESCRIPTION :
Allocates memory and assigns fields for a general node to element map.
Allocates storage for the pointers to the linear combinations of field values
and sets to NULL.
==============================================================================*/

int DESTROY(General_node_to_element_map)(
	struct General_node_to_element_map **map_address);
/*******************************************************************************
LAST MODIFIED : 23 September 1995

DESCRIPTION :
Frees the memory for the map and sets <*map_address> to NULL.
==============================================================================*/

int General_node_to_element_map_get_node_index(
	struct General_node_to_element_map *general_node_map,
	int *node_index_address);
/*******************************************************************************
LAST MODIFIED : 5 November 2002

DESCRIPTION :
Returns the node index from <general_node_map>.
If fails, sets *<node_index_address> to zero.
==============================================================================*/

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

int FE_element_field_component_get_basis(
	struct FE_element_field_component *element_field_component,
	struct FE_basis **basis_address);
/*******************************************************************************
LAST MODIFIED : 5 November 2002

DESCRIPTION :
Gets the <basis> used by <element_field_component>.
If fails, puts NULL in *<basis_address> if supplied.
==============================================================================*/

int FE_element_field_component_get_general_node_map(
	struct FE_element_field_component *element_field_component, int node_number,
	struct General_node_to_element_map **general_node_map_address);
/*******************************************************************************
LAST MODIFIED : 5 November 2002

DESCRIPTION :
Gets the <general_node_map> relating global node values to those at local
<node_number> for <element_field_component> of type
GENERAL_NODE_TO_ELEMENT_MAP. <node_number> starts at 0 and must be less than
the number of nodes in the component.
If fails, puts NULL in *<general_node_map_address> if supplied.
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
STANDARD_NODE_TO_ELEMENT_MAP or GENERAL_NODE_TO_ELEMENT_MAP.
If fails, puts zero in *<number_of_nodes_address> if supplied.
==============================================================================*/

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
	int top_level_element_dimension,int *top_level_number_in_xi,
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

int standard_basis_function_is_monomial(Standard_basis_function *function,
	void *arguments_void);
/*******************************************************************************
LAST MODIFIED : 12 June 2002

DESCRIPTION :
Returns true if the standard basis function is a monomial.
==============================================================================*/

int calculate_FE_element_field_nodes(struct FE_element *element,
	struct FE_field *field,int *number_of_element_field_nodes_address,
	struct FE_node ***element_field_nodes_array_address,
	struct FE_element *top_level_element);
/*******************************************************************************
LAST MODIFIED : 12 February 2003

DESCRIPTION :
If <field> is NULL, element nodes are calculated for the coordinate field.  The
function allocates an array, <*element_field_nodes_array_address> to store the
pointers to the ACCESS'd element nodes.  Components that are not node-based are 
ignored.  The element nodes are ordered by increasing xi (fastest in xi1, next 
fastest in xi2 and so on).
The optional <top_level_element> forces inheritance from it as needed.
NB.  The nodes need to be DEACCESS'd before the nodes array is DEALLOCATE'd.
==============================================================================*/

int calculate_FE_element_field(int component_number,
	struct FE_element_field_values *element_field_values,FE_value *xi_coordinates,
	FE_value *values,FE_value *jacobian);
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
	struct FE_element_field_values *element_field_values,FE_value *xi_coordinates,
	char **string);
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
	struct FE_element_field_values *element_field_values,FE_value *xi_coordinates,
	int *values);
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
	struct FE_element_field_values *element_field_values,FE_value *xi_coordinates,
	char **values);
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

int calculate_FE_element_anatomical(
	struct FE_element_field_values *coordinate_element_field_values,
	struct FE_element_field_values *anatomical_element_field_values,
	FE_value *xi_coordinates,FE_value *x,FE_value *y,FE_value *z,FE_value a[3],
	FE_value b[3],FE_value c[3],FE_value *dx_dxi);
/*******************************************************************************
LAST MODIFIED : 16 November 1998

DESCRIPTION :
Calculates the cartesian coordinates (<x>, <y> and <z>), and the fibre (<a>),
cross-sheet (<b>) and sheet-normal (<c>) vectors from a coordinate element field
and an anatomical element field.  The storage for the <x>, <y>, <z>, <a>, <b>
and <c> should have been allocated outside the function.
If later conversion of a, b and c to vectors in xi space is required, the
optional <dx_dxi> parameter should be supplied and point to a enough memory to
contain the nine derivatives of x,y,z w.r.t. three xi. These are returned in the
order dx/dxi1, dx/dxi2, dx/dxi3, dy/dxi1 etc. Note that there will always be
nine values returned, regardless of the element dimension.
==============================================================================*/

struct FE_element_shape *CREATE(FE_element_shape)(int dimension, int *type,
	struct FE_region *fe_region);
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

int FE_element_shape_is_unspecified(struct FE_element_shape *element_shape);
/*******************************************************************************
LAST MODIFIED : 18 November 2002

DESCRIPTION :
Returns true if the only thing know about <element_shape> is its dimension.
==============================================================================*/

struct FE_element_shape *get_FE_element_shape_of_face(
	struct FE_element_shape *shape,int face_number, struct FE_region *fe_region);
/*******************************************************************************
LAST MODIFIED : 7 July 2003

DESCRIPTION :
From the parent <shape> returns the FE_element_shape for its face <face_number>.
The <shape> must be of dimension 2 or 3. Faces of 2-D elements are always lines.
==============================================================================*/

int get_FE_element_shape_dimension(struct FE_element_shape *element_shape,
	int *dimension_address);
/*******************************************************************************
LAST MODIFIED : 5 November 2002

DESCRIPTION :
Returns the dimension of <element_shape>.
If fails, puts zero at <dimension_address>.
==============================================================================*/

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

char *CM_element_type_string(enum CM_element_type cm_element_type);
/*******************************************************************************
LAST MODIFIED : 26 August 1999

DESCRIPTION :
Returns a static string describing the <cm_element_type>, eg. CM_LINE = 'line'.
Returned string must not be deallocated!
==============================================================================*/

int FE_element_to_element_string(struct FE_element *element,char **name_ptr);
/*******************************************************************************
LAST MODIFIED : 19 March 2003

DESCRIPTION :
Writes the cm.number of <element> into a newly allocated string and
points <*name_ptr> at it.
==============================================================================*/

struct FE_element *element_string_to_FE_element(char *name,
	struct LIST(FE_element) *element_list);
/*******************************************************************************
LAST MODIFIED : 19 March 2003

DESCRIPTION :
Converts the <name> into a CM_element_information.number sets 
CM_element_information.type to CM_ELEMENT, finds
and returns the element in the <element_list> with that CM_element_information.
==============================================================================*/

int FE_element_to_any_element_string(struct FE_element *element,
	char **name_ptr);
/*******************************************************************************
LAST MODIFIED : 19 March 2003

DESCRIPTION :
Writes the element as an allocated string containing TYPE NUMBER. Now does not
write element for CM_ELEMENT types.
==============================================================================*/

struct FE_element *any_element_string_to_FE_element(char *name,
	struct LIST(FE_element) *element_list);
/*******************************************************************************
LAST MODIFIED : 19 March 2003

DESCRIPTION :
Converts name string of format "TYPE NUMBER" to a CM_element_information and
returns the element in the <element_list> with that CM_element_information.
==============================================================================*/

struct FE_element *CREATE(FE_element)(struct CM_element_information *cm,
	struct FE_element_shape *element_shape,
	struct FE_region *fe_region, struct FE_element *template_element);
/*******************************************************************************
LAST MODIFIED : 20 February 2003

DESCRIPTION :
Creates and returns an element with the specified <cm> identifier.
If <fe_region> is supplied a blank element with the given identifier but no
fields is returned. If <template_element> is supplied, a copy of it, including
all fields and values but with the new identifier, is returned.
Exactly one of <fe_region> or <template_element> must be supplied.
The <element_shape> is required unless a <template_element> is supplied.
The new element is set to belong to the ultimate master FE_region of <fe_region>
if supplied, or to the same master FE_region as <template_element> if supplied.
Note that the element number in <cm> must be non-negative.
==============================================================================*/

int DESTROY(FE_element)(struct FE_element **element_address);
/*******************************************************************************
LAST MODIFIED : 23 September 1995

DESCRIPTION :
Frees the memory for the element, sets <*element_address> to NULL.
==============================================================================*/

PROTOTYPE_OBJECT_FUNCTIONS(FE_element);
PROTOTYPE_COPY_OBJECT_FUNCTION(FE_element);

int adjacent_FE_element(struct FE_element *element,
	int face_number, int *number_of_adjacent_elements, 
	struct FE_element ***adjacent_elements);
/*******************************************************************************
LAST MODIFIED : 13 March 2003

DESCRIPTION :
Returns the list of <adjacent_elements> not including <element> which share the 
face indicated by <face_number>.  <adjacent_elements> is ALLOCATED to the 
correct size and should be DEALLOCATED when finished with.
==============================================================================*/

int FE_element_meets_topological_criteria(struct FE_element *element,
	int dimension, enum CM_element_type cm_element_type, int exterior,
	int face_number, struct LIST(FE_element) *element_list);
/*******************************************************************************
LAST MODIFIED : 14 March 2003

DESCRIPTION :
Returns true if <element> meets all the supplied criteria:
- it has the given <dimension> OR <cm_element_type>;
- it is an exterior face or line of its contiguous mesh if <exterior> set;
- it is on the <face_number> of a parent element if <face_number> non-negative,
  and if <element_list> is supplied the parent element must be in it as well.
Note that <exterior> and <face_number> requirements are ignored if they make no
sense for the element, eg. for n-D elements in an n-D mesh.
???RC Only complete up to 3-D.
==============================================================================*/

int equivalent_FE_field_in_elements(struct FE_field *field,
	struct FE_element *element_1, struct FE_element *element_2);
/*******************************************************************************
LAST MODIFIED : 10 September 2001

DESCRIPTION :
Returns true if <field> is equivalently listed in the field information for
<element_1> and <element_2>. If neither element has field information or if they
do but the field is not defined in either, this is also equivalent.
==============================================================================*/

int equivalent_FE_fields_in_elements(struct FE_element *element_1,
	struct FE_element *element_2);
/*******************************************************************************
LAST MODIFIED : 10 September 2001

DESCRIPTION :
Returns true if all fields are defined in the same way at the two elements.
==============================================================================*/

int get_FE_element_dimension(struct FE_element *element);
/*******************************************************************************
LAST MODIFIED : 4 November 1999

DESCRIPTION :
Returns the dimension of the <element> or an error if it does not have a shape.
==============================================================================*/

int FE_element_get_cm_number(struct FE_element *element);
/*******************************************************************************
LAST MODIFIED : 13 March 2003

DESCRIPTION :
Returns the cm number of the <element> or an error if it does not have a shape.
==============================================================================*/

int get_FE_element_identifier(struct FE_element *element,
	struct CM_element_information *identifier);
/*******************************************************************************
LAST MODIFIED : 29 October 2002

DESCRIPTION :
Fills in the <identifier> of <element>.
==============================================================================*/

int set_FE_element_identifier(struct FE_element *element,
	struct CM_element_information *identifier);
/*******************************************************************************
LAST MODIFIED : 10 May 2003

DESCRIPTION :
Changes the identifier of <element> to <identifier>.
Caution! Should only call for elements that are NOT in indexed lists, i.e.
temporary/non-global elements that are not in FE_regions.
To enable identifier changes, must wrap calls to this function between
LIST_BEGIN_IDENTIFIER_CHANGE/LIST_END_IDENTIFIER_CHANGE to ensure
element is temporarily removed from all the indexed lists it is in and re-added
afterwards.
If <element> is in an FE_region, FE_region_change_FE_element_identifier should
be called to handle the above complications.
==============================================================================*/

int FE_element_or_parent_changed(struct FE_element *element,
	struct CHANGE_LOG(FE_element) *fe_element_change_log,
	struct CHANGE_LOG(FE_node) *fe_node_change_log);
/*******************************************************************************
LAST MODIFIED : 11 February 2003

DESCRIPTION :
Returns true if <element> or any of its parent elements is listed in the
<fe_element_change_log> with any of OBJECT_IDENTIFIER_CHANGED,
OBJECT_NOT_IDENTIFIER_CHANGED or OBJECT_REMOVED.
Since element fields depend on node fields, the element is also considered as
changed if it or any of its parents uses a node listed in the
<fe_node_change_log> with OBJECT_IDENTIFIER_CHANGED and/or
OBJECT_NOT_IDENTIFIER_CHANGED.
==============================================================================*/

int get_FE_element_number_of_fields(struct FE_element *element);
/*******************************************************************************
LAST MODIFIED : 4 November 2002

DESCRIPTION :
Returns the number of fields defined at <element>.
Does not include fields inherited from parent elements.
==============================================================================*/

int get_FE_element_number_of_parents(struct FE_element *element,
	int *number_of_parents_address);
/*******************************************************************************
LAST MODIFIED : 14 January 2003

DESCRIPTION :
Returns the number of parents of <element>.
Can be used to determine if a face is in use by more than one parent elements.
==============================================================================*/

int get_FE_element_number_of_parents_in_list(struct FE_element *element,
	struct LIST(FE_element) *element_list, int *number_of_parents_address);
/*******************************************************************************
LAST MODIFIED : 14 January 2003

DESCRIPTION :
Returns the number of parents of <element> that are in <element_list>.
==============================================================================*/

int FE_element_get_first_parent(struct FE_element *element,
	struct FE_element **parent_element_address, int *face_number_address);
/*******************************************************************************
LAST MODIFIED : 7 April 2003

DESCRIPTION :
Returns the first <parent_element> of <element> and the <face_number> it is at.
If there is no parent, a true return_code is returned but with a NULL
<parent_element>.
==============================================================================*/

int get_FE_element_shape(struct FE_element *element,
	struct FE_element_shape **shape);
/*******************************************************************************
LAST MODIFIED : 7 October 1999

DESCRIPTION :
Returns the <shape> of the <element>, if any. Only newly created blank elements
should have no shape.
==============================================================================*/

int get_FE_element_number_of_faces(struct FE_element *element,
	int *number_of_faces_address);
/*******************************************************************************
LAST MODIFIED : 5 November 2002

DESCRIPTION :
Returns the number of faces of <element>.
If fails, puts zero at <number_of_faces_address>.
==============================================================================*/

int get_FE_element_face(struct FE_element *element,int face_number,
	struct FE_element **face_element);
/*******************************************************************************
LAST MODIFIED : 7 October 1999

DESCRIPTION :
Returns the <face_element> for face <face_number> of <element>, where NULL means
there is no face. Element must have a shape and face.
==============================================================================*/

int set_FE_element_face(struct FE_element *element,int face_number,
	struct FE_element *face_element);
/*******************************************************************************
LAST MODIFIED : 7 October 1999

DESCRIPTION :
Sets face <face_number> of <element> to <face_element>, ensuring the
<face_element> has <element> as a parent. <face_element> may be NULL = no face.
Must have set the shape with set_FE_element_shape first.
Should only be called for unmanaged elements.
==============================================================================*/

int set_FE_element_number_of_nodes(struct FE_element *element,
	int number_of_nodes);
/*******************************************************************************
LAST MODIFIED : 4 November 2002

DESCRIPTION :
Establishes storage for <number_of_nodes> in <element>.
May only be set once; should only be called for unmanaged elements.
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

int set_FE_element_number_of_scale_factor_sets(struct FE_element *element,
	int number_of_scale_factor_sets, void **scale_factor_set_identifiers,
	int *numbers_in_scale_factor_sets);
/*******************************************************************************
LAST MODIFIED : 4 November 2002

DESCRIPTION :
Establishes storage for <number_of_scale_factor_sets> in <element>, each
containing <numbers_in_scale_factor_sets> and identifier by
<scale_factor_set_identifiers>.
May only be set once; should only be called for unmanaged elements.
==============================================================================*/

int get_FE_element_number_of_scale_factor_sets(struct FE_element *element,
	int *number_of_scale_factor_sets_address);
/*******************************************************************************
LAST MODIFIED : 5 November 2002

DESCRIPTION :
Returns the number of scale factor_sets in <element>.
If fails, puts zero at <number_of_scale_factor_sets_address>.
==============================================================================*/

int get_FE_element_numbers_in_scale_factor_set(struct FE_element *element,
	int scale_factor_set_number, int *numbers_in_scale_factor_set_address);
/*******************************************************************************
LAST MODIFIED : 5 November 2002

DESCRIPTION :
Returns the number of scale factors in <scale_factor_set_number> of <element>,
where <scale_factor_set_number> is from 0 to one less than the number of sets.
If fails, puts zero in *<numbers_in_scale_factor_set_address> if supplied.
==============================================================================*/

int get_FE_element_scale_factor_set_identifier(struct FE_element *element,
	int scale_factor_set_number, void **scale_factor_set_identifier_address);
/*******************************************************************************
LAST MODIFIED : 5 November 2002

DESCRIPTION :
Returns the identifier of <scale_factor_set_number> of <element>,
where <scale_factor_set_number> is from 0 to one less than the number of sets.
If fails, puts NULL in *<scale_factor_set_identifier_address> if supplied.
==============================================================================*/

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

int set_FE_element_scale_factor(struct FE_element *element,
	int scale_factor_number,FE_value scale_factor);
/*******************************************************************************
LAST MODIFIED : 15 November 1999

DESCRIPTION :
Sets scale_factor <scale_factor_number>, from 0 to number_of_scale_factors-1 of
<element> to <scale_factor>.
<element> must already have a shape and node_scale_field_information.
Should only be called for unmanaged elements.
This function is a bit naughty. Should really use 
FE_element_set_scale_factor_for_nodal_value . 
==============================================================================*/

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

int for_each_FE_field_at_element_indexer_first(
	FE_element_field_iterator_function *iterator,void *user_data,
	struct FE_element *element);
/*******************************************************************************
LAST MODIFIED : 5 October 1999

DESCRIPTION :
Calls the <iterator> for each field defined at the <element> until the
<iterator> returns 0 or it runs out of fields.  Returns the result of the last
<iterator> called. This version insists that any field used as an indexer_field
for another field in the list is output first.
==============================================================================*/

struct FE_field *get_FE_element_default_coordinate_field(
	struct FE_element *element);
/*******************************************************************************
LAST MODIFIED : 13 May 1999

DESCRIPTION :
Returns the first coordinate field defined over <element>, recursively getting
it from its first parent if it has no node scale field information.
==============================================================================*/

int FE_element_find_default_coordinate_field_iterator(
	struct FE_element *element, void *fe_field_void);
/*******************************************************************************
LAST MODIFIED : 30 November 2001

DESCRIPTION :
An FE_element iterator that returns 1 when an appropriate default_coordinate
fe_field is found.  The fe_field found is returned as fe_field_void.
==============================================================================*/

struct CM_element_type_Multi_range_data
/*******************************************************************************
LAST MODIFIED : 1 March 2001

DESCRIPTION :
Iterator data for functions working with elements of a given CM_element_type
and a Multi_range, eg. FE_element_of_CM_element_type_is_in_Multi_range.
==============================================================================*/
{
	enum CM_element_type cm_element_type;
	struct Multi_range *multi_range;
};

struct FE_element_conditional_iterator_data
/*******************************************************************************
LAST MODIFIED : 15 January 2003

DESCRIPTION :
Data for passing to FE_element_conditional_iterator function.
==============================================================================*/
{
	LIST_CONDITIONAL_FUNCTION(FE_element) *conditional_function;
	void *conditional_user_data;
	LIST_ITERATOR_FUNCTION(FE_element) *iterator_function;
	void *iterator_user_data;
}; /* struct FE_element_conditional_iterator_data */

int FE_element_conditional_iterator(struct FE_element *element,
	void *data_void);
/*******************************************************************************
LAST MODIFIED : 15 January 2003

DESCRIPTION :
If <element> satisfies the <conditional_function> with <conditional_user_data>,
calls <iterator_function> with it and the <iterator_user_data>.
<data_void> points at a struct FE_element_conditional_iterator_data.
==============================================================================*/

int FE_element_of_CM_element_type_is_in_Multi_range(struct FE_element *element,
	void *element_type_ranges_data_void);
/*******************************************************************************
LAST MODIFIED : 1 March 2001

DESCRIPTION :
Conditional function returning true if <element> is of the given
<cm_element_type> and whose number is in the <multi_range>.
Second argument is a struct CM_element_type_Multi_range_data.
==============================================================================*/

int FE_element_of_CM_element_type_is_not_in_Multi_range(
	struct FE_element *element, void *element_type_ranges_data_void);
/*******************************************************************************
LAST MODIFIED : 1 March 2001

DESCRIPTION :
Conditional function returning true if <element> is of the given
<cm_element_type> and whose number is not in the <multi_range>.
Second argument is a struct CM_element_type_Multi_range_data.
==============================================================================*/

int FE_element_of_CM_element_type_add_number_to_Multi_range(
	struct FE_element *element, void *element_type_ranges_data_void);
/*******************************************************************************
LAST MODIFIED : 1 March 2001

DESCRIPTION :
Iterator function which, if <element> is of the given <cm_element_type>, adds
its CMISS number to <multi_range>.
Second argument is a struct CM_element_type_Multi_range_data.
==============================================================================*/

int FE_element_is_in_list(struct FE_element *element, void *element_list_void);
/*******************************************************************************
LAST MODIFIED : 14 January 2003

DESCRIPTION :
Returns true if <element> is in <element_list>.
==============================================================================*/

int FE_element_is_not_in_list(struct FE_element *element,
	void *element_list_void);
/*******************************************************************************
LAST MODIFIED : 14 January 2003

DESCRIPTION :
Returns true if <element> is not in <element_list>.
==============================================================================*/

int FE_element_is_wholly_within_element_list_tree(
	struct FE_element *element, void *element_list_void);
/*******************************************************************************
LAST MODIFIED : 1 March 2001

DESCRIPTION :
Returns true if <element> is either in <element_list> or has all its parents
directly or indirectly in the <element_list> tree. Used to check if elements
will be destroyed, since faces and lines are destroyed with their parents if
they are not also faces or lines of other elements not being destroyed.
==============================================================================*/

struct FE_element_count_if_type_data
{
	enum CM_element_type cm_type;
	int number_of_elements;
};

int FE_element_count_if_type(struct FE_element *element,
	void *count_data_void);
/*******************************************************************************
LAST MODIFIED : 15 January 2003

DESCRIPTION :
If <element> is of the given CM_type, increment number_of_elements.
<count_data_void> points at a struct FE_element_count_if_type_data.
==============================================================================*/

int add_FE_element_and_faces_to_list(struct FE_element *element,
	void *element_list_void);
/*******************************************************************************
LAST MODIFIED : 1 June 2001

DESCRIPTION :
Ensures <element>, its faces (and theirs etc.) are in <element_list>.
Note: this function is recursive.
==============================================================================*/

struct FE_element_add_faces_not_in_list_data
/*******************************************************************************
LAST MODIFIED : 17 February 2003

DESCRIPTION :
Data for FE_element_add_faces_not_in_list function.
==============================================================================*/
{
	struct LIST(FE_element) *add_element_list;
	struct LIST(FE_element) *current_element_list;
};

int FE_element_add_faces_not_in_list(struct FE_element *element,
	void *data_void);
/*******************************************************************************
LAST MODIFIED : 17 February 2003

DESCRIPTION :
If any face of <element> is not in <current_element_list>, and not in
<add_element_list>, adds it to <add_element_list> and asks the same of all its
faces and their lines, etc.
<data_void> points at a struct FE_element_add_faces_not_in_list_data.
Note: this function is recursive.
==============================================================================*/

#if defined (OLD_CODE)
enum Remove_element_mode
/*******************************************************************************
LAST MODIFIED : 1 March 2001

DESCRIPTION :
Controls the mode of element and recursive face removal in functions:
remove_FE_element_and_faces_from_[group|list|manager].
NOTE: these functions will have to be carefully modified when new modes added.
==============================================================================*/
{
	/* nothing is removed */
	NO_REMOVE_ELEMENT,
	/* Element is removed unconditionally. Its faces, and their faces etc., are
		 removed recursively if they are not faces of any other element in
		 group/list/manager */
	RECURSIVE_REMOVE_ELEMENT_AND_PARENTLESS_FACES,
	/* Element and its faces, and their faces etc., are removed recursively if
		 they are not faces of any other element in group/list/manager */
	RECURSIVE_REMOVE_PARENTLESS_ELEMENT_AND_PARENTLESS_FACES
};
#endif /* defined (OLD_CODE) */

int FE_element_for_each_element_and_parent(struct FE_element *element,
	LIST_ITERATOR_FUNCTION(FE_element) *iterator_function, void *user_data);
/*******************************************************************************
LAST MODIFIED : 14 May 2003

DESCRIPTION :
Iterates over <element> and all its parent elements and so on recursively,
calling <iterator_function> with <user_data> for each of them.
Note that parents of parent elements may be iterated over more than once since,
for example, lines can be in two faces of the same parent element.
Stops as soon as all parent elements have been iterated through, or the
iterator_function returns FALSE for any element.
This function is recursive.
==============================================================================*/

int merge_FE_element(struct FE_element *destination, struct FE_element *source,
	struct LIST(FE_field) *changed_fe_field_list);
/*******************************************************************************
LAST MODIFIED : 30 May 2003

DESCRIPTION :
Merges the fields from <source> into <destination>. Existing fields in the
<destination> keep the same element field description as before with new field
storage following them. Where existing fields in <destination> are passed in
<source>, values from <source> take precedence, but the element field structure
remains unchanged.
Function is atomic; <destination> is unchanged if <source> cannot be merged.
The <changed_fe_field_list> must be supplied. On return it contains the list
of FE_fields that have been changed or added to <destination>. Note it is not
sufficient to assume just the fields in <source> are changed since changes to
common scale factors affect different fields in <destination>; the
<change_fe_field_list> includes these fields.
???RC Move to finite_element_private.h?
==============================================================================*/

int list_FE_element(struct FE_element *element,void *dummy);
/*******************************************************************************
LAST MODIFIED : 28 January 1998

DESCRIPTION :
Outputs the information contained at the element.
==============================================================================*/

int list_FE_element_name(struct FE_element *element,void *dummy);
/*******************************************************************************
LAST MODIFIED : 26 August 1999

DESCRIPTION :
Outputs the name of the element as element/face/line #.
==============================================================================*/

PROTOTYPE_LIST_FUNCTIONS(FE_element);

PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(FE_element,identifier, \
	struct CM_element_information *);

PROTOTYPE_CHANGE_LOG_FUNCTIONS(FE_element);

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

struct FE_field *CREATE(FE_field)(char *name, struct FE_region *fe_region);
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

PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(FE_field,name,char *);

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

int FE_field_matches_description(struct FE_field *field,char *name,
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

int FE_fields_match_fundamental(struct FE_field *field1,
	struct FE_field *field2);
/*******************************************************************************
LAST MODIFIED : 16 December 2002

DESCRIPTION :
Returns true if <field1> and <field2> describe the same fundamental quantities
including number of components, value type etc. which should be sufficient to
allow field1 and field2 to be interchanged without affecting the rest of the
program. Check this function fits will with FE_fields_match_exact.
Does not check the fields have the same name as this is a trivial change.
==============================================================================*/

int FE_fields_match_exact(struct FE_field *field1, struct FE_field *field2);
/*******************************************************************************
LAST MODIFIED : 16 December 2002

DESCRIPTION :
Returns true if <field1> and <field2> have exactly the same contents.
==============================================================================*/

int FE_field_can_be_merged(struct FE_field *field, void *field_list_void);
/*******************************************************************************
LAST MODIFIED : 14 November 2002

DESCRIPTION :
Fetches a field with the same name as <field> from <field_list>.
Returns true if there is either no such field in the list or two fields are
identically defined.
==============================================================================*/

int FE_field_has_multiple_times(struct FE_field *fe_field);
/*******************************************************************************
LAST MODIFIED : 26 February 2003

DESCRIPTION :
Returns true if any node_fields corresponding to <field> have time_versions.
This will be improved when regionalised, so that hopefully the node field
list we will be looking at will not be global but will belong to the region.
==============================================================================*/

struct FE_field *find_first_time_field_at_FE_node(struct FE_node *node);
/*******************************************************************************
LAST MODIFIED : 9 June 1999

Find the first time based field at a node
==============================================================================*/

int set_FE_field(struct Parse_state *state,void *field_address_void,
	void *fe_field_list_void);
/*******************************************************************************
LAST MODIFIED : 3 December 2002

DESCRIPTION :
Modifier function to set the field from the command line.
==============================================================================*/

int set_FE_field_conditional(struct Parse_state *state,
	void *field_address_void,void *set_field_data_void);
/*******************************************************************************
LAST MODIFIED : 3 December 2002

DESCRIPTION :
Modifier function to set the field from a command. <set_field_data_void> should
point to a struct Set_FE_field_conditional_data containing the
fe_field_list and an optional conditional function for narrowing the
range of fields available for selection. If the conditional_function is NULL,
this function works just like set_FE_field.
==============================================================================*/

int set_FE_fields(struct Parse_state *state,
	void *field_order_info_address_void, void *fe_field_list_void);
/*******************************************************************************
LAST MODIFIED : 3 December 2002

DESCRIPTION :
Modifier function to set an ordered list of fields, each separated by white
space until an unrecognised field name is encountered. Two special tokens are
understood in place of any fields: 'all' and 'none'.
For the case of 'all', a NULL FE_field_order_info structure is returned.
For the case of 'none', an empty FE_field_order_info structure is returned.
It is up to the calling function to destroy any FE_field_order_info structure
returned by this function, however, any such structure passed to this function
may be destroyed here - ie. in the 'all' case.
==============================================================================*/

int FE_field_has_value_type(struct FE_field *field,void *user_data_value_type);
/*******************************************************************************
LAST MODIFIED : 4 May 1999

DESCRIPTION :
Returns true if the VALUE_TYPE specified in the <user_data_value_type> matches
the VALUE_TYPE of the <field>.
==============================================================================*/

struct FE_region *FE_field_get_FE_region(struct FE_field *fe_field);
/*******************************************************************************
LAST MODIFIED : 26 February 2003

DESCRIPTION :
Returns the FE_region that <fe_field> belongs to.
==============================================================================*/

int FE_field_get_access_count(struct FE_field *fe_field);
/*******************************************************************************
LAST MODIFIED : 10 March 2003

DESCRIPTION :
Returns the number of objects using the <fe_field>.
==============================================================================*/

char *get_FE_field_component_name(struct FE_field *field,int component_no);
/*******************************************************************************
LAST MODIFIED : 10 May 2000

DESCRIPTION :
Returns the name of component <component_no> of <field>. If no name is stored
for the component, a string comprising the value component_no+1 is returned.
Up to calling function to DEALLOCATE the returned string.
==============================================================================*/

int set_FE_field_component_name(struct FE_field *field,int component_no,
	char *component_name);
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

int get_FE_field_access_count(struct FE_field *field);
/*******************************************************************************
LAST MODIFIED : 17 August 1999

DESCRIPTION :Debug function. May be naughty.
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
=========================================================================*/

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

char *get_FE_field_name(struct FE_field *field);
/*******************************************************************************
LAST MODIFIED : 19 February 1999

DESCRIPTION :
Returns a pointer to the name for the <field>.
Should only call this function for unmanaged fields.
==============================================================================*/

int set_FE_field_name(struct FE_field *field, char *name);
/*******************************************************************************
LAST MODIFIED : 19 February 1999

DESCRIPTION :
Sets the name of the <field>.
Should only call this function for unmanaged fields.
==============================================================================*/

PROTOTYPE_GET_OBJECT_NAME_FUNCTION(FE_field_component);

int set_FE_field_component(struct Parse_state *state,void *component_void,
	void *fe_field_list_void);
/*******************************************************************************
LAST MODIFIED : 3 December 2002

DESCRIPTION :
Used in command parsing to translate a field component name into an field
component.
???DB.  Should it be here ?
???RC.  Does not ACCESS the field (unlike set_FE_field, above).
==============================================================================*/

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

int FE_element_get_top_level_xi_number(struct FE_element *element,
	int xi_number);
/*******************************************************************************
LAST MODIFIED : 16 September 1998

DESCRIPTION :
Returns the xi_number of the parent or parent's parent that changes with the
<xi_number> (0..dimension-1) of <element>.  If there is no parent, then
<xi_number> is returned, or 0 if there is an error.
???RC Could stuff up for graphical finite elements if parent is not in same
element_group and xi directions are different for neighbouring elements.
==============================================================================*/

struct FE_region *FE_element_get_FE_region(struct FE_element *element);
/*******************************************************************************
LAST MODIFIED : 13 February 2003

DESCRIPTION :
Returns the FE_region that <element> belongs to.
==============================================================================*/

int FE_element_has_top_level_element(struct FE_element *element,
	void *top_level_element_void);
/*******************************************************************************
LAST MODIFIED : 8 June 2000

DESCRIPTION :
Returns true if <top_level_element> is indeed a top_level parent of <element>.
==============================================================================*/

int FE_element_or_parent_is_element(struct FE_element *element,
	void *match_element_void);
/*******************************************************************************
LAST MODIFIED : 21 June 2000

DESCRIPTION :
Returns true if <element> or any of its parents matches <match_element>.
==============================================================================*/

int FE_element_is_top_level_parent_of_element(
	struct FE_element *top_level_element,void *element_void);
/*******************************************************************************
LAST MODIFIED : 8 June 2000

DESCRIPTION :
Returns true if <top_level_element> is a top_level parent of <element>.
==============================================================================*/

struct FE_element *FE_element_get_top_level_element_conversion(
	struct FE_element *element,struct FE_element *check_top_level_element,
	struct LIST(FE_element) *element_list,int face_number,
	FE_value *element_to_top_level);
/*******************************************************************************
LAST MODIFIED : 3 December 2002

DESCRIPTION :
Returns the/a top level [ultimate parent] element for <element>. If supplied,
the function attempts to verify that the <check_top_level_element> is in
fact a valid top_level_element for <element>, otherwise it tries to find one in
the <element_list> and with <element> on its <face_number> (if positive), if
either are specified.

If the returned element is different to <element> (ie. is of higher dimension),
then this function also fills the matrix <element_to_top_level> with values for
converting the xi coordinates in <element> to those in the returned element.
<element_to_top_level> should be preallocated to store at least nine FE_values.

The use of the <element_to_top_level> matrix is similar to <face_to_element> in
FE_element_shape - in fact it is either a copy of it, or calculated from it.
It gives the transformation xi(top_level) = b + A xi(element), where b is in the
first column of the matrix, and the rest of the matrix is A. Its size depends
on the dimension of element:top_level, ie.,
1:2 First 4 values, in form of 2 row X 2 column matrix, used only.
1:3 First 6 values, in form of 3 row X 2 column matrix, used only.
2:3 First 9 values, in form of 3 row X 3 column matrix, used only.
NOTE: recursive to handle 1-D to 3-D case.
==============================================================================*/

int FE_element_get_top_level_element_and_xi(struct FE_element *element, 
	FE_value *xi, int element_dimension, struct FE_element **top_level_element,
	FE_value *top_level_xi, int *top_level_element_dimension);
/*******************************************************************************
LAST MODIFIED : 13 March 2003

DESCRIPTION :
Finds the <top_level_element>, <top_level_xi> and <top_level_element_dimension>
for the given <element> and <xi>.  If <top_level_element> is already set it 
is checked and the <top_level_xi> calculated.
==============================================================================*/

int get_FE_element_discretization_from_top_level(struct FE_element *element,
	int *number_in_xi,struct FE_element *top_level_element,
	int *top_level_number_in_xi,FE_value *element_to_top_level);
/*******************************************************************************
LAST MODIFIED : 21 December 1999

DESCRIPTION :
Returns in <number_in_xi> the equivalent discretization of <element> for its
position - element, face or line - in <top_level_element>. Uses
<element_to_top_level> array for line/face conversion as returned by
FE_element_get_top_level_element_conversion.
<number_in_xi> must have space at lease MAXIMUM_ELEMENT_XI_DIMENSIONS integers,
as remaining values up to this size are cleared to zero.
==============================================================================*/

int get_FE_element_discretization(struct FE_element *element,
	struct LIST(FE_element) *element_list,int face_number,
	struct FE_field *native_discretization_field,
	int *top_level_number_in_xi,struct FE_element **top_level_element,
	int *number_in_xi);
/*******************************************************************************
LAST MODIFIED : 3 December 2002

DESCRIPTION :
Returns the discretization as <number_in_xi> for displaying graphics over
<element>, given its <element_list>, <face_number> and suggested
<*top_level_element>. If <native_discretization_field> is defined over the
element and is grid-based, it's native discretization is used in preference
to the <top_level_number_in_xi>.
<*top_level_element> can be NULL; final element used will be returned.
<top_level_number_in_xi> should be set by the caller as it will be used if there
is no native_discretization field or it is not defined over the element; in
either case the top_level_number_in_xi used is returned.
==============================================================================*/

int FE_element_or_parent_contains_node(struct FE_element *element,
	void *node_void);
/*******************************************************************************
LAST MODIFIED : 25 February 1998

DESCRIPTION :
FE_element conditional function returning 1 if <element> or all of its parents
or parent's parents contains <node>.
Routine is used with graphical finite elements to redraw only those elements
affected by a node change when the mesh is edited.
==============================================================================*/

int FE_element_or_parent_contains_node_in_list(struct FE_element *element,
	void *node_list_void);
/*******************************************************************************
LAST MODIFIED : 31 May 2001

DESCRIPTION :
FE_element conditional function returning 1 if <element> or all of its parents
or parent's parents contains nodes in <node_list>.
Routine is used with graphical finite elements to redraw only those elements
affected by a node change when the mesh is edited.
==============================================================================*/

struct FE_element_add_nodes_to_list_data
/*******************************************************************************
LAST MODIFIED : 12 February 2003

DESCRIPTION :
Data for passing to FE_element_add_nodes_to_list.
==============================================================================*/
{
	/* the entire list being iterated over by function. Having this allows us
		 to avoid determining the nodes on faces and lines if all their parents are
		 to have their nodes added to the list already */
	struct LIST(FE_element) *element_list;
	/* the list being added to */
	struct LIST(FE_node) *node_list;
	/* optional intersect_node_list. Only nodes in this list are candidates for
		 adding to the above node_list */
	struct LIST(FE_node) *intersect_node_list;
};

int FE_element_add_nodes_to_list(struct FE_element *element, void *data_void);
/*******************************************************************************
LAST MODIFIED : 12 February 2003

DESCRIPTION :
Adds all the nodes used by <element> to the <node_list> in the data. Since it
is very inefficient to determine the nodes in use by faces or lines, the
<element_list> being iterated over is also passed to this function so that only
faces or lines with parents not in the <element_list> are checked, and only
nodes inherited from top level elements not in the list are considered.
The optional <intersect_node_list> restricts nodes added to <node_list> to also
be in it.
<data_void> points at a struct FE_element_add_nodes_to_list_data.
==============================================================================*/

int add_FE_element_using_node_list_to_list(struct FE_element *element,
	void *element_list_node_list_data_void);
/*******************************************************************************
LAST MODIFIED : 1 June 2001

DESCRIPTION :
If <element> has a parent already in <element_list>, or it or any of its parents
uses any nodes in <node_list>, then <element> is added to <element_list>.
Used to build up a list of elements [probably] affected by changes to the nodes
in <node_list>.
Second argument is pointer to a struct FE_element_list_FE_node_list_data.
Note: for the sake of speed it is sometimes inaccurate for faces and lines. It
also relies on list being ordered with CM_ELEMENT first, then CM_FACE, then
CM_LINE for efficiency -- that's why it checks if any parents are in list first.
==============================================================================*/

int FE_element_is_dimension(struct FE_element *element,void *dimension_void);
/*******************************************************************************
LAST MODIFIED : 21 September 1998

DESCRIPTION :
Returns true if <element> is of the given <dimension>.
<dimension_void> must be a pointer to an int containing the dimension.
==============================================================================*/

int FE_element_is_dimension_3(struct FE_element *element,void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 1 December 1999

DESCRIPTION :
Returns true if <element> is a 3-D element (ie. not a 2-D face or 1-D line).
==============================================================================*/

int FE_element_has_CM_element_type(struct FE_element *element,
	void *cm_element_type_void);
/*******************************************************************************
LAST MODIFIED : 28 November 2000

DESCRIPTION :
Returns true if <element> has the given CM_element_type: CM_ELEMENT, CM_FACE or
CM_LINE. Note the enum CM_element_type is directly cast to (void *).
==============================================================================*/

int FE_element_is_top_level(struct FE_element *element,void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 1 December 1999

DESCRIPTION :
Returns true if <element> is a top-level element - CM_ELEMENT/no parents.
==============================================================================*/

int FE_element_is_not_top_level(struct FE_element *element,void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 20 July 2000

DESCRIPTION :
Returns true if <element> is not a top-level element = CM_ELEMENT/no parents.
==============================================================================*/

struct FE_element_can_be_merged_data
/*******************************************************************************
LAST MODIFIED : 24 March 2003

DESCRIPTION :
Data to be passed to FE_element_can_be_merged.
<number_of_compatible_element_field_info> and <compatible_element_field_info>
must be cleared before first call to 0 and NULL respectively. After using the
function, the <compatible_element_field_info> array must be deallocated. Note
this array grows in increments of 2 since first element_field_info of the pair
is that of a element passed to the function, every second info is that of its
counterpart obtained from <element_list>.
Since the element in question may refer to non-global nodes from a separate
region, the <global_node_list> is provided to substitute the appropriate
global node when comparing element field components. Note the nodes are
expected to have already passed their own "can be merged" check.
If the field is not defined on the nodes in the element, can alternatively
accept definition of the same-named field in same-numbered node from
<global_node_list>.
==============================================================================*/
{
	int number_of_compatible_element_field_info;
	/* store in pairs in the single array to reduce allocations */
	struct FE_element_field_info **compatible_element_field_info;
	struct LIST(FE_element) *global_element_list;
	struct LIST(FE_node) *global_node_list;
}; /* struct FE_element_can_be_merged_data */

int FE_element_can_be_merged(struct FE_element *element, void *data_void);
/*******************************************************************************
LAST MODIFIED : 24 March 2003

DESCRIPTION :
Fetches an element with the same identifier as <element> from the <data>
<element_list>. Returns true if either:
1. There is no namesake element and <element> has a valid shape.
2. There is a namesake element and <element> has an "unspecified" shape of the
same dimension as it, but no fields.
3. There is a namesake element with the same valid shape and all element fields
in common with the same-named field are defined identically apart from field
pointer and times.
Since the element in question may refer to non-global nodes from a separate
region, the <global_node_list> is provided to substitute the appropriate
global node when comparing element field components. Note the nodes are
expected to have already passed their own "can be merged" check.
Additionally, each nodally-based field in <element> is checked for appropriate
definition of that field in the nodes in <element>, or if the field is
undefined there, of the same-named field of the same-numbered node in the
<global_node_list>.
After using the function, deallocate data->compatible_element_field_info!
<data_void> points at a struct FE_element_can_be_merged_data.
???RC Check on definition of node field has not been written.
==============================================================================*/

int ensure_FE_element_is_in_list(struct FE_element *element,
	void *element_list_void);
/*******************************************************************************
LAST MODIFIED : 25 February 2000

DESCRIPTION :
Iterator function for adding <element> to <element_list> if not currently in it.
==============================================================================*/

struct FE_element_list_conditional_data
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Data for passing to ensure_FE_element_is_in_list_conditional.
==============================================================================*/
{
	struct LIST(FE_element) *element_list;
	LIST_CONDITIONAL_FUNCTION(FE_element) *function;
	void *user_data;
}; /* FE_element_list_conditional_data */

int ensure_FE_element_is_in_list_conditional(struct FE_element *element,
	void *list_conditional_data_void);
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Iterator function for adding <element> to a list - if not already in it - if a
conditional function with user_data is true.
The element_list, conditional function and user_data are passed in a
struct FE_element_list_conditional_data * in the second argument.
Warning: Must not be iterating over the list being added to!
==============================================================================*/

int ensure_FE_element_is_not_in_list(struct FE_element *element,
	void *element_list_void);
/*******************************************************************************
LAST MODIFIED : 25 February 2000

DESCRIPTION :
Iterator function for removing <element> from <element_list> if currently in it.
==============================================================================*/

int toggle_FE_element_in_list(struct FE_element *element,
	void *element_list_void);
/*******************************************************************************
LAST MODIFIED : 25 February 2000

DESCRIPTION :
If <element> is in <element_list> it is taken out, otherwise it is added.
==============================================================================*/

struct FE_element_list_CM_element_type_data
/*******************************************************************************
LAST MODIFIED : 1 March 2001

DESCRIPTION :
Iterator data for functions working with elements of a given CM_element_type
in an element list, eg. add_FE_element_of_CM_element_type_to_list.
==============================================================================*/
{
	enum CM_element_type cm_element_type;
	struct LIST(FE_element) *element_list;
};

int add_FE_element_of_CM_element_type_to_list(struct FE_element *element,
	void *element_list_type_data_void);
/*******************************************************************************
LAST MODIFIED : 1 March 2001

DESCRIPTION :
Iterator function which, if <element> is of the given CM_element_type, adds it
to the element_list if not currently in it.
==============================================================================*/

int ensure_top_level_FE_element_nodes_are_in_list(struct FE_element *element,
	void *node_list_void);
/*******************************************************************************
LAST MODIFIED : 20 April 1999

DESCRIPTION :
Iterator function which, if <element> is top-level (ie. cm.type is CM_ELEMENT),
ensures all its nodes are added to the <node_list> if not currently in it.
==============================================================================*/

int ensure_top_level_FE_element_nodes_are_not_in_list(
	struct FE_element *element,void *node_list_void);
/*******************************************************************************
LAST MODIFIED : 20 April 1999

DESCRIPTION :
Iterator function which, if <element> is top-level (ie. cm.type is CM_ELEMENT),
ensures none of its nodes are in <node_list>.
==============================================================================*/

int FE_element_or_parent_has_field(struct FE_element *element,
	struct FE_field *field, struct LIST(FE_element) *element_list);
/*******************************************************************************
LAST MODIFIED : 12 November 2002

DESCRIPTION :
Returns true if the <element> or any of its parents has the <field> defined
over it. By supplying an <element_list> this limits the test to elements that
are also in the list.
==============================================================================*/

struct FE_field *FE_node_get_position_cartesian(struct FE_node *node,
	struct FE_field *coordinate_field,FE_value *node_x,FE_value *node_y,
	FE_value *node_z,FE_value *coordinate_jacobian);
/*******************************************************************************
LAST MODIFIED : 2 November 1998

DESCRIPTION :
Evaluates the supplied coordinate_field (or the first one in the node if NULL).
Sets non-present components to zero (eg if only had x and y, z would be set to
zero).  Converts to rectangular Cartesian coordinates: x,y,z.  If
<coordinate_jacobian>, then its is filled with the Jacobian for the
transformation from native coordinates to rectangular Cartesian.  Returns the
field it actually calculated.
???RC Does not handle multiple versions.
==============================================================================*/

int FE_node_set_position_cartesian(struct FE_node *node,
	struct FE_field *coordinate_field,
	FE_value node_x,FE_value node_y,FE_value node_z);
/*******************************************************************************
LAST MODIFIED : 28 April 1998

DESCRIPTION :
Sets the position of <node> in Cartesian coordinates: x[,y[,z]] using the
supplied coordinate_field (or the first one in the node if NULL). The given
Cartesian coordinates are converted into the coordinate system of the node
for the coordinate_field used.
==============================================================================*/

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

int FE_field_is_defined_in_element(struct FE_field *field,
	struct FE_element *element);
/*******************************************************************************
LAST MODIFIED : 13 May 1999

DESCRIPTION :
Returns true if the <field> is defined for the <element>.
==============================================================================*/

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

int get_FE_element_field_grid_map_number_in_xi(struct FE_element *element,
	struct FE_field *field,int *number_in_xi);
/*******************************************************************************
LAST MODIFIED : 5 October 1999 

DESCRIPTION :
If <field> is grid-based in <element>, returns in <number_in_xi> the numbers of
finite difference cells in each xi-direction of <element>. Note that this number
is one less than the number of grid points in each direction. <number_in_xi>
should be allocated with at least as much space as the number of dimensions in
<element>, but is assumed to have no more than MAXIMUM_ELEMENT_XI_DIMENSIONS so
that int number_in_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS] can be passed to this
function.
==============================================================================*/

int get_FE_element_field_number_of_grid_values(struct FE_element *element,
	struct FE_field *field);
/*******************************************************************************
LAST MODIFIED : 12 October 1999 

DESCRIPTION :
If <field> is grid-based in <element>, returns the total number of grid points
at which data is stored for <field>, equal to product of <number_in_xi>+1 in
all directions. Returns 0 without error for non grid-based fields.
==============================================================================*/

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

int get_FE_element_field_component_grid_FE_value_values(
	struct FE_element *element,struct FE_field *field,int component_number,
	FE_value **values);
/*******************************************************************************
LAST MODIFIED : 12 October 1999

DESCRIPTION :
If <field> is grid-based in <element>, returns an allocated array of the grid
values stored for <component_number>. To get number of values returned, call
get_FE_element_field_number_of_grid_values; Grids change in xi0 fastest.
It is up to the calling function to DEALLOCATE the returned values.
==============================================================================*/

int set_FE_element_field_component_grid_FE_value_values(
	struct FE_element *element,struct FE_field *field,int component_number,
	FE_value *values);
/*******************************************************************************
LAST MODIFIED : 12 October 1999

DESCRIPTION :
If <field> is grid-based in <element>, copies <values> into the values storage
for <component_number>. To get number of values to pass, call
get_FE_element_field_number_of_grid_values; Grids change in xi0 fastest.
==============================================================================*/

int get_FE_element_field_component_grid_int_values(struct FE_element *element,
	struct FE_field *field,int component_number,int **values);
/*******************************************************************************
LAST MODIFIED : 14 October 1999

DESCRIPTION :
If <field> is grid-based in <element>, returns an allocated array of the grid
values stored for <component_number>. To get number of values returned, call
get_FE_element_field_number_of_grid_values; Grids change in xi0 fastest.
It is up to the calling function to DEALLOCATE the returned values.
==============================================================================*/

int set_FE_element_field_component_grid_int_values(struct FE_element *element,
	struct FE_field *field,int component_number,int *values);
/*******************************************************************************
LAST MODIFIED : 14 October 1999

DESCRIPTION :
If <field> is grid-based in <element>, copies <values> into the values storage
for <component_number>. To get number of values to pass, call
get_FE_element_field_number_of_grid_values; Grids change in xi0 fastest.
==============================================================================*/

int FE_element_field_get_copy_components(struct FE_element *element,
	struct FE_field *fe_field,
	struct FE_element_field_component ***components_address);
/*******************************************************************************
LAST MODIFIED : 12 May 2003

DESCRIPTION :
Constructs copies of the FE_element_field_components of <fe_field> at <element>
and stores them at <components_address>.
Up to the calling function to clean up the returned components.
==============================================================================*/

int FE_element_field_get_component_FE_basis(struct FE_element *element,
	struct FE_field *field, int component_number, struct FE_basis **fe_basis);
/*******************************************************************************
LAST MODIFIED : 19 March 2003

DESCRIPTION :
If <field> is standard node based in <element>, returns the <fe_basis> used for
<component_number>.
==============================================================================*/

int FE_node_smooth_FE_field(struct FE_node *node, struct FE_field *fe_field,
	FE_value time, struct FE_field *element_count_fe_field);
/*******************************************************************************
LAST MODIFIED : 12 March 2003

DESCRIPTION :
Partner function to FE_element_smooth_FE_field.
Divides the nodal first derivatives of <fe_field> at <time> in <node> by the
corresponding integer from <element_count_fe_field>, then undefines
<element_count_fe_field>.
<node> is not a global object, as from FE_element_smooth_FE_field.
==============================================================================*/

int FE_element_smooth_FE_field(struct FE_element *element,
	struct FE_field *fe_field, FE_value time,
	struct FE_field *element_count_fe_field,
	struct LIST(FE_node) *copy_node_list);
/*******************************************************************************
LAST MODIFIED : 12 March 2003

DESCRIPTION :
For each node contributing to <fe_field> in <element> either its copy is found
in <copy_node_list> or one is made and added to this list. The copy node will
have <fe_field> defined on it in the same way as the original, plus the integer
<element_count_fe_field> with the same number of components as <fe_field> for
accumulating the number of elements each first derivative of <fe_field> is
referenced by. The copy node starts with all derivatives set to zero.

After making calls to this function for all the intended elements, call
FE_node_smooth_FE_field for each node to divide the accumulated derivatives
by the number of elements they are over and to undefine the
<element_count_fe_field>.
It is up to the calling funcion to merge the copied information into global
elements and nodes.

Sets all scale factors used for <fe_field> to 1.

Notes:
- Only works for "line" shapes with Hermite basis functions.
- <element> shold not be global = not merged into an FE_region.
- <fe_field> should be of type FE_VALUE_VALUE.
- returns 1 without errors if fe_field is not defined on this element or the
  element has no field information, or the field cannot be smoothed.
- only handles 1 version at nodes.
==============================================================================*/

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

int FE_element_get_scale_factor_for_nodal_value( 
	struct FE_element *element, struct FE_node *node, struct FE_field *field,
	int component_number,	enum FE_nodal_value_type nodal_value_type,
	FE_value *scale_factor);
/*******************************************************************************
LAST MODIFIED : 26 October 2000

DESCRIPTION :
Given  <component_number>  and <nodal_value_type> of <field> at a 
<node> in an <element>, find the  corresponding <scale_factor>.
==============================================================================*/

int FE_element_set_scale_factor_for_nodal_value(
	struct FE_element *element, struct FE_node *node, struct FE_field *field,
	int component_number,	enum FE_nodal_value_type nodal_value_type,
	FE_value scale_factor);
/*******************************************************************************
LAST MODIFIED : 31 January 2001

DESCRIPTION :
Given  <component_number>  and <nodal_value_type> of <field> at a 
<node> in an <element>, set the  corresponding scale_factor to <scale_factor>.
==============================================================================*/

int FE_element_xi_increment_within_element(struct FE_element *element, FE_value *xi,
	FE_value *increment, FE_value *fraction, int *face_number, FE_value *xi_face);
/*******************************************************************************
LAST MODIFIED : 20 January 2004

DESCRIPTION :
Adds the <increment> to <xi>.  If this moves <xi> outside of the element, then
the step is limited to take <xi> to the boundary, <face_number> is set to be
the limiting face, <fraction> is updated with the fraction of the <increment>
actually used, the <increment> is updated to contain the part not used,
the <xi_face> are calculated for that face and the <xi> are changed to be
on the boundary of the element.
==============================================================================*/

int FE_element_change_to_adjacent_element(struct FE_element **element_address,
	FE_value *xi, FE_value *increment, int *face_number, FE_value *xi_face);
/*******************************************************************************
LAST MODIFIED : 20 January 2004

DESCRIPTION :
Steps into the adjacent element through face <face_number>, updating the 
<element_address> location.
If <xi> is not NULL then the <xi_face> coordinates are converted to an xi
location in the new element.
If <increment> is not NULL then it is converted into an equvalent increment
in the new element.
==============================================================================*/

int FE_element_xi_increment(struct FE_element **element_address,FE_value *xi,
	FE_value *increment);
/*******************************************************************************
LAST MODIFIED : 21 January 2004

DESCRIPTION :
Adds the <increment> to <xi> changing to another element if necessary.
==============================================================================*/

int make_square_FE_element(struct FE_element **element_ptr,
	struct FE_region *fe_region, int element_dimension, int basis_type,
	struct FE_field *field);
/*******************************************************************************
LAST MODIFIED : 5 August 2003

DESCRIPTION :
Generates a square element with the specified <element_dimension>, <fe_region> and
<basis_type>.  If a <field> is supplied it is defined on the element with unit
scale factors.
Shifted and generalised from element_creator.c.
==============================================================================*/
#endif /* !defined (FINITE_ELEMENT_H) */
