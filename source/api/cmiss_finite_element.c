/*******************************************************************************
FILE : cmiss_finite_element.c

LAST MODIFIED : 10 November 2004

DESCRIPTION :
The public interface to the Cmiss_finite_elements.
==============================================================================*/
#include <stdarg.h>
#include "api/cmiss_finite_element.h"
#include "general/debug.h"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_region.h"
#include "user_interface/message.h"

/*
Global functions
----------------
*/

int Cmiss_node_get_identifier(struct Cmiss_node *node)
/*******************************************************************************
LAST MODIFIED : 1 April 2004

DESCRIPTION :
Returns the integer identifier of the <node>.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_node_get_identifier);
	return_code = get_FE_node_identifier(node);
	LEAVE;

	return (return_code);
} /* Cmiss_node_get_identifier */

/* Note that Cmiss_node_set_identifier is not here as the function does 
	not currently exist in finite_element.[ch] either. */

Cmiss_node_id create_Cmiss_node(int node_identifier,
	Cmiss_region_id region)
/*******************************************************************************
LAST MODIFIED : 10 November 2004

DESCRIPTION :
Creates and returns a node with the specified <cm_node_identifier>.
Note that <cm_node_identifier> must be non-negative.
A blank node with the given identifier but no fields is returned.
The new node is set to belong to the ultimate master FE_region of <region>.
==============================================================================*/
{
	Cmiss_node_id node;
	struct FE_region *fe_region;

	ENTER(create_Cmiss_node);
	node = (Cmiss_node_id)NULL;
	if (fe_region=Cmiss_region_get_FE_region(region))
	{
		node = ACCESS(FE_node)(CREATE(FE_node)(node_identifier, fe_region, (struct FE_node *)NULL));
	}
	LEAVE;

	return (node);
} /* create_Cmiss_node */

Cmiss_node_id create_Cmiss_node_from_template(int node_identifier,
	Cmiss_node_id template_node)
/*******************************************************************************
LAST MODIFIED : 10 November 2004

DESCRIPTION :
Creates and returns a node with the specified <cm_node_identifier>.
Note that <cm_node_identifier> must be non-negative.
The node copies all the fields and values of the <template_node> and will
belong to the same region.
==============================================================================*/
{
	Cmiss_node_id node;

	ENTER(create_Cmiss_node_from_template);
	node = ACCESS(FE_node)(CREATE(FE_node)(node_identifier, (struct FE_region *)NULL, 
			template_node));
	LEAVE;

	return (node);
} /* create_Cmiss_node_from_template */

int destroy_Cmiss_node(Cmiss_node_id *node_id_address)
/*******************************************************************************
LAST MODIFIED : 10 November 2004

DESCRIPTION :
Frees the memory for the node, sets <*node_address> to NULL.
==============================================================================*/
{
	int return_code;

	ENTER(destroy_Cmiss_node);
	return_code = DEACCESS(FE_node)(node_id_address);
	LEAVE;

	return (return_code);
} /* destroy_Cmiss_node */

int Cmiss_element_get_identifier(struct Cmiss_element *element)
/*******************************************************************************
LAST MODIFIED : 1 April 2004

DESCRIPTION :
Returns the integer identifier of the <element>.
==============================================================================*/
{
	int return_code;
	struct CM_element_information cm_id;

	ENTER(Cmiss_element_get_identifier);
	if (return_code = get_FE_element_identifier(element, &cm_id))
	{
		/* Check it is truly a CM_ELEMENT? */
		return_code = cm_id.number;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_element_get_identifier */

int Cmiss_element_set_node(Cmiss_element_id element, int node_index,
	Cmiss_node_id node)
/*******************************************************************************
LAST MODIFIED : 11 November 2004

DESCRIPTION :
Sets node <node_number>, from 0 to number_of_nodes-1 of <element> to <node>.
<element> must already have a shape and node_scale_field_information.
Should only be called for unmanaged elements.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_element_set_node);
	return_code = set_FE_element_node(element, node_index, node);
	LEAVE;

	return (return_code);
} /* Cmiss_element_set_node */

Cmiss_element_id create_Cmiss_element_with_line_shape(int element_identifier,
	Cmiss_region_id region, int dimension)
/*******************************************************************************
LAST MODIFIED : 1 December 2004

DESCRIPTION :
Creates an element that has a line shape product of the specified <dimension>.
==============================================================================*/
{
	Cmiss_element_id element;
	struct FE_region *fe_region;

	ENTER(create_Cmiss_element);
	element = (Cmiss_element_id)NULL;
	if (fe_region=Cmiss_region_get_FE_region(region))
	{
		element = ACCESS(FE_element)(create_FE_element_with_line_shape(
			element_identifier, fe_region, dimension));
	}
	LEAVE;

	return (element);
} /* create_Cmiss_element */

