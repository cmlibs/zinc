/*******************************************************************************
FILE : interactive_tool_private.h

LAST MODIFIED : 6 October 2000

DESCRIPTION :
Privileged functions that different types of Interactive Tool are allowed to
use.
==============================================================================*/
#if !defined (INTERACTIVE_TOOL_PRIVATE_H)
#define INTERACTIVE_TOOL_PRIVATE_H

void *Interactive_tool_get_tool_data(
	struct Interactive_tool *interactive_tool);
/*******************************************************************************
LAST MODIFIED : 6 October 2000

DESCRIPTION :
Priveleged function for objects of type Interactive tool to get their own
type specific data.
==============================================================================*/

#endif /* !defined (INTERACTIVE_TOOL_PRIVATE_H) */
