/*******************************************************************************
FILE : cmiss_finite_element.c

LAST MODIFIED : 1 April 2004

DESCRIPTION :
The public interface to the Cmiss_finite_elements.
==============================================================================*/
#include <stdarg.h>
#include "api/cmiss_finite_element.h"
#include "general/debug.h"
#include "finite_element/finite_element.h"
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
