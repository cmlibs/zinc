/*******************************************************************************
FILE : cmiss_node.h

LAST MODIFIED : 3 November 2004

DESCRIPTION :
The public interface to Cmiss_node.
==============================================================================*/
#ifndef __CMISS_NODE_H__
#define __CMISS_NODE_H__

/*
Global types
------------
*/

/* SAB Temporary until we decide how to fix things up internally instead of externally.*/
#define Cmiss_node FE_node

struct Cmiss_node;
/*******************************************************************************
LAST MODIFIED : 14 August 2002

DESCRIPTION :
==============================================================================*/

typedef struct Cmiss_node_field_creator * Cmiss_node_field_creator_id;

/* Put temporarily into api/cmiss_region.h until we resolve the circularness,
	maybe we need a types declaration file that we can include from everywhere
	typedef int (*Cmiss_node_iterator_function)(struct Cmiss_node *node, void *user_data); */

/* SAB Temporary until we decide how to fix things up internally instead of externally.*/
#define Cmiss_node_field_creator FE_node_field_creator

struct Cmiss_node_field_creator;
/*******************************************************************************
LAST MODIFIED : 14 August 2002

DESCRIPTION :
==============================================================================*/

typedef struct Cmiss_node_field_creator * Cmiss_node_field_creator_id;

/*
Global functions
----------------
*/

struct Cmiss_node_field_creator *CREATE(Cmiss_node_field_creator)(
	int number_of_components);
/*******************************************************************************
LAST MODIFIED : 16 November 2001

DESCRIPTION :
An object for defining the components, number_of_versions,
number_of_derivatives and their types at a node.
By default each component has 1 version and no derivatives.
==============================================================================*/

struct Cmiss_node_field_creator *create_Cmiss_node_field_creator_from_node_field(
	struct Cmiss_node *node, struct Cmiss_field *field);
/*******************************************************************************
LAST MODIFIED : 4 February 2001

DESCRIPTION :
Creates an Cmiss_node_field_creator from <node>,<field>
==============================================================================*/

int DESTROY(Cmiss_node_field_creator)(
	struct Cmiss_node_field_creator **node_field_creator_address);
/*******************************************************************************
LAST MODIFIED : 16 November 2001

DESCRIPTION :
Frees the memory for the node field creator and sets 
<*node_field_creator_address> to NULL.
==============================================================================*/

int Cmiss_node_field_creator_define_derivative(
	struct Cmiss_node_field_creator *node_field_creator, int component_number,
	enum Cmiss_nodal_value_type derivative_type);
/*******************************************************************************
LAST MODIFIED: 16 November 2001

DESCRIPTION:
Adds the derivative of specified <derivative_type> to the <component_number>
specified.
==============================================================================*/

int Cmiss_node_field_creator_define_versions(
	struct Cmiss_node_field_creator *node_field_creator, int component_number,
	int number_of_versions);
/*******************************************************************************
LAST MODIFIED: 16 November 2001

DESCRIPTION:
Specifies the <number_of_versions> for <component_number> specified.
==============================================================================*/

int Cmiss_node_get_identifier(struct Cmiss_node *node);
/*******************************************************************************
LAST MODIFIED : 1 April 2004

DESCRIPTION :
Returns the integer identifier of the <node>.
==============================================================================*/

/* SAB Temporary until we decide how to fix things up internally instead of externally.*/
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

Cmiss_node_id create_Cmiss_node_in_region(int node_identifier,
	struct Cmiss_region *region);
/*******************************************************************************
LAST MODIFIED : 1 November 2004

DESCRIPTION :
Creates and returns a node with the specified <cm_node_identifier>.
Note that <cm_node_identifier> must be non-negative.
A blank node with the given identifier but no fields is returned.
The new node is set to belong to the ultimate master FE_region of <region>.
==============================================================================*/

Cmiss_node_id create_Cmiss_node_from_template(int node_identifier,
	Cmiss_node_id template_node);
/*******************************************************************************
LAST MODIFIED : 1 November 2004

DESCRIPTION :
Creates and returns a node with the specified <cm_node_identifier>.
Note that <cm_node_identifier> must be non-negative.
The node copies all the fields and values of the <template_node> and will
belong to the same region.
==============================================================================*/

int destroy_Cmiss_node(Cmiss_node_id *node_id_address);
/*******************************************************************************
LAST MODIFIED : 1 November 2004

DESCRIPTION :
Frees the memory for the node, sets <*node_address> to NULL.
==============================================================================*/

int Cmiss_node_define_Cmiss_field(Cmiss_node_id node, Cmiss_field_id field,
	Cmiss_time_version_id time_version, Cmiss_node_field_creator_id field_creator);
/*******************************************************************************
LAST MODIFIED : 3 November 2004

DESCRIPTION :
==============================================================================*/

#endif /* __CMISS_NODE_H__ */
