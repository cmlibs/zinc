/*******************************************************************************
FILE : api/cmiss_variable_composite.h

LAST MODIFIED : 1 September 2003

DESCRIPTION :
The public interface to the Cmiss_variable_composite object.
==============================================================================*/
#ifndef __API_CMISS_VARIABLE_COMPOSITE_H__
#define __API_CMISS_VARIABLE_COMPOSITE_H__

/* If this is going to be in the API then it needs to have an interface there */
#include "general/object.h"
#include "api/cmiss_variable.h"

Cmiss_variable_id CREATE(Cmiss_variable_composite)(char *name,
	int number_of_variables, Cmiss_variable_id *variables_array);
/*******************************************************************************
LAST MODIFIED : 1 September 2003

DESCRIPTION :
Creates a Cmiss_variable which represents a composite concatenation of serveral
other variables.
==============================================================================*/
#endif /* __API_CMISS_VARIABLE_COMPOSITE_H__ */
