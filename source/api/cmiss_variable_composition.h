/*******************************************************************************
FILE : api/cmiss_variable_composition.h

LAST MODIFIED : 28 August 2003

DESCRIPTION :
The public interface to the Cmiss_variable_composition object.
==============================================================================*/
#ifndef __API_CMISS_VARIABLE_COMPOSITION_H__
#define __API_CMISS_VARIABLE_COMPOSITION_H__

/* If this is going to be in the API then it needs to have an interface there */
#include "general/object.h"
#include "api/cmiss_variable.h"

Cmiss_variable_id CREATE(Cmiss_variable_composition)(char *name,
	Cmiss_variable_id dependent_variable,int number_of_source_variables,
	Cmiss_variable_id *source_variables,Cmiss_variable_id *independent_variables);
/*******************************************************************************
LAST MODIFIED : 28 August 2003

DESCRIPTION :
Creates a Cmiss_variable which represents a coordinate of the specified <dimension>.
==============================================================================*/

#endif /* __API_CMISS_VARIABLE_COMPOSITION_H__ */
