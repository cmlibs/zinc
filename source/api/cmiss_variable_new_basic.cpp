/*******************************************************************************
FILE : api/cmiss_variable_new_basic.cpp

LAST MODIFIED : 8 September 2003

DESCRIPTION :
The public interface to the Cmiss_variable_new basic objects.
==============================================================================*/

#include <string>
#include "api/cmiss_variable_new_basic.h"
#include "computed_variable/variable_basic.h"

/*
Global functions
----------------
*/
Cmiss_variable_new_id Cmiss_variable_new_scalar_create(char *name,Scalar value)
/*******************************************************************************
LAST MODIFIED : 8 September 2003

DESCRIPTION :
Creates a Cmiss_variable_new scalar with the supplied <name> and <value>.
==============================================================================*/
{
	return (new(nothrow) Variable_scalar(name,value));
}
