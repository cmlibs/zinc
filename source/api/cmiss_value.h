/*******************************************************************************
FILE : api/cmiss_value.h

LAST MODIFIED : 30 July 2003

DESCRIPTION :
The public interface to the Cmiss_value object.
==============================================================================*/
#ifndef __API_CMISS_VALUE_H__
#define __API_CMISS_VALUE_H__

/* If this is going to be in the API then it needs to have an interface there */
#include "general/object.h"

struct Cmiss_value;

typedef struct Cmiss_value *Cmiss_value_id;
/*******************************************************************************
LAST MODIFIED : 30 July 2003

DESCRIPTION :
A value that knows what type it is.

???DB.  An update of Value_type (general/value).  Want to be able to add new
	value types (in the same way that Computed_field types can be added).  Will
	need a package?  Will replace Value_type?
???DB.  At present just extend?
==============================================================================*/

Cmiss_value_id CREATE(Cmiss_value)(void);
/*******************************************************************************
LAST MODIFIED : 22 January 2003

DESCRIPTION :
Creates an empty value with no type.  Each type of value has its own "set_type"
function.
==============================================================================*/

int Cmiss_value_get_string(Cmiss_value_id value,
	char **result);
/*******************************************************************************
LAST MODIFIED : 25 July 2003

DESCRIPTION :
Creates a string representation of the Cmiss_value useful for output.
If successful <result> contains an ALLOCATED string, it is up to the calling
function to DEALLOCATE the string when it is no longer required.
==============================================================================*/

#endif /* __API_CMISS_VALUE_H__ */
