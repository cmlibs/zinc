/*******************************************************************************
FILE : cmiss_computed_field.c

LAST MODIFIED : 29 March 2004

DESCRIPTION :
The public interface to the Cmiss computed_fields.
==============================================================================*/
#include <stdlib.h>
#include "api/cmiss_computed_field.h"
#include "computed_field/computed_field.h"
#include "finite_element/finite_element_region.h"
#include "general/debug.h"
#include "user_interface/message.h"

/*
Global functions
----------------
*/

struct Cmiss_computed_field *Cmiss_computed_field_manager_get_field(
	struct Cmiss_computed_field_manager *manager, char *field_name)
/*******************************************************************************
LAST MODIFIED : 29 March 2004

DESCRIPTION :
Returns the computed_field of <field_name> from the <manager> if it is defined.
==============================================================================*/
{
	struct Cmiss_computed_field *computed_field;

	ENTER(Cmiss_computed_field_manager_get_field);
	if (manager && field_name)
	{
		computed_field=FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
			field_name, manager);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_computed_field_manager_get_field.  Invalid argument(s)");
		computed_field = (struct Cmiss_computed_field *)NULL;
	}
	LEAVE;

	return (computed_field);
} /* Cmiss_computed_field_manager_get_field */

int Cmiss_computed_field_evaluate_at_node(struct Cmiss_computed_field *field,
	struct Cmiss_node *node, float time, int number_of_values, float *values)
/*******************************************************************************
LAST MODIFIED : 29 March 2004

DESCRIPTION :
Returns the <values> of <field> at <node> and <time> if it is defined there.

The <values> array must be large enough to store as many FE_values as there are
number_of_components, the function checks that <number_of_values> is 
greater than or equal to the number of components.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_computed_field_evaluate_at_node);
	if (field && node && values &&
		(number_of_values>=Computed_field_get_number_of_components(field)))
	{
		return_code = Computed_field_evaluate_at_node(field, node, time, values);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_computed_field_evaluate_at_node.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_computed_field_evaluate_at_node */

int Cmiss_computed_field_set_values_at_node(struct Cmiss_computed_field *field,
	struct Cmiss_node *node, int number_of_values, float time, float *values)
/*******************************************************************************
LAST MODIFIED : 28 October 2004

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
==============================================================================*/
{
	int return_code;
	struct FE_region *fe_region;

	ENTER(Cmiss_computed_field_set_values_at_node);
	if (field && node && values &&
		(number_of_values >= Computed_field_get_number_of_components(field)))
	{
		if (fe_region = FE_node_get_FE_region(node))
		{
			return_code = Computed_field_set_values_at_node_in_FE_region(field,
				node, time, fe_region, values);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_computed_field_set_values_at_node.  Unable to get region for node.");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_computed_field_set_values_at_node.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_computed_field_set_values_at_node */

int Cmiss_computed_field_evaluate_in_element(struct Cmiss_computed_field *field,
	struct Cmiss_element *element, float *xi, float time, 
	struct Cmiss_element *top_level_element, int number_of_values,
	float *values, int number_of_derivatives, float *derivatives)
/*******************************************************************************
LAST MODIFIED : 29 March 2004

DESCRIPTION :
Returns the values and derivatives (if <derivatives> != NULL) of <field> at
<element>:<xi>, if it is defined over the element.

The optional <top_level_element> may be supplied for the benefit of this or
any source fields that may require calculation on it instead of a face or line.
FIBRE_AXES and GRADIENT are examples of such fields, since they require
top-level coordinate derivatives. The term "top_level" refers to an ultimate
parent element for the face or line, eg. the 3-D element parent to 2-D faces.
If no such top level element is supplied and one is required, then the first
available parent element will be chosen - if the user requires a top-level
element in the same group as the face or with the face on the correct side,
then they should supply the top_level_element here.

The <values> and <derivatives> arrays must be large enough to store all the
values and derivatives for this field in the given element, ie. values is
number_of_components in size, derivatives has the element dimension times the
number_of_components
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_computed_field_evaluate_in_element);
	if (field && element && xi && values &&
		(number_of_values >= Computed_field_get_number_of_components(field))
		&& (!derivatives || (number_of_derivatives >= 
		Computed_field_get_number_of_components(field) * get_FE_element_dimension(element))))
	{
		return_code = Computed_field_evaluate_in_element(field, element, xi, time,
			top_level_element, values, derivatives);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_computed_field_evaluate_in_element.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_computed_field_evaluate_in_element */

