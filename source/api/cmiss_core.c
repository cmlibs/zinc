/*******************************************************************************
FILE : cmiss_core.c

LAST MODIFIED : 12 September 2002

DESCRIPTION :
The public interface to the some of the internal functions of cmiss.
==============================================================================*/
#include <stdlib.h>
#include "general/debug.h"
#include "user_interface/message.h"

/*
Global functions
----------------
*/

int Cmiss_deallocate(void *ptr)
/*******************************************************************************
LAST MODIFIED : 12 September 2002

DESCRIPTION :
Frees the memory associated with the pointer.  Used to clean up when functions
return buffers allocated internally to cmiss.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_deallocate);
	if (ptr)
	{
		DEALLOCATE(ptr);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_deallocate.  "
			"Invalid pointer.");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_deallocate */
